#if !defined(AFX_ROLLDIALOG_H__6A4F9190_C52E_4A56_BE24_6BD88D36BFE4__INCLUDED_)
#define AFX_ROLLDIALOG_H__6A4F9190_C52E_4A56_BE24_6BD88D36BFE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RollDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRollDialog dialog

class CRollDialog : public CDialog
{
// Construction
public:
	CRollDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRollDialog)
	enum { IDD = IDD_DIALOG_ROLL };
	int		m_HPixels;
	int		m_VPixels;
	//}}AFX_DATA

	void	HPixels( double inDegrees )  { m_HPixels = inDegrees; }
	double	HPixels( )		     { return m_HPixels; }

	void	VPixels( double inDegrees )  { m_VPixels = inDegrees; }
	double	VPixels( )		     { return m_VPixels; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRollDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRollDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROLLDIALOG_H__6A4F9190_C52E_4A56_BE24_6BD88D36BFE4__INCLUDED_)
