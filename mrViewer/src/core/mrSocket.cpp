
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "mrvIO.h"
#include "mrSocket.h"

#define QLEN            6               /* size of request queue        */

namespace {
const char* kModule = "sock";
}


bool mr_init_socket_library()
{
#if defined(_WIN32) || defined(_WIN64)
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
	    fprintf(stderr, "WSA not ready\n"); break;
	 case WSAVERNOTSUPPORTED:
	    fprintf(stderr, "The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.\n"); break;
	 case WSAEINPROGRESS:
	    fprintf( stderr, "A blocking Windows Sockets 1.1 operation is in progress.\n");
	    break;
	 case WSAEPROCLIM:
	    fprintf(stderr,"Limit on the number of tasks supported by the Windows Sockets implementation has been reached.\n");
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


MR_SOCKET mr_new_socket_client( const char* host, const int port,
				const char* protocol )
{
   struct  hostent  *ptrh;  /* pointer to a host table entry       */
   struct  protoent *ptrp;  /* pointer to a protocol table entry   */
   struct  sockaddr_in sad; /* structure to hold server's address  */
   MR_SOCKET sd;            /* socket descriptor                   */

   memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
   sad.sin_family = AF_INET;         /* set family to Internet     */


   if (port > 0)                   /* test for illegal value       */
      sad.sin_port = htons((u_short)port);
   else {                          /* print error message and exit */
      LOG_ERROR("Bad port number " << port);
      return -1;
   }

   /* Convert host name to equivalent IP address and copy to sad. */
   ptrh = gethostbyname(host);
   if ( ((char *)ptrh) == NULL ) {
      LOG_ERROR( "Invalid host: " << host);
      return -1;
   }
   memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

   /* Map TCP transport protocol name to protocol number */
   if ( (ptrp = getprotobyname( protocol )) == 0) {
      LOG_ERROR( "Cannot map \"" << protocol << "\" to protocol number");
      return -1;
   }

   /* Create a socket */
   sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
   if (sd < 0) {
      LOG_ERROR( "Socket creation failed\n" );
      return -1;
   }

   /* Connect the socket to the specified server. */
   if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
      return -1;
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
      LOG_ERROR( "Bad port number " << port);
      return -1;
   }

   /* Map TCP transport protocol name to protocol number */
   if ( (ptrp = getprotobyname( protocol )) == 0) {
      LOG_ERROR( "Cannot map \"" << protocol << "\" to protocol number");
      return -1;
   }

   /* Create a socket */
   sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
   if (sd < 0) {
      LOG_ERROR( "Socket creation failed" );
      return -1;
   }

   /* Bind a local address to the socket */
   if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
      LOG_ERROR( "bind failed\n");
      closesocket(sd);
      return -1;
   }

   /* Specify size of request queue */
   if (listen(sd, QLEN) < 0) 
   {
      LOG_ERROR("listen failed\n");
      closesocket(sd);
      return -1;
   }

   return sd;
}
