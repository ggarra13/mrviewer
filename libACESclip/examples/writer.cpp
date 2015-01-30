
#include <iostream>

#include "ACESclipWriter.h"


int main( int argc, char** argv )
{
    if ( argc != 2 )
    {
        std::cerr << argv[0] << " v" << ACES::kLibVersion << std::endl
                  << std::endl
                  << argv[0] << " <file>"
                  << std::endl
                  << std::endl
                  << "Example: "
                  << std::endl
                  << std::endl
                  << argv[0] << " ACESclip.output.xml"
                  << std::endl;
        exit(-1);
    }

    ACES::ACESclipWriter c;

    c.info( "mrViewer", "v2.6.9", "Sample file" );
    c.clip_id( "/media/Linux/image/capture.exr",
               "POTC-ad20");
    c.config();

    c.ITL_start();
    c.add_IDT( "IDT.Sony.F60" );
    c.ITL_end();

    c.PTL_start();
    c.add_LMT( "LMT.Sat.1.0.0" );
    c.add_LMT( "LMT.Sat.2.0.0" );
    c.add_RRT( "RRT.a1.0.0" );
    c.add_ODT( "ODT.RGB.Monitor", ACES::kApplied );
    c.PTL_end();

    if ( ! c.save( argv[1] ) )
        std::cerr << "Could not save '" << argv[1] << "'." << std::endl;

    return 0;
}
