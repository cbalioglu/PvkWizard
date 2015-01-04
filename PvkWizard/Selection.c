#include <windows.h>
#include <windowsx.h>

#include "PvkWizard.h"

//
//	Define the global variables.
//

DWORD g_WizChoice;
PBLGP_IMPORT_DATA g_ImportData;
PBLGP_EXPORT_DATA g_ExportData;

INT_PTR
CALLBACK
BlgpSelectionPageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    )
{
    switch (Msg)
    {
    case WM_INITDIALOG:
        if (g_BoldFont != NULL)
        {
            SetWindowFont(GetDlgItem(HwndDlg, IDC_IMPORT), g_BoldFont, TRUE);
            SetWindowFont(GetDlgItem(HwndDlg, IDC_EXPORT), g_BoldFont, TRUE);
        }

        CheckRadioButton(HwndDlg, IDC_IMPORT, IDC_EXPORT, IDC_IMPORT);

        break;

    case WM_NOTIFY:
        switch (((LPNMHDR) LParam)->code)
        {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(HwndDlg), PSWIZB_BACK | PSWIZB_NEXT);

            return TRUE;


        case PSN_WIZNEXT:
            if (Button_GetCheck(GetDlgItem(HwndDlg, IDC_IMPORT)) == BST_CHECKED)
            {
                g_WizChoice = BLGP_CHOICE_IMPORT;

                if (!g_ImportData)
                {
                    g_ImportData = HeapAlloc(g_Heap, HEAP_ZERO_MEMORY, sizeof(BLGP_IMPORT_DATA));
                    if (!g_ImportData)
                    {
                        ExitProcess(-1);
                    }
                }

                SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_IMPORT1);
            }
            else
            {
                g_WizChoice = BLGP_CHOICE_EXPORT;

                if (!g_ExportData)
                {
                    g_ExportData = HeapAlloc(g_Heap, HEAP_ZERO_MEMORY, sizeof(BLGP_EXPORT_DATA));
                    if (!g_ExportData)
                    {
                        ExitProcess(-1);
                    }
                }

                SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_EXPORT1);
            }

            return TRUE;
        }

        break;
    }

    return FALSE;
}