// IMDisplayDoc.h : interface of the CIMDisplayDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMDISPLAYDOC_H__21186681_0BAF_480B_8089_E05E4AB95D1D__INCLUDED_)
#define AFX_IMDISPLAYDOC_H__21186681_0BAF_480B_8089_E05E4AB95D1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CIMDisplayDoc : public CDocument
{
protected: // create from serialization only
	CIMDisplayDoc();
	DECLARE_DYNCREATE(CIMDisplayDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIMDisplayDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIMDisplayDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// these are the custom functions and vars we've added!
	Image&	GetImage( void )	    { return m_pImage; }
	void	SetImage( Image inImage )   { m_pImage = inImage; m_pImage.modifyImage(); }

	BOOL	DoReadImage( void );
	BOOL	DoWriteImage( void );

protected:
	// these are the custom functions and vars we've added!
  void	DoDisplayError(CString szFunction, CString szCause);
  void	DoDisplayWarning(CString szFunction, CString szCause);

	Image         m_pImage;
	CString       m_szFile;

// Generated message map functions
protected:
	//{{AFX_MSG(CIMDisplayDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMDISPLAYDOC_H__21186681_0BAF_480B_8089_E05E4AB95D1D__INCLUDED_)
