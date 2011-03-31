/*
 * ZoneMinder Jpeg Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
 * Copyright (C) 2001-2008 Philip Coombes
 * 
*/  

#include <setjmp.h>

#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

/**
    这个是 ZM 自己封装后的 JPEG操作实现。

    关于 JPEG的实现，请参考：
        http://www.google.com/search?hl=zh-CN&biw=1280&bih=611&q=jpeg+jpeglib
        http://my.unix-center.net/~Simon_fu/?p=565
        http://cn.webdiscussion.info/question/1443390/%E5%AF%B9JPEG%E5%8E%8B%E7%BC%A9IplImage%E7%BB%93%E6%9E%84%E4%BD%BF%E7%94%A8OpenCV%E4%B8%AD%E7%9A%84libjpeg
 */

/* Stuff for overriden error handlers */
struct zm_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

typedef struct zm_error_mgr *zm_error_ptr;

void zm_jpeg_error_exit( j_common_ptr cinfo );
void zm_jpeg_emit_message( j_common_ptr cinfo, int msg_level );

// Prototypes for memory compress/decompression object */
void jpeg_mem_src(j_decompress_ptr cinfo, const JOCTET *inbuffer, int inbuffer_size );
void jpeg_mem_dest(j_compress_ptr cinfo, JOCTET *outbuffer, int *outbuffer_size );
