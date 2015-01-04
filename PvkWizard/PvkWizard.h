#pragma once

#ifndef PVKWIZ_H
#define PVKWIZ_H

#include "Resource.h"

#define BLG_FLAGON(x, Flag) (((x) & (Flag)) > 0)

// Valid values for g_WizChoice.
#define BLGP_CHOICE_IMPORT 0x01
#define BLGP_CHOICE_EXPORT 0x02

#define BLGP_MAX_PSWD 512

typedef struct _BLGP_IMPORT_DATA
{
    WCHAR CerFile[MAX_PATH];
    WCHAR PvkFile[MAX_PATH];
    WCHAR PvkPswd[BLGP_MAX_PSWD];
    DWORD ProvType;
    DWORD Flags;

} BLGP_IMPORT_DATA, *PBLGP_IMPORT_DATA;


typedef struct _BLGP_EXPORT_DATA
{
    PCCERT_CONTEXT CertContext;
    HCRYPTPROV CryptProv;
    DWORD KeySpec;
    WCHAR PvkFile[MAX_PATH];
    WCHAR PvkPswd[BLGP_MAX_PSWD];
    DWORD Flags;

} BLGP_EXPORT_DATA, *PBLGP_EXPORT_DATA;

extern HINSTANCE g_Instance;
extern BOOL g_ComCtrl6;
extern HANDLE g_Heap;
extern HFONT g_TitleFont, g_BoldFont;
extern DWORD g_WizChoice;
extern PBLGP_IMPORT_DATA g_ImportData;
extern PBLGP_EXPORT_DATA g_ExportData;

BOOL
BlgpPvkImport(
    IN HWND HwndOwner
    );

BOOL
BlgpPvkExport(
    IN HWND HwndOwner
    );

VOID
BlgpDestroyExportData(
    VOID
    );

#endif