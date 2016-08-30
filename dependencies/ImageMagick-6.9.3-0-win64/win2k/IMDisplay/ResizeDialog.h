#if !defined(AFX_RESIZEDIALOG_H__8F9DC4C3_D281_415B_8E7C_B9D2D5F3BA6F__INCLUDED_)
#define AFX_RESIZEDIALOG_H__8F9DC4C3_D281_415B_8E7C_B9D2D5F3BA6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResizeDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResizeDialog dialog

class CResizeDialog : public CDialog
{
// Construction
public:
	CResizeDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CResizeDialog)
	enum { IDD = IDD_DIALOG_RESIZE };
	BOOL	m_IsPercentages;
	int		m_Width;
	int		m_Height;
	//}}AFX_DATA

	int	Width()			{ return m_Width; }
	void	Width( int inWidth )	{ m_Width = inWidth; }
	
	int Height()			{ return m_Height; }
	void	Height( int inHeight )	{ m_Height = inHeight; }	

	bool IsPercentage() { return m_IsPercentages; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizeDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CResizeDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZEDIALOG_H__8F9DC4C3_D281_415B_8E7C_B9D2D5F3BA6F__INCLUDED_)
