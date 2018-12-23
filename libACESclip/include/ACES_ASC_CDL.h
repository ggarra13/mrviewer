
#ifndef ACES_ASC_CDL_h
#define ACES_ASC_CDL_h

#include <cassert>

namespace ACES {

class ASC_CDL
{
  public:
    ASC_CDL() : _saturation(1.0)
    {
        _slope[0]  = _slope[1]  = _slope[2]  = 1.0f;
        _offset[0] = _offset[1] = _offset[2] = 0.0f;
        _power[0]  = _power[1]  = _power[2]  = 1.0f;
    };

    ASC_CDL( const ASC_CDL& b ) :
    _saturation( b.saturation() )
    {
        for ( unsigned short i = 0; i < 3; ++i )
        {
            _slope[i] = b.slope(i);
            _offset[i] = b.offset(i);
            _power[i] = b.power(i);
        }
    }

    inline void slope( const unsigned i, const float x )
    {
        assert( i < 3 );
        _slope[i] = x;
    }

    inline void offset( const unsigned i, const float x )
    {
        assert( i < 3 );
        _offset[i] = x;
    }
    
    inline void power( const unsigned i, const float x )
    {
        assert( i < 3 );
        _power[i] = x;
    }
    
    inline void slope( const float x, const float y, const float z )
    {
        _slope[0] = x;
        _slope[1] = y;
        _slope[2] = z;
    };

    inline void offset( const float x, const float y, const float z )
    {
        _offset[0] = x;
        _offset[1] = y;
        _offset[2] = z;
    };

    inline void power( const float x, const float y, const float z )
    {
        _power[0] = x;
        _power[1] = y;
        _power[2] = z;
    };

    inline float slope( unsigned short i ) const  {
        assert( i < 3 ); return _slope[i];
    };
    inline float offset( unsigned short i ) const {
        assert( i < 3 ); return _offset[i];
    };
    inline float power( unsigned short i ) const  {
        assert( i < 3 ); return _power[i];
    };

    inline void saturation( float s ) { _saturation = s; };
    inline float saturation() const { return _saturation; };

    inline friend std::ostream& operator<<( std::ostream& o, const ASC_CDL& b ) 
    {
        o << "slope " << b.slope(0) << ", " << b.slope(1) << ", " << b.slope(2)
          << std::endl
          << "offset " << b.offset(0) << ", " << b.offset(1) << ", "
          << b.offset(2) << std::endl
          << "power " << b.power(0) << ", " << b.power(1) << ", " << b.power(2)
          << std::endl
          << "saturation " << b.saturation() << std::endl;
        return o; 
    }

    inline bool operator==( const ASC_CDL& b )
    {
        if ( _saturation != b._saturation ) return false;
        for ( int i = 0; i < 3; ++i )
        {
            if ( _slope[i]  != b._slope[i]  ) return false;
            if ( _offset[i] != b._offset[i] ) return false;
            if ( _power[i]  != b._power[i]  ) return false;
        }
        return true;
    }
    
    inline bool operator!=( const ASC_CDL& b )
    {
        return ! (operator==(b));
    }
    
  protected:
    float _slope[3];
    float _offset[3];
    float _power[3];

    float _saturation;
};

} // namespace ACES

#endif
