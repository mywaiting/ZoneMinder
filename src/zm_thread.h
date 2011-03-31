//
// ZoneMinder Thread Class Interface, $Date: 2009-06-03 09:10:02 +0100 (Wed, 03 Jun 2009) $, $Revision: 2906 $
//

#ifndef ZM_THREAD_H
#define ZM_THREAD_H

#include <pthread.h>
#include "zm_exception.h"
#include "zm_utils.h"

/**
    用于抛出 Exception的Class
 */
class ThreadException : public Exception
{
public:
    ThreadException( const std::string &message ) : Exception( stringtf( "(%d) "+message, (long int)syscall(224) ) )
    {
    }
};

/**
    用于实现 C++多线程切换的“互斥锁”

    任何写过多线程程序的人都知道避免不同线程同时访问共享区域的重要性。
    如果一个线程要改变共享区域中某个数据，而与此同时另一线程正在读这个数据，那么结果将是未定义的。
    为了避免这种情况的发生就要使用一些特殊的原始类型和操作。
    其中最基本的就是互斥体（mutex，mutual exclusion的缩写）。
    一个互斥体一次只允许一个线程访问共享区。当一个线程想要访问共享区时，首先要做的就是锁住（lock）互斥体。
    如果其他的线程已经锁住了互斥体，那么就必须先等那个线程将互斥体解锁，这样就保证了同一时刻只有一个线程能访问共享区域。
 **/
class Mutex
{
friend class Condition;

private:
    pthread_mutex_t mMutex;

public:
    Mutex();
    ~Mutex();

private:
    pthread_mutex_t *getMutex()
    {
        return( &mMutex );
    }

public:
    void lock();
    void lock( int secs );
    void lock( double secs );
    void unlock();
    bool locked();
};

/**

*/
class ScopedMutex
{
private:
    Mutex &mMutex;

public:
    ScopedMutex( Mutex &mutex ) : mMutex( mutex )
    {
        mMutex.lock();
    }
    ~ScopedMutex()
    {
        mMutex.unlock();
    }

private:
    ScopedMutex( const ScopedMutex & );
};

class Condition
{
private:
    Mutex &mMutex;
    pthread_cond_t mCondition;

public:
    Condition( Mutex &mutex );
    ~Condition();

    void wait();
    bool wait( int secs );
    bool wait( double secs );
    void signal();
    void broadcast();
};

class Semaphore : public Condition
{
private:
    Mutex mMutex;

public:
    Semaphore() : Condition( mMutex )
    {
    }

    void wait()
    {
        mMutex.lock();
        Condition::wait();
        mMutex.unlock();
    }
    bool wait( int secs )
    {
        mMutex.lock();
        bool result = Condition::wait( secs );
        mMutex.unlock();
        return( result );
    }
    bool wait( double secs )
    {
        mMutex.lock();
        bool result = Condition::wait( secs );
        mMutex.unlock();
        return( result );
    }
    void signal()
    {
        mMutex.lock();
        Condition::signal();
        mMutex.unlock();
    }
    void broadcast()
    {
        mMutex.lock();
        Condition::broadcast();
        mMutex.unlock();
    }
};

template <class T> class ThreadData
{
private:
    T mValue;
    mutable bool mChanged;
    mutable Mutex mMutex;
    mutable Condition mCondition;

public:
    ThreadData() : mCondition( mMutex )
    {
    }
    ThreadData( T value ) : mValue( value ), mCondition( mMutex )
    {
    }
    //~ThreadData() {}

    operator T() const
    {
        return( getValue() );
    }
    const T operator=( const T value )
    {
        return( setValue( value ) );
    }

    const T getValueImmediate() const
    {
        return( mValue );
    }
    T setValueImmediate( const T value )
    {
        return( mValue = value );
    }
    const T getValue() const;
    T setValue( const T value );
    const T getUpdatedValue() const;
    const T getUpdatedValue( double secs ) const;
    const T getUpdatedValue( int secs ) const;
    void updateValueSignal( const T value );
    void updateValueBroadcast( const T value );
};

class Thread
{
public:
    typedef void *(*ThreadFunc)( void * );

protected:
    pthread_t mThread;

    Mutex mThreadMutex;
    Condition mThreadCondition;
    pid_t mPid;
    bool  mStarted;
    bool  mRunning;

protected:
    Thread();
    virtual ~Thread();

    pid_t id() const
    {
        return( (pid_t)syscall(224) );
    }
    void exit( int status = 0 )
    {
        //INFO( "Exiting" );
        pthread_exit( (void *)status );
    }
    static void *mThreadFunc( void *arg );

public:
    virtual int run() = 0;

    void start();
    void join();
    void kill( int signal );
    bool isThread()
    {
        return( mPid > -1 && pthread_equal( pthread_self(), mThread ) );
    }
    bool isStarted() const { return( mStarted ); }
    bool isRunning() const { return( mRunning ); }
};

#endif // ZM_THREAD_H
