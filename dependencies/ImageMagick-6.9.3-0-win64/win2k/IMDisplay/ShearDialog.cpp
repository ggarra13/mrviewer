// ShearDialog.cpp : implementation file
//

#include "stdafx.h"
#include "IMDisplay.h"
#include "ShearDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShearDialog dialog


CShearDialog::CShearDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CShearDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShearDialog)
	m_XShear = 0.0;
	m_YShear = 0.0;
	//}}AFX_DATA_INIT
}


void CShearDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShearDialog)
	DDX_Text(pDX, IDC_XSHEAR, m_XShear);
	DDX_Text(pDX, IDC_YSHEAR, m_YShear);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShearDialog, CDialog)
	//{{AFX_MSG_MAP(CShearDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShearDialog message handlers

BOOL CShearDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd*	firstEdit = GetDlgItem( IDC_XSHEAR );
	if ( firstEdit ) {
	    firstEdit->SetFocus();
	}
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
