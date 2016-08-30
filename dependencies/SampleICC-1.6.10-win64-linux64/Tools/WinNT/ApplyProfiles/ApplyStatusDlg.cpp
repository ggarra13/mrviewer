  /*
    File:       ApplyStatusDlg.cpp

    Contains:   Implementation of CApplyStatusDlg

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

#include "stdafx.h"
#include "ApplyProfiles.h"
#include "ApplyStatusDlg.h"
#include "IccCmm.h"
#include "IccUtil.h"
#include "TiffImg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_MSGBOX WM_USER+100

/////////////////////////////////////////////////////////////////////////////
// CApplyStatusDlg dialog


CApplyStatusDlg::CApplyStatusDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CApplyStatusDlg::IDD, pParent)
{
  m_pParent = pParent;
  //{{AFX_DATA_INIT(CApplyStatusDlg)
  m_sPercent = _T("");
  m_sTotalTime = _T("");
  //}}AFX_DATA_INIT
}


void CApplyStatusDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CApplyStatusDlg)
  DDX_Control(pDX, IDC_PROGRESS, m_Progress);
  DDX_Text(pDX, IDC_PERCENT, m_sPercent);
  DDX_Text(pDX, IDC_TOTAL_TIME, m_sTotalTime);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CApplyStatusDlg, CDialog)
  //{{AFX_MSG_MAP(CApplyStatusDlg)
  ON_WM_CTLCOLOR()
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
  ON_MESSAGE(WM_MSGBOX, OnMsgBox)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApplyStatusDlg message handlers

LPARAM CApplyStatusDlg::OnMsgBox(WPARAM wParam, LPARAM lParam)
{
  const char *szMsg = (const char*)lParam;
  return MessageBox(szMsg, NULL, wParam);
}

bool CApplyStatusDlg::InitApply(const char *szSrcImage/*=NULL*/,
                                const char *szSrcProfile/*=NULL*/,
                                const char *szDstProfile/*=NULL*/,
                                const char *szDstImage/*=NULL*/,
                                int nIntent /*= 0*/)
{
  m_sSrcImage = szSrcImage;
  m_sSrcProfile = szSrcProfile;
  m_sDstProfile = szDstProfile;
  m_sDstImage = szDstImage;
  m_nIntent = nIntent;

  return true;
}


HBRUSH CApplyStatusDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
  HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
  int nID = pWnd->GetDlgCtrlID();

  if (nCtlColor==CTLCOLOR_STATIC && 
    (nID == IDC_PERCENT || nID == IDC_TOTAL_TIME)) {
    pDC->SetTextColor(0xff0000);
  }

  return hbr;
}

UINT ApplyThreadFunc( LPVOID pParam )
{
  CApplyStatusDlg* pDlg = (CApplyStatusDlg*)pParam;
  pDlg->Start();
  pDlg->m_bDone = true;

  return 0;
}

BOOL CApplyStatusDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
  
  SetTimer(100, 2000, NULL);

  time_t ltime;
  m_nStartTime = time(&ltime);
  m_nPercent = 0;
  m_nLastPercent = -1;

  m_bAbort = false;
  m_bDone = false;
  m_bSuccess = false;

  m_Progress.SetRange(0, 100);

  AfxBeginThread(ApplyThreadFunc, this);
  
  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

void CApplyStatusDlg::OnTimer(UINT nIDEvent) 
{
  if (nIDEvent==100) {
    time_t ltime, elapsed;
    elapsed = time(&ltime) - m_nStartTime;

    int hours, mins, secs;

    hours = elapsed /3600;
    mins = (elapsed - hours*3600) / 60;
    secs = elapsed - hours*3600 - mins*60;

    m_sTotalTime.Format("%02d:%02d:%02d", hours, mins, secs);

    if (m_nLastPercent != m_nPercent) {
      m_sPercent.Format("%d%%", m_nPercent);
      m_nLastPercent = m_nPercent;
      m_Progress.SetPos(m_nPercent);
    }

    if (!m_bAbort && m_bDone) {
      if (m_bSuccess)
        CDialog::OnOK();
      else
        CDialog::OnCancel();
    }
    else {
      UpdateData(FALSE);
    }
  }
  else {
    CDialog::OnTimer(nIDEvent);
  }
}

UINT CApplyStatusDlg::DoMsgBox(const char *szMsg, UINT id)
{
  return SendMessage(WM_MSGBOX, id, (LPARAM)szMsg);
}

static icFloatNumber UnitClip(icFloatNumber v)
{
  if (v<0.0)
    return 0.0;
  if (v>1.0)
    return 1.0;
  return v;
}

void CApplyStatusDlg::Start()
{
  unsigned long i, j, k, sn, sphoto, dn, photo, space;
  CString Msg;
  CTiffImg SrcImg, DstImg;
  unsigned char *sptr, *dptr;
  bool bConvert = false;

	if (!SrcImg.Open(m_sSrcImage)) {
		Msg.Format("Invalid Tiff file - '%s'", (LPCTSTR)m_sSrcImage);
		DoMsgBox(Msg, MB_OK);
		return;
	}
	sn = SrcImg.GetSamples();
	sphoto = SrcImg.GetPhoto();

	bool bInput = true;
	if (sn==3 && sphoto==PHOTO_CIELAB) {
		bInput = false;
	}

	CIccCmm cmm(icSigUnknownData, icSigUnknownData, bInput);
  if (cmm.AddXform(m_sSrcProfile, m_nIntent==0 ? icUnknownIntent : (icRenderingIntent)(m_nIntent-1))!=icCmmStatOk) {
    Msg.Format("Invalid Source Profile:\n  %s\n", (LPCTSTR)m_sSrcProfile);
    DoMsgBox(Msg, MB_OK);
    return;
  }

	if (!m_sDstProfile.IsEmpty()) {
		if (cmm.AddXform(m_sSrcProfile, m_nIntent==0 ? icUnknownIntent : (icRenderingIntent)(m_nIntent-1))!=icCmmStatOk) {
			Msg.Format("Invalid Destination Profile:\n  %s\n", m_sDstProfile);
			DoMsgBox(Msg, MB_OK);
			return;
		}
	}

  if (cmm.Begin() != icCmmStatOk) {
    Msg.Format("Invalid Profiles:\n  %s\n  %s'", (LPCTSTR)m_sSrcProfile, m_sDstProfile);
    DoMsgBox(Msg, MB_OK);
    return;
  }

  space = cmm.GetSourceSpace();

  if (SrcImg.GetBitsPerSample()!=8 ||
      !((space==icSigRgbData && sn==3 && sphoto==PHOTO_MINISBLACK) ||
        (space==icSigLabData && sn==3 && sphoto==PHOTO_CIELAB) ||
        (space==icSigXYZData && sn==3 && sphoto==PHOTO_CIELAB) ||
        (space==icSigCmykData && sn==4 && sphoto==PHOTO_MINISWHITE) ||
        (space==icSigMCH4Data && sn==4 && sphoto==PHOTO_MINISWHITE) ||
        (space==icSigMCH5Data && sn==5 && sphoto==PHOTO_MINISWHITE) ||
        (space==icSigMCH6Data && sn==6 && sphoto==PHOTO_MINISWHITE))) {
    Msg.Format("Invalid source profile/image pixel format");
    DoMsgBox(Msg, MB_OK);
    return;
  }

  switch (cmm.GetDestSpace()) {
  case icSigRgbData:
    photo = PHOTO_MINISBLACK;
    dn = 3;
    break;

  case icSigCmyData:
    photo = PHOTO_MINISWHITE;
    dn = 3;
    break;

  case icSigXYZData:
    bConvert = true;
    //Fall through - No break here

  case icSigLabData:
    photo = PHOTO_CIELAB;
    dn = 3;
    break;

  case icSigCmykData:
  case icSig4colorData:
    photo = PHOTO_MINISWHITE;
    dn = 4;
    break;

  case icSig5colorData:
    photo = PHOTO_MINISWHITE;
    dn = 5;
    break;

  case icSig6colorData:
    photo = PHOTO_MINISWHITE;
    dn = 6;
    break;

  case icSig7colorData:
    photo = PHOTO_MINISWHITE;
    dn = 7;
    break;

  case icSig8colorData:
    photo = PHOTO_MINISWHITE;
    dn = 8;
    break;

  default:
    Msg.Format("Invalid destination profile/image pixel format");
    DoMsgBox(Msg, MB_OK);
    return;
  }

  if (!DstImg.Create(m_sDstImage, SrcImg.GetWidth(), SrcImg.GetHeight(), 8, photo, dn, SrcImg.GetXRes(), SrcImg.GetYRes())) {
    Msg.Format("Unable to create Tiff file - '%s'", (LPCTSTR)m_sDstImage);
    DoMsgBox(Msg, MB_OK);
    return;
  }

  unsigned char *pSBuf = (unsigned char *)malloc(SrcImg.GetBytesPerLine());
  unsigned char *pDBuf = (unsigned char *)malloc(DstImg.GetBytesPerLine());
  icFloatNumber Pixel[16];

  if (!pSBuf) {
    Msg.Format("Out of Memory!");
    DoMsgBox(Msg, MB_OK);
    return;
  }

  if (!pDBuf) {
    Msg.Format("Out of Memory!");
    DoMsgBox(Msg, MB_OK);
    free(pSBuf);
    return;
  }

  for (i=0; i<SrcImg.GetHeight() && !m_bAbort; i++) {
    if (!SrcImg.ReadLine(pSBuf)) {
      m_bAbort = 1;
    return;
    }
    for (sptr=pSBuf, dptr=pDBuf, j=0; j<SrcImg.GetWidth(); j++, sptr+=sn, dptr+=dn) {
      if (sphoto==PHOTO_CIELAB) {
        Pixel[0] = (icFloatNumber)sptr[0] / 255.0f;
        Pixel[1] = ((icFloatNumber)((signed char)sptr[1]) + 128.0f) / 255.0f;
        Pixel[2] = ((icFloatNumber)((signed char)sptr[2]) + 128.0f) / 255.0f;

        if (space==icSigXYZData) {
          icLabFromPcs(Pixel);

          icLabtoXYZ(Pixel);

          icXyzToPcs(Pixel);
        }
      }
      else {
        for (k=0; k<sn; k++) {
          Pixel[k] = (icFloatNumber)sptr[k] / 255.0f;
        }
      }

      cmm.Apply(Pixel, Pixel);

      if (photo==PHOTO_CIELAB) {
        if (bConvert) {
          icXyzFromPcs(Pixel);

          icXYZtoLab(Pixel);

          icLabToPcs(Pixel);
        }
        dptr[0] = (unsigned char)(UnitClip(Pixel[0]) * 255.0 + 0.5);
        dptr[1] = (unsigned char)(UnitClip(Pixel[1]) * 255.0 - 128.0);
        dptr[2] = (unsigned char)(UnitClip(Pixel[2]) * 255.0 - 128.0);
      }
      else {
        for (k=0; k<dn; k++) {
          dptr[k] = (unsigned char)(UnitClip(Pixel[k]) * 255.0 + 0.5);
        }
      }
    }
    if (!DstImg.WriteLine(pDBuf)) {
      m_bAbort = 1;
      break;
    }
    m_nPercent = (i+1) * 100 / SrcImg.GetHeight();
  }
  if (m_nPercent==100)
    Sleep(2000);

  SrcImg.Close();

  free(pSBuf);
  free(pDBuf);

  m_bSuccess = true;
}