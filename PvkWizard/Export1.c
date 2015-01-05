/*++

Copyright (c) 2007 Can Balioglu. All rights reserved.

See License.txt in the project root for license information.

--*/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <cryptdlg.h>
#include <cryptuiapi.h>
#include <shellapi.h>

#include "PvkWizard.h"

typedef BOOL(WINAPI *SELECTCERTPROC)(IN PCERT_SELECT_STRUCT_W CertSel);

static
BOOL
BlgpGetCertificate(
    IN HWND HwndOwner
    )
{
    DWORD ErrMsg = 0;
    HCERTSTORE Store = NULL;
    HMODULE Module;
    SELECTCERTPROC SelCertProc;
    CERT_SELECT_STRUCT CertSel = {sizeof(CERT_SELECT_STRUCT)};
    PCCERT_CONTEXT CertContext = NULL;
    HCRYPTPROV CryptProv = 0;
    DWORD KeySpec;
    BOOL Ignored;
    HCRYPTKEY Key = 0;
    DWORD Permissions;
    DWORD Size = sizeof(DWORD);

    Store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0,
        CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_CURRENT_USER, L"MY");

    if (!Store)
    {
        ErrMsg = IDS_STORE_FAILED;

        goto Leave;
    }


    Module = LoadLibrary(L"cryptdlg.dll");
    if (!Module)
    {
        goto Leave;
    }

    SelCertProc = (SELECTCERTPROC) GetProcAddress(Module, "CertSelectCertificateW");
    if (!SelCertProc)
    {
        goto Leave;
    }

    CertSel.hwndParent = HwndOwner;
    CertSel.cCertStore = 1;
    CertSel.arrayCertStore = &Store;
    CertSel.cCertContext = 1;
    CertSel.arrayCertContext = &CertContext;

    if (!SelCertProc(&CertSel))
    {
        goto Leave;
    }

    CertContext = CertSel.arrayCertContext[0];
    if (!CertContext)
    {
        goto Leave;
    }

    if (!CryptAcquireCertificatePrivateKey(CertContext, CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL,
            &CryptProv, &KeySpec, &Ignored))
    {
        ErrMsg = (GetLastError() == NTE_BAD_PUBLIC_KEY) ? IDS_BAD_KEY : IDS_PVK_NOT_FOUND;

        goto Leave;
    }

    if (!CryptGetUserKey(CryptProv, KeySpec, &Key) ||
        !CryptGetKeyParam(Key, KP_PERMISSIONS, (PBYTE) &Permissions, &Size, 0))
    {
        ErrMsg = IDS_PVK_UNKNOWN_ERR;

        goto Leave;
    }

    if (!BLG_FLAGON(Permissions, CRYPT_EXPORT))
    {
        ErrMsg = IDS_PVK_NOT_EXPORTABLE;

        goto Leave;
    }

    BlgpDestroyExportData();

    g_ExportData->CertContext = CertContext;
    g_ExportData->CryptProv = CryptProv;
    g_ExportData->KeySpec = KeySpec;

Leave:
    if (Key)
    {
        CryptDestroyKey(Key);
    }

    if (Store)
    {
        CertCloseStore(Store, 0);
    }

    if (ErrMsg)
    {
        if (CryptProv)
        {
            CryptReleaseContext(CryptProv, 0);
        }

        if (CertContext)
        {
            CertFreeCertificateContext(CertContext);
        }

        ShellMessageBox(g_Instance, HwndOwner, MAKEINTRESOURCE(ErrMsg), NULL, MB_ICONERROR | MB_OK);
    }

    return !ErrMsg && CryptProv != 0;
}


INT_PTR
CALLBACK
BlgpExport1PageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    )
{
    switch (Msg)
    {
    case WM_INITDIALOG:
    {
        HWND HwndCtrl = GetDlgItem(HwndDlg, IDC_CERTIFICATE);
        if (HwndCtrl)
        {
            RECT Rect = {70, 110};
            LVCOLUMN Column = {LVCF_WIDTH};

            ListView_SetExtendedListViewStyle(HwndCtrl, LVS_EX_FULLROWSELECT);

            MapDialogRect(HwndDlg, &Rect);

            Column.cx = Rect.left;
            Column.cx = Rect.top;
            ListView_InsertColumn(HwndCtrl, 0, &Column);
            ListView_InsertColumn(HwndCtrl, 1, &Column);
        }

        EnableWindow(GetDlgItem(HwndDlg, IDC_VIEW_CERT), FALSE);

        break;
    }

    case WM_COMMAND:

        if (HIWORD(WParam) == BN_CLICKED)
        {
            switch (LOWORD(WParam))
            {
            case IDC_SELECT_CERT:
            {
                if (BlgpGetCertificate(GetAncestor(HwndDlg, GA_ROOT)))
                {
                    HWND HwndCtrl = GetDlgItem(HwndDlg, IDC_CERTIFICATE);
                    if (HwndCtrl)
                    {
                        LVITEM Item = {LVIF_TEXT};
                        WCHAR Text[128];

                        Item.pszText = Text;

                        ListView_DeleteAllItems(HwndCtrl);

                        if (LoadString(g_Instance, IDS_ISSUED_TO, Text, ARRAYSIZE(Text)) > 0)
                        {
                            Item.iSubItem = 0;

                            ListView_InsertItem(HwndCtrl, &Item);

                            CertGetNameString(g_ExportData->CertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL,
                                Text, ARRAYSIZE(Text));

                            Item.iSubItem = 1;

                            ListView_SetItem(HwndCtrl, &Item);

                            Item.iItem++;
                        }

                        if (LoadString(g_Instance, IDS_ISSUED_BY, Text, ARRAYSIZE(Text)) > 0)
                        {
                            Item.iSubItem = 0;

                            ListView_InsertItem(HwndCtrl, &Item);

                            CertGetNameString(g_ExportData->CertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                CERT_NAME_ISSUER_FLAG, NULL, Text, ARRAYSIZE(Text));

                            Item.iSubItem = 1;

                            ListView_SetItem(HwndCtrl, &Item);

                            Item.iItem++;
                        }

                        if (LoadString(g_Instance, IDS_EXPIRATION_DATE, Text, ARRAYSIZE(Text)) > 0)
                        {
                            SYSTEMTIME Time;

                            Item.iSubItem = 0;

                            ListView_InsertItem(HwndCtrl, &Item);

                            if (FileTimeToSystemTime(&g_ExportData->CertContext->pCertInfo->NotAfter, &Time))
                            {
                                GetDateFormat(LOCALE_USER_DEFAULT, 0, &Time, NULL, Text, ARRAYSIZE(Text));

                                Item.iSubItem = 1;

                                ListView_SetItem(HwndCtrl, &Item);
                            }

                            Item.iItem++;
                        }
                    }

                    EnableWindow(GetDlgItem(HwndDlg, IDC_VIEW_CERT), TRUE);

                    PropSheet_SetWizButtons(GetParent(HwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }

                return TRUE;
            }

            case IDC_VIEW_CERT:
            {
                CRYPTUI_VIEWCERTIFICATE_STRUCT ViewCert = {sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT)};
                BOOL Ignored;

                ViewCert.hwndParent = GetAncestor(HwndDlg, GA_ROOT);
                ViewCert.dwFlags = CRYPTUI_DISABLE_EDITPROPERTIES;
                ViewCert.pCertContext = g_ExportData->CertContext;

                CryptUIDlgViewCertificate(&ViewCert, &Ignored);

                return TRUE;
            }
            }
        }

        break;

    case WM_NOTIFY:

        switch (((LPNMHDR) LParam)->code)
        {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(HwndDlg),
                PSWIZB_BACK | (g_ExportData->CryptProv != 0 ? PSWIZB_NEXT : 0));

            return TRUE;

        case PSN_WIZNEXT:
            SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_EXPORT2);

            return TRUE;

        case PSN_WIZBACK:
            SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_SELECTION);

            return TRUE;
        }

        break;
    }

    return FALSE;
}