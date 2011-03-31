﻿//
// ZoneMinder RTCP Class Implementation, $Date: 2009-05-28 09:47:59 +0100 (Thu, 28 May 2009) $, $Revision: 2905 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#include "zm.h"

#if HAVE_LIBAVFORMAT

#include "zm_rtp_ctrl.h"

#include "zm_time.h"
#include "zm_rtsp.h"

#include <errno.h>

RtpCtrlThread::RtpCtrlThread( RtspThread &rtspThread, RtpSource &rtpSource ) : mRtspThread( rtspThread ), mRtpSource( rtpSource ), mStop( false )
{
}

int RtpCtrlThread::recvPacket( const unsigned char *packet, ssize_t packetLen )
{
    const RtcpPacket *rtcpPacket;
    rtcpPacket = (RtcpPacket *)packet;

    int consumed = 0;

    //printf( "C: " );
    //for ( int i = 0; i < packetLen; i++ )
        //printf( "%02x ", (unsigned char)packet[i] );
    //printf( "\n" );
    int ver = rtcpPacket->header.version;
    int count = rtcpPacket->header.count;
    int pt = rtcpPacket->header.pt;
    int len = ntohs(rtcpPacket->header.lenN);

    Debug( 5, "RTCP Ver: %d", ver );
    Debug( 5, "RTCP Count: %d", count );
    Debug( 5, "RTCP Pt: %d", pt );
    Debug( 5, "RTCP len: %d", len );

    switch( pt )
    {
        case RTCP_SR :
        {
            U32 ssrc = ntohl(rtcpPacket->body.sr.ssrcN);

            Debug( 5, "RTCP Got SR (%lx)", ssrc );
            if ( mRtpSource.getSsrc() )
            {
                if ( ssrc != mRtpSource.getSsrc() )
                {
                    Warning( "Discarding packet for unrecognised ssrc %lx", ssrc );
                    return( -1 );
                }
            }
            else if ( ssrc )
            {
                mRtpSource.setSsrc( ssrc );
            }

            if ( len > 1 )
            {
                //printf( "NTPts:%d.%d, RTPts:%d\n", $ntptsmsb, $ntptslsb, $rtpts );
                U16 ntptsmsb = ntohl(rtcpPacket->body.sr.ntpSecN);
                U16 ntptslsb = ntohl(rtcpPacket->body.sr.ntpFracN);
                //printf( "NTPts:%x.%04x, RTPts:%x\n", $ntptsmsb, $ntptslsb, $rtpts );
                //printf( "Pkts:$sendpkts, Octs:$sendocts\n" );
                U32 rtpTime = ntohl(rtcpPacket->body.sr.rtpTsN);

                mRtpSource.updateRtcpData( ntptsmsb, ntptslsb, rtpTime );
            }
            break;
        }
        case RTCP_SDES :
        {
            ssize_t contentLen = packetLen - sizeof(rtcpPacket->header);
            while ( contentLen )
            {
                Debug( 5, "RTCP CL: %d", contentLen );
                U32 ssrc = ntohl(rtcpPacket->body.sdes.srcN);

                Debug( 5, "RTCP Got SDES (%lx), %d items", ssrc, count );
                if ( mRtpSource.getSsrc() && (ssrc != mRtpSource.getSsrc()) )
                {
                    Warning( "Discarding packet for unrecognised ssrc %lx", ssrc );
                    return( -1 );
                }

                unsigned char *sdesPtr = (unsigned char *)&rtcpPacket->body.sdes.item;
                for ( int i = 0; i < count; i++ )
                {
                    RtcpSdesItem *item = (RtcpSdesItem *)sdesPtr;
                    Debug( 5, "RTCP Item length %d", item->len );
                    switch( item->type )
                    {
                        case RTCP_SDES_CNAME :
                        {
                            std::string cname( item->data, item->len );
                            Debug( 5, "RTCP Got CNAME %s", cname.c_str() );
                            break;
                        }
                        case RTCP_SDES_END :
                        case RTCP_SDES_NAME :
                        case RTCP_SDES_EMAIL :
                        case RTCP_SDES_PHONE :
                        case RTCP_SDES_LOC :
                        case RTCP_SDES_TOOL :
                        case RTCP_SDES_NOTE :
                        case RTCP_SDES_PRIV :
                        default :
                        {
                            Error( "Received unexpected SDES item type %d, ignoring", item->type );
                            return( -1 );
                        }
                    }
                    int paddedLen = 4+2+item->len+1; // Add null byte
                    paddedLen = (((paddedLen-1)/4)+1)*4;
                    Debug( 5, "RTCP PL:%d", paddedLen );
                    sdesPtr += paddedLen;
                    contentLen -= paddedLen;
                }
            }
            break;
        }
        case RTCP_BYE :
        {
            Debug( 5, "RTCP Got BYE" );
            mStop = true;
            break;
        }
        case RTCP_RR :
        case RTCP_APP :
        default :
        {
            Error( "Received unexpected packet type %d, ignoring", pt );
            return( -1 );
        }
    }
    consumed = sizeof(U32)*(len+1);
    return( consumed );
}

int RtpCtrlThread::generateRr( const unsigned char *packet, ssize_t packetLen )
{
    RtcpPacket *rtcpPacket = (RtcpPacket *)packet;

    int byteLen = sizeof(rtcpPacket->header)+sizeof(rtcpPacket->body.rr)+sizeof(rtcpPacket->body.rr.rr[0]);
    int wordLen = ((byteLen-1)/sizeof(U32))+1;

    rtcpPacket->header.version = RTP_VERSION;
    rtcpPacket->header.p = 0;
    rtcpPacket->header.pt = RTCP_RR;
    rtcpPacket->header.count = 1;
    rtcpPacket->header.lenN = htons(wordLen-1);

    mRtpSource.updateRtcpStats();

    Debug( 5, "Ssrc = %ld", mRtspThread.getSsrc() );
    Debug( 5, "Ssrc_1 = %ld", mRtpSource.getSsrc() );
    Debug( 5, "Last Seq = %ld", mRtpSource.getMaxSeq() );
    Debug( 5, "Jitter = %ld", mRtpSource.getJitter() );
    Debug( 5, "Last SR = %ld", mRtpSource.getLastSrTimestamp() );

    rtcpPacket->body.rr.ssrcN = htonl(mRtspThread.getSsrc());
    rtcpPacket->body.rr.rr[0].ssrcN = htonl(mRtpSource.getSsrc());
    rtcpPacket->body.rr.rr[0].lost = mRtpSource.getLostPackets();
    rtcpPacket->body.rr.rr[0].fraction = mRtpSource.getLostFraction();
    rtcpPacket->body.rr.rr[0].lastSeqN = htonl(mRtpSource.getMaxSeq());
    rtcpPacket->body.rr.rr[0].jitterN = htonl(mRtpSource.getJitter());
    rtcpPacket->body.rr.rr[0].lsrN = htonl(mRtpSource.getLastSrTimestamp());
    rtcpPacket->body.rr.rr[0].dlsrN = 0;

    return( wordLen*sizeof(U32) );
}

int RtpCtrlThread::generateSdes( const unsigned char *packet, ssize_t packetLen )
{
    RtcpPacket *rtcpPacket = (RtcpPacket *)packet;

    const std::string &cname = mRtpSource.getCname();

    int byteLen = sizeof(rtcpPacket->header)+sizeof(rtcpPacket->body.sdes)+sizeof(rtcpPacket->body.sdes.item[0])+cname.size();
    int wordLen = ((byteLen-1)/sizeof(U32))+1;

    rtcpPacket->header.version = RTP_VERSION;
    rtcpPacket->header.p = 0;
    rtcpPacket->header.pt = RTCP_SDES;
    rtcpPacket->header.count = 1;
    rtcpPacket->header.lenN = htons(wordLen-1);

    rtcpPacket->body.sdes.srcN = htonl(mRtpSource.getSsrc());
    rtcpPacket->body.sdes.item[0].type = RTCP_SDES_CNAME;
    rtcpPacket->body.sdes.item[0].len = cname.size();
    memcpy( rtcpPacket->body.sdes.item[0].data, cname.data(), cname.size() );

    return( wordLen*sizeof(U32) );
}

int RtpCtrlThread::generateBye( const unsigned char *packet, ssize_t packetLen )
{
    RtcpPacket *rtcpPacket = (RtcpPacket *)packet;

    int byteLen = sizeof(rtcpPacket->header)+sizeof(rtcpPacket->body.bye)+sizeof(rtcpPacket->body.bye.srcN[0]);
    int wordLen = ((byteLen-1)/sizeof(U32))+1;

    rtcpPacket->header.version = RTP_VERSION;
    rtcpPacket->header.p = 0;
    rtcpPacket->header.pt = RTCP_BYE;
    rtcpPacket->header.count = 1;
    rtcpPacket->header.lenN = htons(wordLen-1);

    rtcpPacket->body.bye.srcN[0] = htonl(mRtpSource.getSsrc());

    return( wordLen*sizeof(U32) );
}

int RtpCtrlThread::recvPackets( unsigned char *buffer, ssize_t nBytes )
{
    unsigned char *bufferPtr = buffer;

    // u_int32 len;        /* length of compound RTCP packet in words */
    // rtcp_t *r;          /* RTCP header */
    // rtcp_t *end;        /* end of compound RTCP packet */

    // if ((*(u_int16 *)r & RTCP_VALID_MASK) != RTCP_VALID_VALUE) {
        // /* something wrong with packet format */
    // }
    // end = (rtcp_t *)((u_int32 *)r + len);

    // do r = (rtcp_t *)((u_int32 *)r + r->common.length + 1);
    // while (r < end && r->common.version == 2);

    // if (r != end) {
        // /* something wrong with packet format */
    // }

    while ( nBytes > 0 )
    {
        int consumed = recvPacket( bufferPtr, nBytes );
        if ( consumed <= 0 )
            break;
        bufferPtr += consumed;
        nBytes -= consumed;
    }
    return( nBytes );
}

int RtpCtrlThread::run()
{
    Debug( 2, "Starting control thread %lx on port %d", mRtpSource.getSsrc(), mRtpSource.getLocalCtrlPort() );
    SockAddrInet localAddr, remoteAddr;

    bool sendReports;
    UdpInetSocket rtpCtrlServer;
    if ( mRtpSource.getLocalHost() != "" )
    {
        localAddr.resolve( mRtpSource.getLocalHost().c_str(), mRtpSource.getLocalCtrlPort(), "udp" );
        if ( !rtpCtrlServer.bind( localAddr ) )
            Fatal( "Failed to bind RTCP server" );
        sendReports = false;
        Debug( 3, "Bound to %s:%d",  mRtpSource.getLocalHost().c_str(), mRtpSource.getLocalCtrlPort() );
    }
    else
    {
        localAddr.resolve( mRtpSource.getLocalCtrlPort(), "udp" );
        if ( !rtpCtrlServer.bind( localAddr ) )
            Fatal( "Failed to bind RTCP server" );
        Debug( 3, "Bound to %s:%d",  mRtpSource.getLocalHost().c_str(), mRtpSource.getLocalCtrlPort() );
        remoteAddr.resolve( mRtpSource.getRemoteHost().c_str(), mRtpSource.getRemoteCtrlPort(), "udp" );
        if ( !rtpCtrlServer.connect( remoteAddr ) )
            Fatal( "Failed to connect RTCP server" );
        Debug( 3, "Connected to %s:%d",  mRtpSource.getRemoteHost().c_str(), mRtpSource.getRemoteCtrlPort() );
        sendReports = true;
    }

    Select select( 10 );
    select.addReader( &rtpCtrlServer );

    unsigned char buffer[BUFSIZ];
    while ( !mStop && select.wait() >= 0 )
    {
        if ( mStop )
            break;
        Select::CommsList readable = select.getReadable();
        if ( readable.size() == 0 )
        {
            Error( "RTCP timed out" );
            break;
        }
        for ( Select::CommsList::iterator iter = readable.begin(); iter != readable.end(); iter++ )
        {
            if ( UdpInetSocket *socket = dynamic_cast<UdpInetSocket *>(*iter) )
            {
                ssize_t nBytes = socket->recv( buffer, sizeof(buffer) );
                Debug( 4, "Read %d bytes on sd %d", nBytes, socket->getReadDesc() );

                if ( nBytes )
                {
                    recvPackets( buffer, nBytes );

                    if ( sendReports )
                    {
                        unsigned char *bufferPtr = buffer;
                        bufferPtr += generateRr( bufferPtr, sizeof(buffer)-(bufferPtr-buffer) );
                        bufferPtr += generateSdes( bufferPtr, sizeof(buffer)-(bufferPtr-buffer) );
                        Debug( 4, "Sending %d bytes on sd %d", bufferPtr-buffer, rtpCtrlServer.getWriteDesc() );
                        if ( (nBytes = rtpCtrlServer.send( buffer, bufferPtr-buffer )) < 0 )
                            Error( "Unable to send: %s", strerror( errno ) );
                        //Debug( 4, "Sent %d bytes on sd %d", nBytes, rtpCtrlServer.getWriteDesc() );
                    }
                }
                else
                {
                    mStop = true;
                    break;
                }
            }
            else
            {
                Fatal( "Barfed" );
            }
        }
    }
    rtpCtrlServer.close();
    mRtspThread.stop();
    return( 0 );
}

#endif // HAVE_LIBAVFORMAT