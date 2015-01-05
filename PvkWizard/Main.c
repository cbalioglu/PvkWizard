/*++

Copyright (c) 2007 Can Balioglu. All rights reserved.

See License.txt in the project root for license information.

--*/

#include <windows.h>
#include <commctrl.h>
#include <VersionHelpers.h>

#include "PvkWizard.h"

HINSTANCE g_Instance;
BOOL g_ComCtrl6;
HANDLE g_Heap;

INT_PTR
CALLBACK
BlgpStartPageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    );

INT_PTR
CALLBACK
BlgpSelectionPageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    );

INT_PTR
CALLBACK
BlgpImport1PageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    );

INT_PTR
CALLBACK
BlgpImport2PageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    );

INT_PTR
CALLBACK
BlgpExport1PageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    );

INT_PTR
CALLBACK
BlgpExport2PageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    );

INT_PTR
CALLBACK
BlgpSummaryPageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    );

INT
WINAPI
wWinMain(
    IN HINSTANCE Instance,
    IN HINSTANCE PrevInstance,
    IN LPWSTR CmdLine,
    IN INT ShowCmd
    )
{
    OSVERSIONINFO OSInfo = {sizeof(OSVERSIONINFO)};
    INITCOMMONCONTROLSEX Icex = {sizeof(INITCOMMONCONTROLSEX)};
    PROPSHEETPAGE Pages[7] = {0};
    PROPSHEETHEADER Header = {0};

    UNREFERENCED_PARAMETER(PrevInstance);
    UNREFERENCED_PARAMETER(ShowCmd);

    g_Instance = Instance;

    if (!IsWindowsVersionOrGreater(5, 0, 0))
    {
        MessageBox(NULL, L"This program requires Windows 2000 or later.", L"Private Key Import/Export Wizard",
            MB_ICONERROR | MB_OK);

        return -1;
    }

    if (IsWindowsXPOrGreater())
    {
        g_ComCtrl6 = TRUE;
    }

    g_Heap = GetProcessHeap();

    Icex.dwICC = ICC_LISTVIEW_CLASSES | (g_ComCtrl6 ? ICC_LINK_CLASS : 0);

    if (!InitCommonControlsEx(&Icex))
    {
        return -1;
    }

    Pages[0].dwSize = sizeof(PROPSHEETPAGE);
    Pages[0].dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;
    Pages[0].hInstance = Instance;
    Pages[0].pszTemplate = MAKEINTRESOURCE(IDD_START);
    Pages[0].pfnDlgProc = BlgpStartPageProc;

    Pages[1].dwSize = sizeof(PROPSHEETPAGE);
    Pages[1].dwFlags = PSP_DEFAULT | PSP_USEHEADERTITLE;
    Pages[1].hInstance = Instance;
    Pages[1].pszTemplate = MAKEINTRESOURCE(IDD_SELECTION);
    Pages[1].pfnDlgProc = BlgpSelectionPageProc;
    Pages[1].pszHeaderTitle = MAKEINTRESOURCE(IDS_SELECTION_TITLE);

    Pages[2].dwSize = sizeof(PROPSHEETPAGE);
    Pages[2].dwFlags = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[2].hInstance = Instance;
    Pages[2].pszTemplate = MAKEINTRESOURCE(IDD_IMPORT1);
    Pages[2].pfnDlgProc = BlgpImport1PageProc;
    Pages[2].pszHeaderTitle = MAKEINTRESOURCE(IDS_IMPORT1_TITLE);
    Pages[2].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_IMPORT1_SUBTITLE);

    Pages[3].dwSize = sizeof(PROPSHEETPAGE);
    Pages[3].dwFlags = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[3].hInstance = Instance;
    Pages[3].pszTemplate = MAKEINTRESOURCE(IDD_IMPORT2);
    Pages[3].pfnDlgProc = BlgpImport2PageProc;
    Pages[3].pszHeaderTitle = MAKEINTRESOURCE(IDS_IMPORT2_TITLE);
    Pages[3].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_IMPORT2_SUBTITLE);

    Pages[4].dwSize = sizeof(PROPSHEETPAGE);
    Pages[4].dwFlags = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[4].hInstance = Instance;
    Pages[4].pszTemplate = MAKEINTRESOURCE(IDD_EXPORT1);
    Pages[4].pfnDlgProc = BlgpExport1PageProc;
    Pages[4].pszHeaderTitle = MAKEINTRESOURCE(IDS_EXPORT1_TITLE);
    Pages[4].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_EXPORT1_SUBTITLE);

    Pages[5].dwSize = sizeof(PROPSHEETPAGE);
    Pages[5].dwFlags = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    Pages[5].hInstance = Instance;
    Pages[5].pszTemplate = MAKEINTRESOURCE(IDD_EXPORT2);
    Pages[5].pfnDlgProc = BlgpExport2PageProc;
    Pages[5].pszHeaderTitle = MAKEINTRESOURCE(IDS_EXPORT2_TITLE);
    Pages[5].pszHeaderSubTitle = MAKEINTRESOURCE(IDS_EXPORT2_SUBTITLE);

    Pages[6].dwSize = sizeof(PROPSHEETPAGE);
    Pages[6].dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;
    Pages[6].hInstance = Instance;
    Pages[6].pszTemplate = MAKEINTRESOURCE(IDD_SUMMARY);
    Pages[6].pfnDlgProc = BlgpSummaryPageProc;

    Header.dwSize = sizeof(PROPSHEETHEADER);
    Header.dwFlags = PSH_PROPSHEETPAGE | PSH_WATERMARK | PSH_HEADER | PSH_WIZARD97;
    Header.hInstance = Instance;
    Header.nPages = ARRAYSIZE(Pages);
    Header.ppsp = Pages;
    Header.pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);
    Header.pszbmHeader = MAKEINTRESOURCE(IDB_HEADER);

    // Display the wizard.
    if (PropertySheet(&Header) == -1)
    {
        return -1;
    }

    if (g_ImportData != NULL)
    {
        SecureZeroMemory(g_ImportData, sizeof(BLGP_IMPORT_DATA));
    }

    if (g_ExportData != NULL)
    {
        BlgpDestroyExportData();

        SecureZeroMemory(g_ExportData, sizeof(BLGP_EXPORT_DATA));
    }

    return 0;
}