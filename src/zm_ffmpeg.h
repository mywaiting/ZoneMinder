/*
 * ZoneMinder FFMPEG Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
 * Copyright (C) 2001-2008 Philip Coombes
 * 
*/  

/**
    什么是FFmpeg？

    FFmpeg是一套完整的录制、转换、流化音视频的解决方案，也是一个在LGPL协议下的开源项目。它包含了业界领先的音视频编解码库。
    FFmpeg是在Linux操作系统下开发的，但它也能在其他操作系统下编译，包括Windows。

    整个项目由以下几个部分组成：

        * ffmpeg：一个用来转换视频文件格式的命令行工具，它也支持从电视卡中实时的抓取和编码视频。
        * ffserver：一个基于HTTP协议（基于RTSP的版本正在开发中）用于实时广播的多媒体服务器，它也支持实时广播的时间平移。
        * ffplay：一个用SDL和FFmpeg库开发的简单的媒体播放器。
        * libavcodec：一个包含了所有FFmpeg音视频编解码器的库。为了保证最优的性能和高可复用性,大多数编解码器都是从头开发的。
        * libavformat：一个包含了所有的普通音视频格式的解析器和产生器的库。

   这个库用于FFmpeg的视频编码，解码，录制及输出,以下的这段INCLUDE实际上是为了载入
   1. avutil.h
       尝试载入 libavutil/avutil.h
       尝试载入 ffmpeg/avutil.h

   2. avcodec.h
       尝试载入 libavcodec/avcodec.h
       尝试载入 ffmpeg/avcodec.h

   3. avformat.h
       尝试载入 libavformat/avformat.h
       尝试载入 ffmpeg/avformat.h

   4. swscale.h
       尝试载入 libswscale/swscale.h
       尝试载入 ffmpeg/swscale.h

   库的实际使用请参考：
        FFmepg官方文档：http://www.ffmpeg.org/documentation.html
        使用教程1： http://www.inb.uni-luebeck.de/~boehme/using_libavcodec.html
        使用教程2： http://dranger.com/ffmpeg/tutorial01.html

   库的全部函数参考（中文）：
        http://www.ffmpeg.com.cn/index.php/Ffmpeg%E5%BC%80%E5%8F%91%E6%89%8B%E5%86%8C

 */
#ifndef ZM_FFMPEG_H
#define ZM_FFMPEG_H

#if HAVE_LIBAVCODEC

#ifdef __cplusplus
extern "C" {
#endif
#if HAVE_LIBAVUTIL_AVUTIL_H
#include <libavutil/avutil.h>
#elif HAVE_FFMPEG_AVUTIL_H
#include <ffmpeg/avutil.h>
#else
#error "No location for avutils.h found"
#endif
#if HAVE_LIBAVCODEC_AVCODEC_H
#include <libavcodec/avcodec.h>
#elif HAVE_FFMPEG_AVCODEC_H
#include <ffmpeg/avcodec.h>
#else
#error "No location for avcodec.h found"
#endif
#if HAVE_LIBAVFORMAT_AVFORMAT_H
#include <libavformat/avformat.h>
#elif HAVE_FFMPEG_AVFORMAT_H
#include <ffmpeg/avformat.h>
#else
#error "No location for avformat.h found"
#endif
#if HAVE_LIBSWSCALE
#if HAVE_LIBSWSCALE_SWSCALE_H
#include <libswscale/swscale.h>
#elif HAVE_FFMPEG_SWSCALE_H
#include <ffmpeg/swscale.h>
#else
#error "No location for swscale.h found"
#endif
#endif // HAVE_LIBSWSCALE
#ifdef __cplusplus
}
#endif

#if FFMPEG_VERSION_INT == 0x000408
#define ZM_FFMPEG_048	1
#elif FFMPEG_VERSION_INT == 0x000409
#if LIBAVCODEC_VERSION_INT < ((50<<16)+(0<<8)+0)
#define ZM_FFMPEG_049	1
#else // LIBAVCODEC_VERSION_INT
#define ZM_FFMPEG_SVN	1
#endif // LIBAVCODEC_VERSION_INT
#else // FFMPEG_VERSION_INT
#define ZM_FFMPEG_SVN	1
#endif // FFMPEG_VERSION_INT
                               
#endif // HAVE_LIBAVCODEC

#endif // ZM_FFMPEG_H
