// ResizeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "IMDisplay.h"
#include "ResizeDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizeDialog dialog


CResizeDialog::CResizeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CResizeDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResizeDialog)
	m_IsPercentages = FALSE;
	m_Width = 0;
	m_Height = 0;
	//}}AFX_DATA_INIT
}


void CResizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResizeDialog)
	DDX_Check(pDX, IDC_PERCENT, m_IsPercentages);
	DDX_Text(pDX, IDC_WIDTH, m_Width);
	DDX_Text(pDX, IDC_HEIGHT, m_Height);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResizeDialog, CDialog)
	//{{AFX_MSG_MAP(CResizeDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResizeDialog message handlers

BOOL CResizeDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd*	widthEdit = GetDlgItem( IDC_WIDTH );
	if ( widthEdit ) {
	    widthEdit->SetFocus();
	}
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
