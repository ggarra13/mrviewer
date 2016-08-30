/*
    File:       ApplyProfilesDlg.cpp

    Contains:   Implementation of CApplyProfilesDlg

    Version:    V1

    Copyright:  © see below
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

////////////////////////////////////////////////////////////////////// 
// HISTORY:
//
// -Initial implementation by Max Derhak 5-15-2003
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ApplyProfiles.h"
#include "ApplyProfilesDlg.h"
#include "ApplyStatusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();

// Dialog Data
  //{{AFX_DATA(CAboutDlg)
  enum { IDD = IDD_ABOUTBOX };
  //}}AFX_DATA

  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAboutDlg)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  //{{AFX_MSG(CAboutDlg)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
  //{{AFX_DATA_INIT(CAboutDlg)
  //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDlg)
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
  //{{AFX_MSG_MAP(CAboutDlg)
    // No message handlers
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApplyProfilesDlg dialog

CApplyProfilesDlg::CApplyProfilesDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CApplyProfilesDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CApplyProfilesDlg)
  m_sDstImage = _T("");
  m_sDstProfile = _T("");
  m_sSrcImage = _T("");
  m_sSrcProfile = _T("");
  m_nRenderingIntent = 0;
  //}}AFX_DATA_INIT
  // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CApplyProfilesDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CApplyProfilesDlg)
  DDX_Text(pDX, IDC_EDIT_DST_IMAGE, m_sDstImage);
  DDX_Text(pDX, IDC_EDIT_DST_PROFILE, m_sDstProfile);
  DDX_Text(pDX, IDC_EDIT_SRC_IMAGE, m_sSrcImage);
  DDX_Text(pDX, IDC_EDIT_SRC_PROFILE, m_sSrcProfile);
  DDX_CBIndex(pDX, IDC_RENDERING_INTENT, m_nRenderingIntent);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CApplyProfilesDlg, CDialog)
  //{{AFX_MSG_MAP(CApplyProfilesDlg)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_BROWSE_DST_IMAGE, OnBrowseDstImage)
  ON_BN_CLICKED(IDC_BROWSE_DST_PROFILE, OnBrowseDstProfile)
  ON_BN_CLICKED(IDC_BROWSE_SRC_IMAGE, OnBrowseSrcImage)
  ON_BN_CLICKED(IDC_BROWSE_SRC_PROFILE, OnBrowseSrcProfile)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApplyProfilesDlg message handlers

BOOL CApplyProfilesDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Add "About..." menu item to system menu.

  // IDM_ABOUTBOX must be in the system command range.
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != NULL)
  {
    CString strAboutMenu;
    strAboutMenu.LoadString(IDS_ABOUTBOX);
    if (!strAboutMenu.IsEmpty())
    {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
    }
  }

  // Set the icon for this dialog.  The framework does this automatically
  //  when the application's main window is not a dialog
  SetIcon(m_hIcon, TRUE);      // Set big icon
  SetIcon(m_hIcon, FALSE);    // Set small icon
  
  // TODO: Add extra initialization here
  
  return TRUE;  // return TRUE  unless you set the focus to a control
}

void CApplyProfilesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  if ((nID & 0xFFF0) == IDM_ABOUTBOX)
  {
    CAboutDlg dlgAbout;
    dlgAbout.DoModal();
  }
  else
  {
    CDialog::OnSysCommand(nID, lParam);
  }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CApplyProfilesDlg::OnPaint() 
{
  if (IsIconic())
  {
    CPaintDC dc(this); // device context for painting

    SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    // Draw the icon
    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    CDialog::OnPaint();
  }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CApplyProfilesDlg::OnQueryDragIcon()
{
  return (HCURSOR) m_hIcon;
}

void CApplyProfilesDlg::OnBrowseDstImage() 
{
  CFileDialog dlg(FALSE, "tif", m_sDstImage, OFN_HIDEREADONLY, "TIF Files (*.tif)|*.tif||", this);
  
  if (dlg.DoModal()==IDOK) {
    m_sDstImage = dlg.m_ofn.lpstrFile;
  }
  UpdateData(FALSE);
}

void CApplyProfilesDlg::OnBrowseDstProfile() 
{
  CFileDialog dlg(TRUE, "icc", m_sDstProfile, OFN_HIDEREADONLY, "ICC Files (*.icc)|*.icc||", this);
  
  if (dlg.DoModal()==IDOK) {
    m_sDstProfile = dlg.m_ofn.lpstrFile;
  }
  UpdateData(FALSE);
}

void CApplyProfilesDlg::OnBrowseSrcImage() 
{
  CFileDialog dlg(TRUE, "tif", m_sSrcImage, OFN_HIDEREADONLY, "TIF Files (*.tif)|*.tif||", this);
  
  if (dlg.DoModal()==IDOK) {
    m_sSrcImage = dlg.m_ofn.lpstrFile;
  }
  UpdateData(FALSE);
}

void CApplyProfilesDlg::OnBrowseSrcProfile() 
{
  CFileDialog dlg(TRUE, "icc", m_sSrcProfile, OFN_HIDEREADONLY, "ICC Files (*.icc)|*.icc||", this);
  
  if (dlg.DoModal()==IDOK) {
    m_sSrcProfile = dlg.m_ofn.lpstrFile;
  }
  UpdateData(FALSE);
}

void CApplyProfilesDlg::OnOK() 
{
  CApplyStatusDlg Dlg(this);

  UpdateData(TRUE);

  if (m_sSrcImage.IsEmpty())
    return;

  if (Dlg.InitApply(m_sSrcImage, m_sSrcProfile, m_sDstProfile, m_sDstImage, m_nRenderingIntent)) {
  
    if (Dlg.DoModal()==IDOK) {
      MessageBox("Profiles successfully applied!");
    }
    else {
      MessageBox("Apply Canceled!");
    }

    m_sDstImage = _T("");
    m_sDstProfile = _T("");
    m_sSrcImage = _T("");
    m_sSrcProfile = _T("");
    m_nRenderingIntent = 0;

    UpdateData(FALSE);
  }
}

