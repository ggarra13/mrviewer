
#include "FL/Fl.H"
#include "FL/fl_draw.H"
#include "gui/mrvIO.h"
#include "gui/mrvColorSchemes.h"

namespace {
    const char* kModule = "themes";
}

namespace mrv
{
    std::string ColorSchemes::remove_comments( std::string line )
    {
        size_t pos = line.find( "#" );
        if ( pos != std::string::npos )
        {
            return line.substr( 0, pos );
        }
        return line;
    }

    ColorSchemes::ColorSchemes()
    {
    }

    bool
    ColorSchemes::read_themes( const char* file )
    {
        FILE* f = fopen( file, "r" );
        if (!f) {
            return false;
        }
        char line[256];
        while ( fgets( line, sizeof(line), f ) != NULL )
        {
            std::string text = remove_comments( line );
            size_t pos = text.find( "theme" );
            if ( pos != std::string::npos )
            {
                pos = text.find( '"', pos+1 );
                size_t pos2 = text.find( '"', pos+1 );
                if ( pos2 == std::string::npos )
                {
                    pos2 = text.size();
                }

                std::string name = text.substr( pos+1, pos2-pos-1 );
                themes.push_back( Theme( name ) );


                while ( fgets( line, sizeof(line), f ) != NULL )
                {
                    text = line;
                    if ( text.find( '{' ) == std::string::npos )
                        continue;
                    break;
                }

                if ( ! read_colors( f, themes.back() ) )
                    return false;
            }
        }
        fclose( f );

        return true;
    }

    bool ColorSchemes::read_colors( FILE* f, Theme& scheme )
    {
        char line[256];
        while ( fgets( line, sizeof(line), f ) != NULL )
        {
            std::string text = remove_comments( line );
            size_t pos = text.rfind( '}' );
            if ( pos != std::string::npos ) break;

            char cmap[24];
            int idx, ri, gi, bi;
            int num = sscanf( text.c_str(), "%s %d %d %d %d",
                              cmap, &idx, &ri, &gi, &bi );
            if ( num != 5 ) {
                continue;
            }
            uchar r, g, b;
            r = (uchar)ri; g = (uchar)gi; b = (uchar)bi;
            Fl_Color c = fl_rgb_color( r, g, b );
            scheme.colormaps.insert( std::make_pair( idx, c ) );
        }
        return true;
    }

    void ColorSchemes::apply_colors( std::string name )
    {
        for ( auto& s : themes )
        {
            if ( s.name == name )
            {
                for ( auto& c: s.colormaps )
                {
                    Fl::set_color( c.first, Fl_Color(c.second) );
                }
            }
        }
    }

}
