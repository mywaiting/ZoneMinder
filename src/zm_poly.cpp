//
// ZoneMinder Polygon Class Implementation, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#include "zm.h"
#include "zm_poly.h"

#include <math.h>

void Polygon::calcArea()
{
	double float_area = 0.0L;
	for ( int i = 0, j = n_coords-1; i < n_coords; j = i++ )
	{
		double trap_area = ((coords[i].X()-coords[j].X())*((coords[i].Y()+coords[j].Y())))/2.0L;
		float_area += trap_area;
		//printf( "%.2f (%.2f)\n", float_area, trap_area );
	}
	area = (int)round(fabs(float_area));
}

void Polygon::calcCentre()
{
	if ( !area && n_coords )
		calcArea();
	double float_x = 0.0L, float_y = 0.0L;
	for ( int i = 0, j = n_coords-1; i < n_coords; j = i++ )
	{
		float_x += ((coords[i].Y()-coords[j].Y())*((coords[i].X()*2)+(coords[i].X()*coords[j].X())+(coords[j].X()*2)));
		float_y += ((coords[j].X()-coords[i].X())*((coords[i].Y()*2)+(coords[i].Y()*coords[j].Y())+(coords[j].Y()*2)));
	}
	float_x /= (6*area);
	float_y /= (6*area);
	//printf( "%.2f,%.2f\n", float_x, float_y );
	centre = Coord( (int)round(float_x), (int)round(float_y) );
}

Polygon::Polygon( int p_n_coords, const Coord *p_coords ) : n_coords( p_n_coords )
{
	coords = new Coord[n_coords];

	int min_x = -1;
	int max_x = -1;
	int min_y = -1;
	int max_y = -1;
	for( int i = 0; i < n_coords; i++ )
	{
		coords[i] = p_coords[i];
		if ( min_x == -1 || coords[i].X() < min_x )
			min_x = coords[i].X();
		if ( max_x == -1 || coords[i].X() > max_x )
			max_x = coords[i].X();
		if ( min_y == -1 || coords[i].Y() < min_y )
			min_y = coords[i].Y();
		if ( max_y == -1 || coords[i].Y() > max_y )
			max_y = coords[i].Y();
	}
	extent = Box( min_x, min_y, max_x, max_y );
	calcArea();
	calcCentre();
}

Polygon::Polygon( const Polygon &p_polygon ) : n_coords( p_polygon.n_coords ), extent( p_polygon.extent ), area( p_polygon.area ), centre( p_polygon.centre )
{
	coords = new Coord[n_coords];
	for( int i = 0; i < n_coords; i++ )
	{
		coords[i] = p_polygon.coords[i];
	}
}

Polygon &Polygon::operator=( const Polygon &p_polygon )
{
	if ( n_coords < p_polygon.n_coords )
	{
		delete[] coords;
		coords = new Coord[p_polygon.n_coords];
	}
	n_coords = p_polygon.n_coords;
	for( int i = 0; i < n_coords; i++ )
	{
		coords[i] = p_polygon.coords[i];
	}
	extent = p_polygon.extent;
	area = p_polygon.area;
	centre = p_polygon.centre;
	return( *this );
}

bool Polygon::isInside( const Coord &coord ) const
{
	bool inside = false;
	for ( int i = 0, j = n_coords-1; i < n_coords; j = i++ )
	{
		if ( (((coords[i].Y() <= coord.Y()) && (coord.Y() < coords[j].Y()) )
		|| ((coords[j].Y() <= coord.Y()) && (coord.Y() < coords[i].Y())))
		&& (coord.X() < (coords[j].X() - coords[i].X()) * (coord.Y() - coords[i].Y()) / (coords[j].Y() - coords[i].Y()) + coords[i].X()))
		{
			inside = !inside;
		}
	}
	return( inside );
}
