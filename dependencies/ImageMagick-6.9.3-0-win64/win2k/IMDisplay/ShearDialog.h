#if !defined(AFX_SHEARDIALOG_H__71835E4A_AA57_4BDA_B473_7C0EB34B6BEE__INCLUDED_)
#define AFX_SHEARDIALOG_H__71835E4A_AA57_4BDA_B473_7C0EB34B6BEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShearDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CShearDialog dialog

class CShearDialog : public CDialog
{
// Construction
public:
	CShearDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShearDialog)
	enum { IDD = IDD_DIALOG_SHEAR };
	double	m_XShear;
	double	m_YShear;
	//}}AFX_DATA

	void	XShear( double inDegrees )  { m_XShear = inDegrees; }
	double	XShear( )		    { return m_XShear; }

	void	YShear( double inDegrees )  { m_YShear = inDegrees; }
	double	YShear( )		    { return m_YShear; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShearDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShearDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHEARDIALOG_H__71835E4A_AA57_4BDA_B473_7C0EB34B6BEE__INCLUDED_)
