#if !defined(AFX_ROTATEDIALOG_H__768F7985_4DE9_4041_86C0_D45392C64018__INCLUDED_)
#define AFX_ROTATEDIALOG_H__768F7985_4DE9_4041_86C0_D45392C64018__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RotateDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRotateDialog dialog

class CRotateDialog : public CDialog
{
// Construction
public:
	CRotateDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRotateDialog)
	enum { IDD = IDD_DIALOG_ROTATE };
	double	m_Angle;
	//}}AFX_DATA

	void	Angle( double inAngle )	    { m_Angle = inAngle; }
	double	Angle( void )		    { return m_Angle; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRotateDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRotateDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROTATEDIALOG_H__768F7985_4DE9_4041_86C0_D45392C64018__INCLUDED_)
