//
// ZoneMinder RTP/RTCP Class Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 

/**
    百度百科参考： http://baike.baidu.com/view/1149098.htm
    
    
    RTP/RTCP协议简介
　　实时传输协议RTP（Realtime Transport Protocol）：
        是针对Internet上多媒体数据流的一个传输协议, 由IETF(Internet工程任务组)作为RFC1889发布。
        RTP被定义为在一对一或一对多的传输情况下工作，其目的是提供时间信息和实现流同步。
        RTP的典型应用建立在UDP上，但也可以在TCP或ATM等其他协议之上工作。
        RTP本身只保证实时数据的传输，并不能为按顺序传送数据包提供可靠的传送机制，也不提供流量控制或拥塞控制，它依靠RTCP提供这些服务。
　　实时传输控制协议RTCP（Realtime Transport Control Protocol）：
        负责管理传输质量在当前应用进程之间交换控制信息。
        在RTP会话期间，各参与者周期性地传送RTCP包，包中含有已发送的数据包的数量、丢失的数据包的数量等统计资料，因此，服务器可以利用这些信息动态地改变传输速率，甚至改变有效载荷类型。
        RTP和RTCP配合使用，能以有效的反馈和最小的开销使传输效率最佳化，故特别适合传送网上的实时数据。 
*/


#ifndef ZM_RTP_H
#define ZM_RTP_H

#include "zm.h"

#define RTP_VERSION  2

#endif // ZM_RTP_H
