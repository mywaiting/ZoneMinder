//
// ZoneMinder Coordinate Class Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#ifndef ZM_COORD_H
#define ZM_COORD_H

#include "zm.h"

//
// Class used for storing an x,y pair, i.e. a coordinate
//
class Coord
{
private:
	int x, y;

public:
	inline Coord() : x(0), y(0)
	{
	}
	inline Coord( int p_x, int p_y ) : x(p_x), y(p_y)
	{
	}
	inline Coord( const Coord &p_coord ) : x(p_coord.x), y(p_coord.y)
	{
	}
	inline int &X() { return( x ); }
	inline const int &X() const { return( x ); }
	inline int &Y() { return( y ); }
	inline const int &Y() const { return( y ); }
        /**

          */
	inline static Coord Range( const Coord &coord1, const Coord &coord2 )
	{
		Coord result( (coord1.x-coord2.x)+1, (coord1.y-coord2.y)+1 );
		return( result );
	}

	inline bool operator==( const Coord &coord ) { return( x == coord.x && y == coord.y ); }
	inline bool operator!=( const Coord &coord ) { return( x != coord.x || y != coord.y ); }
	inline bool operator>( const Coord &coord ) { return( x > coord.x && y > coord.y ); }
	inline bool operator>=( const Coord &coord ) { return( !(operator<(coord)) ); }
	inline bool operator<( const Coord &coord ) { return( x < coord.x && y < coord.y ); }
	inline bool operator<=( const Coord &coord ) { return( !(operator>(coord)) ); }
	inline Coord &operator+=( const Coord &coord ) { x += coord.x; y += coord.y; return( *this ); }
	inline Coord &operator-=( const Coord &coord ) { x -= coord.x; y -= coord.y; return( *this ); }

	inline friend Coord operator+( const Coord &coord1, const Coord &coord2 ) { Coord result( coord1 ); result += coord2; return( result ); }
	inline friend Coord operator-( const Coord &coord1, const Coord &coord2 ) { Coord result( coord1 ); result -= coord2; return( result ); }
};

#endif // ZM_COORD_H
