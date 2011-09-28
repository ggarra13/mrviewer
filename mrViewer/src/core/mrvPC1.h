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
