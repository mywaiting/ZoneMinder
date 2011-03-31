//
// ZoneMinder Video Device Fixer, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mysql/mysql.h>

#include "zm.h"
#include "zm_db.h"

bool fixDevice( const char *device_path )
{
	struct stat stat_buf;

	if ( stat( device_path, &stat_buf ) < 0 )
	{
		Error( "Can't stat %s: %s", device_path, strerror(errno));
		return( false );
	}

	uid_t uid = getuid();
	gid_t gid = getgid();

	mode_t mask = 0; 
	if ( uid == stat_buf.st_uid )
	{
		// If we are the owner
		mask = 00600;
	}
	else if ( gid == stat_buf.st_gid )
	{
		// If we are in the owner group
		mask = 00060;
	}
	else
	{
		// We are neither the owner nor in the group
		mask = 00006;
	}

	mode_t mode = stat_buf.st_mode;
	if ( (mode & mask) == mask )
	{
		Debug( 1, "Permissions on %s are ok at %o", device_path, mode );
		return( true );
	}
	mode |= mask;

	Info( "Resetting permissions on %s to %o", device_path, mode );
	if ( chmod( device_path, mode ) < 0 )
	{
		Error( "Can't chmod %s to %o: %s", device_path, mode, strerror(errno));
		return( false );
	}
	return( true );
}

int main( int argc, char *argv[] )
{
	zmDbgInit( "zmfix", "", -1 );

	zmLoadConfig();

	// Only do registered devices
	static char sql[BUFSIZ];
	snprintf( sql, sizeof(sql), "select distinct Device from Monitors where not isnull(Device) and Type = 'Local'" );
	if ( mysql_query( &dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}

	MYSQL_RES *result = mysql_store_result( &dbconn );
	if ( !result )
	{
		Error( "Can't use query result: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}

	for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	{
		fixDevice( dbrow[0] );
	}

	if ( mysql_errno( &dbconn ) )
	{
		Error( "Can't fetch row: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}
	// Yadda yadda
	mysql_free_result( result );

	snprintf( sql, sizeof(sql), "select distinct ControlDevice from Monitors where not isnull(ControlDevice)" );
	if ( mysql_query( &dbconn, sql ) )
	{
		Error( "Can't run query: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}

	result = mysql_store_result( &dbconn );
	if ( !result )
	{
		Error( "Can't use query result: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}

	for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	{
		fixDevice( dbrow[0] );
	}

	if ( mysql_errno( &dbconn ) )
	{
		Error( "Can't fetch row: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}
	// Yadda yadda
	mysql_free_result( result );

	if ( config.opt_x10 )
	{
		if ( config.x10_device )
		{
			fixDevice( config.x10_device );
		}
	}

	return( 0 );
}
