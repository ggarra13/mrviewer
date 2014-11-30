/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//
//  Copyright (c) 2004, Gonzalo Garramuno
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//  *       Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  *       Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following disclaimer
//  in the documentation and/or other materials provided with the
//  distribution.
//  *       Neither the name of Gonzalo Garramuno nor the names of
//  its other contributors may be used to endorse or promote products derived
//  from this software without specific prior written permission. 
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//


#include <ctime>

#include <fstream>
#include <string>
#include <algorithm>

#include "mrStackTrace_win32.h"


#define gle (GetLastError())
#define lenof(a) (sizeof(a) / sizeof((a)[0]))
#define MAXNAMELEN 1024 // max name length for found symbols
#define IMGSYMLEN ( sizeof IMAGEHLP_SYMBOL )
#define TTBUFLEN 65536 // for a temp buffer

#define EH  ExceptionHandler

#if defined(_M_X64) || defined(__x86_64__)
#  define ARCH_X86_64
#endif

namespace mr {


// SymCleanup()
EH::tSC            ExceptionHandler::pSC = NULL;

// SymFunctionTableAccess()
EH::tSFTA ExceptionHandler::pSFTA = NULL;

// SymGetLineFromAddr()
EH::tSGLFA    ExceptionHandler::pSGLFA = NULL;

// SymGetModuleBase()
EH::tSGMB      ExceptionHandler::pSGMB = NULL;

// SymGetModuleInfo()
EH::tSGMI      ExceptionHandler::pSGMI = NULL;

// SymGetOptions()
EH::tSGO         ExceptionHandler::pSGO = NULL;

// SymGetSymFromAddr()
EH::tSGSFA     ExceptionHandler::pSGSFA = NULL;

// SymInitialize()
EH::tSI         ExceptionHandler::pSI = NULL;

// SymLoadModule()
EH::tSLM         ExceptionHandler::pSLM = NULL;

// SymSetOptions()
EH::tSSO         ExceptionHandler::pSSO = NULL;

// StackWalk()
EH::tSW             ExceptionHandler::pSW = NULL;

// UnDecorateSymbolName()
EH::tUDSN  ExceptionHandler::pUDSN = NULL;

HINSTANCE              ExceptionHandler::hImagehlpDll = NULL;
TCHAR                  ExceptionHandler::m_szLogFileName[MAX_PATH];
LPTOP_LEVEL_EXCEPTION_FILTER ExceptionHandler::m_previousFilter = NULL;




/* Pauses for a specified number of milliseconds. */
void sleep( clock_t wait )
{
   clock_t goal;
   goal = wait + clock();
   while( goal > clock() )
      ;
}


ExceptionHandler::ExceptionHandler()
{
   if (!InitImagehlpFunctions())
   {
      fprintf( stderr, "Could not init imagehlp.dll.");
   }
   else
   {
      m_previousFilter = SetUnhandledExceptionFilter( Filter );
   }
   
   // Figure out what the report file will be named, and store it away
   GetModuleFileName( 0, m_szLogFileName, MAX_PATH );
   
   // Look for the '.' before the "EXE" extension.  Replace the extension
   // with "RPT"
   PTSTR pszDot = _tcsrchr( m_szLogFileName, _T('.') );
   if ( pszDot )
   {
      pszDot++;   // Advance past the '.'
      if ( _tcslen(pszDot) >= 3 )
	 _tcscpy( pszDot, _T("RPT") );   // "RPT" -> "Report"
   }
}

ExceptionHandler::~ExceptionHandler()
{
   SetUnhandledExceptionFilter( m_previousFilter );
   FreeLibrary( hImagehlpDll );
}

void ExceptionHandler::SetLogFileName( PTSTR pszLogFileName )
{
    _tcscpy( m_szLogFileName, pszLogFileName );
}


BOOL ExceptionHandler::InitImagehlpFunctions( void )
{
   // we load imagehlp.dll dynamically because the NT4-version does not
   // offer all the functions that are in the NT5 lib
   hImagehlpDll = LoadLibrary( "imagehlp.dll" );
   if ( hImagehlpDll == NULL )
   {
      fprintf( stderr,  "LoadLibrary( \"imagehlp.dll\" ): gle = %lu\n", gle );
      return FALSE;
   }

   pSC = (tSC) GetProcAddress( hImagehlpDll, "SymCleanup" );
   pSFTA = (tSFTA) GetProcAddress( hImagehlpDll, "SymFunctionTableAccess" );
   pSGLFA = (tSGLFA) GetProcAddress( hImagehlpDll, "SymGetLineFromAddr" );
   pSGMB = (tSGMB) GetProcAddress( hImagehlpDll, "SymGetModuleBase" );
   pSGMI = (tSGMI) GetProcAddress( hImagehlpDll, "SymGetModuleInfo" );
   pSGO = (tSGO) GetProcAddress( hImagehlpDll, "SymGetOptions" );
   pSGSFA = (tSGSFA) GetProcAddress( hImagehlpDll, "SymGetSymFromAddr" );
   pSI = (tSI) GetProcAddress( hImagehlpDll, "SymInitialize" );
   pSSO = (tSSO) GetProcAddress( hImagehlpDll, "SymSetOptions" );
   pSW = (tSW) GetProcAddress( hImagehlpDll, "StackWalk" );
   pUDSN = (tUDSN) GetProcAddress( hImagehlpDll, "UnDecorateSymbolName" );
   pSLM = (tSLM) GetProcAddress( hImagehlpDll, "SymLoadModule" );

   if ( pSC == NULL || pSFTA == NULL || pSGMB == NULL || pSGMI == NULL ||
	pSGO == NULL || pSGSFA == NULL || pSI == NULL || pSSO == NULL ||
	pSW == NULL || pUDSN == NULL || pSLM == NULL )
   {
      fprintf( stderr,  "GetProcAddress(): some required function not found." );
      FreeLibrary( hImagehlpDll );
      return FALSE;
   }

   return TRUE;
}



// if you use C++ exception handling: install a translator function
// with set_se_translator(). In the context of that function (but *not*
// afterwards), you can either do your stack dump, or save the CONTEXT
// record as a local copy. Note that you must do the stack sump at the
// earliest opportunity, to avoid the interesting stackframes being gone
// by the time you do the dump.
LONG WINAPI ExceptionHandler::Filter( EXCEPTION_POINTERS *ep )
{
   HANDLE hThread;

   DuplicateHandle( GetCurrentProcess(), GetCurrentThread(),
		    GetCurrentProcess(), &hThread, 0, false,
		    DUPLICATE_SAME_ACCESS );
   ShowStack( hThread, *(ep->ContextRecord) );
   CloseHandle( hThread );

   return EXCEPTION_EXECUTE_HANDLER;
}







void ExceptionHandler::ShowStack( HANDLE hThread, CONTEXT& c )
{
   // normally, call ImageNtHeader() and use machine info from PE header
   DWORD imageType = IMAGE_FILE_MACHINE_I386;
   // hProcess normally comes from outside
   HANDLE hProcess = GetCurrentProcess(); 
   int frameNum; // counts walked frames
   DWORD offsetFromSymbol; // tells us how far from the symbol we were
   DWORD symOptions; // symbol handler settings
   IMAGEHLP_SYMBOL *pSym = (IMAGEHLP_SYMBOL *)malloc( IMGSYMLEN + MAXNAMELEN );
   char undName[MAXNAMELEN]; // undecorated name
   char undFullName[MAXNAMELEN]; // undecorated name with all shenanigans
   IMAGEHLP_MODULE Module;
   IMAGEHLP_LINE Line;
   std::string symSearchPath;
   char *tt = 0, *p;

   STACKFRAME s; // in/out stackframe
   memset( &s, '\0', sizeof s );

   // NOTE: normally, the exe directory and the current directory should be
   //       taken from the target process. The current dir would be gotten
   //       through injection of a remote thread; the exe fir through either
   //       ToolHelp32 or PSAPI.
   
   // this is a _sample_. you can do the error checking yourself.
   tt = new char[TTBUFLEN]; 

   // build symbol search path from:
   symSearchPath = "";
   // current directory
   if ( GetCurrentDirectory( TTBUFLEN, tt ) )
   {
      symSearchPath += tt;
      symSearchPath += ";";
   }
   // dir with executable
   if ( GetModuleFileName( 0, tt, TTBUFLEN ) )
   {
      for ( p = tt + strlen( tt ) - 1; p >= tt; -- p )
      {
	 // locate the rightmost path separator
	 if ( *p == '\\' || *p == '/' || *p == ':' )
	    break;
      }
      // if we found one, p is pointing at it; if not, tt only contains
      // an exe name (no path), and p points before its first byte
      if ( p != tt ) // path sep found?
      {
	 if ( *p == ':' ) // we leave colons in place
	    ++ p;
	 *p = '\0'; // eliminate the exe name and last path sep
	 symSearchPath += tt;
	 symSearchPath += ";";
      }
   }
   // environment variable _NT_SYMBOL_PATH
   if ( GetEnvironmentVariable( "_NT_SYMBOL_PATH", tt, TTBUFLEN ) )
   {
      symSearchPath += tt;
      symSearchPath += ";";
   }
   // environment variable _NT_ALTERNATE_SYMBOL_PATH
   if ( GetEnvironmentVariable( "_NT_ALTERNATE_SYMBOL_PATH", tt, TTBUFLEN ) )
   {
      symSearchPath += tt;
      symSearchPath += ";";
   }
   // environment variable SYSTEMROOT
   if ( GetEnvironmentVariable( "SYSTEMROOT", tt, TTBUFLEN ) )
   {
      symSearchPath += tt;
      symSearchPath += ";";
   }
   
   // if we added anything, we have a trailing semicolon
   if ( symSearchPath.size() > 0 )
      symSearchPath = symSearchPath.substr( 0, symSearchPath.size() - 1 );

   std::ofstream log( m_szLogFileName );

   log << "symbols path: " << symSearchPath << std::endl;

   // why oh why does SymInitialize() want a writeable string?
   strncpy( tt, symSearchPath.c_str(), TTBUFLEN );
   // if strncpy() overruns, it doesn't add the null terminator
   tt[TTBUFLEN - 1] = '\0'; 

   // init symbol handler stuff (SymInitialize())
   if ( ! pSI( hProcess, tt, false ) )
   {
      fprintf( stderr,  "SymInitialize(): gle = %lu\n", gle );
      goto cleanup;
   }

   // SymGetOptions()
   symOptions = pSGO();
   symOptions |= SYMOPT_LOAD_LINES;
   symOptions &= ~SYMOPT_UNDNAME;
   pSSO( symOptions ); // SymSetOptions()

   // Enumerate modules and tell imagehlp.dll about them.
   // On NT, this is not necessary, but it won't hurt.
   // Well, actually, when debugging DLLs as we are, we do need to do this.
   enumAndLoadModuleSymbols( hProcess, GetCurrentProcessId() );

   // init STACKFRAME for first call
   // Notes: AddrModeFlat is just an assumption. I hate VDM debugging.
   // Notes: will have to be #ifdef-ed for Alphas; MIPSes are dead anyway,
   // and good riddance.

   s.AddrPC.Mode = AddrModeFlat;
#ifdef ARCH_X86_64
#else
   s.AddrPC.Offset = c.Eip;
   s.AddrFrame.Offset = c.Ebp;
#endif
   s.AddrFrame.Mode = AddrModeFlat;

   memset( pSym, '\0', IMGSYMLEN + MAXNAMELEN );
   pSym->SizeOfStruct = IMGSYMLEN;
   pSym->MaxNameLength = MAXNAMELEN;

   memset( &Line, '\0', sizeof Line );
   Line.SizeOfStruct = sizeof Line;

   memset( &Module, '\0', sizeof Module );
   Module.SizeOfStruct = sizeof Module;

   offsetFromSymbol = 0;

   log << std::endl;
   log << "--# FV EIP----- RetAddr- FramePtr StackPtr Symbol" << std::endl;

   char tmp[256];
   for ( frameNum = 0; ; ++ frameNum )
   {
      // get next stack frame (StackWalk(), SymFunctionTableAccess(),
      //                       SymGetModuleBase())
      // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998),
      // you can assume that either you are done, or that the stack is so
      // hosed that the next deeper frame could not be found.

#ifdef ARCH_X86_64
#else
      if ( ! pSW( imageType, hProcess, hThread, &s, &c, NULL,
		  pSFTA, pSGMB, NULL ) )
	 break;
#endif
      // display its contents
      sprintf( tmp, "%3d %c%c %08lx %08lx %08lx %08lx ",
	       frameNum, s.Far? 'F': '.', s.Virtual? 'V': '.',
	       s.AddrPC.Offset, s.AddrReturn.Offset,
	       s.AddrFrame.Offset, s.AddrStack.Offset );

      if ( s.AddrPC.Offset == 0 )
      {
	 sprintf( tmp, "%s(-nosymbols- PC == 0)", tmp );
	 log << tmp << std::endl;
      }
      else
      { // we seem to have a valid PC
	 // show procedure info (SymGetSymFromAddr())
	 if ( ! pSGSFA( hProcess, s.AddrPC.Offset, &offsetFromSymbol, pSym ) )
	 {
	    if ( gle != 487 )
	       fprintf( stderr,  "SymGetSymFromAddr(): gle = %lu\n", gle );
	 }
	 else
	 {
				// UnDecorateSymbolName()
	    pUDSN( pSym->Name, undName, MAXNAMELEN, UNDNAME_NAME_ONLY );
	    pUDSN( pSym->Name, undFullName, MAXNAMELEN, UNDNAME_COMPLETE );
	    sprintf( tmp, "%s%s", tmp, undName );
	    if ( offsetFromSymbol != 0 )
	       sprintf( tmp, "%s%+ld bytes", tmp, (long) offsetFromSymbol );
	    log << tmp << std::endl;
	    log << "    Sig:  " << pSym->Name << std::endl;
	    log << "    Decl: " << undFullName << std::endl;
	 }

	 // show line number info, NT5.0-method (SymGetLineFromAddr())
	 if ( pSGLFA != NULL )
	 { // yes, we have SymGetLineFromAddr()
	    if ( ! pSGLFA( hProcess, s.AddrPC.Offset, &offsetFromSymbol,
			   &Line ) )
	    {
	       if ( gle != 487 )
		  fprintf( stderr,  "SymGetLineFromAddr(): gle = %lu\n", gle );
	    }
	    else
	    {
	       sprintf( tmp,"    Line: %s(%lu) %+ld bytes",
			Line.FileName, Line.LineNumber,
			offsetFromSymbol );
	       log << tmp << std::endl;
	    }
	 } // yes, we have SymGetLineFromAddr()

	 // show module info (SymGetModuleInfo())
	 if ( ! pSGMI( hProcess, s.AddrPC.Offset, &Module ) )
	 {
	    fprintf( stderr,  "SymGetModuleInfo): gle = %lu\n", gle );
	 }
	 else
	 { // got module info OK
	    char ty[80];
	    switch ( Module.SymType )
	    {
	       case SymNone:
		  strcpy( ty, "-nosymbols-" );
		  break;
	       case SymCoff:
		  strcpy( ty, "COFF" );
		  break;
	       case SymCv:
		  strcpy( ty, "CV" );
		  break;
	       case SymPdb:
		  strcpy( ty, "PDB" );
		  break;
	       case SymExport:
		  strcpy( ty, "-exported-" );
		  break;
	       case SymDeferred:
		  strcpy( ty, "-deferred-" );
		  break;
	       case SymSym:
		  strcpy( ty, "SYM" );
		  break;
	       default:
		  sprintf( ty, "symtype=%ld", (long) Module.SymType );
//  		  _snfprintf( stderr,  ty, sizeof ty, "symtype=%ld",
//  			     (long) Module.SymType );
		  break;
	    }

	    sprintf( tmp, "    Mod:  %s[%s], base: %08lxh",
		    Module.ModuleName, Module.ImageName, Module.BaseOfImage );
	    log << tmp << std::endl;
	    sprintf( tmp, "    Sym:  type: %s, file: %s",
		     ty, Module.LoadedImageName );
	    log << tmp << std::endl << std::endl;
	 } // got module info OK
      } // we seem to have a valid PC
      
      // no return address means no deeper stackframe
      if ( s.AddrReturn.Offset == 0 )
      {
	 // avoid misunderstandings in the fprintf( stderr, ) following the loop
	 SetLastError( 0 );
	 break;
      }

   } // for ( frameNum )

   if ( gle != 0 )
      fprintf( stderr,  "\nStackWalk(): gle = %lu\n", gle );

cleanup:
   // sleep( (clock_t)120 * CLOCKS_PER_SEC );
   
   ResumeThread( hThread );
   // de-init symbol handler etc. (SymCleanup())
   pSC( hProcess );
   free( pSym );
   delete [] tt;
}



void ExceptionHandler::enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid )
{
   ModuleList modules;
   ModuleListIter it;
   char *img, *mod;

   // fill in module list
   fillModuleList( modules, pid, hProcess );

   fprintf(stderr, "ExceptionHandler: Loading DLL symbols.  Please wait...");
   for ( it = modules.begin(); it != modules.end(); ++ it )
   {
      // unfortunately, SymLoadModule() wants writeable strings
      img = new char[(*it).imageName.size() + 1];
      strcpy( img, (*it).imageName.c_str() );
      mod = new char[(*it).moduleName.size() + 1];
      strcpy( mod, (*it).moduleName.c_str() );

      if ( pSLM( hProcess, 0, img, mod, (*it).baseAddress, (*it).size ) == 0 )
	 fprintf( stderr,  "Error %lu loading symbols for \"%s\"\n",
		   gle, (*it).moduleName.c_str() );

      delete [] img;
      delete [] mod;
   }
}



bool ExceptionHandler::fillModuleList( EH::ModuleList& modules,
				       DWORD pid, HANDLE hProcess )
{
   // try toolhelp32 first
   if ( fillModuleListTH32( modules, pid ) )
      return true;
   // nope? try psapi, then
   return fillModuleListPSAPI( modules, pid, hProcess );
}



// miscellaneous toolhelp32 declarations; we cannot #include the header
// because not all systems may have it
#define MAX_MODULE_NAME32 255
#define TH32CS_SNAPMODULE   0x00000008
#pragma pack( push, 8 )
typedef struct tagMODULEENTRY32
{
     DWORD   dwSize;
     DWORD   th32ModuleID;       // This module
     DWORD   th32ProcessID;      // owning process
     DWORD   GlblcntUsage;       // Global usage count on the module
     DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
     BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
     DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
     HMODULE hModule;            // The hModule of this module in th32ProcessID's context
     char    szModule[MAX_MODULE_NAME32 + 1];
     char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;
#pragma pack( pop )



bool ExceptionHandler::fillModuleListTH32( EH::ModuleList& modules, DWORD pid )
{
   // CreateToolhelp32Snapshot()
   typedef HANDLE (__stdcall *tCT32S)( DWORD dwFlags, DWORD th32ProcessID );
   // Module32First()
   typedef BOOL (__stdcall *tM32F)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );
   // Module32Next()
   typedef BOOL (__stdcall *tM32N)( HANDLE hSnapshot, LPMODULEENTRY32 lpme );

	// I think the DLL is called tlhelp32.dll on Win9X, so we try both
   const char *dllname[] = { "kernel32.dll", "tlhelp32.dll" };
   HINSTANCE hToolhelp;
   tCT32S pCT32S;
   tM32F pM32F;
   tM32N pM32N;

   HANDLE hSnap;
   MODULEENTRY32 me = { sizeof me };
   bool keepGoing;
   ModuleEntry e;
   int i;

   for ( i = 0; i < lenof( dllname ); ++ i )
   {
      hToolhelp = LoadLibrary( dllname[i] );
      if ( hToolhelp == 0 )
	 continue;
      pCT32S = (tCT32S) GetProcAddress( hToolhelp,
					"CreateToolhelp32Snapshot" );
      pM32F = (tM32F) GetProcAddress( hToolhelp, "Module32First" );
      pM32N = (tM32N) GetProcAddress( hToolhelp, "Module32Next" );
      if ( pCT32S != 0 && pM32F != 0 && pM32N != 0 )
	 break; // found the functions!
      FreeLibrary( hToolhelp );
      hToolhelp = 0;
   }

   if ( hToolhelp == 0 ) // nothing found?
      return false;

   hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
   if ( hSnap == (HANDLE) -1 )
      return false;

   keepGoing = !!pM32F( hSnap, &me );
   while ( keepGoing )
   {
      // here, we have a filled-in MODULEENTRY32
      fprintf( stderr,  "%08lXh %6lu %-15.15s %s\n", me.modBaseAddr, me.modBaseSize,
		me.szModule, me.szExePath );
      e.imageName = me.szExePath;
      e.moduleName = me.szModule;
      e.baseAddress = (DWORD) PtrToUlong( me.modBaseAddr );
      e.size = me.modBaseSize;
      modules.push_back( e );
      keepGoing = !!pM32N( hSnap, &me );
   }

   CloseHandle( hSnap );

   FreeLibrary( hToolhelp );

   return modules.size() != 0;
}



// miscellaneous psapi declarations; we cannot #include the header
// because not all systems may have it
typedef struct _MODULEINFO {
     LPVOID lpBaseOfDll;
     DWORD SizeOfImage;
     LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;



bool ExceptionHandler::fillModuleListPSAPI( EH::ModuleList& modules,
					    DWORD pid, HANDLE hProcess )
{
   // EnumProcessModules()
   typedef BOOL (__stdcall *tEPM)( HANDLE hProcess, HMODULE *lphModule,
				   DWORD cb, LPDWORD lpcbNeeded );
   // GetModuleFileNameEx()
   typedef DWORD (__stdcall *tGMFNE)( HANDLE hProcess, HMODULE hModule,
				      LPSTR lpFilename, DWORD nSize );
   // GetModuleBaseName() -- redundant, as GMFNE() has the same prototype,
   //                        but who cares?
   typedef DWORD (__stdcall *tGMBN)( HANDLE hProcess, HMODULE hModule,
				     LPSTR lpFilename, DWORD nSize );
   // GetModuleInformation()
   typedef BOOL (__stdcall *tGMI)( HANDLE hProcess, HMODULE hModule,
				   LPMODULEINFO pmi, DWORD nSize );

   HINSTANCE hPsapi;
   tEPM pEPM;
   tGMFNE pGMFNE;
   tGMBN pGMBN;
   tGMI pGMI;

   unsigned int i;
   ModuleEntry e;
   DWORD cbNeeded;
   MODULEINFO mi;
   HMODULE *hMods = 0;
   char *tt = 0;

   hPsapi = LoadLibrary( "psapi.dll" );
   if ( hPsapi == 0 )
      return false;

   modules.clear();

   pEPM = (tEPM) GetProcAddress( hPsapi, "EnumProcessModules" );
   pGMFNE = (tGMFNE) GetProcAddress( hPsapi, "GetModuleFileNameExA" );
   pGMBN = (tGMFNE) GetProcAddress( hPsapi, "GetModuleBaseNameA" );
   pGMI = (tGMI) GetProcAddress( hPsapi, "GetModuleInformation" );
   if ( pEPM == 0 || pGMFNE == 0 || pGMBN == 0 || pGMI == 0 )
   {
      // yuck. Some API is missing.
      FreeLibrary( hPsapi );
      return false;
   }

   hMods = new HMODULE[TTBUFLEN / sizeof HMODULE];
   tt = new char[TTBUFLEN];
   // not that this is a sample. Which means I can get away with
   // not checking for errors, but you cannot. :)

   if ( ! pEPM( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
   {
      fprintf( stderr,  "EPM failed, gle = %lu\n", gle );
      goto cleanup;
   }

   if ( cbNeeded > TTBUFLEN )
   {
      fprintf( stderr,  "More than %lu module handles. Huh?\n", lenof( hMods ) );
      goto cleanup;
   }

   for ( i = 0; i < cbNeeded / sizeof hMods[0]; ++ i )
   {
      // for each module, get:
      // base address, size
      pGMI( hProcess, hMods[i], &mi, sizeof mi );
      e.baseAddress = (DWORD) PtrToUlong( mi.lpBaseOfDll );
      e.size = mi.SizeOfImage;
      // image file name
      tt[0] = '\0';
      pGMFNE( hProcess, hMods[i], tt, TTBUFLEN );
      e.imageName = tt;
      // module name
      tt[0] = '\0';
      pGMBN( hProcess, hMods[i], tt, TTBUFLEN );
      e.moduleName = tt;
      fprintf( stderr,  "%08lXh %6lu %-15.15s %s\n", e.baseAddress,
	       e.size, e.moduleName.c_str(), e.imageName.c_str() );

      modules.push_back( e );
   }

cleanup:
   if ( hPsapi )
      FreeLibrary( hPsapi );
   delete [] tt;
   delete [] hMods;

   return modules.size() != 0;
}

} // namespace mr
