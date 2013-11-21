
#include "mrvReel.h"

namespace mrv {

mrv::media Reel_t::media_at( const boost::int64_t f ) const
{
    if ( images.size() == 0 ) return mrv::media();

    mrv::MediaList::const_iterator i = images.begin();
    mrv::MediaList::const_iterator e = images.end();

    mrv::media m = images.front();
    double mn = m->position();

    m = images.back();
    double mx = m->position() + m->image()->duration();
    if ( mn > mx ) 
    {
       double t = mx;
       mx = mn; mn = t;
    }

    if ( f < mn ) return mrv::media();
    if ( f > mx ) return mrv::media();

    int64_t  t = 1;
    unsigned r = 0;
    for ( ; i != e; ++i, ++r )
      {
	CMedia* img = (*i)->image();
	t += img->duration();
 	if ( t > f ) break;
      }
    if ( r >= images.size() ) return mrv::media();

    return images[r];
}

}  // namespace mrv

