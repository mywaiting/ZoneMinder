//
// ZoneMinder Configuration, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 

#include "config.h"
#include "zm_config_defines.h"

#include <string>

#define ZM_CONFIG				"@ZM_CONFIG@"	// Path to config file
#define ZM_VERSION				"@VERSION@"	// ZoneMinder Version

#define ZM_MAX_IMAGE_WIDTH		2048				// The largest image we imagine ever handling
#define ZM_MAX_IMAGE_HEIGHT		1536				// The largest image we imagine ever handling
#define ZM_MAX_IMAGE_COLOURS	3					// The largest image we imagine ever handling
#define ZM_MAX_IMAGE_DIM		(ZM_MAX_IMAGE_WIDTH*ZM_MAX_IMAGE_HEIGHT)
#define ZM_MAX_IMAGE_SIZE		(ZM_MAX_IMAGE_DIM*ZM_MAX_IMAGE_COLOURS)

#define ZM_SCALE_BASE			100					// The factor by which we bump up 'scale' to simulate FP
#define ZM_RATE_BASE			100					// The factor by which we bump up 'rate' to simulate FP

#define ZM_MAX_FPS              30                  // The maximum frame rate we expect to handle
#define ZM_SAMPLE_RATE          int(1000000/ZM_MAX_FPS) // A general nyquist sample frequency for delays etc
#define ZM_SUSPENDED_RATE       int(1000000/4) // A slower rate for when disabled etc

extern void zmLoadConfig();

/**
  这里保存了 MySQL的连接信息
 */
struct StaticConfig
{
    std::string DB_HOST;
    std::string DB_NAME;
    std::string DB_USER;
    std::string DB_PASS;
    std::string PATH_WEB;
};

/**
  使用了 extern 的外联关键字，使得该变量可以用于外部访问
  */
extern StaticConfig staticConfig;

class ConfigItem
{
private:
	char *name;
	char *value;
	char *type;

	mutable enum { CFG_BOOLEAN, CFG_INTEGER, CFG_DECIMAL, CFG_STRING } cfg_type;
	mutable union
	{
		bool boolean_value;
		int integer_value;
		double decimal_value;
		char *string_value;
	} cfg_value;
	mutable bool accessed;

public:
        /**
          这个方法非常有意思，在整个ZM里面都可以发现它的踪迹：

          调用格式如下这样：

            lang_default = (const char *)config.Item( ZM_LANG_DEFAULT );
            opt_use_auth = (bool)config.Item( ZM_OPT_USE_AUTH );
            auth_type = (const char *)config.Item( ZM_AUTH_TYPE );
            auth_relay = (const char *)config.Item( ZM_AUTH_RELAY );
            auth_hash_secret = (const char *)config.Item( ZM_AUTH_HASH_SECRET );

          经过上面的调用后，变量可以这样使用：

            config.lang_default 是等同于使用 ZM_LANG_DEFAULT
            config.opt_use_auth 是等同于使用 ZM_OPT_USE_AUTH
            config.auth_type 是等同于使用 ZM_AUTH_TYPE
            config.auth_relay 是等同于使用 ZM_AUTH_RELAY
            config.auth_hash_secret 是等同于使用 ZM_AUTH_HASH_SECRET




         */
	ConfigItem( const char *p_name, const char *p_value, const char *const p_type );
	~ConfigItem();
	void ConvertValue() const;
	bool BooleanValue() const;
	int IntegerValue() const;
	double DecimalValue() const;
	const char *StringValue() const;

	inline operator bool() const
	{
		return( BooleanValue() );
	}
	inline operator int() const
	{
		return( IntegerValue() );
	}
	inline operator double() const
	{
		return( DecimalValue() );
	}
	inline operator const char *() const
	{
		return( StringValue() );
	}
};

class Config
{
public:
	ZM_CFG_DECLARE_LIST

private:
	int n_items;
	ConfigItem **items;

public:
	Config();
	~Config();

	void Load();
	void Assign();
	const ConfigItem &Item( int id );
};

extern Config config;
