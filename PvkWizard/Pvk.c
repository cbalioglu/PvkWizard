#include <windows.h>
#include <shellapi.h>
#include <BlgPvk.h>

#include "PvkWizard.h"

static
VOID
BlgpDisplayError(
    IN HWND HwndOwner
    )
{
    PWSTR Msg;
    DWORD Err = GetLastError();

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, Err,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (PWSTR) &Msg, 0, NULL) == 0)
    {
        ShellMessageBox(g_Instance, HwndOwner, MAKEINTRESOURCE(IDS_UNKNOWN_ERR), NULL, MB_ICONERROR | MB_OK, Err);
    }
    else
    {
        ShellMessageBox(g_Instance, HwndOwner, Msg, NULL, MB_ICONERROR | MB_OK);

        LocalFree(Msg);
    }
}

BOOL
BlgpPvkImport(
    IN HWND HwndOwner
    )
{
    BOOL IsOk = FALSE;
    BOOL IsAdded = FALSE;
    UUID Uuid;
    PWCHAR Container = NULL;
    CHAR Pswd[BLGP_MAX_PSWD];
    HCRYPTPROV Provider = 0;
    HCRYPTKEY Key = 0;
    ALG_ID AlgId;
    DWORD AlgIdCb = sizeof(ALG_ID);
    DWORD KeySpec;
    PCERT_PUBLIC_KEY_INFO PubKeyInfo = NULL;
    DWORD PubKeyInfoCb;
    HCERTSTORE FileStore = NULL;
    HCERTSTORE RootStore = NULL;
    HCERTSTORE CAStore = NULL;
    HCERTSTORE MyStore = NULL;
    HCERTSTORE TrustStore = NULL;
    PCCERT_CONTEXT CertContext = NULL;
    PCCERT_CONTEXT NewCertContext = NULL;

    // Generate the key container name.
    if (UuidCreate(&Uuid) != RPC_S_OK)
    {
        goto Leave;
    }

    if (UuidToString(&Uuid, &Container) != RPC_S_OK)
    {
        goto Leave;
    }

    if (!CryptAcquireContext(&Provider, Container, NULL, g_ImportData->ProvType, CRYPT_NEWKEYSET))
    {
        goto Leave;
    }

    if (!WideCharToMultiByte(CP_ACP, 0, g_ImportData->PvkPswd, -1, Pswd, ARRAYSIZE(Pswd), NULL, NULL))
    {
        goto Leave;
    }

    if (!BlgPvkImport(Provider, g_ImportData->PvkFile, Pswd, g_ImportData->Flags, &Key))
    {
        goto Leave;
    }

    if (!CryptGetKeyParam(Key, KP_ALGID, (PBYTE) &AlgId, &AlgIdCb, 0))
    {
        goto Leave;
    }

    KeySpec = (AlgId == CALG_RSA_KEYX || AlgId == CALG_DH_SF) ? AT_KEYEXCHANGE : AT_SIGNATURE;

    if (!CryptExportPublicKeyInfo(Provider, KeySpec, 
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, NULL, &PubKeyInfoCb))
    {
        goto Leave;
    }

    PubKeyInfo = HeapAlloc(g_Heap, 0, PubKeyInfoCb);

    if (PubKeyInfo == NULL)
    {
        ExitProcess(-1);
    }

    if (!CryptExportPublicKeyInfo(Provider, KeySpec, 
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PubKeyInfo, &PubKeyInfoCb))
    {
        goto Leave;
    }

    FileStore = CertOpenStore(CERT_STORE_PROV_FILENAME, PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, 0,
        CERT_STORE_OPEN_EXISTING_FLAG, g_ImportData->CerFile);

    if (FileStore == NULL)
    {
        goto Leave;
    }

    RootStore = CertOpenSystemStore(0, L"ROOT");
    CAStore = CertOpenSystemStore(0, L"CA");
    MyStore = CertOpenSystemStore(0, L"MY");
    TrustStore = CertOpenSystemStore(0, L"TRUST");

    if (RootStore == NULL || CAStore == NULL || MyStore == NULL || TrustStore == NULL)
    {
        goto Leave;
    }

    while (CertContext = CertEnumCertificatesInStore(FileStore, CertContext))
    {
        DWORD Usage = 0;
        BOOL  Authority = FALSE;

        // If the public keys match, add the certificate to the personal certificate store.
        if (!IsAdded &&
            CertComparePublicKeyInfo(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, PubKeyInfo,
                &CertContext->pCertInfo->SubjectPublicKeyInfo))
        {
            CRYPT_KEY_PROV_INFO ProvInfo = {0};

            if (!CertAddCertificateContextToStore(MyStore, CertContext,
                    CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES, &NewCertContext))
            {
                goto Leave;
            }

            // Associate the key container with the certificate.
            ProvInfo.pwszContainerName = Container;
            ProvInfo.dwProvType = g_ImportData->ProvType;
            ProvInfo.dwFlags = CERT_SET_KEY_PROV_HANDLE_PROP_ID;
            ProvInfo.dwKeySpec = KeySpec;

            if (!CertSetCertificateContextProperty(NewCertContext, CERT_KEY_PROV_INFO_PROP_ID, 0, &ProvInfo))
            {
                goto Leave;
            }

            IsAdded = TRUE;

            continue;
        }

        // If the certificate has no key usages defined or if it has the certificate signing
        // usage, then add it to the intermediate or root certificate store depending on its
        // signature.
        if (CertGetIntendedKeyUsage(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, CertContext->pCertInfo,
                (PBYTE) &Usage, sizeof(DWORD)))
        {
            if (BLG_FLAGON(Usage, CERT_KEY_CERT_SIGN_KEY_USAGE))
            {
                Authority = TRUE;
            }
        }
        else
        {
            if (GetLastError() == 0)
            {
                Authority = TRUE;
            }
            else
            {
                goto Leave;
            }
        }

        if (!Authority)
        {
            // The certificate has no certificate signing usage, so add it to the trust store.
            if (!CertAddCertificateContextToStore(TrustStore, CertContext, CERT_STORE_ADD_USE_EXISTING, NULL))
            {
                goto Leave;
            }
        }
        else
        {
            // If the certificate is self signed, then add it to the root certificate store;
            // otherwise to the intermediate certificate store.
            if (CryptVerifyCertificateSignature(0, X509_ASN_ENCODING,
                    CertContext->pbCertEncoded, CertContext->cbCertEncoded,
                    &CertContext->pCertInfo->SubjectPublicKeyInfo))
            {
                if (!CertAddCertificateContextToStore(RootStore, CertContext, CERT_STORE_ADD_USE_EXISTING, NULL))
                {
                    goto Leave;
                }
            }
            else
            {
                if (!CertAddCertificateContextToStore(CAStore, CertContext, CERT_STORE_ADD_USE_EXISTING, NULL))
                {
                    goto Leave;
                }
            }
        }
    }

    if (IsAdded)
    {
        ShellMessageBox(g_Instance, HwndOwner, MAKEINTRESOURCE(IDS_IMPORTED), NULL, MB_ICONINFORMATION | MB_OK);
    }
    else
    {
        ShellMessageBox(g_Instance, HwndOwner, MAKEINTRESOURCE(IDS_IMPORTED_NO_PVK), NULL, MB_ICONWARNING | MB_OK);
    }

    IsOk = TRUE;

Leave:
    if (!IsOk)
    {
        BlgpDisplayError(HwndOwner);
    }

    if (NewCertContext)
    {
        CertFreeCertificateContext(NewCertContext);
    }

    if (CertContext)
    {
        CertFreeCertificateContext(CertContext);
    }

    if (TrustStore)
    {
        CertCloseStore(TrustStore, 0);
    }

    if (MyStore)
    {
        CertCloseStore(MyStore, 0);
    }

    if (CAStore)
    {
        CertCloseStore(CAStore, 0);
    }

    if (RootStore)
    {
        CertCloseStore(RootStore, 0);
    }

    if (FileStore)
    {
        CertCloseStore(FileStore, 0);
    }

    if (PubKeyInfo)
    {
        HeapFree(GetProcessHeap(), 0, PubKeyInfo);
    }

    if (Key)
    {
        CryptDestroyKey(Key);
    }

    if (Provider)
    {
        CryptReleaseContext(Provider, 0);

        if (!IsAdded)
        {
            //
            //	Delete the private key.
            //

            CryptAcquireContext(&Provider, Container, NULL, g_ImportData->ProvType, CRYPT_DELETEKEYSET);
        }
    }

    if (Container)
    {
        RpcStringFree(&Container);
    }

    return IsOk;
}

BOOL
BlgpPvkExport(
    IN HWND HwndOwner
    )
{
    BOOL IsOk = FALSE;
    CHAR Pswd[BLGP_MAX_PSWD];

    if (!WideCharToMultiByte(CP_ACP, 0, g_ExportData->PvkPswd, -1, Pswd, ARRAYSIZE(Pswd), NULL, NULL))
    {
        goto Leave;
    }

    if (!BlgPvkExport(g_ExportData->CryptProv, g_ExportData->KeySpec, g_ExportData->PvkFile, Pswd, g_ExportData->Flags))
    {
        goto Leave;
    }

    ShellMessageBox(g_Instance, HwndOwner, MAKEINTRESOURCE(IDS_EXPORTED), NULL, MB_ICONINFORMATION | MB_OK);

    IsOk = TRUE;

Leave:
    if (!IsOk)
    {
        BlgpDisplayError(HwndOwner);
    }

    return IsOk;
}

VOID
BlgpDestroyExportData(
    VOID
    )
{
    if (g_ExportData->CryptProv != 0)
    {
        CryptReleaseContext(g_ExportData->CryptProv, 0);
    }

    if (g_ExportData->CertContext != NULL)
    {
        CertFreeCertificateContext(g_ExportData->CertContext);
    }
}