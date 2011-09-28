
#include <iostream>
#include <exception>
#include <assert.h>
#include <sstream>

#include <CtlSimdInterpreter.h>
#include <CtlFunctionCall.h>
// #include <ImathMath.h>
// #include <IexBaseExc.h>

using namespace std;
using namespace Ctl;

void usage(const char** argv)
{
  cerr << argv[0] << " - v1.0" << endl
       << endl
       << endl
       << "A simple parser to validate ctl transforms." << endl
       << endl
       << "Example:" << endl
       << endl
       << argv[0] << " transform_RRT.ctl" << endl
       << endl;
}

int main( const int argc, const char** argv )
{
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
      cerr << "ERROR -- caught exception: " << endl << e.what() << endl;
      return 1;
    }
  return 0;
}
