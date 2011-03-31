/*
 * ZoneMinder User Class Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
 * Copyright (C) 2001-2008 Philip Coombes
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/  

#include "zm.h"
#include "zm_db.h"

#ifndef ZM_USER_H
#define ZM_USER_H

#if HAVE_LIBGNUTLS_OPENSSL
#include <gnutls/openssl.h>
#elif HAVE_LIBCRYPTO
#include <openssl/md5.h>
#endif // HAVE_LIBGNUTLS_OPENSSL || HAVE_LIBCRYPTO

class User
{
public:
    /**
      参考 ZoneMinder 的数据库结构来看：
          `Username` varchar(32) character set latin1 collate latin1_bin NOT NULL default '',
          `Password` varchar(64) NOT NULL default '',
          `Language` varchar(8) NOT NULL default '',
          `Enabled` tinyint(3) unsigned NOT NULL default '1',
          `Stream` enum('None','View') NOT NULL default 'None',
          `Events` enum('None','View','Edit') NOT NULL default 'None',
          `Control` enum('None','View','Edit') NOT NULL default 'None',
          `Monitors` enum('None','View','Edit') NOT NULL default 'None',
          `Devices` enum('None','View','Edit') NOT NULL default 'None',
          `System` enum('None','View','Edit') NOT NULL default 'None',
          `MaxBandwidth` varchar(16) NOT NULL default '',
          `MonitorIds` tinytext NOT NULL,
       参考安装的时候插入的数据：
           insert into Users values ('','admin',password('admin'),'',1,'View','Edit','Edit','Edit','Edit','Edit','','');
       可以这么确定：
           对于 Username = Admin 则有：

               Stream = View
               Events = Edit
               Control = Edit
               ....
               System = Edit

        那么有以下的枚举代表的意义：
            PERM_NONE = 1 是一个int整数
            PERM_VIEW = View 是字符串
            PERM_EDIT = Edit 是字符串


      */
	typedef enum { PERM_NONE=1, PERM_VIEW, PERM_EDIT } Permission;



protected:
	char username[32+1];
	char password[64+1];
	bool enabled;
	Permission stream;
	Permission events;
	Permission control;
	Permission monitors;
	Permission system;
	int *monitor_ids;

public:
	User();
	User( MYSQL_ROW &dbrow );
	~User();

	const char *getUsername() const { return( username ); }
	const char *getPassword() const { return( password ); }
	bool isEnabled() const { return( enabled ); }
	Permission getStream() const { return( stream ); }
	Permission getEvents() const { return( events ); }
	Permission getControl() const { return( control ); }
	Permission getMonitors() const { return( monitors ); }
	Permission getSystem() const { return( system ); }
	bool canAccess( int monitor_id );
};

/**
    根据 username 和 password取得用户的信息
  */
User *zmLoadUser( const char *username, const char *password=0 );

/**
    检查已经登录的用户，相当于检查已经登录的用户，检查的是cookie数据
  */
User *zmLoadAuthUser( const char *auth, bool use_remote_addr );

#endif // ZM_USER_H
