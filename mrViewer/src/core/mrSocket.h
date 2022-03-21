/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
/**
 * @file   mrSocket.h
 * @author gga
 * @date   Sat Aug 25 00:59:25 2007
 *
 * @brief
 *
 *
 */
#ifndef mrSocket_h
#define mrSocket_h

#if defined(WIN32) || defined(WIN64)
#  include <winsock2.h>
#  undef min
#  undef max
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <unistd.h>
#  define closesocket close
#  define SOCKET_ERROR -1
#endif

#if defined(WIN32) || defined(WIN64)
#define MR_SOCKET SOCKET
#else
#define MR_SOCKET int
#endif

/**
 * Init socket library (Windows)
 *
 *
 * @return true if inited properly, false if not
 */
bool mr_init_socket_library();

MR_SOCKET  mr_new_socket_server( const char* host, const int port = 6500,
                                 const char* protocol = "tcp" );
MR_SOCKET  mr_new_socket_client( const char* host, const int port = 6500,
                                 const char* protocol = "tcp" );
void mr_closesocket( MR_SOCKET sd );

void mr_cleanup_socket_library();


#endif // mrSocket_h
