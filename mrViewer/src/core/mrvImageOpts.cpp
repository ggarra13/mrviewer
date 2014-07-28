
#include "core/mrvImageOpts.h"
#include "EXROptionsUI.h"
#include "WandOptionsUI.h"

namespace mrv {

ImageOpts* ImageOpts::build( std::string ext )
{
    if ( ext == ".exr" || ext == ".sxr" )
        return new EXROptionsUI();
    return new WandOptionsUI();
}

}
