//
// ZoneMinder Ffmpeg Camera Class Implementation, $Date: 2009-01-16 12:18:50 +0000 (Fri, 16 Jan 2009) $, $Revision: 2713 $
// Copyright (C) 2001-2008 Philip Coombes
// 

/**
  附带 FFmepg基本函数参考：

    * 1 FFMPEG之初始化
          o 1.1 av_register_all()
          o 1.2 avcodec_open()
          o 1.3 avcodec_close()
          o 1.4 av_open_input_file()
          o 1.5 av_find_input_format()
          o 1.6 av_find_stream_info()
          o 1.7 av_close_input_file()
    * 2 FFMPEG之文件操作
          o 2.1 av_write_frame()
          o 2.2 dump_format()
    * 3 FFMPEG之音视频解码器
          o 3.1 avcodec_find_decoder()
          o 3.2 avcodec_alloc_frame()
          o 3.3 avpicture_get_size()
          o 3.4 avpicture_fill()
          o 3.5 img_convert()
          o 3.6 avcodec_alloc_context()
          o 3.7 avcodec_decode_audio()
          o 3.8 avcodec_decode_video()
          o 3.9 av_free_packet()
          o 3.10 av_free()
    * 4 FFMPEG之其他函数参考
          o 4.1 avpicture_deinterlace()
    * 5 FFMPEG之重要数据结构参考
          o 5.1 AVFormatContext
          o 5.2 AVFormatParameters
          o 5.3 AVCodecContext
          o 5.4 AVCodec
          o 5.5 AVFrame
          o 5.6 AVPacket
          o 5.7 AVPicture
          o 5.8 AVStream
    * 6 FFMPEG之其它数据结构参考
          o 6.1 ImgReSampleContext


 */

#include "zm.h"

#if HAVE_LIBAVFORMAT

#include "zm_ffmpeg_camera.h"

FfmpegCamera::FfmpegCamera(
    int p_id,
    const std::string &p_path,
    int p_width,
    int p_height,
    int p_colours,
    int p_brightness,
    int p_contrast,
    int p_hue,
    int p_colour,
    bool p_capture ) :
        Camera(
            p_id,
            FFMPEG_SRC,
            p_width,
            p_height,
            p_colours,
            p_brightness,
            p_contrast,
            p_hue,
            p_colour,
            p_capture ),
            mPath( p_path )
{
	if ( capture )
	{
		Initialise();
	}

    mFormatContext = NULL;
    mVideoStreamId = -1;
    mCodecContext = NULL;
    mCodec = NULL;
    mConvertContext = NULL;
    mRawFrame = NULL;
    mFrame = NULL;
}

FfmpegCamera::~FfmpegCamera()
{
    av_free( mFrame );
    av_free( mRawFrame );
    
    avcodec_close( mCodecContext );
    av_free( mCodecContext );
    av_close_input_file( mFormatContext );
    av_free( mFormatContext );

	if ( capture )
	{
		Terminate();
	}
}

void FfmpegCamera::Initialise()
{
	int max_size = width*height*colours;

	mBuffer.size( max_size );

    if ( zm_dbg_level > ZM_DBG_INF )
        av_log_set_level( AV_LOG_DEBUG ); 
    else
        av_log_set_level( AV_LOG_QUIET ); 
    /**
        FFMPEG之初始化: 与FFMPEG初始化和收尾工作相关的函数；

            av_register_all()

        说明：注册全部的文件格式和编解码器；
     */
    av_register_all();
}

void FfmpegCamera::Terminate()
{
}

int FfmpegCamera::PrimeCapture()
{
    Info( "Priming capture from %s", mPath.c_str() );

    // Open the input, not necessarily a file
    if ( av_open_input_file( &mFormatContext, mPath.c_str(), NULL, 0, NULL ) !=0 )
        Fatal( "Unable to open input %s due to: %s", mPath.c_str(), strerror(errno) );

    // Locate stream info from input
    if ( av_find_stream_info( mFormatContext ) < 0 )
        Fatal( "Unable to find stream info from %s due to: %s", mPath.c_str(), strerror(errno) );
    
    // Find first video stream present
    mVideoStreamId = -1;
    for ( int i=0; i < mFormatContext->nb_streams; i++ )
    {
        if ( mFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO )
        {
            mVideoStreamId = i;
            break;
        }
    }
    if ( mVideoStreamId == -1 )
        Fatal( "Unable to locate video stream in %s", mPath.c_str() );

    mCodecContext = mFormatContext->streams[mVideoStreamId]->codec;

    // Try and get the codec from the codec context
    if ( (mCodec = avcodec_find_decoder( mCodecContext->codec_id )) == NULL )
        Fatal( "Can't find codec for video stream from %s", mPath.c_str() );

    // Open the codec
    if ( avcodec_open( mCodecContext, mCodec ) < 0 )
        Fatal( "Unable to open codec for video stream from %s", mPath.c_str() );

    // Allocate space for the native video frame
    mRawFrame = avcodec_alloc_frame();

    // Allocate space for the converted video frame
    mFrame = avcodec_alloc_frame();

    // Determine required buffer size and allocate buffer
    int pictureSize = avpicture_get_size( PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height );
    mBuffer.size( pictureSize );
    
    avpicture_fill( (AVPicture *)mFrame, (unsigned char *)mBuffer, PIX_FMT_RGB24, mCodecContext->width, mCodecContext->height);

#if HAVE_LIBSWSCALE
    if ( (mConvertContext = sws_getCachedContext( mConvertContext, mCodecContext->width, mCodecContext->height, mCodecContext->pix_fmt, width, height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL )) == NULL )
        Fatal( "Unable to create conversion context for %s", mPath.c_str() );
#else // HAVE_LIBSWSCALE
    Fatal( "You must compile ffmpeg with the --enable-swscale option to use ffmpeg cameras" );
#endif // HAVE_LIBSWSCALE

    return( 0 );
}

int FfmpegCamera::PreCapture()
{
    // Nothing to do here
    return( 0 );
}

int FfmpegCamera::Capture( Image &image )
{
    static int frameCount = 0;
    AVPacket packet;
    int frameComplete = false;
    while ( !frameComplete && (av_read_frame( mFormatContext, &packet ) >= 0) )
    {
        Debug( 5, "Got packet from stream %d", packet.stream_index );
        if ( packet.stream_index == mVideoStreamId )
        {
            if ( avcodec_decode_video( mCodecContext, mRawFrame, &frameComplete, packet.data, packet.size) < 0 )
                Fatal( "Unable to decode frame at frame %d", frameCount );

            Debug( 3, "Decoded video packet at frame %d", frameCount );

            if ( frameComplete )
            {
                Debug( 1, "Got frame %d", frameCount );

#if HAVE_LIBSWSCALE
                if ( sws_scale( mConvertContext, mRawFrame->data, mRawFrame->linesize, 0, mCodecContext->height, mFrame->data, mFrame->linesize ) < 0 )
                    Fatal( "Unable to convert raw format %d to RGB at frame %d", mCodecContext->pix_fmt, frameCount );
#else // HAVE_LIBSWSCALE
    Fatal( "You must compile ffmpeg with the --enable-swscale option to use ffmpeg cameras" );
#endif // HAVE_LIBSWSCALE
 
                image.Assign( mCodecContext->width, mCodecContext->height, colours, (unsigned char *)mFrame->data[0] );

                frameCount++;
            }
        }
        av_free_packet( &packet );
    }
    return (0);
}

int FfmpegCamera::PostCapture()
{
    // Nothing to do here
    return( 0 );
}

#endif // HAVE_LIBAVFORMAT
