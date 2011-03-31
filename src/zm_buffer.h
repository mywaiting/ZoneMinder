/*
 * ZoneMinder Flexible Memory Interface, $Date: 2008-10-09 10:15:23 +0100 (Thu, 09 Oct 2008) $, $Revision: 2657 $
 * Copyright (C) 2001-2008 Philip Coombes
 * 
*/  

#ifndef ZM_BUFFER_H
#define ZM_BUFFER_H

#include "zm.h"

#include <string.h>

class Buffer
{
protected:
    unsigned char *mStorage;
    unsigned int mAllocation;
    unsigned int mSize;
    unsigned char *mHead;
    unsigned char *mTail;

public:
    Buffer() : mStorage( 0 ), mAllocation( 0 ), mSize( 0 ), mHead( 0 ), mTail( 0 )
    {
    }
    Buffer( unsigned int pSize ) : mAllocation( pSize ), mSize( 0 )
    {
        mHead = mStorage = new unsigned char[mAllocation];
        mTail = mHead;
    }
    Buffer( const unsigned char *pStorage, unsigned int pSize ) : mAllocation( pSize ), mSize( pSize )
    {
        mHead = mStorage = new unsigned char[mSize];
        memcpy( mStorage, pStorage, mSize );
        mTail = mHead + mSize;
    }
    Buffer( const Buffer &buffer ) : mAllocation( buffer.mSize ), mSize( buffer.mSize )
    {
        mHead = mStorage = new unsigned char[mSize];
        memcpy( mStorage, buffer.mHead, mSize );
        mTail = mHead + mSize;
    }
    ~Buffer()
    {
        delete[] mStorage;
    }
    unsigned char *head() const { return( mHead ); }
    unsigned char *tail() const { return( mTail ); }
    unsigned int size() const { return( mSize ); }
    bool empty() const { return( mSize == 0 ); }
    unsigned int size( unsigned int pSize )
    {
        if ( mSize < pSize )
        {
            expand( pSize-mSize );
        }
        return( mSize );
    }
    //unsigned int Allocation() const { return( mAllocation ); }

    void clear()
    {
        mSize = 0;
        mHead = mTail = mStorage;
    }

    unsigned int assign( const unsigned char *pStorage, unsigned int pSize );
    unsigned int assign( const Buffer &buffer )
    {
        return( assign( buffer.mHead, buffer.mSize ) );
    }

    // Trim from the front of the buffer
    unsigned int consume( unsigned int count )
    {
        if ( count > mSize )
        {
            Warning( "Attempt to consume %d bytes of buffer, size is only %d bytes", count, mSize );
            count = mSize;
        }
        mHead += count;
        mSize -= count;
        tidy( 0 );
        return( count );
    }
    // Trim from the end of the buffer
    unsigned int shrink( unsigned int count )
    {
        if ( count > mSize )
        {
            Warning( "Attempt to shrink buffer by %d bytes, size is only %d bytes", count, mSize );
            count = mSize;
        }
        mSize -= count;
        if ( mTail > (mHead + mSize) )
            mTail = mHead + mSize;
        tidy( 0 );
        return( count );
    }
    // Add to the end of the buffer
    unsigned int expand( unsigned int count );

    // Return pointer to the first pSize bytes and advance the head
    unsigned char *extract( unsigned int pSize )
    {
        if ( pSize > mSize )
        {
            Warning( "Attempt to extract %d bytes of buffer, size is only %d bytes", pSize, mSize );
            pSize = mSize;
        }
        unsigned char *oldHead = mHead;
        mHead += pSize;
        mSize -= pSize;
        tidy( 0 );
        return( oldHead );
    }
    // Add bytes to the end of the buffer
    unsigned int append( const unsigned char *pStorage, unsigned int pSize )
    {
        expand( pSize );
        memcpy( mTail, pStorage, pSize );
        mTail += pSize;
        mSize += pSize;
        return( mSize );
    }
    unsigned int append( const char *pStorage, unsigned int pSize )
    {
        return( append( (const unsigned char *)pStorage, pSize ) );
    }
    unsigned int append( const Buffer &buffer )
    {
        return( append( buffer.mHead, buffer.mSize ) );
    }
    void tidy( bool level=0 )
    {
        if ( mHead != mStorage )
        {
            if ( mSize == 0 )
                mHead = mTail = mStorage;
            else if ( level >= 1 )
            {
                if ( (mHead-mStorage) > mSize )
                {
                    memcpy( mStorage, mHead, mSize );
                    mHead = mStorage;
                    mTail = mHead + mSize;
                }
                else if ( level >= 2 )
                {
                    memmove( mStorage, mHead, mSize );
                    mHead = mStorage;
                    mTail = mHead + mSize;
                }
            }
        }
    }

    Buffer &operator=( const Buffer &buffer )
    {
        assign( buffer );
        return( *this );
    }
    Buffer &operator+=( const Buffer &buffer )
    {
        append( buffer );
        return( *this );
    }
    Buffer &operator+=( unsigned int count )
    {
        expand( count );
        return( *this );
    }
    Buffer &operator-=( unsigned int count )
    {
        consume( count );
        return( *this );
    }
    operator unsigned char *() const
    {
        return( mHead );
    }
    operator char *() const
    {
        return( (char *)mHead );
    }
    unsigned char *operator+(int offset) const
    {
        return( (unsigned char *)(mHead+offset) );
    }
    unsigned char operator[](int index) const
    {
        return( *(mHead+index) );
    }
    operator int () const
    {
        return( (int)mSize );
    }
};

#endif // ZM_BUFFER_H
