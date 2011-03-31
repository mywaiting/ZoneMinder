//
// ZoneMinder MySQL Implementation, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#include <stdlib.h>
#include <string.h>

#include "zm.h"
#include "zm_db.h"

MYSQL dbconn;

void zmDbConnect()
{
	if ( !mysql_init( &dbconn ) )
	{
		Error( "Can't initialise structure: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}
        std::string::size_type colonIndex = staticConfig.DB_HOST.find( ":/" );
        if ( colonIndex != std::string::npos )
        {
            std::string dbHost = staticConfig.DB_HOST.substr( 0, colonIndex );
            std::string dbPort = staticConfig.DB_HOST.substr( colonIndex+1 );
                if ( !mysql_real_connect( &dbconn, dbHost.c_str(), staticConfig.DB_USER.c_str(), staticConfig.DB_PASS.c_str(), 0, atoi(dbPort.c_str()), 0, 0 ) )
                {
                        Error( "Can't connect to server: %s", mysql_error( &dbconn ) );
                        exit( mysql_errno( &dbconn ) );
                }
        }
        else
        {
                if ( !mysql_real_connect( &dbconn, staticConfig.DB_HOST.c_str(), staticConfig.DB_USER.c_str(), staticConfig.DB_PASS.c_str(), 0, 0, 0, 0 ) )
                {
                        Error( "Can't connect to server: %s", mysql_error( &dbconn ) );
                        exit( mysql_errno( &dbconn ) );
                }
        }
	if ( mysql_select_db( &dbconn, staticConfig.DB_NAME.c_str() ) )
	{
		Error( "Can't select database: %s", mysql_error( &dbconn ) );
		exit( mysql_errno( &dbconn ) );
	}
}

