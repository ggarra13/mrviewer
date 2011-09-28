#ifndef byteSwap_h
#define byteSwap_h


//! Swap a float between network and host byte-order
inline float ByteSwap( float f )
{
  union
  {
    float f;
    unsigned char b[4];
  } dat1, dat2;

  dat1.f = f;
  dat2.b[0] = dat1.b[3];
  dat2.b[1] = dat1.b[2];
  dat2.b[2] = dat1.b[1];
  dat2.b[3] = dat1.b[0];
  return dat2.f;
}

#ifdef MRV_BIG_ENDIAN
#define MAKE_BIGENDIAN(t)
#else
#define MAKE_BIGENDIAN(t) t = ByteSwap(t);
#endif


#endif // byteSwap_h
