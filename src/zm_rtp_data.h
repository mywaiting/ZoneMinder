﻿//
// ZoneMinder RTP Data Class Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 

#ifndef ZM_RTP_DATA_H
#define ZM_RTP_DATA_H

#include "zm_thread.h"
#include "zm_buffer.h"

class RtspThread;
class RtpSource;

struct RtpDataHeader
{
    U8 cc:4;         // CSRC count
    U8 x:1;          // header extension flag
    U8 p:1;          // padding flag
    U8 version:2;    // protocol version
    U8 pt:7;         // payload type
    U8 m:1;          // marker bit
    U16 seqN;        // sequence number, network order
    U32 timestampN;  // timestamp, network order
    U32 ssrcN;       // synchronization source, network order
    U32 csrc[];      // optional CSRC list
};

class RtpDataThread : public Thread
{
friend class RtspThread;

private:
    RtspThread &mRtspThread;
    RtpSource &mRtpSource;
    bool mStop;

private:
    bool recvPacket( const unsigned char *packet, size_t packetLen );
    int run();

public:
    RtpDataThread( RtspThread &rtspThread, RtpSource &rtpSource );

    void stop()
    {
        mStop = true;
    }
};

#endif // ZM_RTP_DATA_H
