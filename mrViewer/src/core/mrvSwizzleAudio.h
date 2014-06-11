#ifndef mrvSwizzleAudio_h
#define mrvSwizzleAudio_h


namespace mrv {

template< typename T >
struct Swizzle
{
     T* ptr;
     unsigned last;
     
     inline Swizzle(void* data, unsigned data_size ) :
     ptr( (T*) data ),
     last( data_size / 8*sizeof(T) )
     {
     }

     inline void do_it() 
     {
	unsigned i;
	T tmp;
	for (i = 0; i < last; i++, ptr += 6) {	
	   tmp = ptr[2]; ptr[2] = ptr[4]; ptr[4] = tmp;
	   tmp = ptr[3]; ptr[3] = ptr[5]; ptr[5] = tmp;
	}
     }
};

template< typename T >
struct SwizzlePlanar
{
    T** ptr;
     
    inline SwizzlePlanar(void** data ) :
    ptr( (T**) data )
    {
    }

    inline void do_it() 
    {
        T* tmp;
        tmp = ptr[2]; ptr[2] = ptr[4]; ptr[4] = tmp;
        tmp = ptr[3]; ptr[3] = ptr[5]; ptr[5] = tmp;
    }
};

}

#endif

