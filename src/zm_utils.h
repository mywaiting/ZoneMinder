//
// ZoneMinder General Utility Functions, $Date: 2009-05-28 09:47:59 +0100 (Thu, 28 May 2009) $, $Revision: 2905 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#ifndef ZM_UTILS_H
#define ZM_UTILS_H

#include <string>
#include <vector>

typedef std::vector<std::string> StringVector;

const std::string stringtf( const char *format, ... );
const std::string stringtf( const std::string &format, ... );

bool startsWith( const std::string &haystack, const std::string &needle );
StringVector split( const std::string &string, const std::string chars, int limit=0 );

/**
    对字符串进行 Base64 编码
 **/
const std::string base64Encode( const std::string &inString );

/**
    求两个数最大、最小的函数
**/
inline int max( int a, int b )
{
    return( a>=b?a:b );
}

inline int min( int a, int b )
{
    return( a<=b?a:b );
}

#endif // ZM_UTILS_H
