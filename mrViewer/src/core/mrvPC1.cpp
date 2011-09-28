/**
 * @file   mrvPC1.h
 * @author gga
 * @date   Fri Oct 12 00:38:52 2007
 * 
 * @brief  PC-1 text encoding with 128-key
 * 
 * 
 */

#include <iostream>
#include <cstring>
#include <cstdio>
#include <malloc.h>
#include "mrvPC1.h"

namespace mrv {

  namespace cipher {

    //
    // Constructor
    //
    PC1::PC1()
    {
      int j;

      for (j=0;j<=16;j++) {
	cle[j]=0;
      }

      for (j=0;j<=8;j++) {
	x1a0[j]=0;
      }

      pkax=0;
      pkbx=0;
      pkcx=0;
      pkdx=0;
      pksi=0;
      pktmp=0;
      x1a2=0;
      pkres=0;
      pki=0;
      inter=0;
      cfc=0;
      cfd=0;
      compte=0;
      pkc=0;

    }

    void PC1::pkfin()
    {
      int j;

      for (j=0;j<=16;j++) {
	cle[j]=0;
      }

      for (j=0;j<=8;j++) {
	x1a0[j]=0;
      }

      pkax=0;
      pkbx=0;
      pkcx=0;
      pkdx=0;
      pksi=0;
      pktmp=0;
      x1a2=0;
      pkres=0;
      pki=0;
      inter=0;
      cfc=0;
      cfd=0;
      compte=0;
      pkc=0;

    }

    void PC1::pkcode()
    {

      pkdx=x1a2+pki;
      pkax=x1a0[pki];
      pkcx=0x015a;
      pkbx=0x4e35;

      pktmp=pkax;
      pkax=pksi;
      pksi=pktmp;

      pktmp=pkax;
      pkax=pkdx;
      pkdx=pktmp;

      if (pkax!=0)	{
	pkax=pkax*pkbx;
      }

      pktmp=pkax;
      pkax=pkcx;
      pkcx=pktmp;

      if (pkax!=0)	{
	pkax=pkax*pksi;
	pkcx=pkax+pkcx;
      }

      pktmp=pkax;
      pkax=pksi;
      pksi=pktmp;
      pkax=pkax*pkbx;
      pkdx=pkcx+pkdx;

      pkax++;

      x1a2=pkdx;
      x1a0[pki]=pkax;

      pkres=pkax^pkdx;
      pki++;
    }

    void PC1::pkassemble(void)
    {
      x1a0[0]= ( cle[0]*256 )+ cle[1];
      pkcode();
      inter=pkres;

      x1a0[1]= x1a0[0] ^ ( (cle[2]*256) + cle[3] );
      pkcode();
      inter=inter^pkres;

      x1a0[2]= x1a0[1] ^ ( (cle[4]*256) + cle[5] );
      pkcode();
      inter=inter^pkres;

      x1a0[3]= x1a0[2] ^ ( (cle[6]*256) + cle[7] );
      pkcode();
      inter=inter^pkres;


      x1a0[4]= x1a0[3] ^ ( (cle[8]*256) + cle[9] );
      pkcode();
      inter=inter^pkres;

      x1a0[5]= x1a0[4] ^ ( (cle[10]*256) + cle[11] );
      pkcode();
      inter=inter^pkres;

      x1a0[6]= x1a0[5] ^ ( (cle[12]*256) + cle[13] );
      pkcode();
      inter=inter^pkres;

      x1a0[7]= x1a0[6] ^ ( (cle[14]*256) + cle[15] );
      pkcode();
      inter=inter^pkres;

      pki=0;
    }

    void PC1::ascii_encrypt128(const char *in, const char *key)
    {
      int count, k=0;
      short pkd, pke;

      pkfin();

      for (count=0;count<16;count++) {
	cle[count]=key[count];
      }
      cle[count]='\0';

      ascCipherText = (char*)malloc(2*plainlen*sizeof(char)+1);
      for (count=0;count<=plainlen-1;count++) {
	pkc=in[count];

	pkassemble();
	cfc=inter>>8;
	cfd=inter&255;

	for (compte=0;compte<=15;compte++) {
	  cle[compte]=cle[compte]^pkc;
	}
	pkc = pkc ^ (cfc^cfd);

	pkd =(pkc >> 4);
	pke =(pkc & 15);

	ascCipherText[k] = 0x61+pkd; k++;
	ascCipherText[k] = 0x61+pke; k++;
      }
      ascCipherText[k] = '\0';

    }

    void PC1::ascii_decrypt128(const char *in, const char *key)
    {
      int count, k=0;
      short pkd, pke;

      pkfin();

      for (count=0;count<16;count++) {
	cle[count]=key[count];
      }
      cle[count]='\0';

      plainText = (char*)malloc(ascipherlen/2*sizeof(char)+1);

      for (count=0;count<ascipherlen/2;count++) {
	pkd =in[k]; k++;
	pke =in[k]; k++;

	pkd=pkd-0x61;
	pkd=pkd<<4;

	pke=pke-0x61;
	pkc=pkd+pke;

	pkassemble();
	cfc=inter>>8;
	cfd=inter&255;

	pkc = pkc ^ (cfc^cfd);

	for (compte=0;compte<=15;compte++)
	  {
	    cle[compte]=cle[compte]^pkc;
	  }
	plainText[count] = (char)pkc;

      }
      plainText[count] = '\0';

    }

  }

}

