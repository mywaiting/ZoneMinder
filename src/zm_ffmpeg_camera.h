//
// ZoneMinder Ffmpeg Class Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
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

#ifndef ZM_FFMPEG_CAMERA_H
#define ZM_FFMPEG_CAMERA_H

#include "zm_camera.h"

#include "zm_buffer.h"
//#include "zm_utils.h"
#include "zm_ffmpeg.h"

//
// Class representing 'remote' cameras, i.e. those which are
// accessed over a network connection.
//
class FfmpegCamera : public Camera
{
protected:
    std::string         mPath;

/**
  以下带大写 AV开头的都是 FFmpeg的数据结构：
    FFMPEG之重要数据结构参考
          o 5.1 AVFormatContext
          o 5.2 AVFormatParameters
          o 5.3 AVCodecContext
          o 5.4 AVCodec
          o 5.5 AVFrame
          o 5.6 AVPacket
          o 5.7 AVPicture
          o 5.8 AVStream

    AVFormatContext 这个结构体会记录有关Format的许多信息，在面向文件格式的时候，这是一个主要的结构体。
    目前我知道他会包含一些流，音频或视频的流，并且会有一个最数目，人们加入的流不能超过这个数目。

    ZoneMinder在这里将 FFmpeg的结构转为自己的结构体。

    AVFormatContext   =  *mFormatContext;
    // 下面这个 Id是ZM定义的流Id，用于分别区别不同的视频流
    int               =  mVideoStreamId;
    AVCodecContext    =  *mCodecContext;
    AVCodec           =  *mCodec;
    struct SwsContext =  *mConvertContext;
    AVFrame           =  *mRawFrame;
    AVFrame           =  *mFrame;

 */
#if HAVE_LIBAVFORMAT
    AVFormatContext     *mFormatContext;
    int                 mVideoStreamId;
    AVCodecContext      *mCodecContext;
    AVCodec             *mCodec;
    struct SwsContext   *mConvertContext;
    AVFrame             *mRawFrame; 
    AVFrame             *mFrame;
#endif // HAVE_LIBAVFORMAT

	Buffer              mBuffer;

public:
	FfmpegCamera( int p_id, const std::string &path, int p_width, int p_height, int p_colours, int p_brightness, int p_contrast, int p_hue, int p_colour, bool p_capture );
	~FfmpegCamera();

        const std::string &Path() const { return( mPath ); }

	void Initialise();
	void Terminate();

	int PrimeCapture();
	int PreCapture();
	int Capture( Image &image );
	int PostCapture();
};

#endif // ZM_FFMPEG_CAMERA_H
