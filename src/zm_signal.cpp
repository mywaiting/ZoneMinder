﻿//
// ZoneMinder Signal Handling Implementation, $Date: 2009-02-17 12:27:03 +0000 (Tue, 17 Feb 2009) $, $Revision: 2769 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#include <string.h>
#include <stdlib.h>

#include "zm.h"
#include "zm_signal.h"

bool zm_reload = false;

RETSIGTYPE zm_hup_handler( int signal )
{
#if HAVE_STRSIGNAL
	Info( "Got signal %d (%s), reloading", signal, strsignal(signal) );
#else // HAVE_STRSIGNAL
	Info( "Got HUP signal, reloading" );
#endif // HAVE_STRSIGNAL
	zm_reload = true;
}

bool zm_terminate = false;

RETSIGTYPE zm_term_handler( int signal )
{
#if HAVE_STRSIGNAL
	Info( "Got signal %d (%s), exiting", signal, strsignal(signal) );
#else // HAVE_STRSIGNAL
	Info( "Got TERM signal, exiting" );
#endif // HAVE_STRSIGNAL
	zm_terminate = true;
}

#define TRACE_SIZE 16

#if HAVE_STRUCT_SIGCONTEXT
RETSIGTYPE zm_die_handler( int signal, struct sigcontext context )
#elif ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T )
#include <ucontext.h>
RETSIGTYPE zm_die_handler( int signal, siginfo_t *info, void *context )
#else
RETSIGTYPE zm_die_handler( int signal )
#endif
{
    if ( signal == SIGABRT )
    {
#if HAVE_STRSIGNAL
	    Info( "Got signal %d (%s), exiting and forcing backtrace", signal, strsignal(signal) );
#else // HAVE_STRSIGNAL
	    Error( "Got signal %d, exiting and forcing backtrace", signal );
#endif // HAVE_STRSIGNAL
    }
    else
    {
#if HAVE_STRSIGNAL
	    Info( "Got signal %d (%s), crashing", signal, strsignal(signal) );
#else // HAVE_STRSIGNAL
	    Error( "Got signal %d, crashing", signal );
#endif // HAVE_STRSIGNAL
    }

#ifndef ZM_NO_CRASHTRACE
#if ( ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T ) || HAVE_STRUCT_SIGCONTEXT )
	void *trace[TRACE_SIZE];
	int trace_size = 0;

#if HAVE_STRUCT_SIGCONTEXT_EIP
	Error( "Signal address is %p, from %p", (void *)context.cr2, (void *)context.eip );

	trace_size = backtrace( trace, TRACE_SIZE );
	// overwrite sigaction with caller's address
	trace[1] = (void *)context.eip;
#elif HAVE_STRUCT_SIGCONTEXT
	Error( "Signal address is %p, no eip", context.cr2 );

	trace_size = backtrace( trace, TRACE_SIZE );
#else // HAVE_STRUCT_SIGCONTEXT
	if ( info && context )
	{
		ucontext_t *uc = (ucontext_t *)context;

		Error( "Signal address is %p, from %p", info->si_addr, uc->uc_mcontext.gregs[REG_EIP] );

		trace_size = backtrace( trace, TRACE_SIZE );
		// overwrite sigaction with caller's address
		trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];
	}
#endif // HAVE_STRUCT_SIGCONTEXT
#if HAVE_DECL_BACKTRACE
    char cmd[1024] = "addr2line -e ";
    char *cmd_ptr = cmd+strlen(cmd);
    // Try and extract the binary path from the last backtrace frame
	char **messages = backtrace_symbols( trace, trace_size );
    if ( size_t offset = strcspn( messages[trace_size-1], " " ) )
    {
        snprintf( cmd_ptr, sizeof(cmd)-(cmd_ptr-cmd), messages[trace_size-1] );
        cmd_ptr += offset;
    }
    else
    {
        cmd_ptr += snprintf( cmd_ptr, sizeof(cmd)-(cmd_ptr-cmd), "/path/to/%s", zm_dbg_name );
    }
	// skip first stack frame (points here)
	for ( int i=1; i < trace_size; i++ )
    {
		Error( "Backtrace: %s", messages[i] );
        cmd_ptr += snprintf( cmd_ptr, sizeof(cmd)-(cmd_ptr-cmd), " %p", trace[i] );
    }
	Info( "Backtrace complete, please execute the following command for more information" );
    Info( cmd );
#endif // HAVE_DECL_BACKTRACE
#endif // ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T ) || HAVE_STRUCT_SIGCONTEXT
#endif // ZM_NO_CRASHTRACE

	exit( signal );
}

void zmSetHupHandler( SigHandler *handler )
{
	sigset_t block_set;
	sigemptyset( &block_set );
	struct sigaction action, old_action;

	action.sa_handler = (SigHandler *)handler;
	action.sa_mask = block_set;
	action.sa_flags = 0;
	sigaction( SIGHUP, &action, &old_action );
}

void zmSetTermHandler( SigHandler *handler )
{
	sigset_t block_set;
	sigemptyset( &block_set );
	struct sigaction action, old_action;

	action.sa_handler = (SigHandler *)handler;
	action.sa_mask = block_set;
	action.sa_flags = 0;
	sigaction( SIGTERM, &action, &old_action );
}

void zmSetDieHandler( SigHandler *handler )
{
	sigset_t block_set;
	sigemptyset( &block_set );
	struct sigaction action, old_action;

	action.sa_handler = (SigHandler *)handler;
	action.sa_mask = block_set;
	action.sa_flags = 0;

	sigaction( SIGBUS, &action, &old_action );
	sigaction( SIGSEGV, &action, &old_action );
	sigaction( SIGABRT, &action, &old_action );
	sigaction( SIGILL, &action, &old_action );
	sigaction( SIGFPE, &action, &old_action );
}

void zmSetDefaultHupHandler()
{
	zmSetHupHandler( (SigHandler *)zm_hup_handler );
}

void zmSetDefaultTermHandler()
{
	zmSetTermHandler( (SigHandler *)zm_term_handler );
}

void zmSetDefaultDieHandler()
{
    if ( config.dump_cores )
    {
        // Do nothing
    }
    else
    {
	    zmSetDieHandler( (SigHandler *)zm_die_handler );
    }
}
