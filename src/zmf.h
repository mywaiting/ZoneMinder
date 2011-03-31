//
// ZoneMinder Image File Write Class Interface, $Date: 2008-07-25 10:33:23 +0100 (Fri, 25 Jul 2008) $, $Revision: 2611 $
// Copyright (C) 2001-2008 Philip Coombes
// 


#ifndef ZMFILE_H
#define ZMFILE_H

struct FrameHeader
{
	unsigned long event_id;
    time_t event_time;
	unsigned long frame_id;
	bool alarm_frame;
	unsigned long image_length;
};

#endif // ZMFILE_H
