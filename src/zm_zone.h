//
// ZoneMinder Zone Class Interfaces, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 



#ifndef ZM_ZONE_H
#define ZM_ZONE_H

#include "zm_rgb.h"
#include "zm_coord.h"
#include "zm_poly.h"
#include "zm_image.h"
#include "zm_event.h"

class Monitor;

//
// This describes a 'zone', or an area of an image that has certain
// detection characteristics.
//
class Zone
{
protected:
	struct Range
	{
		int lo_x;
		int hi_x;
		int off_x;
	};

public:
        /**
            `Type` enum('Active','Inclusive','Exclusive','Preclusive','Inactive') NOT NULL default 'Active',
            `Units` enum('Pixels','Percent') NOT NULL default 'Pixels',
        */
	typedef enum { ACTIVE=1, INCLUSIVE, EXCLUSIVE, PRECLUSIVE, INACTIVE } ZoneType;
	typedef enum { ALARMED_PIXELS=1, FILTERED_PIXELS, BLOBS } CheckMethod;

protected:
	// Inputs
	Monitor			*monitor;

        int			id;
	char			*label;
	ZoneType		type;
	Polygon			polygon;
        Rgb			alarm_rgb;
	CheckMethod		check_method;

	int				min_pixel_threshold;
	int				max_pixel_threshold;

	int				min_alarm_pixels;
	int				max_alarm_pixels;

	Coord			filter_box;
	int				min_filter_pixels;
	int				max_filter_pixels;

	int				min_blob_pixels;
	int				max_blob_pixels;
	int				min_blobs;
	int				max_blobs;

        int             overload_frames;

	// Outputs/Statistics
	bool			alarmed;
	int				pixel_diff;
	int				alarm_pixels;
	int				alarm_filter_pixels;
	int				alarm_blob_pixels;
	int				alarm_blobs;
	int				min_blob_size;
	int				max_blob_size;
	Box				alarm_box;
	Coord			alarm_centre;
	unsigned int	score;
	Image			*pg_image;
	Range			*ranges;
	Image			*image;

    int             overload_count;

protected:
	void Setup( Monitor *p_monitor, int p_id, const char *p_label, ZoneType p_type, const Polygon &p_polygon, const Rgb p_alarm_rgb, CheckMethod p_check_method, int p_min_pixel_threshold, int p_max_pixel_threshold, int p_min_alarm_pixels, int p_max_alarm_pixels, const Coord &p_filter_box, int p_min_filter_pixels, int p_max_filter_pixels, int p_min_blob_pixels, int p_max_blob_pixels, int p_min_blobs, int p_max_blobs, int p_overload_frames );

public:
	Zone( Monitor *p_monitor, int p_id, const char *p_label, ZoneType p_type, const Polygon &p_polygon, const Rgb p_alarm_rgb, CheckMethod p_check_method, int p_min_pixel_threshold=15, int p_max_pixel_threshold=0, int p_min_alarm_pixels=50, int p_max_alarm_pixels=75000, const Coord &p_filter_box=Coord( 3, 3 ), int p_min_filter_pixels=50, int p_max_filter_pixels=50000, int p_min_blob_pixels=10, int p_max_blob_pixels=0, int p_min_blobs=0, int p_max_blobs=0, int p_overload_frames=0 )
	{
		Setup( p_monitor, p_id, p_label, p_type, p_polygon, p_alarm_rgb, p_check_method, p_min_pixel_threshold, p_max_pixel_threshold, p_min_alarm_pixels, p_max_alarm_pixels, p_filter_box, p_min_filter_pixels, p_max_filter_pixels, p_min_blob_pixels, p_max_blob_pixels, p_min_blobs, p_max_blobs, p_overload_frames );
	}
	Zone( Monitor *p_monitor, int p_id, const char *p_label, const Polygon &p_polygon, const Rgb p_alarm_rgb, CheckMethod p_check_method, int p_min_pixel_threshold=15, int p_max_pixel_threshold=0, int p_min_alarm_pixels=50, int p_max_alarm_pixels=75000, const Coord &p_filter_box=Coord( 3, 3 ), int p_min_filter_pixels=50, int p_max_filter_pixels=50000, int p_min_blob_pixels=10, int p_max_blob_pixels=0, int p_min_blobs=0, int p_max_blobs=0, int p_overload_frames=0 )
	{
		Setup( p_monitor, p_id, p_label, Zone::ACTIVE, p_polygon, p_alarm_rgb, p_check_method, p_min_pixel_threshold, p_max_pixel_threshold, p_min_alarm_pixels, p_max_alarm_pixels, p_filter_box, p_min_filter_pixels, p_max_filter_pixels, p_min_blob_pixels, p_max_blob_pixels, p_min_blobs, p_max_blobs, p_overload_frames );
	}
	Zone( Monitor *p_monitor, int p_id, const char *p_label, const Polygon &p_polygon )
	{
		Setup( p_monitor, p_id, p_label, Zone::INACTIVE, p_polygon, RGB_BLACK, (Zone::CheckMethod)0, 0, 0, 0, 0, Coord( 0, 0 ), 0, 0, 0, 0, 0, 0, 0 );
	}

public:
	~Zone();

	inline int Id() const { return( id ); }
	inline const char *Label() const { return( label ); }
	inline ZoneType Type() const { return( type ); }
	inline bool IsActive() const { return( type == ACTIVE ); }
	inline bool IsInclusive() const { return( type == INCLUSIVE ); }
	inline bool IsExclusive() const { return( type == EXCLUSIVE ); }
	inline bool IsPreclusive() const { return( type == PRECLUSIVE ); }
	inline bool IsInactive() const { return( type == INACTIVE ); }
	inline const Image *AlarmImage() const { return( image ); }
	inline const Polygon &GetPolygon() const { return( polygon ); }
	inline bool Alarmed() const { return( alarmed ); }
	inline void SetAlarm() { alarmed = true; }
	inline void ClearAlarm() { alarmed = false; }
	inline Coord GetAlarmCentre() const { return( alarm_centre ); }
	inline unsigned int Score() const { return( score ); }

	inline void ResetStats()
	{
		alarmed = false;
		pixel_diff = 0;
		alarm_pixels = 0;
		alarm_filter_pixels = 0;
		alarm_blob_pixels = 0;
		alarm_blobs = 0;
		min_blob_size = 0;
		max_blob_size = 0;
		score = 0;
	}
	void RecordStats( const Event *event );
	bool CheckAlarms( const Image *delta_image );
	bool DumpSettings( char *output, bool verbose );

	static bool ParsePolygonString( const char *polygon_string, Polygon &polygon );
	static bool ParseZoneString( const char *zone_string, int &zone_id, int &colour, Polygon &polygon );
	static int Load( Monitor *monitor, Zone **&zones );
};

#endif // ZM_ZONE_H
