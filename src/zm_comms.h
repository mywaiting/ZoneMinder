//
// ZoneMinder Communicatoions Class Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
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

#ifndef ZM_COMMS_H
#define ZM_COMMS_H

#include "zm_exception.h"

#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <sys/un.h>

#include <set>
#include <vector>


/**
  用于抛出错误的 Exception
 */
class CommsException : public Exception
{
public:
    CommsException( const std::string &message ) : Exception( message )
    {
    }
};


class CommsBase
{
protected:
	const int	&mRd;
	const int	&mWd;

protected:
	CommsBase( int &rd, int &wd ) : mRd( rd ), mWd( wd )
	{
	}
	virtual ~CommsBase()
	{
	}

public:
	virtual bool close()=0;
	virtual bool isOpen() const=0;
	virtual bool isClosed() const=0;
	virtual bool setBlocking( bool blocking )=0;

public:
	int getReadDesc() const
    {
        return( mRd );
    }
	int getWriteDesc() const
    {
        return( mWd );
    }
	int getMaxDesc() const
    {
        return( mRd>mWd?mRd:mWd );
    }

	virtual int read( void *msg, int len )
	{
		ssize_t nBytes = ::read( mRd, msg, len );
        if ( nBytes < 0 )
            Debug( 1, "Read of %d bytes max on rd %d failed: %s", len, mRd, strerror(errno) );
		return( nBytes );
	}
	virtual int write( const void *msg, int len )
	{
		ssize_t nBytes = ::write( mWd, msg, len );
        if ( nBytes < 0 )
            Debug( 1, "Write of %d bytes on wd %d failed: %s", len, mWd, strerror(errno) );
		return( nBytes );
	}
	virtual int readV( const struct iovec *iov, int iovcnt )
	{
		int nBytes = ::readv( mRd, iov, iovcnt );
        if ( nBytes < 0 )
            Debug( 1, "Readv of %d buffers max on rd %d failed: %s", iovcnt, mRd, strerror(errno) );
		return( nBytes );
	}
	virtual int writeV( const struct iovec *iov, int iovcnt )
	{
		ssize_t nBytes = ::writev( mWd, iov, iovcnt );
        if ( nBytes < 0 )
            Debug( 1, "Writev of %d buffers on wd %d failed: %s", iovcnt, mWd, strerror(errno) );
		return( nBytes );
	}
	virtual int readV( int iovcnt, /* const void *msg1, int len1, */ ... );
	virtual int writeV( int iovcnt, /* const void *msg1, int len1, */ ... );
};


class Pipe : public CommsBase
{
protected:
	int mFd[2];

public:
	Pipe() : CommsBase( mFd[0], mFd[1] )
	{
		mFd[0] = -1;
		mFd[1] = -1;
	}
	~Pipe()
	{
        close();
	}

public:
	bool open();
	bool close();

	bool isOpen() const
	{
		return( mFd[0] != -1 && mFd[1] != -1 );
	}
	int getReadDesc() const
	{
		return( mFd[0] );
	}
	int getWriteDesc() const
	{
		return( mFd[1] );
	}

	bool setBlocking( bool blocking );
};


/**

    C++ Socket 编程:

1. 创建套接字──socket()
    应用程序在使用套接字前，首先必须拥有一个套接字，系统调用socket()向应用程序提供创建套接字的手段，
    其调用格式如下：

        SOCKET PASCAL FAR socket(int af, int type, int protocol);

    该调用要接收三个参数：af、type、protocol
    参数af指定通信发生的区域，：AF_UNIX、AF_INET、AF_NS等，而DOS、 WINDOWS中仅支持AF_INET，它是网际网区域。因此，地址族与协议族相同。
    参数type 描述要建立的套接字的类型。这里分三种:
        一是TCP流式套接字(SOCK_STREAM)提供了一个面向连接、可靠的数据传输服务，数据无差错、无重复地发送，且按发送顺序接收。内设流量控制，避免数据流超限；数据被看作是字节流，无长度限制。文件传送协议（FTP）即使用流式套接字。
        二是数据报式套接字(SOCK_DGRAM)提供了一个无连接服务。数据包以独立包形式被发送，不提供无错保证,数据可能丢失或重复，并且接收顺序混乱。网络文件系统（NFS）使用数据报式套接字。
        三是原始式套接字 (SOCK_RAW)该接口允许对较低层协议，如IP、ICMP直接访问。常用于检验新的协议实现或访问现有服务中配置的新设备.
    参数protocol说明该套接字使用的特定协议，如果调用者不希望特别指定使用的协议，则置为0，使用默认的连接模式。根据这三个参数建立一个套接字，并将相应的资源分配给它，同时返回一个整型套接字号。因此，socket()系统调用实际上指定了相关五元组中的“协议”这一元。

2. 指定本地地址──bind()
    当一个套接字用socket()创建后，存在一个名字空间(地址族),但它没有被命名
    bind()将套接字地址（包括本地主机地址和本地端口地址）与所创建的套接字号联系起来，即将名字赋予套接字，以指定本地半相关。
    其调用格式如下：

        int PASCAL FAR bind(SOCKET s, const struct sockaddr FAR * name, int namelen);

    参数s是由socket()调用返回的并且未作连接的套接字描述符(套接字号)。
    参数name 是赋给套接字s的本地地址（名字），其长度可变，结构随通信域的不同而不同。
    namelen表明了name的长度.如果没有错误发生，bind()返回0。否则返回SOCKET_ERROR。

3. 建立套接字连接──connect()与accept()
    这两个系统调用用于完成一个完整相关的建立，其中connect()用于建立连接。
    无连接的套接字进程也可以调用connect()，但这时在进程之间没有实际的报文交换，
    调用将从本地操作系统直接返回。这样做的优点是程序员不必为每一数据指定目的地址，而且如果收到的一个数据报，其目的端口未与任何套接字建立“连接”，便能判断该端靠纪纪可操作。
    而accept()用于使服务器等待来自某客户进程的实际连接。

    connect()的调用格式如下：

        int PASCAL FAR connect(SOCKET s, const struct sockaddr FAR * name, int namelen);

    参数s是欲建立连接的本地套接字描述符。
    参数name指出说明对方套接字地址结构的指针。
    对方套接字地址长度由namelen说明。

    如果没有错误发生，connect()返回0。否则返回值SOCKET_ERROR。
    在面向连接的协议中，该调用导致本地系统和外部系统之间连接实际建立。

    由于地址族总被包含在套接字地址结构的前两个字节中，并通过socket()调用与某个协议族相关。
    因此bind()和connect()无须协议作为参数。



    accept()的调用格式如下：

        SOCKET PASCAL FAR accept(SOCKET s, struct sockaddr FAR* addr, int FAR* addrlen);

    参数s为本地套接字描述符，在用做accept()调用的参数前应该先调用过listen()。
    addr 指向客户方套接字地址结构的指针，用来接收连接实体的地址。addr的确切格式由套接字创建时建立的地址族决定。
    addrlen 为客户方套接字地址的长度（字节数）。

    如果没有错误发生，accept()返回一个SOCKET类型的值，表示接收到的套接字的描述符。否则返回值INVALID_SOCKET。

    accept() 用于面向连接服务器。参数addr和addrlen存放客户方的地址信息。
    调用前，参数addr 指向一个初始值为空的地址结构，而addrlen 的初始值为0；
    调用accept()后，服务器等待从编号为s的套接字上接受客户连接请求，而连接请求是由客户方的connect()调用发出的。
    当有连接请求到达时，accept()调用将请求连接队列上的第一个客户方套接字地址及长度放入addr 和addrlen，并创建一个与s有相同特性的新套接字号。新的套接字可用于处理服务器并发请求。

4. 监听连接──listen()
    此调用用于面向连接服务器，表明它愿意接收连接。listen()需在 accept()之前调用，其调用格式如下：

        int PASCAL FAR listen(SOCKET s, int backlog);

    参数s标识一个本地已建立、尚未连接的套接字号，服务器愿意从它上面接收请求。
    参数backlog表示请求连接队列的最大长度，用于限制排队请求的个数，目前允许的最大值为5。如果没有错误发生，listen()返回0。否则它返回SOCKET_ERROR。

    listen()在执行调用过程中可为没有调用过bind()的套接字s完成所必须的连接，并建立长度为backlog的请求连接队列。

    调用listen()是服务器接收一个连接请求的四个步骤中的第三步。
    它在调用socket()分配一个流套接字，且调用bind()给s赋于一个名字之后调用，而且一定要在 accept()之前调用。

5. 数据传输──send()与recv()

    send()调用用于钥纪纪数s指定的已连接的数据报或流套接字上发送输出数据，格式如下：

        int PASCAL FAR send(SOCKET s, const char FAR *buf, int len, int flags);

    参数s为已连接的本地套接字描述符。
    buf 指向存有发送数据的缓冲区的指针，其长度由len 指定。
    flags 指定传输控制方式，如是否发送带外数据等。

    如果没有错误发生，send()返回总共发送的字节数。否则它返回SOCKET_ERROR。

    recv()调用用于s指定的已连接的数据报或流套接字上接收输入数据，格式如下：

        int PASCAL FAR recv(SOCKET s, char FAR *buf, int len, int flags);

    参数s 为已连接的套接字描述符。
    buf指向接收输入数据缓冲区的指针，其长度由len 指定。
    flags 指定传输控制方式，如是否接收带外数据等。

    如果没有错误发生，recv()返回总共接收的字节数。如果连接被关闭，返回0。否则它返回SOCKET_ERROR。

6. 输入/输出多路复用──select()
    select()调用用来检测一个或多个套接字的状态。
    对每一个套接字来说，这个调用可以请求读、写或错误状态方面的信息。
    请求给定状态的套接字集合由一个fd_set结构指示。在返回时，此结构被更新，以反映那些满足特定条件的套接字的子集，同时， select()调用返回满足条件的套接字的数目，
    其调用格式如下：

        int PASCAL FAR select(int nfds, fd_set FAR * readfds, fd_set FAR * writefds, fd_set FAR * exceptfds, const struct timeval FAR * timeout);

    参数nfds指明被检查的套接字描述符的值域，此变量一般被忽略。
    参数readfds指向要做读检测的套接字描述符集合的指针，调用者希望从中读取数据。
    参数writefds 指向要做写检测的套接字描述符集合的指针。
    exceptfds指向要检测是否出错的套接字描述符集合的指针。
    timeout指向select()函数等待的最大时间，如果设为NULL则为阻塞操作。

    select()返回包含在fd_set结构中已准备好的套接字描述符的总数目，或者是发生错误则返回SOCKET_ERROR。

7. 关闭套接字──closesocket()
    closesocket()关闭套接字s，并释放分配给该套接字的资源；
    如果s涉及一个打开的TCP连接，则该连接被释放。
    closesocket()的调用格式如下：

        BOOL PASCAL FAR closesocket(SOCKET s);

    参数s待关闭的套接字描述符。

    如果没有错误发生，closesocket()返回0。否则返回值SOCKET_ERROR。

 */

class SockAddr
{
private:
    const struct sockaddr *mAddr;

public:
    SockAddr( const struct sockaddr *addr );
    virtual ~SockAddr()
    {
    }

    static SockAddr *newSockAddr( const struct sockaddr &addr, socklen_t len );
    static SockAddr *newSockAddr( const SockAddr *addr );

    int getDomain() const
    {
        return( mAddr?mAddr->sa_family:AF_UNSPEC );
    }

    const struct sockaddr *getAddr() const
    {
        return( mAddr );
    }
    virtual socklen_t getAddrSize() const=0;
    virtual struct sockaddr *getTempAddr() const=0;
};

class SockAddrInet : public SockAddr
{
private:
    struct sockaddr_in  mAddrIn;
    struct sockaddr_in  mTempAddrIn;

public:
	SockAddrInet();
	SockAddrInet( const SockAddrInet &addr ) : SockAddr( (const struct sockaddr *)&mAddrIn ), mAddrIn( addr.mAddrIn )
    {
    }
	SockAddrInet( const struct sockaddr_in *addr ) : SockAddr( (const struct sockaddr *)&mAddrIn ), mAddrIn( *addr )
    {
    }


	bool resolve( const char *host, const char *serv, const char *proto );
	bool resolve( const char *host, int port, const char *proto );
	bool resolve( const char *serv, const char *proto );
	bool resolve( int port, const char *proto );

    socklen_t getAddrSize() const
    {
        return( sizeof(mAddrIn) );
    }
    struct sockaddr *getTempAddr() const
    {
        return( (sockaddr *)&mTempAddrIn );
    }

public:
    static socklen_t addrSize()
    {
        return( sizeof(sockaddr_in) );
    }
};

class SockAddrUnix : public SockAddr
{
private:
    struct sockaddr_un  mAddrUn;
    struct sockaddr_un  mTempAddrUn;

public:
	SockAddrUnix();
	SockAddrUnix( const SockAddrUnix &addr ) : SockAddr( (const struct sockaddr *)&mAddrUn ), mAddrUn( addr.mAddrUn )
    {
    }
	SockAddrUnix( const struct sockaddr_un *addr ) : SockAddr( (const struct sockaddr *)&mAddrUn ), mAddrUn( *addr )
    {
    }

	bool resolve( const char *path, const char *proto );

    socklen_t getAddrSize() const
    {
        return( sizeof(mAddrUn) );
    }
    struct sockaddr *getTempAddr() const
    {
        return( (sockaddr *)&mTempAddrUn );
    }

public:
    static socklen_t addrSize()
    {
        return( sizeof(sockaddr_un) );
    }
};

class Socket : public CommsBase
{
protected:
    /**

     */
    typedef enum { CLOSED, DISCONNECTED, LISTENING, CONNECTED } State;

protected:
	int	mSd;
    State mState;
    SockAddr *mLocalAddr;
    SockAddr *mRemoteAddr;

protected:
	Socket() : CommsBase( mSd, mSd ), mSd( -1 ), mState( CLOSED ), mLocalAddr( 0 ), mRemoteAddr( 0 )
	{
	}
	Socket( const Socket &socket, int newSd ) : CommsBase( mSd, mSd ), mSd( newSd ), mState( CONNECTED ), mLocalAddr( 0 ), mRemoteAddr( 0 )
	{
        if ( socket.mLocalAddr )
            mLocalAddr = SockAddr::newSockAddr( mLocalAddr );
        if ( socket.mRemoteAddr )
            mRemoteAddr = SockAddr::newSockAddr( mRemoteAddr );
	}
	virtual ~Socket()
	{
		close();
        delete mLocalAddr;
        delete mRemoteAddr;
	}

public:
    bool isOpen() const
    {
        return( !isClosed() );
    }
    bool isClosed() const
    {
        return( mState == CLOSED );
    }
    bool isDisconnected() const
    {
        return( mState == DISCONNECTED );
    }
    bool isConnected() const
    {
        return( mState == CONNECTED );
    }
	virtual bool close();

protected:
    bool isListening() const
    {
        return( mState == LISTENING );
    }

protected:
	virtual bool socket();
	virtual bool bind();

protected:
	virtual bool connect();
	virtual bool listen();
	virtual bool accept();
	virtual bool accept( int & );

public:
	virtual int send( const void *msg, int len ) const
	{
		ssize_t nBytes = ::send( mSd, msg, len, 0 );
        if ( nBytes < 0 )
            Debug( 1, "Send of %d bytes on sd %d failed: %s", len, mSd, strerror(errno) );
		return( nBytes );
	}
	virtual int recv( void *msg, int len ) const
	{
		ssize_t nBytes = ::recv( mSd, msg, len, 0 );
        if ( nBytes < 0 )
            Debug( 1, "Recv of %d bytes max on sd %d failed: %s", len, mSd, strerror(errno) );
		return( nBytes );
	}
	virtual int send( const std::string &msg ) const
	{
		ssize_t nBytes = ::send( mSd, msg.data(), msg.size(), 0 );
        if ( nBytes < 0 )
            Debug( 1, "Send of string '%s' (%d bytes) on sd %d failed: %s", msg.c_str(), msg.size(), mSd, strerror(errno) );
		return( nBytes );
	}
	virtual int recv( std::string &msg ) const
	{
        char buffer[msg.capacity()];
        int nBytes = 0;
		if ( (nBytes = ::recv( mSd, buffer, sizeof(buffer), 0 )) < 0 )
        {
            Debug( 1, "Recv of %d bytes max to string on sd %d failed: %s", sizeof(buffer), mSd, strerror(errno) );
            return( nBytes );
        }
        buffer[nBytes] = '\0';
        msg = buffer;
        return( nBytes );
	}
	virtual int recv( std::string &msg, size_t maxLen ) const
	{
        char buffer[maxLen];
        int nBytes = 0;
		if ( (nBytes = ::recv( mSd, buffer, sizeof(buffer), 0 )) < 0 )
        {
            Debug( 1, "Recv of %d bytes max to string on sd %d failed: %s", maxLen, mSd, strerror(errno) );
            return( nBytes );
        }
        buffer[nBytes] = '\0';
        msg = buffer;
        return( nBytes );
	}
	virtual int bytesToRead() const;

	int getDesc() const
	{
		return( mSd );
	}
	//virtual bool isOpen() const
	//{
		//return( mSd != -1 );
	//}

    virtual int getDomain() const=0;
    virtual int getType() const=0;
    virtual const char *getProtocol() const=0;

    const SockAddr *getLocalAddr() const
    {
        return( mLocalAddr );
    }
    const SockAddr *getRemoteAddr() const
    {
        return( mRemoteAddr );
    }
    virtual socklen_t getAddrSize() const=0;

	bool getBlocking( bool &blocking );
	bool setBlocking( bool blocking );

	bool getSendBufferSize( int & ) const;
	bool getRecvBufferSize( int & ) const;

	bool setSendBufferSize( int );
	bool setRecvBufferSize( int );

	bool getRouting( bool & ) const;
	bool setRouting( bool );

	bool getNoDelay( bool & ) const;
	bool setNoDelay( bool );
};

class InetSocket : virtual public Socket
{
public:
    int getDomain() const
    {
        return( AF_INET );
    }
    virtual socklen_t getAddrSize() const
    {
        return( SockAddrInet::addrSize() );
    }

protected:
    bool resolveLocal( const char *host, const char *serv, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mLocalAddr = addr;
        return( addr->resolve( host, serv, proto ) );
    }
    bool resolveLocal( const char *host, int port, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mLocalAddr = addr;
        return( addr->resolve( host, port, proto ) );
    }
    bool resolveLocal( const char *serv, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mLocalAddr = addr;
        return( addr->resolve( serv, proto ) );
    }
    bool resolveLocal( int port, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mLocalAddr = addr;
        return( addr->resolve( port, proto ) );
    }

    bool resolveRemote( const char *host, const char *serv, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mRemoteAddr = addr;
        return( addr->resolve( host, serv, proto ) );
    }
    bool resolveRemote( const char *host, int port, const char *proto )
    {
        SockAddrInet *addr = new SockAddrInet;
        mRemoteAddr = addr;
        return( addr->resolve( host, port, proto ) );
    }

protected:
    bool bind( const SockAddrInet &addr )
    {
        mLocalAddr = new SockAddrInet( addr );
        return( Socket::bind() );
    }
    bool bind( const char *host, const char *serv ) 
    {
        if ( !resolveLocal( host, serv, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }
    bool bind( const char *host, int port )
    {
        if ( !resolveLocal( host, port, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }
    bool bind( const char *serv )
    {
        if ( !resolveLocal( serv, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }
    bool bind( int port )
    {
        if ( !resolveLocal( port, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }

    bool connect( const SockAddrInet &addr )
    {
        mRemoteAddr = new SockAddrInet( addr );
        return( Socket::connect() );
    }
    bool connect( const char *host, const char *serv )
    {
        if ( !resolveRemote( host, serv, getProtocol() ) )
            return( false );
        return( Socket::connect() );
    }
    bool connect( const char *host, int port )
    {
        if ( !resolveRemote( host, port, getProtocol() ) )
            return( false );
        return( Socket::connect() );
    }
};

class UnixSocket : virtual public Socket
{
public:
    int getDomain() const
    {
        return( AF_UNIX );
    }
    virtual socklen_t getAddrSize() const
    {
        return( SockAddrUnix::addrSize() );
    }

protected:
    bool resolveLocal( const char *serv, const char *proto )
    {
        SockAddrUnix *addr = new SockAddrUnix;
        mLocalAddr = addr;
        return( addr->resolve( serv, proto ) );
    }

    bool resolveRemote( const char *path, const char *proto )
    {
        SockAddrUnix *addr = new SockAddrUnix;
        mRemoteAddr = addr;
        return( addr->resolve( path, proto ) );
    }

protected:
    bool bind( const char *path )
    {
        if ( !UnixSocket::resolveLocal( path, getProtocol() ) )
            return( false );
        return( Socket::bind() );
    }

    bool connect( const char *path )
    {
        if ( !UnixSocket::resolveRemote( path, getProtocol() ) )
            return( false );
        return( Socket::connect() );
    }
};

class UdpSocket : virtual public Socket
{
public:
    int getType() const
    {
        return( SOCK_DGRAM );
    }
    const char *getProtocol() const
    {
        return( "udp" );
    }

public:
	virtual int sendto( const void *msg, int len, const SockAddr *addr=0 ) const
	{
		ssize_t nBytes = ::sendto( mSd, msg, len, 0, addr?addr->getAddr():NULL, addr?addr->getAddrSize():0 );
        if ( nBytes < 0 )
            Debug( 1, "Sendto of %d bytes on sd %d failed: %s", len, mSd, strerror(errno) );
		return( nBytes );
	}
	virtual int recvfrom( void *msg, int len, SockAddr *addr=0 ) const
	{
        ssize_t nBytes = 0;
        if ( addr )
        {
            struct sockaddr sockAddr;
            socklen_t sockLen;
		    nBytes = ::recvfrom( mSd, msg, len, 0, &sockAddr, &sockLen );
            if ( nBytes < 0 )
            {
                Debug( 1, "Recvfrom of %d bytes max on sd %d (with address) failed: %s", len, mSd, strerror(errno) );
            }
            else if ( sockLen )
            {
                addr = SockAddr::newSockAddr( sockAddr, sockLen );
            }
        }   
        else
        {
		    nBytes = ::recvfrom( mSd, msg, len, 0, NULL, 0 );
            if ( nBytes < 0 )
                Debug( 1, "Recvfrom of %d bytes max on sd %d (no address) failed: %s", len, mSd, strerror(errno) );
        }
        return( nBytes );
	}
};

class UdpInetSocket : virtual public UdpSocket, virtual public InetSocket
{
public:
    bool bind( const SockAddrInet &addr ) 
    {
        return( InetSocket::bind( addr ) );
    }
    bool bind( const char *host, const char *serv ) 
    {
        return( InetSocket::bind( host, serv ) );
    }
    bool bind( const char *host, int port )
    {
        return( InetSocket::bind( host, port ) );
    }
    bool bind( const char *serv )
    {
        return( InetSocket::bind( serv ) );
    }
    bool bind( int port )
    {
        return( InetSocket::bind( port ) );
    }

    bool connect( const SockAddrInet &addr ) 
    {
        return( InetSocket::connect( addr ) );
    }
    bool connect( const char *host, const char *serv )
    {
        return( InetSocket::connect( host, serv ) );
    }
    bool connect( const char *host, int port )
    {
        return( InetSocket::connect( host, port ) );
    }
};

class UdpUnixSocket : virtual public UdpSocket, virtual public UnixSocket
{
public:
    bool bind( const char *path )
    {
        return( UnixSocket::bind( path ) );
    }

    bool connect( const char *path )
    {
        return( UnixSocket::connect( path ) );
    }
};

class UdpInetClient : public UdpInetSocket
{
protected:
    bool bind( const SockAddrInet &addr ) 
    {
        return( UdpInetSocket::bind( addr ) );
    }
    bool bind( const char *host, const char *serv )
    {
        return( UdpInetSocket::bind( host, serv ) );
    }
    bool bind( const char *host, int port )
    {
        return( UdpInetSocket::bind( host, port ) );
    }
    bool bind( const char *serv )
    {
        return( UdpInetSocket::bind( serv ) );
    }
    bool bind( int port )
    {
        return( UdpInetSocket::bind( port ) );
    }

public:
    bool connect( const SockAddrInet &addr ) 
    {
        return( UdpInetSocket::connect( addr ) );
    }
    bool connect( const char *host, const char *serv )
    {
        return( UdpInetSocket::connect( host, serv ) );
    }
    bool connect( const char *host, int port )
    {
        return( UdpInetSocket::connect( host, port ) );
    }
};

class UdpUnixClient : public UdpUnixSocket
{
public:
    bool bind( const char *path )
    {
        return( UdpUnixSocket::bind( path ) );
    }

public:
    bool connect( const char *path )
    {
        return( UdpUnixSocket::connect( path) );
    }
};

class UdpInetServer : public UdpInetSocket
{
public:
    bool bind( const SockAddrInet &addr ) 
    {
        return( UdpInetSocket::bind( addr ) );
    }
    bool bind( const char *host, const char *serv )
    {
        return( UdpInetSocket::bind( host, serv ) );
    }
    bool bind( const char *host, int port )
    {
        return( UdpInetSocket::bind( host, port ) );
    }
    bool bind( const char *serv )
    {
        return( UdpInetSocket::bind( serv ) );
    }
    bool bind( int port )
    {
        return( UdpInetSocket::bind( port ) );
    }

protected:
    bool connect( const char *host, const char *serv )
    {
        return( UdpInetSocket::connect( host, serv ) );
    }
    bool connect( const char *host, int port )
    {
        return( UdpInetSocket::connect( host, port ) );
    }
};

class UdpUnixServer : public UdpUnixSocket
{
public:
    bool bind( const char *path )
    {
        return( UdpUnixSocket::bind( path ) );
    }

protected:
    bool connect( const char *path )
    {
        return( UdpUnixSocket::connect( path ) );
    }
};

class TcpSocket : virtual public Socket
{
public:
    TcpSocket()
    {
    }
    TcpSocket( const TcpSocket &socket, int newSd ) : Socket( socket, newSd )
    {
    }

public:
    int getType() const
    {
        return( SOCK_STREAM );
    }
    const char *getProtocol() const
    {
        return( "tcp" );
    }
};

class TcpInetSocket : virtual public TcpSocket, virtual public InetSocket
{
public:
    TcpInetSocket()
    {
    }
    TcpInetSocket( const TcpInetSocket &socket, int newSd ) : TcpSocket( socket, newSd )
    {
    }
};

class TcpUnixSocket : virtual public TcpSocket, virtual public UnixSocket
{
public:
    TcpUnixSocket()
    {
    }
    TcpUnixSocket( const TcpUnixSocket &socket, int newSd ) : TcpSocket( socket, newSd )
    {
    }
};

class TcpInetClient : public TcpInetSocket
{
public:
    bool connect( const char *host, const char *serv )
    {
        return( TcpInetSocket::connect( host, serv ) );
    }
    bool connect( const char *host, int port )
    {
        return( TcpInetSocket::connect( host, port ) );
    }
};

class TcpUnixClient : public TcpUnixSocket
{
public:
    bool connect( const char *path )
    {
        return( TcpUnixSocket::connect( path) );
    }
};

class TcpInetServer : public TcpInetSocket
{
public:
    bool bind( const char *host, const char *serv )
    {
        return( TcpInetSocket::bind( host, serv ) );
    }
    bool bind( const char *host, int port )
    {
        return( TcpInetSocket::bind( host, port ) );
    }
    bool bind( const char *serv )
    {
        return( TcpInetSocket::bind( serv ) );
    }
    bool bind( int port )
    {
        return( TcpInetSocket::bind( port ) );
    }

public:
    bool isListening() const { return( Socket::isListening() ); }
    bool listen();
    bool accept();
    bool accept( TcpInetSocket *&newSocket );
};

class TcpUnixServer : public TcpUnixSocket
{
public:
    bool bind( const char *path )
    {
        return( TcpUnixSocket::bind( path ) );
    }

public:
    bool isListening() const { return( Socket::isListening() ); }
    bool listen();
    bool accept();
    bool accept( TcpUnixSocket *&newSocket );
};

class Select
{
public:
    typedef std::set<CommsBase *> CommsSet;
    typedef std::vector<CommsBase *> CommsList;

protected:
    CommsSet        mReaders;
    CommsSet        mWriters;
    CommsList       mReadable;
    CommsList       mWriteable;
    bool            mHasTimeout;
    struct timeval  mTimeout;
    int             mMaxFd;

public:
    Select();
    Select( struct timeval timeout );
    Select( int timeout );
    Select( double timeout );

    void setTimeout( int timeout );
    void setTimeout( double timeout );
    void setTimeout( struct timeval timeout );
    void clearTimeout();

    void calcMaxFd();

    bool addReader( CommsBase *comms );
    bool deleteReader( CommsBase *comms );
    void clearReaders();

    bool addWriter( CommsBase *comms );
    bool deleteWriter( CommsBase *comms );
    void clearWriters();

    int wait();

    const CommsList &getReadable() const;
    const CommsList &getWriteable() const;
};

#endif // ZM_COMMS_H
