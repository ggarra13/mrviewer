/*
    File:       ApplyStatusDlg.h 

    Contains:   header file for CApplyStatusDlg

    Version:    V1

    Copyright:  © see below
*/

/*
 * The ICC Software License, Version 0.2
 *
 *
 * Copyright (c) 2003-2015 The International Color Consortium. All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. In the absence of prior written permission, the names "ICC" and "The
 *    International Color Consortium" must not be used to imply that the
 *    ICC organization endorses or promotes products derived from this
 *    software.
 *
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNATIONAL COLOR CONSORTIUM OR
 * ITS CONTRIBUTING MEMBERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the The International Color Consortium. 
 *
 *
 * Membership in the ICC is encouraged when this software is used for
 * commercial purposes. 
 *
 *  
 * For more information on The International Color Consortium, please
 * see <http://www.color.org/>.
 *  
 * 
 */

////////////////////////////////////////////////////////////////////// 
// HISTORY:
//
// -Initial implementation by Max Derhak 5-15-2003
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APPLYSTATUSDLG_H__E2D35030_FFFE_4FF5_A4D2_43A6BE6BC38E__INCLUDED_)
#define AFX_APPLYSTATUSDLG_H__E2D35030_FFFE_4FF5_A4D2_43A6BE6BC38E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ApplyStatusDlg.h : header file
//

#include "TiffImg.h"
#include "IccCmm.h"

/////////////////////////////////////////////////////////////////////////////
// CApplyStatusDlg dialog

class CApplyStatusDlg : public CDialog
{
// Construction
public:
  CApplyStatusDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CApplyStatusDlg)
  enum { IDD = IDD_APPLY_STAT_DLG };
  CProgressCtrl  m_Progress;
  CString  m_sPercent;
  CString  m_sTotalTime;
  //}}AFX_DATA

  unsigned long m_nStartTime;
  int m_nPercent;
  int m_nLastPercent;

  bool m_bDone;
  bool m_bAbort;
  bool m_bSuccess;

  void Start();
  bool InitApply(const char *szSrcImage = NULL,
                 const char *szSrcProfile = NULL,
                 const char *szDstProfile = NULL,
                 const char *szDstImage  = NULL,
                 int nIntent = 0);

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CApplyStatusDlg)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:
  CTiffImg m_SrcImg;
  CTiffImg m_DestImg;

  CIccCmm m_IccXform;

  CWnd *m_pParent;

  CString m_sSrcImage;
  CString m_sSrcProfile;
  CString m_sDstProfile;
  CString m_sDstImage;
  int m_nIntent;
  
  UINT DoMsgBox(const char *szMsg, UINT id);

  // Generated message map functions
  //{{AFX_MSG(CApplyStatusDlg)
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  virtual BOOL OnInitDialog();
  afx_msg void OnTimer(UINT nIDEvent);
  //}}AFX_MSG
  afx_msg LPARAM OnMsgBox(WPARAM wParam, LPARAM lParam);
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPLYSTATUSDLG_H__E2D35030_FFFE_4FF5_A4D2_43A6BE6BC38E__INCLUDED_)
