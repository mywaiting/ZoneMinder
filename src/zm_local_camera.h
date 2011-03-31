//
// ZoneMinder Local Camera Class Interface, $Date: 2009-03-31 13:57:34 +0100 (Tue, 31 Mar 2009) $, $Revision: 2827 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#ifndef ZM_LOCAL_CAMERA_H
#define ZM_LOCAL_CAMERA_H

#include "zm.h"
#include "zm_camera.h"

#include "zm_ffmpeg.h"

#ifdef HAVE_LINUX_VIDEODEV2_H
#include <linux/videodev2.h>
#define ZM_V4L2
#endif // HAVE_LINUX_VIDEODEV2_H
#ifdef HAVE_LINUX_VIDEODEV_H
#include <linux/videodev.h>
#endif // HAVE_LINUX_VIDEODEV_H

/**
    先前一直不清楚ZM的硬件驱动在哪里，其实，这个文件就是 ZoneMinder的摄像头硬件驱动。

    头文件：
        linux/videodev.h  对应着  V4L
        linux/videodev2.h 对应着  V4L2

    这些视频接口的驱动，有其特别的实现，具体请参考以下文章：

        Google搜索链接：http://www.google.com/search?hl=zh-CN&biw=1280&bih=611&q=V4L2+V4L

        http://blog.csdn.net/lanmanck/archive/2009/04/13/4069534.aspx
        http://blog.chinaunix.net/space.php?uid=14782631&do=blog&cuid=1961199
        http://hi.baidu.com/dunwin/blog/item/3c8aaeec3964a34178f055e5.html/cmtid/c5c685a3b782c5a3cbefd09c
        http://blog.csdn.net/hongtao_liu/archive/2010/09/06/5867351.aspx


 */

/**
  LocalCamera 意思在于 Local，表示这些 Camera是可以直接连接的，这个是相对于 RemoteCamera 而言的

 */
class LocalCamera : public Camera
{
protected:
#ifdef ZM_V4L2
    struct V4L2MappedBuffer
    {
        void    *start;
        size_t  length;
    };

    struct V4L2Data
    {
        v4l2_cropcap        cropcap;
        v4l2_crop           crop;
        v4l2_format         fmt;
        v4l2_requestbuffers reqbufs;
        V4L2MappedBuffer    *buffers;
        v4l2_buffer         *bufptr;
    };
#endif // ZM_V4L2

    struct V4L1Data
    {
	    int				    active_frame;
	    video_mbuf		    frames;
	    video_mmap		    *buffers;
	    unsigned char	    *bufptr;
    };

protected:
	std::string             device;
        int		        channel;
        int	                standard;
        int                     palette;

        bool                    device_prime;
        bool                    channel_prime;
        int                     channel_index;

protected:
	static int				camera_count;
	static int				channel_count;
        static int              channels[VIDEO_MAX_FRAME];
        static int              standards[VIDEO_MAX_FRAME];

	static int				vid_fd;

    static int              v4l_version;

#ifdef ZM_V4L2
    static V4L2Data         v4l2_data;
#endif // ZM_V4L2
    static V4L1Data         v4l1_data;

#if HAVE_LIBSWSCALE
    PixelFormat             imagePixFormat;
    PixelFormat             capturePixFormat;
    static AVFrame          **capturePictures;
#endif // HAVE_LIBSWSCALE

	static unsigned char	*y_table;
	static signed char		*uv_table;
	static short			*r_v_table;
	static short			*g_v_table;
	static short			*g_u_table;
	static short			*b_u_table;

    static LocalCamera      *last_camera;

public:
	LocalCamera( int p_id, const std::string &device, int p_channel, int p_format, const std::string &p_method, int p_width, int p_height, int p_palette, int p_brightness, int p_contrast, int p_hue, int p_colour, bool p_capture );
	~LocalCamera();

	void Initialise();
	void Terminate();

	const std::string &Device() const { return( device ); }

	int Channel() const { return( channel ); }
	int Standard() const { return( standard ); }
	int Palette() const { return( palette ); }

	int Brightness( int p_brightness=-1 );
	int Hue( int p_hue=-1 );
	int Colour( int p_colour=-1 );
	int Contrast( int p_contrast=-1 );

	int PrimeCapture();
	int PreCapture();
	int Capture( Image &image );
	int PostCapture();

	static bool GetCurrentSettings( const char *device, char *output, int version, bool verbose );
};

#endif // ZM_LOCAL_CAMERA_H
