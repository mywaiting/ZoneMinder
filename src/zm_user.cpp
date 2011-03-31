/*
 * ZoneMinder regular expression class implementation, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
 * Copyright (C) 2001-2008 Philip Coombes
 * 
*/ 

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include "zm.h"
#include "zm_db.h"

#include "zm_user.h"

User::User()
{
	username[0] = password[0] = 0;
	enabled = false;
	stream = events = control = monitors = system = PERM_NONE;
	monitor_ids = 0;
}

/**
    从 MySQL里面载入 用户的所有配置信息
*/
User::User( MYSQL_ROW &dbrow )
{
	int index = 0;
	strncpy( username, dbrow[index++], sizeof(username) );
	strncpy( password, dbrow[index++], sizeof(password) );
	enabled = (bool)atoi( dbrow[index++] );
	stream = (Permission)atoi( dbrow[index++] );
	events = (Permission)atoi( dbrow[index++] );
	control = (Permission)atoi( dbrow[index++] );
	monitors = (Permission)atoi( dbrow[index++] );
	system = (Permission)atoi( dbrow[index++] );
	monitor_ids = 0;
	char *monitor_ids_str = dbrow[index++];
	if ( monitor_ids_str && *monitor_ids_str )
	{
		monitor_ids = new int[strlen(monitor_ids_str)];
		int n_monitor_ids = 0;
		const char *ptr = monitor_ids_str;
		do
		{
			int id = 0;
			while( isdigit( *ptr ) )
			{
				id *= 10;
				id += *ptr-'0';
				ptr++;
			}
			if ( id )
			{
				monitor_ids[n_monitor_ids++] = id;
				if ( !*ptr )
				{
					break;
				}
			}
			while ( !isdigit( *ptr ) )
			{
				ptr++;
			}
		} while( *ptr );
		monitor_ids[n_monitor_ids] = 0;
	}
}

User::~User()
{
	delete monitor_ids;
}

/**
    验证某个用户是否具备访问 某摄像头的权限
*/
bool User::canAccess( int monitor_id )
{
	if ( !monitor_ids )
	{
		return( true );
	}
	for ( int i = 0; monitor_ids[i]; i++ )
	{
		if ( monitor_ids[i] == monitor_id )
		{
			return( true );
		}
	}
	return( false );
}

// Function to load a user from username and password
User *zmLoadUser( const char *username, const char *password )
{
	char sql[BUFSIZ] = "";

	if ( password )
	{
		snprintf( sql, sizeof(sql), "select Username, Password, Enabled, Stream+0, Events+0, Control+0, Monitors+0, System+0, MonitorIds from Users where Username = '%s' and Password = password('%s') and Enabled = 1", username, password );
	}
	else
	{
		snprintf( sql, sizeof(sql), "select Username, Password, Enabled, Stream+0, Events+0, Control+0, Monitors+0, System+0, MonitorIds from Users where Username = '%s' and Enabled = 1", username );
	}

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
	int n_users = mysql_num_rows( result );

	if ( n_users != 1 )
	{
		Warning( "Unable to authenticate user %s", username );
		return( 0 );
	}

	MYSQL_ROW dbrow = mysql_fetch_row( result );

	User *user = new User( dbrow );
	Info( "Authenticated user '%s'", user->getUsername() );

	mysql_free_result( result );

	return( user );
}

// Function to validate an authentication string
User *zmLoadAuthUser( const char *auth, bool use_remote_addr )
{
#if HAVE_DECL_MD5
	const char *remote_addr = "";
	if ( use_remote_addr )
	{
		remote_addr = getenv( "REMOTE_ADDR" );
		if ( !remote_addr )
		{
			Warning( "Can't determine remote address, using null" );
			remote_addr = "";
		}
	}

	Debug( 1, "Attempting to authenticate user from auth string '%s'", auth );
	char sql[BUFSIZ] = "";
	snprintf( sql, sizeof(sql), "select Username, Password, Enabled, Stream+0, Events+0, Control+0, Monitors+0, System+0, MonitorIds from Users where Enabled = 1" );

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
	int n_users = mysql_num_rows( result );

	if ( n_users < 1 )
	{
		Warning( "Unable to authenticate user" );
		return( 0 );
	}

	while( MYSQL_ROW dbrow = mysql_fetch_row( result ) )
	{
		const char *user = dbrow[0];
		const char *pass = dbrow[1];

		char auth_key[512] = "";
		char auth_md5[32+1] = "";
		unsigned char md5sum[MD5_DIGEST_LENGTH];

		time_t now = time( 0 );
		int max_tries = 2;

		for ( int i = 0; i < max_tries; i++, now -= (60*60) )
		{
			struct tm *now_tm = localtime( &now );

			snprintf( auth_key, sizeof(auth_key), "%s%s%s%s%d%d%d%d", 
				config.auth_hash_secret,
				user,
				pass,
				remote_addr,
				now_tm->tm_hour,
				now_tm->tm_mday,
				now_tm->tm_mon,
				now_tm->tm_year
			);

			MD5( (unsigned char *)auth_key, strlen(auth_key), md5sum );
			auth_md5[0] = '\0';
			for ( int j = 0; j < MD5_DIGEST_LENGTH; j++ )
			{
				sprintf( &auth_md5[2*j], "%02x", md5sum[j] );
			}
			Debug( 1, "Checking auth_key '%s' -> auth_md5 '%s'", auth_key, auth_md5 );

			if ( !strcmp( auth, auth_md5 ) )
			{
				// We have a match
				User *user = new User( dbrow );
				Info( "Authenticated user '%s'", user->getUsername() );
				return( user );
			}
		}
	}
#else // HAVE_DECL_MD5
	Error( "You need to build with gnutls or openssl installed to use hash based authentication" );
#endif // HAVE_DECL_MD5
	return( 0 );
}
