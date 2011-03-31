//
// ZoneMinder SDP Class Interface, $Date: 2009-02-16 18:21:50 +0000 (Mon, 16 Feb 2009) $, $Revision: 2765 $
// Copyright (C) 2001-2008 Philip Coombes
// 

/**
    SDP：会话描述协议 （SDP: Session Description Protocol）

　　会话描述协议（SDP）为会话通知、会话邀请和其它形式的多媒体会话初始化等目的提供了多媒体会话描述。

　　会话目录用于协助多媒体会议的通告，并为会话参与者传送相关设置信息。SDP 即用于将这种信息传输到接收端。
    SDP 完全是一种会话描述格式 ― 它不属于传输协议 ― 它只使用不同的适当的传输协议，
        包括会话通知协议（SAP）、
        会话初始协议（SIP）、
        实时流协议（RTSP）、
        MIME 扩展协议的电子邮件以及超文本传输协议（HTTP）。

　　SDP 的设计宗旨是通用性，它可以应用于大范围的网络环境和应用程序，而不仅仅局限于组播会话目录，但 SDP 不支持会话内容或媒体编码的协商。

　　在因特网组播骨干网（Mbone）中，会话目录工具被用于通告多媒体会议，并为参与者传送会议地址和参与者所需的会议特定工具信息，这由 SDP 完成。
    SDP 连接好会话后，传送足够的信息给会话参与者。
    SDP 信息发送利用了会话通知协议（SAP），它周期性地组播通知数据包到已知组播地址和端口处。
    这些信息是 UDP 数据包，其中包含 SAP 协议头和文本有效载荷（text payload）。
    这里文本有效载荷指的是 SDP 会话描述。此外信息也可以通过电子邮件或 WWW （World Wide Web） 进行发送。 


*/


#ifndef ZM_SDP_H
#define ZM_SDP_H

#include "zm.h"

#include "zm_utils.h"
#include "zm_exception.h"
#include "zm_ffmpeg.h"

#include <stdlib.h>

#include <string>
#include <vector>

class SessionDescriptor
{
protected:
    enum { PAYLOAD_TYPE_DYNAMIC=96 };

    struct StaticPayloadDesc
    {
        int payloadType;
        const char payloadName[6];
        enum CodecType codecType;
        enum CodecID codecId;
        int clockRate;
        int autoChannels;
    };

    struct DynamicPayloadDesc
    {
        const char payloadName[32];
        enum CodecType codecType;
        enum CodecID codecId;
        //int clockRate;
        //int autoChannels;
    };

public:
    class ConnInfo
    {
    protected:
        std::string mNetworkType;
        std::string mAddressType;
        std::string mAddress;
        int mTtl;
        int mNoAddresses;

    public:
        ConnInfo( const std::string &connInfo );
    };

    class BandInfo
    {
    protected:
        std::string mType;
        int mValue;

    public:
        BandInfo( const std::string &bandInfo );
    };

    class MediaDescriptor
    {
    protected:
        std::string mType;
        int mPort;
        int mNumPorts;
        std::string mTransport;
        int mPayloadType;

        std::string mPayloadDesc;
        std::string mControlUrl;
        double mFrameRate;
        int mClock;
        int mWidth;
        int mHeight;

        ConnInfo *mConnInfo;

    public:
        MediaDescriptor( const std::string &type, int port, int numPorts, const std::string &transport, int payloadType );

        const std::string &getType() const
        {
            return( mType );
        }
        int getPort() const
        {
            return( mPort );
        }
        int getNumPorts() const
        {
            return( mNumPorts );
        }
        const std::string &getTransport() const
        {
            return( mTransport );
        }
        const int getPayloadType() const
        {
            return( mPayloadType );
        }

        const std::string &getPayloadDesc() const
        {
            return( mPayloadDesc );
        }
        void setPayloadDesc( const std::string &payloadDesc )
        {
            mPayloadDesc = payloadDesc;
        }

        const std::string &getControlUrl() const
        {
            return( mControlUrl );
        }
        void setControlUrl( const std::string &controlUrl )
        {
            mControlUrl = controlUrl;
        }

        const int getClock() const
        {
            return( mClock );
        }
        void setClock( int clock )
        {
            mClock = clock;
        }

        void setFrameSize( int width, int height )
        {
            mWidth = width;
            mHeight = height;
        }
        int getWidth() const
        {
            return( mWidth );
        }
        int getHeight() const
        {
            return( mHeight );
        }

        const double getFrameRate() const
        {
            return( mFrameRate );
        }
        void setFrameRate( double frameRate )
        {
            mFrameRate = frameRate;
        }
    };

    typedef std::vector<MediaDescriptor *> MediaList;

protected:
    static StaticPayloadDesc smStaticPayloads[];
    static DynamicPayloadDesc smDynamicPayloads[];

protected:
    std::string mUrl;

    std::string mVersion;
    std::string mOwner;
    std::string mName;
    std::string mInfo;

    ConnInfo *mConnInfo;
    BandInfo *mBandInfo;
    std::string mTimeInfo;
    StringVector mAttributes;

    MediaList mMediaList;

public:
    SessionDescriptor( const std::string &url, const std::string &sdp );

    const std::string &getUrl() const
    {
        return( mUrl );
    }

    int getNumStreams() const
    {
        return( mMediaList.size() );
    }
    MediaDescriptor *getStream( int index )
    {
        if ( index < 0 || index >= mMediaList.size() )
            return( 0 );
        return( mMediaList[index] );
    }

    AVFormatContext *generateFormatContext() const;
};
#if 0
v=0
o=- 1239719297054659 1239719297054674 IN IP4 192.168.1.11
s=Media Presentation
e=NONE
c=IN IP4 0.0.0.0
b=AS:174
t=0 0
a=control:*
a=range:npt=now-
a=mpeg4-iod: "data:application/mpeg4-iod;base64,AoEAAE8BAf73AQOAkwABQHRkYXRhOmFwcGxpY2F0aW9uL21wZWc0LW9kLWF1O2Jhc2U2NCxBVGdCR3dVZkF4Y0F5U1FBWlFRTklCRUVrK0FBQWEyd0FBR3RzQVlCQkFFWkFwOERGUUJsQlFRTlFCVUFDN2dBQVBvQUFBRDZBQVlCQXc9PQQNAQUABAAAAAAAAAAAAAYJAQAAAAAAAAAAA0IAAkA+ZGF0YTphcHBsaWNhdGlvbi9tcGVnNC1iaWZzLWF1O2Jhc2U2NCx3QkFTZ1RBcUJYSmhCSWhRUlFVL0FBPT0EEgINAAACAAAAAAAAAAAFAwAAQAYJAQAAAAAAAAAA"
m=video 0 RTP/AVP 96
b=AS:110
a=framerate:5.0
a=control:trackID=1
a=rtpmap:96 MP4V-ES/90000
a=fmtp:96 profile-level-id=247; config=000001B0F7000001B509000001000000012008D48D8803250F042D14440F
a=mpeg4-esid:201
m=audio 0 RTP/AVP 0
b=AS:64
a=control:trackID=2
    
#endif

#endif // ZM_SDP_H
