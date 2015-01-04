#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <BlgPvk.h>

#include "PvkWizard.h"

INT_PTR
CALLBACK
BlgpSummaryPageProc(
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
        HWND HwndCtrl = GetDlgItem(HwndDlg, IDC_SUMMARY);
        if (HwndCtrl)
        {
            RECT Rect = {80, 102};
            LVCOLUMN Column = {LVCF_WIDTH | LVCF_TEXT};
            WCHAR Text[16];

            ListView_SetExtendedListViewStyle(HwndCtrl, LVS_EX_FULLROWSELECT);

            MapDialogRect(HwndDlg, &Rect);

            Column.cx = Rect.left;

            if (LoadString(g_Instance, IDS_SETTING_COLUMN, Text, ARRAYSIZE(Text)) > 0)
            {
                Column.pszText = Text;
            }
            else
            {
                Column.pszText = L"\0";
            }

            ListView_InsertColumn(HwndCtrl, 0, &Column);

            Column.cx = Rect.top;

            if (LoadString(g_Instance, IDS_VALUE_COLUMN, Text, ARRAYSIZE(Text)) > 0)
            {
                Column.pszText = Text;
            }
            else
            {
                Column.pszText = L"\0";
            }

            ListView_InsertColumn(HwndCtrl, 1, &Column);
        }

        if (g_TitleFont != NULL)
        {
            SetWindowFont(GetDlgItem(HwndDlg, IDC_TITLE), g_TitleFont, TRUE);
        }

        if (g_ComCtrl6)
        {
            HFONT Font;
            WCHAR Link[128];

            Font = GetWindowFont(HwndDlg);

            if (Font && LoadString(g_Instance, IDS_LINK, Link, ARRAYSIZE(Link)) > 0)
            {
                RECT Rect = {215, 178, 110, 8};
                HWND Hwnd;

                MapDialogRect(HwndDlg, &Rect);

                Hwnd = CreateWindow(WC_LINK, Link, WS_VISIBLE | WS_CHILD,
                    Rect.left, Rect.top, Rect.right, Rect.bottom, HwndDlg, (HMENU) IDC_LINK, NULL, NULL);

                if (Hwnd)
                {
                    SetWindowFont(Hwnd, Font, FALSE);
                }
            }
        }

        break;
    }

    case WM_NOTIFY:
    {
        LPNMHDR Header = (LPNMHDR) LParam;

        if (Header->idFrom == IDC_LINK)
        {
            if (Header->code == NM_CLICK || Header->code == NM_RETURN)
            {
                ShellExecute(HwndDlg, L"open", ((NMLINK *) Header)->item.szUrl, NULL, NULL, SW_SHOWNORMAL);

                return TRUE;
            }

            break;
        }

        switch (Header->code)
        {
        case PSN_SETACTIVE:
        {
            HWND HwndCtrl = GetDlgItem(HwndDlg, IDC_SUMMARY);

            if (HwndCtrl != NULL)
            {
                LVITEM Item = {LVIF_TEXT};
                WCHAR Yes[10] = {0};
                WCHAR No[10] = {0};
                WCHAR Text[128];

                LoadString(g_Instance, IDS_YES, Yes, ARRAYSIZE(Yes));
                LoadString(g_Instance, IDS_NO, No, ARRAYSIZE(No));

                ListView_DeleteAllItems(HwndCtrl);

                if (g_WizChoice == BLGP_CHOICE_IMPORT)
                {
                    if (LoadString(g_Instance, IDS_OPERATION, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        if (LoadString(g_Instance, IDS_IMPORT, Text, ARRAYSIZE(Text)) > 0)
                        {
                            Item.iSubItem = 1;

                            ListView_SetItem(HwndCtrl, &Item);
                        }

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_CER_FILE, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        Item.iSubItem = 1;
                        Item.pszText = g_ImportData->CerFile;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_PVK_FILE, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        Item.iSubItem = 1;
                        Item.pszText = g_ImportData->PvkFile;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_PVK_PSWD, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        Item.iSubItem = 1;
                        Item.pszText = *g_ImportData->PvkPswd != L'\0' ? Yes : No;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_EXPORTABLE, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        Item.iSubItem = 1;
                        Item.pszText = BLG_FLAGON(g_ImportData->Flags, CRYPT_EXPORTABLE) ? Yes : No;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_USER_PROTECTED, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        Item.iSubItem = 1;
                        Item.pszText = BLG_FLAGON(g_ImportData->Flags, CRYPT_USER_PROTECTED) ? Yes : No;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_CSP, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        if (g_ImportData->ProvType == PROV_RSA_FULL)
                        {
                            if (LoadString(g_Instance, IDS_RSA_PROV, Text, ARRAYSIZE(Text)) > 0)
                            {
                                Item.iSubItem = 1;

                                ListView_SetItem(HwndCtrl, &Item);
                            }
                        }
                        else
                        {
                            if (LoadString(g_Instance, IDS_DSS_PROV, Text, ARRAYSIZE(Text)) > 0)
                            {
                                Item.iSubItem = 1;

                                ListView_SetItem(HwndCtrl, &Item);
                            }
                        }

                        Item.iItem++;
                    }
                }
                else
                {
                    if (LoadString(g_Instance, IDS_OPERATION, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        if (LoadString(g_Instance, IDS_EXPORT, Text, ARRAYSIZE(Text)) > 0)
                        {
                            Item.iSubItem = 1;

                            ListView_SetItem(HwndCtrl, &Item);
                        }

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_CERTIFICATE, Text, ARRAYSIZE(Text)) > 0)
                    {
                        DWORD Type = CERT_X500_NAME_STR;

                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        CertGetNameString(g_ExportData->CertContext,
                            CERT_NAME_RDN_TYPE, 0, &Type, Text, ARRAYSIZE(Text));

                        Item.iSubItem = 1;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_PVK_FILE, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        Item.iSubItem = 1;
                        Item.pszText = g_ExportData->PvkFile;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_PVK_PSWD, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        Item.iSubItem = 1;
                        Item.pszText = *g_ExportData->PvkPswd != L'\0' ? Yes : No;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }

                    if (LoadString(g_Instance, IDS_STRONG_ENC, Text, ARRAYSIZE(Text)) > 0)
                    {
                        Item.iSubItem = 0;
                        Item.pszText = Text;

                        ListView_InsertItem(HwndCtrl, &Item);

                        Item.iSubItem = 1;
                        Item.pszText = BLG_FLAGON(g_ExportData->Flags, BLG_PVKEXPORT_FLAG_WEAK) ? No : Yes;

                        ListView_SetItem(HwndCtrl, &Item);

                        Item.iItem++;
                    }
                }
            }

            PropSheet_SetWizButtons(GetParent(HwndDlg), PSWIZB_BACK | PSWIZB_FINISH);

            return TRUE;
        }

        case PSN_WIZFINISH:
            if (g_WizChoice == BLGP_CHOICE_IMPORT)
            {
                if (!BlgpPvkImport(GetAncestor(HwndDlg, GA_ROOT)))
                {
                    SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, TRUE);
                }
            }
            else
            {
                if (!BlgpPvkExport(GetAncestor(HwndDlg, GA_ROOT)))
                {
                    SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, TRUE);
                }
            }

            return TRUE;

        case PSN_WIZBACK:
            if (g_WizChoice == BLGP_CHOICE_IMPORT)
            {
                SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_IMPORT2);
            }
            else
            {
                SetWindowLongPtr(HwndDlg, DWLP_MSGRESULT, IDD_EXPORT2);
            }

            return TRUE;
        }

        break;
    }
    }

    return FALSE;
}