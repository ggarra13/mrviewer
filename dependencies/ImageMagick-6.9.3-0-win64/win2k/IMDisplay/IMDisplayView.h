// IMDisplayView.h : interface of the CIMDisplayView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMDISPLAYVIEW_H__D21B25E3_E9BE_4F28_AE2D_CD6A88B5E1B4__INCLUDED_)
#define AFX_IMDISPLAYVIEW_H__D21B25E3_E9BE_4F28_AE2D_CD6A88B5E1B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CIMDisplayView : public CScrollView
{
protected: // create from serialization only
	CIMDisplayView();
	DECLARE_DYNCREATE(CIMDisplayView)

// Attributes
public:
	CIMDisplayDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIMDisplayView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIMDisplayView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// these are the custom functions and vars we've added!
        void	DoDisplayImage( Image &inImage, CDC* pDC );
	void	SetViewSize();
	void	UpdateTheView( bool inClearTracker = true );
	void	ScaleImage( Geometry& inGeometry );
        void	ResizeImage( Geometry& inGeometry );
        void    MagnifyImage( void );
        void    MinifyImage( void );
	void	RotateImage( double inDegrees );
	void	SetupUndo();

	void	DoDisplayError(CString szFunction, CString szCause);
	void	DoDisplayError(CString szFunction, DWORD inError );

	CDC*	    mOffscreenDC;
	bool	    mViewDirty;
	Geometry    mBaseGeo;	// what we got when we first started!

	CRectTracker	m_tracker;  // marching ants

	Image	    mUndoImage;

	BITMAPINFOHEADER    mBMI;


// Generated message map functions
protected:
	//{{AFX_MSG(CIMDisplayView)
	afx_msg void OnHalfsize();
	afx_msg void OnOriginalsize();
	afx_msg void OnDoublesize();
	afx_msg void OnResize();
	afx_msg void OnRevert();
	afx_msg void OnRotate();
	afx_msg void OnRotateLeft();
	afx_msg void OnRotateRight();
	afx_msg void OnFlip();
	afx_msg void OnFlop();
	afx_msg void OnShear();
	afx_msg void OnRoll();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChop();
	afx_msg void OnCrop();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCrop(CCmdUI* pCmdUI);
	afx_msg void OnTrimedges();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in IMDisplayView.cpp
inline CIMDisplayDoc* CIMDisplayView::GetDocument()
   { return (CIMDisplayDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMDISPLAYVIEW_H__D21B25E3_E9BE_4F28_AE2D_CD6A88B5E1B4__INCLUDED_)
