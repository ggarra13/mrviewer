// RollDialog.cpp : implementation file
//

#include "stdafx.h"
#include "IMDisplay.h"
#include "RollDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRollDialog dialog


CRollDialog::CRollDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRollDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRollDialog)
	m_HPixels = 0;
	m_VPixels = 0;
	//}}AFX_DATA_INIT
}


void CRollDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRollDialog)
	DDX_Text(pDX, IDC_HPIXELS, m_HPixels);
	DDX_Text(pDX, IDC_VPIXELS, m_VPixels);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRollDialog, CDialog)
	//{{AFX_MSG_MAP(CRollDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRollDialog message handlers

BOOL CRollDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd*	firstEdit = GetDlgItem( IDC_HPIXELS );
	if ( firstEdit ) {
	    firstEdit->SetFocus();
	}
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
