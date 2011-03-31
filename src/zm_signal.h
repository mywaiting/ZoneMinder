/*
 * ZoneMinder Signal Handling Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
 * Copyright (C) 2001-2008 Philip Coombes
*/ 

/**
    这个是 ZM 的 Signal模块，跟 Linux有着密不可分的关系。
    
    信号是Linux编程中非常重要的部分，本文将详细介绍信号机制的基本概念、Linux对信号机制的大致实现方法、如何使用信号，以及有关信号的几个系统调用。

    信号机制是进程之间相互传递消息的一种方法，信号全称为软中断信号，也有人称作软中断。从它的命名可以看出，它的实质和使用很象中断。所以，信号可以说是进程控制的一部分。

    参考链接：
    
        http://www.cnblogs.com/taobataoma/archive/2007/08/30/875743.html
        http://www.cnblogs.com/taobataoma/archive/2007/08/30/875662.html

*/

#ifndef ZM_SIGNAL_H
#define ZM_SIGNAL_H

#include <signal.h>
#include <execinfo.h>

#include "zm.h"

typedef RETSIGTYPE (SigHandler)( int );

extern bool zm_reload;
extern bool zm_terminate;

RETSIGTYPE zmc_hup_handler( int signal );
RETSIGTYPE zmc_term_handler( int signal );
#if HAVE_STRUCT_SIGCONTEXT
RETSIGTYPE zmc_die_handler( int signal, struct sigcontext context );
#elif ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T )
#include <ucontext.h>
RETSIGTYPE zmc_die_handler( int signal, siginfo_t *info, void *context );
#else
RETSIGTYPE zmc_die_handler( int signal );
#endif

void zmSetHupHandler( SigHandler *handler );
void zmSetTermHandler( SigHandler *handler );
void zmSetDieHandler( SigHandler *handler );

void zmSetDefaultHupHandler();
void zmSetDefaultTermHandler();
void zmSetDefaultDieHandler();

#endif // ZM_SIGNAL_H
