
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



#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "mrStackTrace_linux.h"
#include "core/mrvHome.h"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <execinfo.h>


/* get REG_EIP from ucontext.h */ 

#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* Necessary for REG_EIP. Why, I don't know. */
#endif

#include <sys/ucontext.h>
#include <cxxabi.h>  // for demangling function

//! @todo:  verify it works.
//! @todo:  pass functions to demangle() for printing out right
//! @todo:  add logging onto a file
//! @todo:  find if linux stack trace can also print function and line #

namespace mr
{

ExceptionHandler::ExceptionHandler()
{
    // install_signal_handler();
}

ExceptionHandler::~ExceptionHandler()
{
   restore_signal_handler();
}

void ExceptionHandler::ShowStack() {
  void *trace[200];
  char **messages = (char **)NULL;
  int i, trace_size = 0;

  trace_size = backtrace(trace, 200);
  messages = backtrace_symbols(trace, trace_size);
  fprintf( stderr, "[bt] Execution path:\n");
  for (i=0; i<trace_size; ++i)
  {
     fprintf( stderr, "[bt] %s\n", messages[i]);
  }
  free( messages );
}

void ExceptionHandler::demangle( const char* name )
{
   int status;
   char* demangle = abi::__cxa_demangle(name,  0, 0, &status);
   if(status == 0) {
      fprintf( stderr, "%s",demangle);
      free(demangle);
   } else if(status == -2)
      fprintf( stderr, "Error: %s  is not a valid name under the C++ ABI mangling "
	       "rules\n", name);
   else if(status == -1)
      fprintf( stderr, "Error: could not allocate memory.\n");
}

void ExceptionHandler::bt_sighandler(int sig, siginfo_t *info,
				     void *secret)
{

   std::string lockfile = mrv::homepath();
   lockfile += "/.fltk/filmaura/mrViewer.lock.prefs";
   if(fs::exists(lockfile))
   {
      if ( ! fs::remove( lockfile ) )
	 std::cerr << "Could not remove lockfile " << lockfile << std::endl;
   }

   if ( sig == SIGINT ) exit(0);

  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;
  ucontext_t *uc = (ucontext_t *)secret;

  /* Do something useful with siginfo_t */
#ifdef REG_EIP
  /* 32-bit OS */
  if (sig == SIGSEGV)
     fprintf( stderr, "Got signal %d, faulty address is %p, "
	      "from %p\n", sig, info->si_addr, 
	      uc->uc_mcontext.gregs[REG_EIP]);
  else if ( sig == SIGFPE )
     fprintf( stderr, "Got Floating Point Exception.\n");
  else
     fprintf( stderr, "Got signal %d\n", sig);
	
  trace_size = backtrace(trace, 16);
  /* overwrite sigaction with caller's address */
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];

#else
  /* 64-bit OS */
  if (sig == SIGSEGV)
     fprintf( stderr, "Got signal %d, faulty address is %p, "
	      "from %p\n", sig, info->si_addr, 
	      (void*)uc->uc_mcontext.gregs[REG_RIP]);
  else if ( sig == SIGFPE )
     fprintf( stderr, "Got Floating Point Exception.\n");
  else
     fprintf( stderr, "Got signal %d\n", sig);
	
  trace_size = backtrace(trace, 16);

  /* overwrite sigaction with caller's address */
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_RIP];
#endif

  messages = backtrace_symbols(trace, trace_size);

  /* skip first stack frame (points to here) */
  fprintf( stderr, "[bt] Execution path:\n");
  for (i=1; i<trace_size; ++i)
    {
      fprintf( stderr, "[bt] %s\n", messages[i]);
    }
  free( messages );

  exit(0);
}


void ExceptionHandler::install_signal_handler() {

  /* Install our signal handler */
  struct sigaction sa;

  sa.sa_sigaction = bt_sighandler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_SIGINFO;

  sigaction(SIGSEGV, &sa, &oldSIGSEGV);
  sigaction(SIGUSR1, &sa, &oldSIGUSR1);
  sigaction(SIGBUS,  &sa, &oldSIGBUS);
  sigaction(SIGILL,  &sa, &oldSIGILL);
  sigaction(SIGFPE,  &sa, &oldSIGFPE);
  sigaction(SIGABRT, &sa, &oldSIGABRT);
  sigaction(SIGINT, &sa, &oldSIGINT);
  sigaction(SIGCHLD, &sa, &oldSIGCHLD);
  sigaction(SIGTRAP, &sa, &oldSIGTRAP);
  sigaction(SIGSTOP, &sa, &oldSIGSTOP);
  /* ... add any other signal here */
}


void ExceptionHandler::restore_signal_handler() {

  /* Install our signal handler */
  struct sigaction sa;

  sa.sa_sigaction = bt_sighandler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_SIGINFO;

  sigaction(SIGSEGV, &oldSIGSEGV, NULL);
  sigaction(SIGUSR1, &oldSIGUSR1, NULL);
  sigaction(SIGBUS,  &oldSIGBUS,  NULL);
  sigaction(SIGILL,  &oldSIGILL,  NULL);
  sigaction(SIGFPE,  &oldSIGFPE,  NULL);
  sigaction(SIGABRT, &oldSIGABRT, NULL);
  sigaction(SIGINT,  &oldSIGINT, NULL);
  sigaction(SIGCHLD, &oldSIGCHLD, NULL);
  sigaction(SIGTRAP, &oldSIGTRAP, NULL);
  sigaction(SIGSTOP, &oldSIGSTOP, NULL);
  /* ... add any other signal here */
}

}


mr::ExceptionHandler gExceptionHandler;
