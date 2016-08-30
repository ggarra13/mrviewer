// RotateDialog.cpp : implementation file
//

#include "stdafx.h"
#include "IMDisplay.h"
#include "RotateDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotateDialog dialog


CRotateDialog::CRotateDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRotateDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRotateDialog)
	m_Angle = 0.0;
	//}}AFX_DATA_INIT
}


void CRotateDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRotateDialog)
	DDX_Text(pDX, IDC_ROTATEANGLE, m_Angle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRotateDialog, CDialog)
	//{{AFX_MSG_MAP(CRotateDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRotateDialog message handlers

BOOL CRotateDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd*	firstEdit = GetDlgItem( IDC_ROTATEANGLE );
	if ( firstEdit ) {
	    firstEdit->SetFocus();
	}
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
