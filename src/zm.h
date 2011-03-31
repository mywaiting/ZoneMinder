//
// ZoneMinder Core Interfaces, $Date: 2008-07-25 10:23:20 +0100 (Fri, 25 Jul 2008) $, $Revision: 2610 $
//
/**
    这里标记了很多的数据类型
*/

#ifndef ZM_H
#define ZM_H

extern "C"
{
#include "zm_debug.h"
}

#include "zm_config.h"

extern "C"
{
#if !HAVE_DECL_ROUND
double round(double);
#endif
}

typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned long       U32;
typedef unsigned long long  U64;

typedef signed char         S8;
typedef signed short        S16;
typedef signed long         S32;
typedef signed long long    S64;

#endif // ZM_H
