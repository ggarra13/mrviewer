
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

    void slope( const float x, const float y, const float z )
    {
        _slope[0] = x;
        _slope[1] = y;
        _slope[2] = z;
    };

    void offset( const float x, const float y, const float z )
    {
        _offset[0] = x;
        _offset[1] = y;
        _offset[2] = z;
    };

    void power( const float x, const float y, const float z )
    {
        _power[0] = x;
        _power[1] = y;
        _power[2] = z;
    };

    float slope( unsigned short i ) const  { assert( i < 3 ); return _slope[i]; };
    float offset( unsigned short i ) const { assert( i < 3 ); return _offset[i]; };
    float power( unsigned short i ) const  { assert( i < 3 ); return _power[i]; };

    void saturation( float s ) { _saturation = s; };
    float saturation() const { return _saturation; };

  protected:
    float _slope[3];
    float _offset[3];
    float _power[3];

    float _saturation;
};

} // namespace ACES

#endif
