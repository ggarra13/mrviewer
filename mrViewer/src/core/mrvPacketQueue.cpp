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
/**
 * @file   mrvPacketQueue.cpp
 * @author gga
 * @date   Sun Jul 15 20:19:05 2007
 * 
 * @brief  
 * 
 * 
 */


#include "mrvPacketQueue.h"

namespace mrv {

const char* PacketQueue::kModule = "pktqueue";
AVPacket  PacketQueue::_flush;
AVPacket  PacketQueue::_seek;
AVPacket  PacketQueue::_seek_end;
AVPacket  PacketQueue::_preroll;
AVPacket  PacketQueue::_loop_start;
AVPacket  PacketQueue::_loop_end;

}
