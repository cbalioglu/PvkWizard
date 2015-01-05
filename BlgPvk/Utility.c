/*++

Copyright (c) 2007 Can Balioglu. All rights reserved.

See License.txt in the project root for license information.

--*/

#include <windows.h>
#include <strsafe.h>

#include "BlgPvk.h"
#include "BlgPvkp.h"

#define BLGP_WEAK_KEY_LENGTH 0x0028 // 40 bit
#define BLGP_STRONG_KEY_LENGTH 0x0080 // 128 bit

#define BLGP_WEAK_KEY_PADDING 0x000B // 11 bytes

BOOL
BLGPVKCALL
BlgpDeriveKey(
    IN HCRYPTPROV Provider,
    IN PBYTE Salt,
    IN DWORD SaltCb,
    IN PCSTR Password,
    IN DWORD Flags,
    OUT HCRYPTKEY *Key
    )

/*++

Routine Description:

    Derives a symmetric RC4 encryption key from the specified password and salt value.

Arguments:

    Provider - Handle to a cryptographic service provider.

    Salt - Pointer to a buffer containing the salt value.

    SaltCb - Size, in bytes, of the buffer pointed to by the Salt parameter.

    Password - Pointer to a null-terminated ASCII string containing the password to be used.

    Flags - Specifies additional options for the routine.

    Key - Pointer to a handle that receives the derived key.

Return Value:

    TRUE if the routine succeeds; otherwise, FALSE.

--*/

{
    BOOL IsOk = FALSE;
    size_t PasswordCb;
    PROV_ENUMALGS_EX *AlgData = NULL;
    DWORD AlgDataCb = 0;
    DWORD EnumFlags = CRYPT_FIRST;
    DWORD KeyLength;
    HCRYPTHASH Hash = 0;

    if (FAILED(StringCbLengthA(Password, STRSAFE_MAX_CCH * sizeof(CHAR), &PasswordCb)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);

        goto Leave;
    }

    if (!CryptGetProvParam(Provider, PP_ENUMALGS_EX, NULL, &AlgDataCb, EnumFlags))
    {
        goto Leave;
    }

    AlgData = HeapAlloc(g_Heap, 0, AlgDataCb);
    if (!AlgData)
    {
        SetLastError(ERROR_INVALID_PARAMETER);

        goto Leave;
    }

    // Enumerate each supported algorithm to determine the key length to be used.
    while (CryptGetProvParam(Provider, PP_ENUMALGS_EX, (PBYTE) AlgData, &AlgDataCb, EnumFlags))
    {
        if (AlgData->aiAlgid == CALG_RC4)
        {
            // If a weak key is requested or the maximum supported key length is shorter than 128
            // bit, check support for 40 bit key length.
            if (BLGPVK_FLAGON(Flags, BLG_PVKEXPORT_FLAG_WEAK) || AlgData->dwMaxLen < BLGP_STRONG_KEY_LENGTH)
            {
                if (AlgData->dwMinLen > BLGP_WEAK_KEY_LENGTH || AlgData->dwMaxLen < BLGP_WEAK_KEY_LENGTH)
                {
                    break;
                }

                KeyLength = BLGP_WEAK_KEY_LENGTH;
            }
            else
            {
                if (AlgData->dwMinLen > BLGP_STRONG_KEY_LENGTH)
                {
                    break;
                }

                KeyLength = BLGP_STRONG_KEY_LENGTH;
            }

            EnumFlags = 0;

            break;
        }

        EnumFlags = CRYPT_NEXT;
    }

    if (EnumFlags != 0)
    {
        SetLastError(ERROR_NOT_SUPPORTED);

        goto Leave;
    }

    if (!CryptCreateHash(Provider, CALG_SHA1, 0, 0, &Hash))
    {
        goto Leave;
    }

    if (!CryptHashData(Hash, Salt, SaltCb, 0))
    {
        goto Leave;
    }

    if (!CryptHashData(Hash, Password, (DWORD) PasswordCb, 0))
    {
        goto Leave;
    }

    if (!CryptDeriveKey(Provider, CALG_RC4, Hash, KeyLength << 16, Key))
    {
        goto Leave;
    }

    IsOk = TRUE;

Leave:
    if (Hash)
    {
        CryptDestroyHash(Hash);
    }

    if (AlgData)
    {
        HeapFree(g_Heap, 0, AlgData);
    }

    return IsOk;
}