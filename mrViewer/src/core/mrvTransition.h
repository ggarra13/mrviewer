#ifndef mrvTransition_h
#define mrvTransition_h

#include <vector>

namespace mrv
{
    class Transition
    {
    public:
        enum Type {
            kDissolve
        };

        Transition() :
            _type( kDissolve ),
            _start( 0 ),
            _end( 1 )
            {
            };

        Transition( Type type, int64_t start, int64_t end ) :
            _type( type ),
            _start( start ),
            _end( end )
            {
            };

        Type    type() const { return _type; }
        void type( Type t ) { _type = t; }

        int64_t start() const { return _start; }
        void start( int64_t s ) { _start = s; }

        int64_t end() const { return _end; }
        void end( int64_t e ) { _end = e; }

        int64_t duration() const { return _end - _start + 1; }
        void duration( int64_t e ) { _end = _start + e; }

    protected:
        Type    _type;
        int64_t _start;
        int64_t _end;
    };

    typedef std::vector< Transition > TransitionList;

}  // namespace mrv

#endif // mrvTransition_h
