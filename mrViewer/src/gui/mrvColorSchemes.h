
#ifndef mrvColorSchemes_h
#define mrvColorSchemes_h

#include <iostream>
#include <map>
#include <vector>
#include <string>

namespace mrv {

    class ColorSchemes
    {
    protected:
        std::string remove_comments( std::string line );
    public:
        struct Theme
        {
            Theme( std::string& n ) : name( n ) { }
            Theme( const Theme& t ) : name( t.name ) { }

            std::string name;
            typedef std::map< int, int > colorMap;
            colorMap colormaps;
        };

        typedef std::vector< Theme > themeArray;
        themeArray themes;

        ColorSchemes();
        bool read_themes( const char* file );
        bool read_colors( FILE* f, Theme& scheme );
        void apply_colors( std::string name );
    };

}

#endif // mrvColorSchemes_h
