/**
 * ZoneMinder Debug Interface, $Date: 2009-02-17 21:22:02 +0000 (Tue, 17 Feb 2009) $, $Revision: 2773 $
 * Copyright (C) 2001-2008 Philip Coombes
 * 
*/  

#ifndef ZM_DEBUG_H
#define ZM_DEBUG_H

#include <sys/types.h>	
#include <limits.h>	

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/**
  分别代表：
    ZM_DBG_INF	= DEBUG INFO 调试信息
    ZM_DBG_WAR  = DEBUG WARNING 调试警告
    ZM_DBG_ERR	= DEBUG ERROR 调试错误
    ZM_DBG_FAT  = DEBUG FATAL ERROR 调试核心错误，核心内部错误
 */
#define ZM_DBG_INF			0
#define	ZM_DBG_WAR			-1
#define ZM_DBG_ERR			-2
#define ZM_DBG_FAT			-3


/**
  定义那种错误信息能够进入系统的错误信息
 */
#define ZM_DBG_SYSLOG		ZM_DBG_INF 


/**
    下面两个函数用于print错误信息的：
    zmDbgPrintf： 用于提示出错的文件，出错的行数，出错的级别
    zmDbgHexdump： 用于调试的时候，打印一些运行时候的一些数据

 */
#define zmDbgPrintf(level,params...)	{\
					if (level <= zm_dbg_level)\
						zmDbgOutput( 0, __FILE__, __LINE__, level, ##params );\
				}

#define zmDbgHexdump(level,data,len)	{\
					if (level <= zm_dbg_level)\
						zmDbgOutput( 1, __FILE__, __LINE__, level, "%p (%d)", data, len );\
				}

/* Turn off debug here */
/**
  ZM_DBG_OFF这个变量代表着 DEBUG功能的开启与否。
  需要开启调试功能，可以这样：

  在这前面加一句：

    #define ZM_DBG_OFF FALSE

  即会全局开启调试功能！
*/
#ifndef ZM_DBG_OFF
#define Debug(level,params...)	zmDbgPrintf(level,##params)
#define Hexdump(level,data,len)	zmDbgHexdump(level,data,len)
#else
#define Debug(level,params...)
#define Hexdump(level,data,len)
#endif

/**
    按照出错的等级INFO、WARNING、ERROR、FATAL ERROR打印出错信息。

    实际上这个是上面的 zmDbgPrintf的快捷方式。
 */
#define Info(params...)		zmDbgPrintf(ZM_DBG_INF,##params)
#define Warning(params...)	zmDbgPrintf(ZM_DBG_WAR,##params)
#define Error(params...)	zmDbgPrintf(ZM_DBG_ERR,##params)
#define Fatal(params...)	zmDbgPrintf(ZM_DBG_FAT,##params)
#define Mark()				Info("Mark/%s/%d",__FILE__,__LINE__)
#define Log()				Info("Log")
#ifdef __GNUC__
#define Enter(level)		zmDbgPrintf(level,("Entering %s",__PRETTY_FUNCTION__))
#define Exit(level)			zmDbgPrintf(level,("Exiting %s",__PRETTY_FUNCTION__))
#else
#if 0
#define Enter(level)		zmDbgPrintf(level,("Entering <unknown>"))
#define Exit(level)			zmDbgPrintf(level,("Exiting <unknown>"))
#endif
#define Enter(level)		
#define Exit(level)			
#endif

#ifdef __cplusplus
extern "C" {
#endif 

/* function declarations */
void zmUsrHandler( int sig );
int zmGetDebugEnv( void );
int zmDebugPrepareLog( void );
int zmDebugInitialise( const char *name, const char *id, int level );
int zmDebugReinitialise( const char *target );
int zmDebugTerminate( void );
void zmDbgSubtractTime( struct timeval * const tp1, struct timeval * const tp2 );

#if defined(__STDC__) || defined(__cplusplus)
int zmDbgInit( const char *name, const char *id, int level );
int zmDbgReinit( const char *target );
int zmDbgTerm(void);
void zmDbgOutput( int hex, const char * const file, const int line, const int level, const char *fstring, ... ) __attribute__ ((format(printf, 5, 6)));
#else
int zmDbgInit();
int zmDbgReinit();
int zmDbgTerm();
void zmDbgOutput();
#endif

extern int zm_dbg_level;
extern int zm_dbg_pid;
extern char zm_dbg_log[];
#ifndef _STDIO_INCLUDED
#include <stdio.h>
#endif
extern FILE *zm_dbg_log_fd;
extern char zm_dbg_name[];
extern char zm_dbg_id[];
extern int zm_dbg_print;
extern int zm_dbg_flush;
extern int zm_dbg_add_log_id;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // ZM_DEBUG_H
