/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvPC1.h
 * @author gga
 * @date   Fri Oct 12 00:38:52 2007
 * 
 * @brief  PC-1 text encoding with 128-key
 * 
 * 
 */

namespace mrv {
  namespace cipher {

    class PC1
    {
    public:
      unsigned short pkax,pkbx,pkcx,pkdx,pksi,pktmp,x1a2;
      unsigned short pkres,pki,inter,cfc,cfd,compte;
      unsigned short x1a0[8];
      unsigned char cle[17];
      short pkc, plainlen, ascipherlen;

      char *plainText, *ascCipherText;

      void pkfin(void);
      void pkcode(void);
      void pkassemble(void);

      void ascii_encrypt128(const char *in, const char *key);
      void ascii_decrypt128(const char *in, const char *key);

      // Constructor
      PC1();

    };

  }
}
