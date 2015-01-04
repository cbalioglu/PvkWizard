#include <windows.h>

#include "BlgPvk.h"
#include "BlgPvkp.h"

#define BLGP_PVK_MAGIC 0xB0B5F11E

// Header of a PVK file.
typedef struct _BLGP_PVK_HEADER
{
    DWORD Magic;
    DWORD Reserved;
    DWORD KeySpec;
    DWORD Encrypted;
    DWORD SaltLen;
    DWORD KeyLen;

} BLGP_PVK_HEADER, *PBLGP_PVK_HEADER;

#define BLGP_SALT_LENGTH 0x0010 // 16 byte

BOOL
BLGPVKCALL
BlgPvkImport(
    IN HCRYPTPROV Provider,
    IN PCWSTR FileName,
    IN PCSTR Password OPTIONAL,
    IN DWORD Flags,
    OUT HCRYPTKEY *Key
    )
/*++

Routine Description:

Imports the private key stored in the specified PVK file into a key container within a
cryptographic service provider.

Arguments:

Provider - Handle to a cryptographic service provider.

FileName - Pointer to a null-terminated string containing the relative or fully qualified
name of the PVK file.

Password - Pointer to a null-terminated ASCII string containing the password for decrypting
the private key.

Flags - Specifies additional options. See the documentation for details.

Key - Pointer to a HCRYPTKEY value that receives the handle of the imported key. When you
have finished using the key, release the  handle by calling the CryptDestroyKey routine.

Return Value:

TRUE if the routine succeeds; otherwise, FALSE.

--*/
{
    BOOL IsOk = FALSE;
    HANDLE File = INVALID_HANDLE_VALUE;
    BLGP_PVK_HEADER Header;
    DWORD BytesRead;
    PBYTE Salt = NULL;
    HCRYPTKEY ExportKey = 0;
    PBYTE Blob = NULL;

    if (!Provider || !FileName || !Key)
    {
        SetLastError(ERROR_INVALID_PARAMETER);

        goto Leave;
    }

    File = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (File == INVALID_HANDLE_VALUE)
    {
        goto Leave;
    }

    if (!ReadFile(File, &Header, sizeof(BLGP_PVK_HEADER), &BytesRead, NULL))
    {
        goto Leave;
    }

    if (BytesRead < sizeof(BLGP_PVK_HEADER) || Header.Magic != BLGP_PVK_MAGIC)
    {
        SetLastError(ERROR_INVALID_DATA);

        goto Leave;
    }

    if (Header.Encrypted)
    {
        // If the file is encrypted, the user must supply a password.
        if (!Password || *Password == '\0')
        {
            SetLastError(ERROR_INVALID_PASSWORDNAME);

            goto Leave;
        }

        if (Header.SaltLen > 0)
        {
            Salt = HeapAlloc(g_Heap, 0, Header.SaltLen);
            if (!Salt)
            {
                SetLastError(ERROR_OUTOFMEMORY);

                goto Leave;
            }

            if (!ReadFile(File, Salt, Header.SaltLen, &BytesRead, NULL))
            {
                goto Leave;
            }

            if (BytesRead < Header.SaltLen)
            {
                SetLastError(ERROR_INVALID_DATA);

                goto Leave;
            }
        }

        if (!BlgpDeriveKey(Provider, Salt, Header.SaltLen, Password, 0, &ExportKey))
        {
            goto Leave;
        }
    }

    Blob = HeapAlloc(g_Heap, 0, Header.KeyLen);
    if (!Blob)
    {
        SetLastError(ERROR_OUTOFMEMORY);

        goto Leave;
    }

    if (!ReadFile(File, Blob, Header.KeyLen, &BytesRead, NULL))
    {
        goto Leave;
    }

    if (BytesRead < Header.KeyLen)
    {
        SetLastError(ERROR_INVALID_DATA);

        goto Leave;
    }

    if (!CryptImportKey(Provider, Blob, Header.KeyLen, ExportKey, Flags, Key))
    {
        // If the decryption fails, try the weak password form. The file could have been
        // encrypted with a 40 bit RC4 key.
        if (Header.Encrypted && GetLastError() == NTE_BAD_DATA)
        {
            CryptDestroyKey(ExportKey);

            if (!BlgpDeriveKey(Provider, Salt, Header.SaltLen, Password, BLG_PVKEXPORT_FLAG_WEAK, &ExportKey))
            {
                goto Leave;
            }

            // Try to decrypt with the weak key.
            if (!CryptImportKey(Provider, Blob, Header.KeyLen, ExportKey, Flags, Key))
            {
                goto Leave;
            }
        }
        else
        {
            goto Leave;
        }
    }

    IsOk = TRUE;

Leave:
    if (Blob)
    {
        HeapFree(g_Heap, 0, Blob);
    }

    if (ExportKey)
    {
        CryptDestroyKey(ExportKey);
    }

    if (Salt)
    {
        HeapFree(g_Heap, 0, Salt);
    }

    if (File != INVALID_HANDLE_VALUE)
    {
        CloseHandle(File);
    }

    if (GetLastError() == NTE_BAD_DATA)
    {
        SetLastError(ERROR_INVALID_PASSWORDNAME);
    }

    return IsOk;
}

BOOL
BLGPVKCALL
BlgPvkExport(
    IN HCRYPTPROV Provider,
    IN DWORD KeySpec,
    IN PCWSTR FileName,
    IN PCSTR Password OPTIONAL,
    IN DWORD Flags
    )
/*++

Routine Description:

Exports a private key from the specified key container within a cryptographic service
provider and places it in a PVK file.

Arguments:

Provider - Handle to a cryptographic service provider associated with the key container
of the private key to be exported.

KeySpec - Specifies the private key to be exported from the key container.

FileName - Pointer to a null-terminated string containing the name of the PVK file to be
created or overwritten.

Password - Pointer to a null-terminated ASCII string containing the password for encrypting
the private key.

Flags - Specifies additional options for the routine.

Return Value:

TRUE if the routine succeeds; otherwise, FALSE.

--*/
{
    BOOL IsOk = FALSE;
    HCRYPTKEY Key = 0;
    BYTE Salt[BLGP_SALT_LENGTH];
    BLGP_PVK_HEADER Header = {BLGP_PVK_MAGIC, 0, KeySpec, 0, 0, 0};
    HCRYPTKEY ExportKey = 0;
    PBYTE Blob = NULL;
    HANDLE File = INVALID_HANDLE_VALUE;
    DWORD Ignored;

    if (!Provider || !FileName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);

        goto Leave;
    }

    if (!CryptGetUserKey(Provider, KeySpec, &Key))
    {
        goto Leave;
    }

    // If the caller has specified a password, encrypt the PVK file.
    if (Password && *Password != '\0')
    {
        if (!CryptGenRandom(Provider, BLGP_SALT_LENGTH, Salt))
        {
            goto Leave;
        }

        if (!BlgpDeriveKey(Provider, Salt, BLGP_SALT_LENGTH, Password, Flags, &ExportKey))
        {
            goto Leave;
        }

        Header.SaltLen = BLGP_SALT_LENGTH;
        Header.Encrypted = 1;
    }

    if (!CryptExportKey(Key, ExportKey, PRIVATEKEYBLOB, 0, NULL, &Header.KeyLen))
    {
        goto Leave;
    }

    Blob = HeapAlloc(g_Heap, 0, Header.KeyLen);
    if (!Blob)
    {
        SetLastError(ERROR_OUTOFMEMORY);

        goto Leave;
    }

    if (!CryptExportKey(Key, ExportKey, PRIVATEKEYBLOB, 0, Blob, &Header.KeyLen))
    {
        goto Leave;
    }

    File = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (File == INVALID_HANDLE_VALUE)
    {
        goto Leave;
    }

    if (!WriteFile(File, &Header, sizeof(BLGP_PVK_HEADER), &Ignored, NULL))
    {
        goto Leave;
    }

    if (Header.Encrypted && !WriteFile(File, Salt, BLGP_SALT_LENGTH, &Ignored, NULL))
    {
        goto Leave;
    }

    if (!WriteFile(File, Blob, Header.KeyLen, &Ignored, NULL))
    {
        goto Leave;
    }

    IsOk = TRUE;

Leave:
    if (File != INVALID_HANDLE_VALUE)
    {
        CloseHandle(File);
    }

    if (Blob)
    {
        HeapFree(g_Heap, 0, Blob);
    }

    if (ExportKey)
    {
        CryptDestroyKey(ExportKey);
    }

    if (Key)
    {
        CryptDestroyKey(Key);
    }

    return IsOk;
}