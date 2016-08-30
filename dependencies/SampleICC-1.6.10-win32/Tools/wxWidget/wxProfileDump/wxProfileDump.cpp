/** @file
File:       wxProfileDump.cpp

Contains:   Main application that displays profile contents using wxWidgets

Version:    V1

Copyright:  � see ICC Software License
*/

/*
* The ICC Software License, Version 0.2
*
*
* Copyright (c) 2003-2015 The International Color Consortium. All rights 
* reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer. 
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*
* 3. In the absence of prior written permission, the names "ICC" and "The
*    International Color Consortium" must not be used to imply that the
*    ICC organization endorses or promotes products derived from this
*    software.
*
*
* THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED.  IN NO EVENT SHALL THE INTERNATIONAL COLOR CONSORTIUM OR
* ITS CONTRIBUTING MEMBERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
* USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
* ====================================================================
*
* This software consists of voluntary contributions made by many
* individuals on behalf of the The International Color Consortium. 
*
*
* Membership in the ICC is encouraged when this software is used for
* commercial purposes. 
*
*  
* For more information on The International Color Consortium, please
* see <http://www.color.org/>.
*  
* 
*/

// ===========================================================================
// declarations
// ===========================================================================

// ---------------------------------------------------------------------------
// headers
// ---------------------------------------------------------------------------
#include "IccProfile.h"
#include "IccTag.h"
#include "IccUtil.h"
#include "IccEval.h"
#include "IccPrmg.h"
#include "IccProfLibVer.h"
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
    #include "wx/mdi.h"
#endif

#include "wx/toolbar.h"
#include "wx/config.h"
#include "wx/filename.h"
#include "wx/filedlg.h"

#if !defined(__WXMSW__)
//    #include "mondrian.xpm"
#endif

#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/copy.xpm"
#include "bitmaps/cut.xpm"
#include "bitmaps/paste.xpm"
#include "bitmaps/print.xpm"
#include "bitmaps/help.xpm"


#include "wxProfileDump.h"

IMPLEMENT_APP(MyApp)

// ---------------------------------------------------------------------------
// global variables
// ---------------------------------------------------------------------------

MyFrame *frame = (MyFrame *) NULL;
wxList my_children;

// For drawing lines in a canvas
static long xpos = -1;
static long ypos = -1;

static int gs_nFrames = 0;

// ---------------------------------------------------------------------------
// event tables
// ---------------------------------------------------------------------------

#define ID_VALIDATE_PROFILE 1000
#define ID_TAG_LIST         1001
#define ID_ROUND_TRIP       1002

BEGIN_EVENT_TABLE(MyFrame, wxMDIParentFrame)
    EVT_MENU(MDI_ABOUT, MyFrame::OnAbout)
    EVT_MENU(MDI_OPEN_PROFILE, MyFrame::OnOpenProfile)
    EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, MyFrame::OnOpenProfile)
    EVT_MENU(MDI_QUIT, MyFrame::OnQuit)

    EVT_CLOSE(MyFrame::OnClose)
    EVT_SIZE(MyFrame::OnSize)
END_EVENT_TABLE()

// Note that MDI_NEW_WINDOW and MDI_ABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.
BEGIN_EVENT_TABLE(MyChild, wxMDIChildFrame)
    EVT_MENU(MDI_CHILD_QUIT, MyChild::OnQuit)
		EVT_CLOSE(MyChild::OnClose)
		EVT_BUTTON(ID_VALIDATE_PROFILE, MyChild::OnValidate)
    EVT_BUTTON(ID_ROUND_TRIP, MyChild::OnRoundTrip)
		EVT_LIST_ITEM_ACTIVATED(ID_TAG_LIST, MyChild::OnTagClicked) 
END_EVENT_TABLE()


// ===========================================================================
// implementation
// ===========================================================================

// ---------------------------------------------------------------------------
// MyApp
// ---------------------------------------------------------------------------

// Initialise this in OnInit, not statically
bool MyApp::OnInit()
{
	// Create the main frame window
    frame = new MyFrame((wxFrame *)NULL, wxID_ANY, _T("ProfileDump"),
                        wxDefaultPosition, wxSize(1024, 768),
                        wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL);

    // using wxConfig instead of writing wxFileConfig or wxRegConfig enhances
    // portability of the code
    wxConfig config(wxT("wxProfileDump"));

    m_history.Load(config);

#ifdef __WXMSW__
#if 0
    // Experimental: change the window menu
    wxMenu* windowMenu = new wxMenu;
    windowMenu->Append(5000, _T("My menu item!"));
    frame->SetWindowMenu(windowMenu);
#endif
#endif

    // Give it an icon
#ifdef __WXMSW__
    frame->SetIcon(wxIcon(_T("wxProfileDump_icn")));
#else
    //frame->SetIcon(wxIcon( wxProfileDump_xpm ));
#endif

    // Make a menubar
    wxMenu *file_menu = new wxMenu;

    file_menu->Append(MDI_OPEN_PROFILE, _T("&Open Profile\tCtrl-O"), _T("Open an ICC profile"));
    file_menu->Append(MDI_QUIT, _T("&Exit\tAlt-X"), _T("Quit the program"));

    m_history.UseMenu(file_menu);
    m_history.AddFilesToMenu(file_menu);

    wxMenu *help_menu = new wxMenu;
    help_menu->Append(MDI_ABOUT, _T("&About\tF1"));

    wxMenuBar *menu_bar = new wxMenuBar;

    menu_bar->Append(file_menu, _T("&File"));
    menu_bar->Append(help_menu, _T("&Help"));

    // Associate the menu bar with the frame
    frame->SetMenuBar(menu_bar);

#if wxUSE_STATUSBAR
    frame->CreateStatusBar();
#endif // wxUSE_STATUSBAR

    frame->Show(true);
    frame->SetDropTarget(new MyDnDFile(frame));

    SetTopWindow(frame);

    if (argc>1) {
      frame->OpenFile(argv[1]);
    }

    return true;
}

int MyApp::OnExit()
{
  wxConfig config;

  m_history.Save(config);

  return 0;
}

// ---------------------------------------------------------------------------
// MyFrame
// ---------------------------------------------------------------------------

// Define my frame constructor
MyFrame::MyFrame(wxWindow *parent,
                 const wxWindowID id,
                 const wxString& title,
                 const wxPoint& pos,
                 const wxSize& size,
                 const long style)
       : wxMDIParentFrame(parent, id, title, pos, size,
                          style | wxNO_FULL_REPAINT_ON_RESIZE)
{
#if wxUSE_TOOLBAR
    CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL);
    InitToolBar(GetToolBar());
#endif // wxUSE_TOOLBAR

    // Accelerators
    wxAcceleratorEntry entries[3];
    entries[0].Set(wxACCEL_CTRL, (int) 'O', MDI_OPEN_PROFILE);
    entries[1].Set(wxACCEL_CTRL, (int) 'X', MDI_QUIT);
    entries[2].Set(wxACCEL_CTRL, (int) 'A', MDI_ABOUT);
    wxAcceleratorTable accel(3, entries);
    SetAcceleratorTable(accel);
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    if ( event.CanVeto() && (gs_nFrames > 0) )
    {
        wxString msg;
        msg.Printf(_T("%d windows still open, close anyhow?"), gs_nFrames);
        if ( wxMessageBox(msg, _T("Please confirm"),
                          wxICON_QUESTION | wxYES_NO) != wxYES )
        {
            event.Veto();

            return;
        }
    }

    event.Skip();
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
    (void)wxMessageBox(_T("wxProfileDump\n")
                       _T("Copyright (C) 2005-2015\n\n")
                       _T("Using ICCProflib version ") ICCPROFLIBVER _T("\n"),
											 _T("About wxProfileDump"));
}

void MyFrame::OpenFile(wxString profilePath)
{
  wxFileName filepath(profilePath);
  wxString profileTitle = filepath.GetName();

  CIccProfile *pIcc = OpenIccProfile((icChar*)profilePath.wx_str());

  if (!pIcc) {
    (void)wxMessageBox(wxString(_T("Unable to open profile '")) + profilePath + _T("'"),
                       _T("Open Error!"));
    return;
  }

  wxGetApp().m_history.AddFileToHistory(profilePath);

  // Make another frame, containing a canvas
  MyChild *subframe = new MyChild(frame, profileTitle, pIcc, profilePath);

  subframe->SetTitle(profileTitle);

  // Give it an icon
#ifdef __WXMSW__
  subframe->SetIcon(wxIcon(_T("ProfileDumpDoc_icn")));
#else
//  subframe->SetIcon(wxIcon( mondrian_xpm ));
#endif

  // Make a menubar
  wxMenu *file_menu = new wxMenu;
  subframe->SetFileMenu(file_menu);

  file_menu->Append(MDI_OPEN_PROFILE, _T("&Open Profile"));
  file_menu->Append(MDI_CHILD_QUIT, _T("&Close"), _T("Close this window"));
  file_menu->Append(MDI_QUIT, _T("&Exit"));

  wxGetApp().m_history.UseMenu(file_menu);
  wxGetApp().m_history.AddFilesToMenu(file_menu);

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(MDI_ABOUT, _T("&About"));

  wxMenuBar *menu_bar = new wxMenuBar;

  menu_bar->Append(file_menu, _T("&File"));
  menu_bar->Append(help_menu, _T("&Help"));

  // Associate the menu bar with the frame
  subframe->SetMenuBar(menu_bar);
  subframe->Show(true);
}

void MyFrame::OnOpenProfile(wxCommandEvent& event )
{
  wxString profilePath;

  if (event.GetId()==MDI_OPEN_PROFILE) {
	  wxFileDialog dialog(this, _T("Open Profile"), wxEmptyString, wxEmptyString, _T("ICC files (*.icc)|*.icc|ICM files (*.icm)|*.icm|All files|*.*"), wxOPEN |wxFILE_MUST_EXIST);

	  if (dialog.ShowModal()!=wxID_OK)
		  return;
	  
		profilePath = dialog.GetPath();
  }
  else {
    profilePath = wxGetApp().m_history.GetHistoryFile(event.GetId() - wxID_FILE1);
  }

  OpenFile(profilePath);
}


#if wxUSE_TOOLBAR
void MyFrame::InitToolBar(wxToolBar* toolBar)
{
    wxBitmap* bitmaps[6];

    int index=0;
    bitmaps[index++] = new wxBitmap( open_xpm );
    //bitmaps[index++] = new wxBitmap( save_xpm );
    //bitmaps[index++] = new wxBitmap( copy_xpm );
    //bitmaps[index++] = new wxBitmap( cut_xpm );
    //bitmaps[index++] = new wxBitmap( paste_xpm );
    bitmaps[index++] = new wxBitmap( help_xpm );

    int width = 24;
    int currentX = 5;

    index = 0;
    toolBar->AddTool( MDI_OPEN_PROFILE, *(bitmaps[index]), wxNullBitmap, false, currentX, wxDefaultCoord, (wxObject *) NULL, _T("Open Profile"));
    currentX += width + 5;

    //index++;
    //toolBar->AddTool(index+1, *bitmaps[index], wxNullBitmap, false, currentX, wxDefaultCoord, (wxObject *) NULL, _T("Save Profile"));
    //currentX += width + 5;

    //toolBar->AddSeparator();

    //index++;
    //toolBar->AddTool(index+1, *bitmaps[index], wxNullBitmap, false, currentX, wxDefaultCoord, (wxObject *) NULL, _T("Copy"));
    //currentX += width + 5;

    //index++;
    //toolBar->AddTool(index+1, *bitmaps[index], wxNullBitmap, false, currentX, wxDefaultCoord, (wxObject *) NULL, _T("Cut"));
    //currentX += width + 5;

    //index++;
    //toolBar->AddTool(index+1, *bitmaps[index], wxNullBitmap, false, currentX, wxDefaultCoord, (wxObject *) NULL, _T("Paste"));
    //currentX += width + 5;

    toolBar->AddSeparator();

    index++;
    toolBar->AddTool( MDI_ABOUT, *bitmaps[index], wxNullBitmap, true, currentX, wxDefaultCoord, (wxObject *) NULL, _T("Help"));

    toolBar->Realize();

    int i;
    for (i = 0; i <= index; i++)
        delete bitmaps[i];
}
#endif // wxUSE_TOOLBAR

static bool IsRoundTripable(CIccProfile *pIcc)
{
  icHeader *pHdr = &pIcc->m_Header;

  if (pHdr->deviceClass == icSigLinkClass) {
    return false;
  }

  if ((pIcc->FindTag(icSigAToB0Tag) && pIcc->FindTag(icSigBToA0Tag)) ||
      (pIcc->FindTag(icSigAToB1Tag) && pIcc->FindTag(icSigBToA1Tag)) ||
      (pIcc->FindTag(icSigAToB2Tag) && pIcc->FindTag(icSigBToA2Tag)) ||
      (pIcc->FindTag(icSigDToB0Tag) && pIcc->FindTag(icSigBToD0Tag)) ||
      (pIcc->FindTag(icSigDToB1Tag) && pIcc->FindTag(icSigBToD1Tag)) ||
      (pIcc->FindTag(icSigDToB2Tag) && pIcc->FindTag(icSigBToD2Tag)) ||
      (pIcc->FindTag(icSigRedMatrixColumnTag) && pIcc->FindTag(icSigGreenMatrixColumnTag) && pIcc->FindTag(icSigBlueMatrixColumnTag) &&
       pIcc->FindTag(icSigRedTRCTag) && pIcc->FindTag(icSigGreenTRCTag) && pIcc->FindTag(icSigBlueTRCTag)))
     return true;

  return false;
}

// ---------------------------------------------------------------------------
// MyChild
// ---------------------------------------------------------------------------

MyChild::MyChild(wxMDIParentFrame *parent, const wxString& title, CIccProfile *pIcc, const wxString &profilePath)
       : wxMDIChildFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
                         wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
	  m_pIcc = pIcc;
		m_profilePath = profilePath;

    my_children.Append(this);
		// this should work for MDI frames as well as for normal ones
		SetSizeHints(100, 200);

		// create controls
		m_panel = new wxPanel(this, wxID_ANY,
			wxDefaultPosition, wxDefaultSize, wxCLIP_CHILDREN);

		wxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);

		wxSizer *sizerBox = new wxStaticBoxSizer(new wxStaticBox(m_panel, wxID_ANY, _T("&ProfileHeader")), wxVERTICAL);

		sizerBox->Add(CreateSizerWithText(_("Size:"), &m_textSize), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Suggested CMM:"), &m_textCMM), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Version:"), &m_textVersion), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Type of Profile:"), &m_textType), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Data Color Space:"), &m_textColorSpace), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("PCS Color Space:"), &m_textPCS), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Creation Date:"), &m_textCreationDate), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Primary Platform:"), &m_textPlatform), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Flags:"), &m_textFlags), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Attributes:"), &m_textAttribute), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Rendering Intent:"), &m_textRenderingIntent), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Illuminant:"), &m_textIlluminant), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Creator:"), &m_textCreator), wxSizerFlags().Expand().Border(wxALL, 0));
		sizerBox->Add(CreateSizerWithText(_("Profile ID:"), &m_textProfileID), wxSizerFlags().Expand().Border(wxALL, 0));

		sizerTop->Add(sizerBox, wxSizerFlags().Expand().Border(wxALL, 5));

		wxSizer *sizerBtn = new wxBoxSizer(wxHORIZONTAL);
    if (IsRoundTripable(pIcc)) {
  		sizerBtn->Add(new wxButton(m_panel, ID_ROUND_TRIP, _("&Round Trip Report")), wxSizerFlags().Border(wxRIGHT, 5));
    }
    sizerBtn->Add(new wxButton(m_panel, ID_VALIDATE_PROFILE, _("&Validate Profile")), wxSizerFlags().Border(wxRIGHT, 5));

		sizerTop->Add(sizerBtn, wxSizerFlags().Right());

		wxSizer *sizerLabel = new wxBoxSizer(wxHORIZONTAL);
		sizerLabel->Add(new wxStaticText(m_panel, wxID_ANY, _("Profile Tags")), wxSizerFlags().Border(wxLEFT, 5));

		sizerTop->Add(sizerLabel, wxSizerFlags().Left());

		m_tagsCtrl = new wxListCtrl(m_panel, ID_TAG_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
		wxSizer *sizerTags = new wxBoxSizer(wxVERTICAL);
		sizerTags->Add(m_tagsCtrl, wxSizerFlags(1).Expand().Border(wxALL, 0));
		sizerTags->SetItemMinSize((size_t)0, 455, 175);
		sizerTop->Add(sizerTags, wxSizerFlags(1).Expand().Border(wxALL, 5));

		m_tagsCtrl->InsertColumn(0, _("Tag ID"), wxLIST_FORMAT_LEFT, 160);
		m_tagsCtrl->InsertColumn(1, _("Tag Type"), wxLIST_FORMAT_LEFT, 160);
		m_tagsCtrl->InsertColumn(2, _("Offset"), wxLIST_FORMAT_RIGHT, 60);
		m_tagsCtrl->InsertColumn(3, _("Size"), wxLIST_FORMAT_RIGHT, 60);

		// don't allow frame to get smaller than what the sizers tell it and also set
		// the initial size as calculated by the sizers
		sizerTop->SetSizeHints( this );

		m_panel->SetSizer(sizerTop);

		icHeader *pHdr = &pIcc->m_Header;
		CIccInfo Fmt;
		char buf[64];
		int n;
    wxString str;

		m_textAttribute->SetLabel(wxT(Fmt.GetDeviceAttrName(pHdr->attributes)));
    m_textCMM->SetLabel(wxT(Fmt.GetCmmSigName((icCmmSignature)pHdr->cmmId)));
		m_textCreationDate->SetLabel(wxString::Format(wxT("%d/%d/%d  %02u:%02u:%02u"),
			pHdr->date.month, pHdr->date.day, pHdr->date.year,
			pHdr->date.hours, pHdr->date.minutes, pHdr->date.seconds));
		m_textCreator->SetLabel(icGetSig(buf, pHdr->creator));
		m_textColorSpace->SetLabel(Fmt.GetColorSpaceSigName(pHdr->colorSpace));
		m_textFlags->SetLabel(Fmt.GetProfileFlagsName(pHdr->flags));
		m_textPCS->SetLabel(Fmt.GetColorSpaceSigName(pHdr->pcs));
		m_textPlatform->SetLabel(Fmt.GetPlatformSigName(pHdr->platform));
		m_textRenderingIntent->SetLabel(Fmt.GetRenderingIntentName((icRenderingIntent)(pHdr->renderingIntent)));
		m_textSize->SetLabel(wxString::Format(_T("%d(0x%x) bytes"), pHdr->size, pHdr->size));
		m_textType->SetLabel(Fmt.GetProfileClassSigName(pHdr->deviceClass));
		m_textVersion->SetLabel(Fmt.GetVersionName(pHdr->version));
		m_textIlluminant->SetLabel(wxString::Format(_T("X=%.4lf, Y=%.4lf, Z=%.4lf"),
			icFtoD(pHdr->illuminant.X),
			icFtoD(pHdr->illuminant.Y),
			icFtoD(pHdr->illuminant.Z)));
		str.Empty();
		for (n=0; n<16; n++) {
			sprintf(buf, "%02x", pHdr->profileID.ID8[n]);
			if (n && !(n%4))
				str += " ";
			str += buf;
		}
		m_textProfileID->SetLabel(str);

		int item;
		TagEntryList::iterator i;

		for (n=0, i=pIcc->m_Tags->begin(); i!=pIcc->m_Tags->end(); i++, n++) {
			item = m_tagsCtrl->InsertItem(n, Fmt.GetTagSigName(i->TagInfo.sig));
      CIccTag *pTag = pIcc->FindTag(i->TagInfo.sig);
      if (!pTag)
        m_tagsCtrl->SetItem(item, 1, _T("***Invalid Tag!***"));
      else
			  m_tagsCtrl->SetItem(item, 1, Fmt.GetTagTypeSigName(pTag->GetType()));

			m_tagsCtrl->SetItem(item, 2, wxString::Format("%d", i->TagInfo.offset));

			m_tagsCtrl->SetItem(item, 3, wxString::Format("%d", i->TagInfo.size));

			m_tagsCtrl->SetItemData(item, (long)i->TagInfo.sig);
		}
		m_panel->Layout();
}

MyChild::~MyChild()
{
    my_children.DeleteObject(this);
		if (m_pIcc)
			delete m_pIcc;
}

wxSizer *MyChild::CreateSizerWithText(const wxString &labelText, wxStaticText **ppText)
{
	wxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);

	wxSize winSize = wxDefaultSize;

	winSize.SetWidth(100);
	wxStaticText *label = new wxStaticText(m_panel, wxID_ANY, labelText, wxDefaultPosition, winSize, wxALIGN_RIGHT);

	winSize.SetWidth(250);
	wxStaticText *text = new wxStaticText(m_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, winSize, wxSTATIC_BORDER|wxTE_RIGHT);

	sizerRow->Add(label, 0, wxRIGHT | wxALIGN_CENTRE_VERTICAL, 5);
	sizerRow->Add(text, 1, wxLEFT | wxALIGN_CENTRE_VERTICAL, 5);

	if ( ppText )
		*ppText = text;

	return sizerRow;
}

void MyChild::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyChild::SetFileMenu(wxMenu *file_menu)
{
  m_fileMenu = file_menu;
}

void MyChild::OnClose(wxCloseEvent& event)
{
  wxGetApp().m_history.RemoveMenu(m_fileMenu);
	gs_nFrames--;

	event.Skip();
}

void MyChild::OnValidate(wxCommandEvent& WXUNUSED(event))
{
	MyDialog dialog(this, _T("Profile Validation Report"), m_profilePath);

	dialog.ShowModal();
}

void MyChild::OnRoundTrip(wxCommandEvent& WXUNUSED(event))
{
  MyRoundTripDialog dialog(this, _T("Round Trip Report"), m_profilePath, m_pIcc);

  dialog.ShowModal();
}

void MyChild::OnTagClicked(wxListEvent& event)
{
  icTagSignature tagSig = (icTagSignature)event.GetData();
	CIccTag *pTag = m_pIcc->FindTag(tagSig);

  MyTagDialog dialog(this, m_pIcc, tagSig, pTag);

  dialog.ShowModal();
}

MyDialog::MyDialog(wxWindow *pParent, const wxString& title, wxString &profilePath) : 
  wxDialog(pParent, wxID_ANY, title,wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  wxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);

  wxSize winSize = wxDefaultSize;

  winSize.SetWidth(100);
  wxStaticText *labelSttus = new wxStaticText(this, wxID_ANY, _T("Validation Status:"), wxDefaultPosition, winSize, wxALIGN_RIGHT);

  winSize.SetWidth(400);
  wxStaticText *textStatus = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, winSize, wxSTATIC_BORDER|wxTE_RIGHT);

  sizerRow->Add(labelSttus, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 5);
  sizerRow->Add(textStatus, 1, wxALL | wxALIGN_CENTRE_VERTICAL, 5);

  sizer->Add(sizerRow, wxSizerFlags().Expand());

  winSize = wxSize(500, 400);
  wxTextCtrl *textReport = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, winSize,
	                                       wxTE_MULTILINE |wxTE_READONLY | wxTE_RICH);

  sizer->Add(textReport, wxSizerFlags(1).Expand()); 

  icValidateStatus nStat;
  wxString theReport, theValidateStatus;

	if (profilePath.IsEmpty()) {
		theReport = "Invalid Profile Path\r\n";
		nStat = (icValidateStatus)-1;
	}
	else {
		std::string sReport;
		wxBeginBusyCursor();
		CIccProfile *pIcc = ValidateIccProfile(profilePath, sReport, nStat);
		if (pIcc)
			delete pIcc;
		wxEndBusyCursor();

		if (sReport.empty())
			sReport = "There is nothing to report\r\n";

		theReport = sReport.c_str();
	}

	switch(nStat) {
		case icValidateOK:
			theValidateStatus = "Valid Profile";
			break;

		case icValidateWarning:
			theValidateStatus = "Validation Warning(s)";
			break;

		case icValidateNonCompliant:
			theValidateStatus = "Profile violates ICC Specification";
			break;

		case icValidateCriticalError:
			theValidateStatus = "Critical Error - Profile Violates ICC Specification";
			break;

		default:
			theValidateStatus = "Unknown Validation Status";
			break;
	}

	textStatus->SetLabel(theValidateStatus);
	textReport->SetLabel(theReport);

  SetSizer(sizer);
	sizer->Fit(this);
}

class CIccMinMaxEval : public CIccEvalCompare
{
public:
  CIccMinMaxEval();

  void Compare(icFloatNumber *pixel, icFloatNumber *deviceLab, icFloatNumber *lab1, icFloatNumber *lab2);

  icFloatNumber GetMean1() { return sum1 / num1; }
  icFloatNumber GetMean2() { return sum2 / num2; }

  icFloatNumber minDE1, minDE2;
  icFloatNumber maxDE1, maxDE2;

  icFloatNumber maxLab1[3], maxLab2[3];

protected:

  icFloatNumber sum1, sum2;
  icFloatNumber num1, num2;
};

CIccMinMaxEval::CIccMinMaxEval()
{
  minDE1 = minDE2 = 10000;
  maxDE1 = maxDE2 = -1;
  sum1 = sum2 = 0;
  num1 = num2 = 0;

  memset(&maxLab1[0], 0, sizeof(maxLab1));
  memset(&maxLab2[0], 0, sizeof(maxLab2));
}

void CIccMinMaxEval::Compare(icFloatNumber *pixel, icFloatNumber *deviceLab, icFloatNumber *lab1, icFloatNumber *lab2)
{
  icFloatNumber DE1 = icDeltaE(deviceLab, lab1);
  icFloatNumber DE2 = icDeltaE(lab1, lab2);

  if (DE1<minDE1) {
    minDE1 = DE1;
  }

  if (DE1>maxDE1) {
    maxDE1 = DE1;
    memcpy(&maxLab1[0], deviceLab, sizeof(maxLab1));
  }

  if (DE2<minDE2) {
    minDE2 = DE2;
  }

  if (DE2>maxDE2) {
    maxDE2 = DE2;
    memcpy(&maxLab2[0], deviceLab, sizeof(maxLab2));
  }

  sum1 += DE1;
  num1 += 1.0;

  sum2 += DE2;
  num2 += 1.0;
}

wxString AnalyzeRoundTrip(wxString &profilePath, icRenderingIntent nIntent, bool bUseMPE)
{
  CIccInfo info;
  wxString report;
  CIccMinMaxEval eval;

  if (bUseMPE)
    report += wxString::Format("MPE Rendering Intent: %s\n", info.GetRenderingIntentName(nIntent));
  else
    report += wxString::Format("Rendering Intent: %s\n", info.GetRenderingIntentName(nIntent));

  icStatusCMM stat = eval.EvaluateProfile(profilePath, 0, nIntent, icInterpTetrahedral, bUseMPE);

  if (stat!=icCmmStatOk) {
    report += wxString::Format("  Unable to perform round trip on '%s'\n", profilePath.c_str());
    report += "\n";

    return report;
  }

  CIccPRMG prmg;

  stat = prmg.EvaluateProfile(profilePath, nIntent, icInterpTetrahedral, bUseMPE);

  if (stat!=icCmmStatOk) {
    report += wxString::Format("  Unable to perform PRMG analysis on '%s'\n", profilePath.c_str());
    report += "\n";

    return report;
  }

  if (nIntent!=icRelativeColorimetric)
    report += wxString::Format("Specified Rendering Intent Gamut:  %s\n", prmg.m_bPrmgImplied ? "Perceptual Reference Medium Gamut" : "Not Specified");

  report += wxString::Format("\n   Round Trip 1\n");
  report += wxString::Format(  "  ------------\n");
  report += wxString::Format("   Min DeltaE:    %8.2" ICFLOATSFX "\n", eval.minDE1);
  report += wxString::Format("   Mean DeltaE:   %8.2" ICFLOATSFX "\n", eval.GetMean1());
  report += wxString::Format("   Max DeltaE:    %8.2" ICFLOATSFX "\n\n", eval.maxDE1);

  report += wxString::Format("   Max L, a, b:   " ICFLOATFMT ", " ICFLOATFMT ", " ICFLOATFMT "\n", eval.maxLab1[0], eval.maxLab1[1], eval.maxLab1[2]);

  report += wxString::Format("\n   Round Trip 2\n");
  report += wxString::Format(  "   ------------\n");
  report += wxString::Format("   Min DeltaE:    %8.2" ICFLOATSFX "\n", eval.minDE2);
  report += wxString::Format("   Mean DeltaE:   %8.2" ICFLOATSFX "\n", eval.GetMean2());
  report += wxString::Format("   Max DeltaE:    %8.2" ICFLOATSFX "\n\n", eval.maxDE2);

  report += wxString::Format("   Max L, a, b:   " ICFLOATFMT ", " ICFLOATFMT ", " ICFLOATFMT "\n", eval.maxLab2[0], eval.maxLab2[1], eval.maxLab2[2]);

  if (prmg.m_nTotal) {
    report += wxString::Format("\n   PRMG Interoperability - Round Trip Results\n");
    report += wxString::Format(  "   ------------------------------------------------------\n");

    report += wxString::Format("   DE <= 1.0 (%8u): %5.1f%%\n", prmg.m_nDE1, (float)prmg.m_nDE1/(float)prmg.m_nTotal*100.0); 
    report += wxString::Format("   DE <= 2.0 (%8u): %5.1f%%\n", prmg.m_nDE2, (float)prmg.m_nDE2/(float)prmg.m_nTotal*100.0);
    report += wxString::Format("   DE <= 3.0 (%8u): %5.1f%%\n", prmg.m_nDE3, (float)prmg.m_nDE3/(float)prmg.m_nTotal*100.0);
    report += wxString::Format("   DE <= 5.0 (%8u): %5.1f%%\n", prmg.m_nDE5, (float)prmg.m_nDE5/(float)prmg.m_nTotal*100.0);
    report += wxString::Format("   DE <=10.0 (%8u): %5.1f%%\n", prmg.m_nDE10, (float)prmg.m_nDE10/(float)prmg.m_nTotal*100.0);
    report += wxString::Format("   Total     (%8u)\n", prmg.m_nTotal);
  }

  report += "\n";

  return report;
}

MyRoundTripDialog::MyRoundTripDialog(wxWindow *pParent, const wxString& title, wxString &profilePath, CIccProfile *pIcc) : 
  wxDialog(pParent, wxID_ANY, title,wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  wxSize winSize = winSize = wxSize(500, 400);
  wxTextCtrl *textReport = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, winSize,
    wxTE_MULTILINE |wxTE_READONLY | wxTE_RICH);

  sizer->Add(textReport, wxSizerFlags(1).Expand()); 

  wxString theReport;

  if (profilePath.IsEmpty()) {
    theReport = "Invalid Profile Path\r\n";
  }
  else {
    bool bRelative = false;

    wxBeginBusyCursor();

    //Check colorimetric rendering intents
    if (pIcc->FindTag(icSigAToB1Tag) && pIcc->FindTag(icSigBToA1Tag)) {
      theReport += AnalyzeRoundTrip(profilePath, icRelativeColorimetric, false);
      bRelative = true;
    }
    if (pIcc->FindTag(icSigDToB1Tag) && pIcc->FindTag(icSigBToD1Tag)) {
      theReport += AnalyzeRoundTrip(profilePath, icRelativeColorimetric, true);
      bRelative = true;
    }
    if (!bRelative &&
        (pIcc->FindTag(icSigRedMatrixColumnTag) && pIcc->FindTag(icSigGreenMatrixColumnTag) && pIcc->FindTag(icSigBlueMatrixColumnTag) &&
        pIcc->FindTag(icSigRedTRCTag) && pIcc->FindTag(icSigGreenTRCTag) && pIcc->FindTag(icSigBlueTRCTag))) {
      theReport += AnalyzeRoundTrip(profilePath, icRelativeColorimetric, false);
      bRelative = true;
    }

    //Check MPE absolute intent
    if (pIcc->FindTag(icSigDToB3Tag) && pIcc->FindTag(icSigBToD3Tag)) {
      theReport += AnalyzeRoundTrip(profilePath, icAbsoluteColorimetric, true);
      bRelative = true;
    }

    //Perceptual intent
    if (pIcc->FindTag(icSigAToB0Tag) && pIcc->FindTag(icSigBToA0Tag)) {
      theReport += AnalyzeRoundTrip(profilePath, icPerceptual, false);
      bRelative = true;
    }
    if (pIcc->FindTag(icSigDToB0Tag) && pIcc->FindTag(icSigBToD0Tag)) {
      theReport += AnalyzeRoundTrip(profilePath, icPerceptual, true);
      bRelative = true;
    }

    //Saturation intent
    if (pIcc->FindTag(icSigAToB2Tag) && pIcc->FindTag(icSigBToA2Tag)) {
      theReport += AnalyzeRoundTrip(profilePath, icSaturation, false);
      bRelative = true;
    }
    if (pIcc->FindTag(icSigDToB2Tag) && pIcc->FindTag(icSigBToD2Tag)) {
      theReport += AnalyzeRoundTrip(profilePath, icSaturation, true);
      bRelative = true;
    }

    wxEndBusyCursor();

    if (theReport.IsEmpty())
      theReport = "There is nothing to report\r\n";
  }

  textReport->SetLabel(theReport);

  SetSizer(sizer);
  sizer->Fit(this);
}

MyTagDialog::MyTagDialog(wxWindow *pParent, CIccProfile *pIcc /*=NULL*/, icTagSignature sig/*=icMaxEnumTag*/, CIccTag *pTag /*=NULL*/) : 
wxDialog(pParent, wxID_ANY, _T("View Tag"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	m_pIcc = pIcc;
	m_sigTag = sig;
	m_pTag = pTag;

	CIccInfo Fmt;

	wxString sTagSignature = Fmt.GetTagSigName(sig);
	wxString sTagType;
  std::string desc;

  if (pTag) {
	  if (pTag->IsArrayType()) {
		  sTagType = _T("Array of ");
	  }
	  else {
		  sTagType.Empty();
	  }
	  sTagType += Fmt.GetTagTypeSigName(pTag->GetType());

	  wxBeginBusyCursor();
	  pTag->Describe(desc);
	  wxEndBusyCursor();
  }
  else if (pIcc) {
    CIccMemIO *pIO = pIcc->GetTagIO(sig);
    sTagType = "***Invalid Tag!***";

    if (pIO) {
      std::string dump;
      icMemDump(dump, pIO->GetData(), pIO->GetLength());
      delete pIO;
      desc = "Data contents of tag:\r\n\r\n";
      desc += dump;
    }
    else {
      desc = "Invalid Tag Directory Entry!\r\n";
    }
  }
  else {
    desc = "Invalid Tag Entry!\r\n";
    sTagType = "***Invalid Tag***";
  }
	
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);

	wxSizer *sizerRow1 = new wxBoxSizer(wxHORIZONTAL);
	wxSize winSize = wxDefaultSize;

	winSize.SetWidth(100);
	wxStaticText *labelSig = new wxStaticText(this, wxID_ANY, _T("Tag Signature:"), wxDefaultPosition, winSize, wxALIGN_RIGHT);

	winSize.SetWidth(400);
	wxStaticText *textSig = new wxStaticText(this, wxID_ANY, sTagSignature, wxDefaultPosition, winSize, wxSTATIC_BORDER|wxTE_RIGHT);

	sizerRow1->Add(labelSig, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 5);
	sizerRow1->Add(textSig, 1, wxALL | wxALIGN_CENTRE_VERTICAL, 5);

	sizer->Add(sizerRow1, wxSizerFlags().Expand());

	wxSizer *sizerRow2 = new wxBoxSizer(wxHORIZONTAL);
	winSize = wxDefaultSize;

	winSize.SetWidth(100);
	wxStaticText *labelType = new wxStaticText(this, wxID_ANY, _T("Tag Type:"), wxDefaultPosition, winSize, wxALIGN_RIGHT);

	winSize.SetWidth(400);
	wxStaticText *textType = new wxStaticText(this, wxID_ANY, sTagType, wxDefaultPosition, winSize, wxSTATIC_BORDER|wxTE_RIGHT);

	sizerRow2->Add(labelType, 0, wxALL | wxALIGN_CENTRE_VERTICAL, 5);
	sizerRow2->Add(textType, 1, wxALL | wxALIGN_CENTRE_VERTICAL, 5);

	sizer->Add(sizerRow2, wxSizerFlags().Expand());

	winSize = wxSize(500, 400);
	wxTextCtrl *textReport = new wxTextCtrl(this, wxID_ANY, wxString(desc.c_str()), wxDefaultPosition, winSize,
		wxTE_MULTILINE |wxTE_READONLY | wxTE_RICH| wxTE_DONTWRAP);

	sizer->Add(textReport, wxSizerFlags(1).Expand());

	SetSizer(sizer);
	sizer->Fit(this);

}
