// IMDisplay.h : main header file for the IMDISPLAY application
//

#if !defined(AFX_IMDISPLAY_H__99C4A975_BD76_4D75_AE23_C3A996FD55C6__INCLUDED_)
#define AFX_IMDISPLAY_H__99C4A975_BD76_4D75_AE23_C3A996FD55C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <Magick++.h>
using namespace Magick;

/////////////////////////////////////////////////////////////////////////////
// CIMDisplayApp:
// See IMDisplay.cpp for the implementation of this class
//

class CIMDisplayApp : public CWinApp
{
public:
	CIMDisplayApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIMDisplayApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CIMDisplayApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMDISPLAY_H__99C4A975_BD76_4D75_AE23_C3A996FD55C6__INCLUDED_)
