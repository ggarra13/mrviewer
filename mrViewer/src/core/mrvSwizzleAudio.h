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
#ifndef mrvSwizzleAudio_h
#define mrvSwizzleAudio_h


namespace mrv {

template< typename T >
struct Swizzle
{
     T* ptr;
     size_t last;
     
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

