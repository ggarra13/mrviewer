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

#ifndef mrStackTrace_win32_h
#define mrStackTrace_win32_h

//#include <math.h>
#include <vector>
#include <windows.h>
#include <tchar.h>
#include <imagehlp.h>

#undef min
#undef max


namespace mr 
{

//! Exception handler
class ExceptionHandler
{
   public:

     //! Initialize Exception Handler
     ExceptionHandler( );
     //! Destroy exception handler and restore original exceptions
     ~ExceptionHandler( );

     //! Show a Stack Trace so far.
     static void ShowStack( HANDLE hThread, CONTEXT& c ); // dump a stack

     //! Set log file to dump trace and info in case of crash.
     //! Default name is name of the main program.
     static void SetLogFileName( PTSTR pszLogFileName );

   private:

     static void write(const char* c, ...);
     
     struct ModuleEntry
     {
	  std::string imageName;
	  std::string moduleName;
	  DWORD baseAddress;  // Should be UINT_PTR?
	  DWORD size;
     };
     typedef std::vector< ModuleEntry > ModuleList;
     typedef ModuleList::iterator ModuleListIter;
     
     static LONG WINAPI Filter( EXCEPTION_POINTERS *ep );
     static void enumAndLoadModuleSymbols( HANDLE hProcess, DWORD pid );
     static bool fillModuleList( ModuleList& modules, DWORD pid,
				 HANDLE hProcess );
     static bool fillModuleListTH32( ModuleList& modules, DWORD pid );
     static bool fillModuleListPSAPI( ModuleList& modules, DWORD pid,
				      HANDLE hProcess );

     static BOOL InitImagehlpFunctions( void );

     // Variables used by the class

     // Make typedefs for some IMAGEHLP.DLL functions so that we can use them
     // with GetProcAddress
     typedef BOOL (__stdcall *tSC)( IN HANDLE hProcess );
     
     typedef PVOID (__stdcall *tSFTA)
     ( HANDLE hProcess, DWORD AddrBase );
     
     typedef BOOL (__stdcall *tSGLFA)
     ( IN HANDLE hProcess, IN DWORD dwAddr,
       OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE Line );
     
     typedef DWORD (__stdcall *tSGMB)( HANDLE, DWORD );
     
     typedef BOOL (__stdcall *tSGMI)
     ( IN HANDLE hProcess, IN DWORD dwAddr, OUT PIMAGEHLP_MODULE ModuleInfo );
     
     typedef DWORD (__stdcall *tSGO)( VOID );
     
     typedef BOOL (__stdcall *tSGSFA)
     ( IN HANDLE hProcess, IN DWORD dwAddr,
       OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_SYMBOL Symbol );
     
     typedef BOOL (__stdcall * tSI)( HANDLE, LPSTR, BOOL );
     
     typedef DWORD (__stdcall *tSLM)
     ( IN HANDLE hProcess, IN HANDLE hFile,
       IN PSTR ImageName, IN PSTR ModuleName,
       IN DWORD BaseOfDll, IN DWORD SizeOfDll );
     
     typedef DWORD (__stdcall *tSSO)( IN DWORD SymOptions );

     typedef BOOL (__stdcall * tSW)
     ( DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID,
       PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE,
       PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE );

     typedef DWORD (__stdcall WINAPI *tUDSN)
     ( PCSTR DecoratedName, PSTR UnDecoratedName,
       DWORD UndecoratedLength, DWORD Flags );

     typedef BOOL (__stdcall *tSSP)( HANDLE, LPSTR, DWORD );
     
     static HINSTANCE                                 hImagehlpDll;
     static TCHAR                        m_szLogFileName[MAX_PATH];
     static LPTOP_LEVEL_EXCEPTION_FILTER          m_previousFilter;

     static tSC            pSC;
     static tSFTA        pSFTA;
     static tSGLFA      pSGLFA;
     static tSGMB        pSGMB;
     static tSGMI        pSGMI;
     static tSGO          pSGO;
     static tSGSFA      pSGSFA;
     static tSI            pSI;
     static tSLM          pSLM;
     static tSSO          pSSO;
     static tSW            pSW;
     static tUDSN        pUDSN;
//       static tSSP           pSSP;

};

}


// global instance of class
extern mr::ExceptionHandler* gExceptionHandler;


#endif  // mrStackTrace_win32_h

