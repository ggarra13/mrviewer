
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

#ifndef mrStackTrace_linux_h
#define mrStackTrace_linux_h


#include <stdio.h>
#include <signal.h>

namespace mr
{


class ExceptionHandler
{
   public:
     ExceptionHandler();
     ~ExceptionHandler();
     
     static void ShowStack();
   private:
     static void demangle( const char* name );
     static void bt_sighandler(int sig, siginfo_t *info,
			       void *secret);
     void install_signal_handler();
     void restore_signal_handler();

     struct sigaction oldSIGSEGV;
     struct sigaction oldSIGUSR1;
     struct sigaction oldSIGBUS;
     struct sigaction oldSIGILL;
     struct sigaction oldSIGFPE;
     struct sigaction oldSIGABRT;
     struct sigaction oldSIGINT;
     struct sigaction oldSIGCHLD;
     struct sigaction oldSIGTRAP;
     struct sigaction oldSIGSTOP;
};

}

// global instance of class
extern mr::ExceptionHandler gExceptionHandler;

#endif // mrStackTrace_linux_h
