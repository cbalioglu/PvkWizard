#include <windows.h>
#include <windowsx.h>

#include "PvkWizard.h"

INT_PTR
CALLBACK
BlgpImport2PageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    )
{
    switch (Msg)
    {
    case WM_INITDIALOG:
        CheckRadioButton(HwndDlg, IDC_RSA_PROV, IDC_DSS_PROV, IDC_RSA_PROV);

        break;

    case WM_NOTIFY:

        switch (((LPNMHDR) LParam)->code)
        {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(HwndDlg), PSWIZB_BACK | PSWIZB_NEXT);

            return TRUE;

        case PSN_WIZNEXT:
            g_ImportData->Flags = 0;

            if (IsDlgButtonChecked(HwndDlg, IDC_EXPORTABLE))
            {
                g_ImportData->Flags |= CRYPT_EXPORTABLE;
            }

            if (IsDlgButtonChecked(HwndDlg, IDC_USER_PROTECTED))
            {
                g_ImportData->Flags |= CRYPT_USER_PROTECTED;
            }

            if (Button_GetCheck(GetDlgItem(HwndDlg, IDC_RSA_PROV)) == BST_CHECKED)
            {
                g_ImportData->ProvType = PROV_RSA_FULL;
            }
            else
            {
                g_ImportData->ProvType = PROV_DSS;
            }

            SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_SUMMARY);

            return TRUE;

        case PSN_WIZBACK:
            SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_IMPORT1);

            return TRUE;
        }

        break;
    }

    return FALSE;
}