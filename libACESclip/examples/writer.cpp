/* 
Copyright (c) 2015, Gonzalo Garramuño
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
*/

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
