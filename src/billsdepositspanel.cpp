﻿/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "attachmentdialog.h"
#include "billsdepositspanel.h"
#include "billsdepositsdialog.h"
#include "util.h"
#include "constants.h"

#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"
#include "model/Model_Setting.h"

#include "../resources/attachment.xpm"
#include "../resources/downarrow.xpm"
#include "../resources/error.xpm"
#include "../resources/rightarrow.xpm"
#include "../resources/rt_exec_auto.xpm"
#include "../resources/rt_exec_user.xpm"
#include "../resources/tipicon.xpm"
#include "../resources/uparrow.xpm"


enum
{
    MENU_TREEPOPUP_NEW = wxID_HIGHEST + 1300,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_DELETE,
    MENU_POPUP_BD_ENTER_OCCUR,
    MENU_POPUP_BD_SKIP_OCCUR,
    MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
    MENU_HEADER_HIDE,
    MENU_HEADER_SORT,
    MENU_HEADER_RESET,
};

const wxString BILLSDEPOSITS_REPEATS[] =
{
    wxTRANSLATE("None"),
    wxTRANSLATE("Weekly"),
    wxTRANSLATE("Bi-Weekly"),
    wxTRANSLATE("Monthly"),
    wxTRANSLATE("Bi-Monthly"),
    wxTRANSLATE("Quarterly"),
    wxTRANSLATE("Half-Yearly"),
    wxTRANSLATE("Yearly"),
    wxTRANSLATE("Four Months"),
    wxTRANSLATE("Four Weeks"),
    wxTRANSLATE("Daily"),
    wxTRANSLATE("In %s Days"),
    wxTRANSLATE("In %s Months"),
    wxTRANSLATE("Every %s Days"),
    wxTRANSLATE("Every %s Months"),
    wxTRANSLATE("Monthly (last day)"),
    wxTRANSLATE("Monthly (last business day)")

};

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmBillsDepositsPanel, wxPanel)
    EVT_BUTTON(wxID_NEW, mmBillsDepositsPanel::OnNewBDSeries)
    EVT_BUTTON(wxID_EDIT, mmBillsDepositsPanel::OnEditBDSeries)
    EVT_BUTTON(wxID_DELETE, mmBillsDepositsPanel::OnDeleteBDSeries)
    EVT_BUTTON(wxID_PASTE, mmBillsDepositsPanel::OnEnterBDTransaction)
    EVT_BUTTON(wxID_IGNORE, mmBillsDepositsPanel::OnSkipBDTransaction)
    EVT_BUTTON(wxID_FILE, mmBillsDepositsPanel::OnOpenAttachment)
wxEND_EVENT_TABLE()
/*******************************************************/
wxBEGIN_EVENT_TABLE(billsDepositsListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,   billsDepositsListCtrl::OnListItemActivated)
    EVT_RIGHT_DOWN(billsDepositsListCtrl::OnItemRightClick)
    EVT_LEFT_DOWN(billsDepositsListCtrl::OnListLeftClick)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, billsDepositsListCtrl::OnListItemSelected)
    EVT_LIST_COL_END_DRAG(wxID_ANY, billsDepositsListCtrl::OnItemResize)
    EVT_LIST_COL_CLICK(wxID_ANY, billsDepositsListCtrl::OnColClick)
    EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, billsDepositsListCtrl::OnColRightClick)

    EVT_MENU(MENU_HEADER_HIDE, billsDepositsListCtrl::OnHeaderHide)
    EVT_MENU(MENU_HEADER_SORT, billsDepositsListCtrl::OnHeaderSort)
    EVT_MENU(MENU_HEADER_RESET, billsDepositsListCtrl::OnHeaderReset)

    EVT_MENU(MENU_TREEPOPUP_NEW,              billsDepositsListCtrl::OnNewBDSeries)
    EVT_MENU(MENU_TREEPOPUP_EDIT,             billsDepositsListCtrl::OnEditBDSeries)
    EVT_MENU(MENU_TREEPOPUP_DELETE,           billsDepositsListCtrl::OnDeleteBDSeries)
    EVT_MENU(MENU_POPUP_BD_ENTER_OCCUR,       billsDepositsListCtrl::OnEnterBDTransaction)
    EVT_MENU(MENU_POPUP_BD_SKIP_OCCUR,        billsDepositsListCtrl::OnSkipBDTransaction)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, billsDepositsListCtrl::OnOrganizeAttachments)

    EVT_LIST_KEY_DOWN(wxID_ANY,   billsDepositsListCtrl::OnListKeyDown)
wxEND_EVENT_TABLE()
/*******************************************************/

billsDepositsListCtrl::billsDepositsListCtrl(mmBillsDepositsPanel* bdp, wxWindow *parent, wxWindowID winid)
: mmListCtrl(parent, winid)
, m_bdp(bdp)
{
    // load the global variables
    m_selected_col = Model_Setting::instance().GetIntSetting("BD_SORT_COL", m_bdp->col_sort());
    m_asc = Model_Setting::instance().GetBoolSetting("BD_ASC", true);
}

billsDepositsListCtrl::~billsDepositsListCtrl()
{
}

void billsDepositsListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = ColumnHeaderNr;
    if (0 > ColumnNr || ColumnNr >= m_bdp->getColumnsNumber() || ColumnNr == 0) return;

    if (m_selected_col == ColumnNr && event.GetId() != MENU_HEADER_SORT) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

    Model_Setting::instance().Set("BD_ASC", m_asc);
    Model_Setting::instance().Set("BD_SORT_COL", m_selected_col);

    int id = -1;
    if (m_selected_row >= 0) id = m_bdp->bills_[m_selected_row].BDID;
    refreshVisualList(m_bdp->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnColRightClick(wxListEvent& event)
{
    ColumnHeaderNr = event.GetColumn();
    if (0 > ColumnHeaderNr || ColumnHeaderNr >= m_bdp->getColumnsNumber()) return;
    wxMenu menu;
    menu.Append(MENU_HEADER_HIDE, _("Hide column"));
    menu.Append(MENU_HEADER_SORT, _("Order by this column"));
    menu.Append(MENU_HEADER_RESET, _("Reset columns size"));
    PopupMenu(&menu);
    this->SetFocus();
}

void billsDepositsListCtrl::OnHeaderHide(wxCommandEvent& event)
{
    billsDepositsListCtrl::SetColumnWidth(ColumnHeaderNr, 0);
    const wxString parameter_name = wxString::Format("BD_COL%i_WIDTH", ColumnHeaderNr);
    Model_Setting::instance().Set(parameter_name, 0);
}

void billsDepositsListCtrl::OnHeaderSort(wxCommandEvent& event)
{
    wxListEvent e;
    e.SetId(MENU_HEADER_SORT);
    billsDepositsListCtrl::OnColClick(e);
}

void billsDepositsListCtrl::OnHeaderReset(wxCommandEvent& event)
{
    wxString parameter_name;
    for (int i = 0; i <= m_bdp->getColumnsNumber(); i++)
    {
        billsDepositsListCtrl::SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
        parameter_name = wxString::Format("BD_COL%i_WIDTH", i);
        Model_Setting::instance().Set(parameter_name, billsDepositsListCtrl::GetColumnWidth(i));
    }
    wxListEvent e;
    e.SetId(MENU_HEADER_SORT);
    ColumnHeaderNr = m_bdp->col_sort();
    m_asc = true;
    billsDepositsListCtrl::OnColClick(e);
    }

mmBillsDepositsPanel::mmBillsDepositsPanel(wxWindow *parent, wxWindowID winid
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : m_imageList(nullptr)
    , listCtrlAccount_(nullptr)
    , transFilterDlg_(nullptr)
    , bitmapTransFilter_(nullptr)
    , m_infoTextMini(nullptr)
    , m_infoText(nullptr)
{
    ColName_[COL_ICON] = (" ");
    ColName_[COL_ID] = _("ID");
    ColName_[COL_PAYMENT_DATE] = _("Payment Date");
    ColName_[COL_DUE_DATE] = _("Due Date");
    ColName_[COL_ACCOUNT] = _("Account");
    ColName_[COL_PAYEE] = _("Payee");
    ColName_[COL_STATUS] = _("Status");
    ColName_[COL_CATEGORY] = _("Category");
    ColName_[COL_TYPE] = _("Type");
    ColName_[COL_AMOUNT] = _("Amount");
    ColName_[COL_FREQUENCY] = _("Frequency");
    ColName_[COL_REPEATS] = _("Repetitions");
    ColName_[COL_AUTO] = _("Autorepeat");
    ColName_[COL_DAYS] = _("Payment");
    ColName_[COL_NUMBER] = _("Number");
    ColName_[COL_NOTES] = _("Notes");

    this->tips_.Add(_("MMEX allows regular payments to be set up as transactions. These transactions can also be regular deposits, or transfers that will occur at some future time. These transactions act as a reminder that an event is about to occur, and appears on the Home Page 14 days before the transaction is due. "));
    this->tips_.Add(_("Tip: These transactions can be set up to activate – allowing the user to adjust any values on the due date."));

    Create(parent, winid, pos, size, style, name);
}

bool mmBillsDepositsPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    this->windowsFreezeThaw();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    /* Set up the transaction filter.  The transFilter dialog will be destroyed
       when the checking panel is destroyed. */
    transFilterActive_ = false;
    transFilterDlg_ = new mmFilterTransactionsDialog(this);

    initVirtualListControl();

    this->windowsFreezeThaw();

    return TRUE;
}

mmBillsDepositsPanel::~mmBillsDepositsPanel()
{
   if (m_imageList)
        delete m_imageList;
   if (transFilterDlg_)
       delete transFilterDlg_;
}

void mmBillsDepositsPanel::CreateControls()
{

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition
        , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(headerPanel, wxID_ANY
        , _("Repeating Transactions"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, 0, wxALL, 1);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

    wxBitmap itemStaticBitmap(rightarrow_xpm);
    bitmapTransFilter_ = new wxStaticBitmap(headerPanel, wxID_ANY, itemStaticBitmap);
    itemBoxSizerHHeader2->Add(bitmapTransFilter_, 0, wxALL, 1);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_LEFT_DOWN
        , wxMouseEventHandler(mmBillsDepositsPanel::OnFilterTransactions), nullptr, this);
    bitmapTransFilter_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmBillsDepositsPanel::OnFilterTransactions), nullptr, this);

    itemBoxSizerHHeader2->AddSpacer(5);
    wxStaticText* statTextTransFilter_ = new wxStaticText(headerPanel, wxID_ANY
        , _("Transaction Filter"));
    itemBoxSizerHHeader2->Add(statTextTransFilter_, 0, wxALIGN_CENTER_VERTICAL, 0);

    /* ---------------------- */
    wxSplitterWindow* itemSplitterWindowBillsDeposit = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    wxSize imageSize(16, 16);
    m_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
    m_imageList->Add(wxBitmap(wxImage(error_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(rt_exec_auto_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(rt_exec_user_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(uparrow_xpm).Scale(16, 16)));
    m_imageList->Add(wxBitmap(wxImage(downarrow_xpm).Scale(16, 16)));

    listCtrlAccount_ = new billsDepositsListCtrl(this, itemSplitterWindowBillsDeposit);

    listCtrlAccount_->SetImageList(m_imageList, wxIMAGE_LIST_SMALL);
    for (const auto& column : ColName_)
    {
        wxListItem itemCol;
        itemCol.SetText(column.second);
        listCtrlAccount_->InsertColumn(column.first, column.second
            , (column.first == COL_PAYMENT_DATE) || (column.first == COL_AMOUNT)
                || (column.first == COL_ID) || (column.first == COL_REPEATS)
                ?  wxLIST_FORMAT_RIGHT : wxLIST_FORMAT_LEFT);

        int col_x = Model_Setting::instance().GetIntSetting(wxString::Format("BD_COL%d_WIDTH", column.first)
            , (column.first > 0 ? wxLIST_AUTOSIZE_USEHEADER : 150));
        listCtrlAccount_->SetColumnWidth(column.first, col_x);
    }

    wxPanel* bdPanel = new wxPanel(itemSplitterWindowBillsDeposit, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    itemSplitterWindowBillsDeposit->SplitHorizontally(listCtrlAccount_, bdPanel);
    itemSplitterWindowBillsDeposit->SetMinimumPaneSize(100);
    itemSplitterWindowBillsDeposit->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindowBillsDeposit, 1, wxGROW | wxALL, 1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    bdPanel->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, g_flags);

    wxButton* itemButtonNew = new wxButton(bdPanel, wxID_NEW, _("&New "));
    itemButtonNew->SetToolTip(_("New Bills & Deposit Series"));
    itemBoxSizer5->Add(itemButtonNew, g_flags);

    wxButton* itemButton81 = new wxButton(bdPanel, wxID_EDIT, _("&Edit "));
    itemButton81->SetToolTip(_("Edit Bills & Deposit Series"));
    itemBoxSizer5->Add(itemButton81, g_flags);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton(bdPanel, wxID_DELETE, _("&Delete "));
    itemButton7->SetToolTip(_("Delete Bills & Deposit Series"));
    itemBoxSizer5->Add(itemButton7, g_flags);
    itemButton7->Enable(false);

    wxButton* itemButton8 = new wxButton(bdPanel, wxID_PASTE, _("En&ter"));
    itemButton8->SetToolTip(_("Enter Next Bills & Deposit Occurrence"));
    itemBoxSizer5->Add(itemButton8, g_flags);
    itemButton8->Enable(false);

    wxButton* buttonSkipTrans = new wxButton(bdPanel, wxID_IGNORE, _("&Skip"));
    buttonSkipTrans->SetToolTip(_("Skip Next Bills & Deposit Occurrence"));
    itemBoxSizer5->Add(buttonSkipTrans, g_flags);
    buttonSkipTrans->Enable(false);

    wxBitmapButton* btnAttachment_ = new wxBitmapButton(bdPanel, wxID_FILE
        , wxBitmap(attachment_xpm), wxDefaultPosition
        , wxSize(30, itemButton8->GetSize().GetY()));
    btnAttachment_->SetToolTip(_("Open attachments"));
    itemBoxSizer5->Add(btnAttachment_, g_flags);
    btnAttachment_->Enable(false);

    //Infobar-mini
    m_infoTextMini = new wxStaticText(bdPanel, wxID_STATIC, "");
    itemBoxSizer5->Add(m_infoTextMini, 1, wxGROW | wxTOP, 12);

    //Infobar
    m_infoText = new wxStaticText(bdPanel, wxID_ANY, ""
        , wxPoint(-1, -1), wxSize(200, -1), wxNO_BORDER | wxTE_MULTILINE | wxTE_WORDWRAP | wxST_NO_AUTORESIZE);
    itemBoxSizer4->Add(m_infoText, 1, wxGROW | wxLEFT | wxRIGHT, 14);

    mmBillsDepositsPanel::updateBottomPanelData(-1);
}

int mmBillsDepositsPanel::initVirtualListControl(int id)
{
    listCtrlAccount_->DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(listCtrlAccount_->m_asc ? 4 : 3);
    listCtrlAccount_->SetColumn(listCtrlAccount_->m_selected_col, item);

    bills_.clear();
    const auto split = Model_Budgetsplittransaction::instance().get_all();
    for (const Model_Billsdeposits::Data& data
        : Model_Billsdeposits::instance().all(Model_Billsdeposits::COL_NEXTOCCURRENCEDATE))
    {
        if (transFilterActive_ && !transFilterDlg_->checkAll(data, split))
            continue;

        Model_Billsdeposits::Full_Data r(data);

        if (Model_Attachment::NrAttachments(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT), r.BDID))
            r.NOTES = r.NOTES.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        bills_.push_back(r);
    }

    sortTable();

    int cnt = 0, selected_item = -1;
    for (const auto& entry: bills_)
    {
        if (id == entry.BDID)
        {
            selected_item = cnt;
            break;
        }
        ++cnt;
    }

    listCtrlAccount_->SetItemCount(static_cast<long>(bills_.size()));
    return selected_item;
}

void mmBillsDepositsPanel::OnNewBDSeries(wxCommandEvent& event)
{
  listCtrlAccount_->OnNewBDSeries(event);
}

void mmBillsDepositsPanel::OnEditBDSeries(wxCommandEvent& event)
{
    listCtrlAccount_->OnEditBDSeries(event);
}

void mmBillsDepositsPanel::OnDeleteBDSeries(wxCommandEvent& event)
{
    listCtrlAccount_->OnDeleteBDSeries(event);
}

void mmBillsDepositsPanel::OnEnterBDTransaction(wxCommandEvent& event)
{
    listCtrlAccount_->OnEnterBDTransaction(event);
}

void mmBillsDepositsPanel::OnSkipBDTransaction(wxCommandEvent& event)
{
    listCtrlAccount_->OnSkipBDTransaction(event);
    listCtrlAccount_->SetFocus();
}

void mmBillsDepositsPanel::OnOpenAttachment(wxCommandEvent& event)
{
    listCtrlAccount_->OnOpenAttachment(event);
    listCtrlAccount_->SetFocus();
}

/*******************************************************/
void billsDepositsListCtrl::OnItemResize(wxListEvent& event)
{
    int i = event.GetColumn();
    int width = event.GetItem().GetWidth();
    Model_Setting::instance().Set(wxString::Format("BD_COL%d_WIDTH", i), width);
}

void billsDepositsListCtrl::OnItemRightClick(wxMouseEvent& event)
{
    if (m_selected_row > -1)
        SetItemState(m_selected_row, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_bdp->updateBottomPanelData(m_selected_row);
    bool item_active = (m_selected_row >= 0);
    wxMenu menu;
    menu.Append(MENU_POPUP_BD_ENTER_OCCUR, _("Enter next Occurrence..."));
    menu.AppendSeparator();
    menu.Append(MENU_POPUP_BD_SKIP_OCCUR, _("Skip next Occurrence"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_NEW, _("&New Bills && Deposit Series..."));
    menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Bills && Deposit Series..."));
    menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Bills && Deposit Series..."));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _("&Organize Attachments"));
    
    menu.Enable(MENU_POPUP_BD_ENTER_OCCUR, item_active);
    menu.Enable(MENU_POPUP_BD_SKIP_OCCUR, item_active);
    menu.Enable(MENU_TREEPOPUP_EDIT, item_active);
    menu.Enable(MENU_TREEPOPUP_DELETE, item_active);
    menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, item_active);

    PopupMenu(&menu, event.GetPosition());
    this->SetFocus();
}

wxString mmBillsDepositsPanel::getItem(long item, long column)
{
    const Model_Billsdeposits::Full_Data& bill = this->bills_.at(item);
    switch (column)
    {
    case COL_ID:
        return wxString::Format("%i", bill.BDID).Trim();
    case COL_PAYMENT_DATE:
        return mmGetDateForDisplay(Model_Billsdeposits::NEXTOCCURRENCEDATE(bill));
    case COL_DUE_DATE:
        return mmGetDateForDisplay(Model_Billsdeposits::TRANSDATE(bill));
    case COL_ACCOUNT:
        return bill.ACCOUNTNAME;
    case COL_PAYEE:
        return bill.PAYEENAME;
    case COL_STATUS:
        return bill.STATUS;
    case COL_CATEGORY:
        return bill.CATEGNAME;
    case COL_TYPE:
        return wxGetTranslation(bill.TRANSCODE);
    case COL_AMOUNT:
        return Model_Account::toCurrency(bill.TRANSAMOUNT, Model_Account::instance().get(bill.ACCOUNTID));
    case COL_FREQUENCY:
        return GetFrequency(&bill);
    case COL_REPEATS:
        if (bill.NUMOCCURRENCES == -1)
            return L"\x221E";
        else
            return wxString::Format("%i", bill.NUMOCCURRENCES).Trim();
    case COL_AUTO:
    {
        int repeats = bill.REPEATS;
        wxString repeatSTR = _("Manual");
        if (repeats >= BD_REPEATS_MULTIPLEX_BASE)
        {
            repeats -= BD_REPEATS_MULTIPLEX_BASE;
            repeatSTR = _("Suggested");
            if (repeats >= BD_REPEATS_MULTIPLEX_BASE)
            {
                repeats -= BD_REPEATS_MULTIPLEX_BASE;
                repeatSTR = _("Automated");
            }
        }
        return repeatSTR;
    }
    case COL_DAYS:
        return GetRemainingDays(&bill);
    case COL_NUMBER:
        return bill.TRANSACTIONNUMBER;
    case COL_NOTES:
        return bill.NOTES;
    default:
        return wxEmptyString;
    }
}

wxString mmBillsDepositsPanel::GetFrequency(const Model_Billsdeposits::Data* item)
{
    int repeats = item->REPEATS;
    // DeMultiplex the Auto Executable fields.
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
        repeats -= BD_REPEATS_MULTIPLEX_BASE;

    wxString text = wxGetTranslation(BILLSDEPOSITS_REPEATS[repeats]);
    if (repeats > 10 && repeats < 15)
        text = wxString::Format(text, wxString::Format("%d", item->NUMOCCURRENCES));
    return text;
}

wxString mmBillsDepositsPanel::GetRemainingDays(const Model_Billsdeposits::Data* item)
{
    int repeats = item->REPEATS;
    // DeMultiplex the Auto Executable fields.
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
        repeats -= BD_REPEATS_MULTIPLEX_BASE;

    int daysRemaining = Model_Billsdeposits::daysPayment(item);
    int daysOverdue = Model_Billsdeposits::daysOverdue(item);
    wxString text = wxString::Format(_("%d days remaining"), daysRemaining);

    if (daysRemaining == 0)
    {
        if (((repeats > 10) && (repeats < 15)) && (item->NUMOCCURRENCES < 0))
            text = _("Inactive");
    }

    if (daysRemaining < 0)
    {
        text = wxString::Format(_("%d days delay!"), abs(daysRemaining));
        if (((repeats > 10) && (repeats < 15)) && (item->NUMOCCURRENCES < 0))
            text = _("Inactive");
    }

    if (daysOverdue < 0)
    {
        text = wxString::Format(_("%d days overdue!"), abs(daysOverdue));
        if (((repeats > 10) && (repeats < 15)) && (item->NUMOCCURRENCES < 0))
            text = _("Inactive");
    }

    return text;
}

wxString billsDepositsListCtrl::OnGetItemText(long item, long column) const
{
    return m_bdp->getItem(item, column);
}

void billsDepositsListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_bdp->updateBottomPanelData(m_selected_row);
}

void billsDepositsListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        m_bdp->updateBottomPanelData(m_selected_row);
    }
    event.Skip();
}

int billsDepositsListCtrl::OnGetItemImage(long item) const
{
    bool bd_repeat_user = false;
    bool bd_repeat_auto = false;
    int repeats = m_bdp->bills_[item].REPEATS;
    // DeMultiplex the Auto Executable fields.
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
    {
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
        bd_repeat_user = true;
    }
    if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
    {
        repeats -= BD_REPEATS_MULTIPLEX_BASE;
        bd_repeat_auto = true;
    }

    int daysRemaining = Model_Billsdeposits::daysPayment(&m_bdp->bills_[item]);
    wxString daysRemainingStr = wxString::Format(_("%d days remaining"), daysRemaining);

    if (daysRemaining == 0)
    {
        if (((repeats > 10) && (repeats < 15)) && (m_bdp->bills_[item].NUMOCCURRENCES < 0))
            daysRemainingStr = _("Inactive");
    }

    if (daysRemaining < 0)
    {
        daysRemainingStr = wxString::Format(_("%d days overdue!"), abs(daysRemaining));
        if (((repeats > 10) && (repeats < 15)) && (m_bdp->bills_[item].NUMOCCURRENCES < 0))
            daysRemainingStr = _("Inactive");
    }

    /* Returns the icon to be shown for each entry */
    if (daysRemainingStr == _("Inactive")) return -1;
    if (daysRemaining < 0) return 0;
    if (bd_repeat_auto) return 1;
    if (bd_repeat_user) return 2;

    return -1;
}

void billsDepositsListCtrl::OnListKeyDown(wxListEvent& event)
{
  switch ( event.GetKeyCode() )
    {
        case WXK_DELETE:
            {
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,
                    MENU_TREEPOPUP_DELETE);
                OnDeleteBDSeries(evt);
            }
            break;

        default:
            event.Skip();
    }
}

void billsDepositsListCtrl::OnNewBDSeries(wxCommandEvent& /*event*/)
{
    mmBDDialog dlg(this, 0, false, false);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(m_bdp->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnEditBDSeries(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    mmBDDialog dlg(this, m_bdp->bills_[m_selected_row].BDID, true, false);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(m_bdp->initVirtualListControl(dlg.GetTransID()));
}

void billsDepositsListCtrl::OnDeleteBDSeries(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;
    if (m_bdp->bills_.size() == 0) return;

    wxMessageDialog msgDlg(this, _("Do you really want to delete the series?")
        , _("Confirm Series Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        int BdId = m_bdp->bills_[m_selected_row].BDID;
        Model_Billsdeposits::instance().remove(BdId);
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT), BdId);
        m_bdp->initVirtualListControl();
        refreshVisualList(m_selected_row);
    }
}

void billsDepositsListCtrl::OnEnterBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int id = m_bdp->bills_[m_selected_row].BDID;
    mmBDDialog dlg(this, id, false, true);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(m_bdp->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnSkipBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int id = m_bdp->bills_[m_selected_row].BDID;
    Model_Billsdeposits::instance().completeBDInSeries(id);
    refreshVisualList(m_bdp->initVirtualListControl(id));
}

void billsDepositsListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    int RefId = m_bdp->bills_[m_selected_row].BDID;
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    refreshVisualList(m_bdp->initVirtualListControl(RefId));
}

void billsDepositsListCtrl::OnOpenAttachment(wxCommandEvent& event)
{
    if (m_selected_row == -1) return;
    int RefId = m_bdp->bills_[m_selected_row].BDID;
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    refreshVisualList(m_bdp->initVirtualListControl(RefId));
}

void billsDepositsListCtrl::OnListItemActivated(wxListEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    mmBDDialog dlg(this, m_bdp->bills_[m_selected_row].BDID, true, false);
    if ( dlg.ShowModal() == wxID_OK )
        refreshVisualList(m_bdp->initVirtualListControl(dlg.GetTransID()));
}

void mmBillsDepositsPanel::updateBottomPanelData(int selIndex)
{
    enableEditDeleteButtons(selIndex >= 0);
    if (selIndex != -1)
    {
        m_infoTextMini->SetLabelText(Model_Category::full_name(bills_[selIndex].CATEGID, bills_[selIndex].SUBCATEGID));
        m_infoText->SetLabelText(bills_[selIndex].NOTES);
    }
}

void mmBillsDepositsPanel::enableEditDeleteButtons(bool en)
{
    wxButton* bE = (wxButton*) FindWindow(wxID_EDIT);
    wxButton* bD = (wxButton*) FindWindow(wxID_DELETE);
    wxButton* bN = (wxButton*) FindWindow(wxID_PASTE);
    wxButton* bS = (wxButton*) FindWindow(wxID_IGNORE);
    wxButton* bA = (wxButton*)FindWindow(wxID_FILE);
    if (bE) bE->Enable(en);
    if (bD) bD->Enable(en);
    if (bN) bN->Enable(en);
    if (bS) bS->Enable(en);
    if (bA) bA->Enable(en);

    m_infoText->SetLabelText(this->tips());
    m_infoTextMini->ClearBackground();
}

void mmBillsDepositsPanel::sortTable()
{
    std::sort(bills_.begin(), bills_.end());
    switch (listCtrlAccount_->m_selected_col)
    {
    case COL_ID:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByBDID());
        break;
    case COL_PAYMENT_DATE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByNEXTOCCURRENCEDATE());
        break;
    case COL_DUE_DATE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSDATE());
        break;
    case COL_ACCOUNT:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByACCOUNTNAME());
        break;
    case COL_PAYEE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByPAYEENAME());
        break;
    case COL_STATUS:
        std::stable_sort(bills_.begin(), bills_.end(), SorterBySTATUS());
        break;
    case COL_CATEGORY:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByCATEGNAME());
        break;
    case COL_TYPE:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSCODE());
        break;
    case COL_AMOUNT:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByTRANSAMOUNT());
        break;
    case COL_FREQUENCY:
        std::stable_sort(bills_.begin(), bills_.end()
            , [&](const Model_Billsdeposits::Full_Data& x, const Model_Billsdeposits::Full_Data& y)
        {
            wxString x_text = this->GetFrequency(&x);
            wxString y_text = this->GetFrequency(&y);
            return x_text < y_text;
        });
        break;
    case COL_REPEATS:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByREPEATS());
        break;
    case COL_DAYS:
        std::stable_sort(bills_.begin(), bills_.end()
            , [&](const Model_Billsdeposits::Data& x, const Model_Billsdeposits::Data& y)
        {
            bool x_useText = false;
            wxString x_text = this->GetRemainingDays(&x);
            long x_num = 0;
            if (isdigit(x_text[0]))
            {
                x_num = wxAtoi(x_text);
                wxString removed;
                if (x_text.EndsWith(wxString("!"), &removed))
                    x_num = -x_num;
            }
            else
                x_useText = true;

            bool y_useText = false;
            wxString y_text = this->GetRemainingDays(&y);
            long y_num = 0;
            if (isdigit(y_text[0]))
            {
                y_num = wxAtoi(y_text);
                wxString removed;
                if (y_text.EndsWith(wxString("!"), &removed))
                    y_num = -y_num;
            }
            else
                y_useText = true;
            return ((!x_useText && !y_useText) ? x_num < y_num : x_text < y_text);
        });
        break;
    case COL_NOTES:
        std::stable_sort(bills_.begin(), bills_.end(), SorterByNOTES());
        break;
    default:
        break;
    }
    if (!listCtrlAccount_->m_asc) std::reverse(bills_.begin(), bills_.end());
}

wxString mmBillsDepositsPanel::tips()
{
    return this->tips_[rand() % this->tips_.GetCount()];
}

void billsDepositsListCtrl::refreshVisualList(int selected_index)
{

    if (selected_index >= (long)m_bdp->bills_.size() || selected_index < 0)
        selected_index = - 1;
    if (!m_bdp->bills_.empty()) {
        RefreshItems(0, m_bdp->bills_.size() - 1);
    }
    else
        selected_index = -1;

    if (selected_index >= 0 && !m_bdp->bills_.empty())
    {
        SetItemState(selected_index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selected_index, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selected_index);
    }
    m_selected_row = selected_index;
    m_bdp->updateBottomPanelData(selected_index);
}

void billsDepositsListCtrl::RefreshList()
{
    if (m_bdp->bills_.size() == 0) return;
    int id = -1;
    if (m_selected_row != -1)
    {
        id = m_bdp->bills_[m_selected_row].BDID;
    }
    refreshVisualList(m_bdp->initVirtualListControl(id));
}

void mmBillsDepositsPanel::RefreshList()
{
    listCtrlAccount_->RefreshList();
}

void mmBillsDepositsPanel::OnFilterTransactions(wxMouseEvent& event)
{

    int e = event.GetEventType();

    wxBitmap bitmapFilterIcon(rightarrow_xpm);

    if (e == wxEVT_LEFT_DOWN)
    {

        if (transFilterDlg_->ShowModal() == wxID_OK && transFilterDlg_->somethingSelected())
        {
            transFilterActive_ = true;
            wxBitmap activeBitmapFilterIcon(tipicon_xpm);
            bitmapFilterIcon = activeBitmapFilterIcon;
        }
        else
        {
            transFilterActive_ = false;
        }

    } else {
        if (transFilterActive_ == false) return;
        transFilterActive_ = false;
    }

    wxImage pic = bitmapFilterIcon.ConvertToImage();
    bitmapTransFilter_->SetBitmap(pic);

    initVirtualListControl();
}
