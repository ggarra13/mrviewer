/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "mrvIO.h"
#include "core/mrvI8N.h"

#include <sys/types.h>

#if defined(LINUX) || defined(OSX)
#include <sys/socket.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#endif

#include "mrSocket.h"

#define QLEN            6               /* size of request queue        */

namespace {
const char* kModule = "sock";
}


bool mr_init_socket_library()
{
#if defined(_WIN32) || defined(_WIN64)
#undef fprintf
   // Initialize winsock
   WORD wVersionRequested = MAKEWORD(1,1);
   WSADATA wsaData;
   int nRet;

   nRet = WSAStartup(wVersionRequested, &wsaData);
   if ( nRet != 0 )
   {
      switch( nRet )
      {
         case WSASYSNOTREADY:
             fprintf(stderr, _("WSA not ready\n") ); break;
         case WSAVERNOTSUPPORTED:
             fprintf(stderr, _("The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.\n") ); break;
         case WSAEINPROGRESS:
             fprintf( stderr, _("A blocking Windows Sockets 1.1 operation is in progress.\n") );
            break;
         case WSAEPROCLIM:
             fprintf(stderr, _("Limit on the number of tasks supported by the Windows Sockets implementation has been reached.\n") );
            break;
      }
      fflush(stderr);
   }

   if (wsaData.wVersion != wVersionRequested)
   {
      WSACleanup();
      LOG_ERROR("Wrong winsock version\n");
      return false;
   }
#endif
   return true;
}


void mr_cleanup_socket_library()
{
#if defined(_WIN32) || defined(_WIN64)
   // release winsock
   WSACleanup();
#endif
}


MR_SOCKET mr_new_socket_client( const char* hostname, const int port,
                                const char* protocol )
{

    int err;
    MR_SOCKET sd;            /* socket descriptor                   */
    struct addrinfo hints = {}, *addrs;
    char port_str[16] = {};

    hints.ai_family = AF_INET; // Since your original code was using sockaddr_in and
                               // PF_INET, I'm using AF_INET here to match.  Use
                               // AF_UNSPEC instead if you want to allow getaddrinfo()
                               // to find both IPv4 and IPv6 addresses for the hostname.
                               // Just make sure the rest of your code is equally family-
                               // agnostic when dealing with the IP addresses associated
                               // with this connection. For instance, make sure any uses
                               // of sockaddr_in are changed to sockaddr_storage,
                               // and pay attention to its ss_family field, etc...
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    sprintf(port_str, "%d", port);

    err = getaddrinfo(hostname, port_str, &hints, &addrs);
    if (err != 0)
    {
        fprintf(stderr, "%s: %s\n", hostname, gai_strerror(err));
        abort();
    }

    for(struct addrinfo *addr = addrs; addr != NULL; addr = addr->ai_next)
    {
        sd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sd == -1)
        {
            // if using AF_UNSPEC above instead of AF_INET/6 specifically,
            // replace this 'break' with 'continue' instead, as the 'ai_family'
            // may be different on the next iteration...
            err = errno;
            break;
        }

        if (connect(sd, addr->ai_addr, (int)addr->ai_addrlen) == 0)
            break;

        err = errno;

        mr_closesocket(sd);
        sd = -1;
    }

    freeaddrinfo(addrs);



    if (sd == -1)
    {
        LOG_ERROR( hostname << ": " <<  strerror(err));
    }


   return sd;
}


void mr_closesocket( MR_SOCKET sd )
{
   closesocket(sd);
}

MR_SOCKET mr_new_socket_server( const char* host, const int port,
                                const char* protocol )
{
   struct  protoent *ptrp;  /* pointer to a protocol table entry   */
   struct  sockaddr_in sad; /* structure to hold server's address  */
   MR_SOCKET   sd;          /* socket descriptor                   */

   memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
   sad.sin_family = AF_INET;         /* set family to Internet     */
   sad.sin_addr.s_addr = htonl( INADDR_ANY ); /* set the local IP address   */

   if (port > 0)                   /* test for illegal value       */
      sad.sin_port = htons((u_short)port);
   else {                          /* print error message and exit */
       LOG_ERROR( _("Bad port number ") << port);
      return -1;
   }

   /* Map TCP transport protocol name to protocol number */
   if ( (ptrp = getprotobyname( protocol )) == 0) {
       LOG_ERROR( _("Cannot map \"") << protocol << _("\" to protocol number"));
      return -1;
   }

   /* Create a socket */
   sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
   if (sd < 0) {
       LOG_ERROR( _("Socket creation failed") );
      return -1;
   }

   /* Bind a local address to the socket */
   if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
       LOG_ERROR( _("bind socket failed\n") );
      closesocket(sd);
      return -1;
   }

   /* Specify size of request queue */
   if (listen(sd, QLEN) < 0)
   {
       LOG_ERROR( _("listen to socket failed\n") );
      closesocket(sd);
      return -1;
   }

   return sd;
}
