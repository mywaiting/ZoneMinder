//
// ZoneMinder Polygon Class Interfaces, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#ifndef ZM_POLY_H
#define ZM_POLY_H

#include "zm.h"
#include "zm_coord.h"
#include "zm_box.h"

#include <math.h>

//
// Class used for storing a box, which is defined as a region
// defined by two coordinates
//
class Polygon
{
protected:
	struct Edge
	{
		int min_y;
		int max_y;
		double min_x;
		double _1_m;

		static int CompareYX( const void *p1, const void *p2 )
		{
			const Edge *e1 = (const Edge *)p1, *e2 = (const Edge *)p2;
			if ( e1->min_y == e2->min_y )
				return( int(e1->min_x - e2->min_x) );
			else
				return( int(e1->min_y - e2->min_y) );
		}
		static int CompareX( const void *p1, const void *p2 )
		{
			const Edge *e1 = (const Edge *)p1, *e2 = (const Edge *)p2;
			return( int(e1->min_x - e2->min_x) );
		}
	};

	struct Slice
	{
		int min_x;
		int max_x;
		int n_edges;
		int *edges;

		Slice()
		{
			n_edges = 0;
			edges = 0;
		}
		~Slice()
		{
			delete edges;
		}
	};

protected:
	int n_coords;
	Coord *coords;
	Box extent;
	int area;
	Coord centre;
	Edge *edges;
	Slice *slices;

protected:
	void initialiseEdges();
	void calcArea();
	void calcCentre();

public:
	inline Polygon() : n_coords( 0 ), coords( 0 ), area( 0 )
	{
	}
	Polygon( int p_n_coords, const Coord *p_coords );
	Polygon( const Polygon &p_polygon );
	~Polygon()
	{
		delete[] coords;
	}

	Polygon &operator=( const Polygon &p_polygon );

	inline int getNumCoords() const { return( n_coords ); }
	inline const Coord &getCoord( int index ) const
	{
		return( coords[index] );
	}

	inline const Box &Extent() const { return( extent ); }
	inline int LoX() const { return( extent.LoX() ); }
	inline int HiX() const { return( extent.HiX() ); }
	inline int LoY() const { return( extent.LoY() ); }
	inline int HiY() const { return( extent.HiY() ); }
	inline int Width() const { return( extent.Width() ); }
	inline int Height() const { return( extent.Height() ); }

	inline int Area() const { return( area ); }
	inline const Coord &Centre() const
	{
		return( centre );
	}
	bool isInside( const Coord &coord ) const;
};

#endif // ZM_POLY_H
