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
AVPacket  PacketQueue::_preroll;
AVPacket  PacketQueue::_loop_start;
AVPacket  PacketQueue::_loop_end;

}
