/*++

Copyright (c) 2007 Can Balioglu. All rights reserved.

See License.txt in the project root for license information.

--*/

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <memory.h>
#include <BlgPvk.h>

#include "PvkWizard.h"

INT_PTR
CALLBACK
BlgpExport2PageProc(
    IN HWND HwndDlg,
    IN UINT Msg,
    IN WPARAM WParam,
    IN LPARAM LParam
    )
{
    switch (Msg)
    {
    case WM_INITDIALOG:
        Edit_LimitText(GetDlgItem(HwndDlg, IDC_PVK_FILE), MAX_PATH);
        Edit_LimitText(GetDlgItem(HwndDlg, IDC_PVK_PSWD), BLGP_MAX_PSWD);

        Button_SetCheck(GetDlgItem(HwndDlg, IDC_STRONG_ENC), TRUE);

        break;

    case WM_COMMAND:

        switch (HIWORD(WParam))
        {
        case BN_CLICKED:
            if (LOWORD(WParam) == IDC_BROWSE_PVK)
            {
                WCHAR Type[128] = {0};

                if (LoadString(g_Instance, IDS_PVK_FILE_TYPE, Type, ARRAYSIZE(Type) - 1) > 0)
                {
                    OPENFILENAME OpenFileName = {sizeof(OPENFILENAME)};
                    OpenFileName.hwndOwner = GetAncestor(HwndDlg, GA_ROOT);
                    OpenFileName.lpstrFilter = Type;
                    OpenFileName.nFilterIndex = 1;
                    OpenFileName.lpstrFile = g_ExportData->PvkFile;
                    OpenFileName.nMaxFile = MAX_PATH;
                    OpenFileName.Flags = OFN_DONTADDTORECENT | OFN_OVERWRITEPROMPT;
                    OpenFileName.lpstrDefExt = L"pvk";

                    if (GetSaveFileName(&OpenFileName))
                    {
                        Edit_SetText(GetDlgItem(HwndDlg, IDC_PVK_FILE), g_ExportData->PvkFile);
                    }
                }

                return TRUE;
            }

            break;

        case EN_CHANGE:
            if (LOWORD(WParam) == IDC_PVK_FILE)
            {
                PropSheet_SetWizButtons(GetParent(HwndDlg),
                    PSWIZB_BACK | (Edit_GetTextLength(GetDlgItem(HwndDlg, IDC_PVK_FILE)) > 0 ? PSWIZB_NEXT : 0));

                return TRUE;
            }

            break;
        }

        break;

    case WM_NOTIFY:
        switch (((LPNMHDR) LParam)->code)
        {
        case PSN_SETACTIVE:
            SendMessage(HwndDlg, WM_COMMAND, MAKELONG(IDC_PVK_FILE, EN_CHANGE), 0);

            return TRUE;


        case PSN_WIZNEXT:
        {
            WCHAR RawPath[MAX_PATH], Path[MAX_PATH], PvkPswd[BLGP_MAX_PSWD];
            INT Cch1, Cch2;

            Edit_GetText(GetDlgItem(HwndDlg, IDC_PVK_FILE), RawPath, MAX_PATH);

            StrTrim(RawPath, L" ");

            if (*RawPath == L'\0' || !PathCanonicalize(Path, RawPath) || PathIsDirectory(Path))
            {
                ShellMessageBox(g_Instance, GetAncestor(HwndDlg, GA_ROOT),
                    MAKEINTRESOURCE(IDS_INVALID_FILENAME), NULL, MB_ICONERROR);

                SendMessage(HwndDlg, WM_NEXTDLGCTL, (WPARAM) GetDlgItem(HwndDlg, IDC_PVK_FILE), TRUE);

                SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, -1);

                return TRUE;
            }

            if (*g_ExportData->PvkFile == L'\0' ||
                CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, Path, -1, g_ExportData->PvkFile, -1) != CSTR_EQUAL)
            {
                if (PathFileExists(Path))
                {
                    if (ShellMessageBox(g_Instance, GetAncestor(HwndDlg, GA_ROOT),
                            MAKEINTRESOURCE(IDS_OVERWRITE), NULL, MB_ICONWARNING | MB_YESNO) == IDNO)
                    {
                        SendMessage(HwndDlg, WM_NEXTDLGCTL, (WPARAM) GetDlgItem(HwndDlg, IDC_PVK_FILE), TRUE);

                        SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, -1);

                        return TRUE;
                    }
                }

                StrCpy(g_ExportData->PvkFile, Path);
            }

            Cch1 = Edit_GetText(GetDlgItem(HwndDlg, IDC_PVK_PSWD), g_ExportData->PvkPswd, BLGP_MAX_PSWD);
            Cch2 = Edit_GetText(GetDlgItem(HwndDlg, IDC_CONFIRM_PSWD), PvkPswd, BLGP_MAX_PSWD);

            if (Cch1 != Cch2 || memcmp(g_ExportData->PvkPswd, PvkPswd, Cch1 * sizeof(WCHAR)) != 0)
            {
                ShellMessageBox(g_Instance, GetAncestor(HwndDlg, GA_ROOT),
                    MAKEINTRESOURCE(IDS_PSWD_DO_NOT_MATCH), NULL, MB_ICONERROR | MB_OK);

                SendMessage(HwndDlg, WM_NEXTDLGCTL, (WPARAM) GetDlgItem(HwndDlg, IDC_PVK_PSWD), TRUE);

                SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, -1);

                return TRUE;
            }

            if (*g_ExportData->PvkPswd == L'\0')
            {
                if (ShellMessageBox(g_Instance, GetAncestor(HwndDlg, GA_ROOT),
                        MAKEINTRESOURCE(IDS_WARNING), NULL, MB_ICONWARNING | MB_YESNO) == IDNO)
                {
                    SendMessage(HwndDlg, WM_NEXTDLGCTL, (WPARAM) GetDlgItem(HwndDlg, IDC_PVK_PSWD), TRUE);

                    SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, -1);

                    return TRUE;
                }
            }

            g_ExportData->Flags = IsDlgButtonChecked(HwndDlg, IDC_STRONG_ENC) ? 0 : BLG_PVKEXPORT_FLAG_WEAK;

            SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_SUMMARY);

            return TRUE;
        }

        case PSN_WIZBACK:
            SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_EXPORT1);

            return TRUE;
        }

        break;
    }

    return FALSE;
}