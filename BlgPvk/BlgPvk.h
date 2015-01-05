/*++

Copyright (c) 2007 Can Balioglu. All rights reserved.

See License.txt in the project root for license information.

--*/

#pragma once

#ifndef BLGPVK_H
#define BLGPVK_H

#include <windows.h>

#if _WIN32_WINNT < 0x0500
#error The PVK File Import/Export Library requires Windows 2000 or later.
#endif

#ifdef __cplusplus
#define BLGPVK_EXTERN_C extern "C"
#else
#define BLGPVK_EXTERN_C
#endif

#if defined(BLGPVK_LIB_IMPL) || defined(BLGPVK_LIB_STATIC)
#define BLGPVKAPI BLGPVK_EXTERN_C
#else
#define BLGPVKAPI BLGPVK_EXTERN_C DECLSPEC_IMPORT
#endif

#define BLGPVKCALL __stdcall

#define BLG_PVKEXPORT_FLAG_WEAK 0x0001 // Use 40 bit RC4 encryption.

BLGPVKAPI
BOOL
BLGPVKCALL
BlgPvkExport(
    IN HCRYPTPROV Provider,
    IN DWORD KeySpec,
    IN PCWSTR FileName,
    IN PCSTR Password OPTIONAL,
    IN DWORD Flags
    );

BLGPVKAPI
BOOL
BLGPVKCALL
BlgPvkImport(
    IN HCRYPTPROV Provider,
    IN PCWSTR FileName,
    IN PCSTR Password OPTIONAL,
    IN DWORD Flags,
    OUT HCRYPTKEY *Key
    );

#endif