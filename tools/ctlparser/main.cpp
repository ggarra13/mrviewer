
#include <iostream>
#include <exception>
#include <assert.h>
#include <sstream>

#include <CtlSimdInterpreter.h>
#include <CtlFunctionCall.h>


#include <libintl.h>
#define _(String)  gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

// #include <ImathMath.h>
// #include <IexBaseExc.h>

using namespace std;
using namespace Ctl;

void usage(const char** argv)
{
  cerr << argv[0] << " - v1.0" << endl
       << endl
       << endl
       << _("A simple parser to validate ctl transforms.") << endl
       << endl
       << _("Example:") << endl
       << endl
       << argv[0] << N_(" transform_RRT") << endl
       << endl;
}

int main( const int argc, const char** argv )
{
   
   setlocale( LC_ALL, "" );
   bindtextdomain( "ctlparser", "/usr/share/locale" );
   textdomain( "ctlparser" );

  if ( argc != 2 )
    {
      usage(argv);
      return 1;
    }

  try
    {
      SimdInterpreter interp;
      interp.loadModule( argv[1] );
    }
  catch(const std::exception &e)
    {
       cerr << _("ERROR -- caught exception: ") << endl << e.what() << endl;
      return 1;
    }
  return 0;
}
