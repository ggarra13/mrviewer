/*
    File:       TiffImg.cpp

    Contains:   Implementation of the CTiffImg class.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TiffImg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTiffImg::CTiffImg()
{
  m_nWidth = 0;
  m_nHeight = 0;
  m_nBitsPerSample = 0;
  m_nSamples = 0;

  m_hTif = NULL;
  m_pStripBuf = NULL;
}

CTiffImg::~CTiffImg()
{
  Close();
}

void CTiffImg::Close()
{
  m_nWidth = 0;
  m_nHeight = 0;
  m_nBitsPerSample = 0;
  m_nSamples = 0;

  if (m_hTif) {
    TIFFClose(m_hTif);

    m_hTif = NULL;
  }

  if (m_pStripBuf) {
    free(m_pStripBuf);
    m_pStripBuf = NULL;
  }
}

bool CTiffImg::Create(const char *szFname, unsigned long nWidth, unsigned long nHeight,
              unsigned long nBPS, unsigned long nPhoto, unsigned long nSamples,
              float fXRes, float fYRes, bool bCompress)
{
  Close();
  m_bRead = false;

  m_nWidth = nWidth;
  m_nHeight = nHeight;
  m_nBitsPerSample = (unsigned short)nBPS;
  m_nSamples = (unsigned short)nSamples;

  switch(nPhoto) {
  case PHOTO_MINISBLACK:
    if (m_nSamples==3) 
      m_nPhoto = PHOTOMETRIC_RGB;
    else
      m_nPhoto = PHOTOMETRIC_MINISBLACK;
    break;

  case PHOTO_MINISWHITE:
    if (m_nSamples==4)
      m_nPhoto = PHOTOMETRIC_SEPARATED;
    else
      m_nPhoto = PHOTOMETRIC_MINISWHITE;
    break;

  case PHOTO_CIELAB:
    m_nPhoto = PHOTOMETRIC_CIELAB;
    break;
  }

  m_hTif = TIFFOpen(szFname, "w");
  if (!m_hTif) {
    TIFFError(szFname,"Can not open output image");
    return false;
  }
  TIFFSetField(m_hTif, TIFFTAG_IMAGEWIDTH, (uint32) m_nWidth);
  TIFFSetField(m_hTif, TIFFTAG_IMAGELENGTH, (uint32) m_nHeight);
  TIFFSetField(m_hTif, TIFFTAG_PHOTOMETRIC, m_nPhoto);
  TIFFSetField(m_hTif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(m_hTif, TIFFTAG_SAMPLESPERPIXEL, m_nSamples);
  TIFFSetField(m_hTif, TIFFTAG_BITSPERSAMPLE, m_nBitsPerSample);
  TIFFSetField(m_hTif, TIFFTAG_ROWSPERSTRIP, 1);
  TIFFSetField(m_hTif, TIFFTAG_COMPRESSION, bCompress ? COMPRESSION_LZW : COMPRESSION_NONE);
  TIFFSetField(m_hTif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(m_hTif, TIFFTAG_XRESOLUTION, fXRes);
  TIFFSetField(m_hTif, TIFFTAG_YRESOLUTION, fYRes);

  m_nCurLine = 0;
  m_nCurStrip = 0;

  m_nBytesPerLine = TIFFStripSize(m_hTif);

  return true;
}

bool CTiffImg::Open(const char *szFname)
{
  Close();
  m_bRead = true;

  m_hTif = TIFFOpen(szFname, "r");
  if (!m_hTif) {
    TIFFError(szFname,"Can not open input image");
    return false;
  }
  unsigned short nPlanar=PLANARCONFIG_CONTIG;
  unsigned short nOrientation=ORIENTATION_TOPLEFT;

  TIFFGetField(m_hTif, TIFFTAG_IMAGEWIDTH, &m_nWidth);
  TIFFGetField(m_hTif, TIFFTAG_IMAGELENGTH, &m_nHeight);
  TIFFGetField(m_hTif, TIFFTAG_PHOTOMETRIC, &m_nPhoto);
  TIFFGetField(m_hTif, TIFFTAG_PLANARCONFIG, &nPlanar);
  TIFFGetField(m_hTif, TIFFTAG_SAMPLESPERPIXEL, &m_nSamples);
  TIFFGetField(m_hTif, TIFFTAG_BITSPERSAMPLE, &m_nBitsPerSample);
  TIFFGetField(m_hTif, TIFFTAG_ROWSPERSTRIP, &m_nRowsPerStrip);
  TIFFGetField(m_hTif, TIFFTAG_ORIENTATION, &nOrientation);
  TIFFGetField(m_hTif, TIFFTAG_XRESOLUTION, &m_fXRes);
  TIFFGetField(m_hTif, TIFFTAG_YRESOLUTION, &m_fYRes);

  if ((m_nSamples>1 && nPlanar != PLANARCONFIG_CONTIG) ||
       nOrientation != ORIENTATION_TOPLEFT) {
    Close();
    return false;
  }
  m_nCurStrip=(unsigned long)-1;
  m_nCurLine = 0;

  m_nStripSize = TIFFStripSize(m_hTif);

  m_pStripBuf = (unsigned char*)malloc(m_nStripSize);

  m_nBytesPerLine = (m_nWidth * m_nBitsPerSample * m_nSamples + 7)>>3;

  if (!m_pStripBuf) {
    Close();
    return false;
  }

  return true;
}


bool CTiffImg::ReadLine(unsigned char *pBuf)
{
  if (!m_bRead)
    return false;

  unsigned long nStrip = m_nCurLine / m_nRowsPerStrip;
  unsigned long nRowOffset = m_nCurLine % m_nRowsPerStrip;

  if (nStrip != m_nCurStrip) {
    m_nCurStrip = nStrip;

    if (TIFFReadEncodedStrip(m_hTif, m_nCurStrip, m_pStripBuf, m_nStripSize) < 0) {
      return false;
    }
  }

  memcpy(pBuf, m_pStripBuf+nRowOffset*m_nBytesPerLine, m_nBytesPerLine);
  m_nCurLine++;

  return true;
}

bool CTiffImg::WriteLine(unsigned char *pBuf)
{
  if (m_bRead)
    return false;

  if (m_nCurStrip < m_nHeight) {
    if (TIFFWriteEncodedStrip(m_hTif, m_nCurStrip, pBuf, m_nBytesPerLine) < 0)
      return false;
    m_nCurStrip++;
  }

  return true;
}

unsigned long CTiffImg::GetPhoto()
{
  if (m_nPhoto==PHOTOMETRIC_MINISBLACK ||
      m_nPhoto==PHOTOMETRIC_RGB) {
    return PHOTO_MINISBLACK;
  }
  else if (m_nPhoto==PHOTOMETRIC_MINISWHITE ||
           m_nPhoto==PHOTOMETRIC_SEPARATED) {
    return PHOTO_MINISWHITE;
  }
  else if (m_nPhoto==PHOTOMETRIC_CIELAB)
    return PHOTO_CIELAB;
  else
    return PHOTO_MINISWHITE;
}
