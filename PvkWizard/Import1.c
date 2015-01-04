#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlwapi.h>

#include "PvkWizard.h"

static
BOOL
BlgpValidateFile(
    IN HWND  HwndDlg,
    IN PWSTR File,
    IN DWORD CtrlId,
    IN DWORD Error
    )
{
    WCHAR Temp[MAX_PATH];
    BOOL IsOk = FALSE;

    Edit_GetText(GetDlgItem(HwndDlg, CtrlId), Temp, MAX_PATH);

    StrTrim(Temp, L" ");

    if (*Temp == L'\0' || !PathCanonicalize(File, Temp) || PathIsDirectory(File))
    {
        ShellMessageBox(g_Instance, GetAncestor(HwndDlg, GA_ROOT),
            MAKEINTRESOURCE(IDS_INVALID_FILENAME), NULL, MB_ICONERROR);

        goto Leave;
    }

    if (!PathFileExists(File))
    {
        ShellMessageBox(g_Instance, GetAncestor(HwndDlg, GA_ROOT), MAKEINTRESOURCE(Error), NULL, MB_ICONERROR);

        goto Leave;
    }

    IsOk = TRUE;

Leave:

    if (!IsOk)
    {
        SendMessage(HwndDlg, WM_NEXTDLGCTL, (WPARAM) GetDlgItem(HwndDlg, CtrlId), TRUE);

        SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, -1);
    }

    return IsOk;
}

INT_PTR
CALLBACK
BlgpImport1PageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    )
{
    switch (Msg)
    {
    case WM_INITDIALOG:
        Edit_LimitText(GetDlgItem(HwndDlg, IDC_CER_FILE), MAX_PATH);
        Edit_LimitText(GetDlgItem(HwndDlg, IDC_PVK_FILE), MAX_PATH);
        Edit_LimitText(GetDlgItem(HwndDlg, IDC_PVK_PSWD), BLGP_MAX_PSWD);

        break;

    case WM_COMMAND:

        switch (HIWORD(WParam))
        {
        case BN_CLICKED:
            if (LOWORD(WParam) == IDC_BROWSE_CER || LOWORD(WParam) == IDC_BROWSE_PVK)
            {
                OPENFILENAME OpenFileName = {sizeof(OPENFILENAME)};
                WCHAR Path[MAX_PATH] = {0};
                WCHAR Type[256] = {0};

                OpenFileName.hwndOwner = GetAncestor(HwndDlg, GA_ROOT);
                OpenFileName.lpstrFilter = Type;
                OpenFileName.nFilterIndex = 1;
                OpenFileName.lpstrFile = Path;
                OpenFileName.nMaxFile = MAX_PATH;
                OpenFileName.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

                if (LOWORD(WParam) == IDC_BROWSE_CER)
                {
                    if (LoadString(g_Instance, IDS_CER_FILE_TYPE, Type, ARRAYSIZE(Type) - 1) > 0)
                    {
                        if (GetOpenFileName(&OpenFileName))
                        {
                            Edit_SetText(GetDlgItem(HwndDlg, IDC_CER_FILE), Path);
                        }
                    }
                }
                else
                {
                    if (LoadString(g_Instance, IDS_PVK_FILE_TYPE, Type, ARRAYSIZE(Type) - 1) > 0)
                    {
                        if (GetOpenFileName(&OpenFileName))
                        {
                            Edit_SetText(GetDlgItem(HwndDlg, IDC_PVK_FILE), Path);
                        }
                    }
                }

                return TRUE;
            }

            break;

        case EN_CHANGE:
            if (LOWORD(WParam) == IDC_CER_FILE || LOWORD(WParam) == IDC_PVK_FILE)
            {
                INT CerFileLen = Edit_GetTextLength(GetDlgItem(HwndDlg, IDC_CER_FILE));
                INT PvkFileLen = Edit_GetTextLength(GetDlgItem(HwndDlg, IDC_PVK_FILE));

                PropSheet_SetWizButtons(GetParent(HwndDlg),
                    PSWIZB_BACK | ((CerFileLen > 0 && PvkFileLen > 0) ? PSWIZB_NEXT : 0));

                return TRUE;
            }

            break;
        }

        break;

    case WM_NOTIFY:
        switch (((LPNMHDR) LParam)->code)
        {
        case PSN_SETACTIVE:
            SendMessage(HwndDlg, WM_COMMAND, MAKELONG(IDC_CER_FILE, EN_CHANGE), 0);

            return TRUE;


        case PSN_WIZNEXT:
        {
            if (!BlgpValidateFile(HwndDlg, g_ImportData->CerFile, IDC_CER_FILE, IDS_CER_FILE_NOT_FOUND))
            {
                return TRUE;
            }

            if (!BlgpValidateFile(HwndDlg, g_ImportData->PvkFile, IDC_PVK_FILE, IDS_PVK_FILE_NOT_FOUND))
            {
                return TRUE;
            }

            Edit_GetText(GetDlgItem(HwndDlg, IDC_PVK_PSWD), g_ImportData->PvkPswd, BLGP_MAX_PSWD);

            SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_IMPORT2);

            return TRUE;
        }

        case PSN_WIZBACK:
            SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_SELECTION);

            return TRUE;
        }

        break;
    }

    return FALSE;
}