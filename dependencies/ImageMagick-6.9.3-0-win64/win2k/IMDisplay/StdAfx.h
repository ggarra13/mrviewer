// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__29DD7CA4_F22D_498C_93BC_0C49A4B06588__INCLUDED_)
#define AFX_STDAFX_H__29DD7CA4_F22D_498C_93BC_0C49A4B06588__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
  WINVER values:

  0x0601 - Windows 7
  0x0600 - Windows Vista / Server 2008
  0x0502 - Windows Server 2003 with SP1, Windows XP with SP2
  0x0501 - Windows XP and Windows .NET Server
  0x0400 - Windows 2000
 */
#ifndef WINVER
#  if defined(_MSC_VER) && (_MSC_VER < 1300)
#    define WINVER 0x0400
#  else
#    define WINVER 0x0501
#  endif 
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__29DD7CA4_F22D_498C_93BC_0C49A4B06588__INCLUDED_)
