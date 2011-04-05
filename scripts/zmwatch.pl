#!/usr/bin/perl -wT
#
# ==========================================================================
#
# ZoneMinder WatchDog Script, $Date: 2009-06-08 10:11:56 +0100 (Mon, 08 Jun 2009) $, $Revision: 2908 $
# Copyright (C) 2001-2008 Philip Coombes
#
#
# 这个脚本将会为 ZoneMinder做一些基本的设置，然后周期性地检查每个活动输出流的 fps
# 如果坚持到他们被锁定或者有问题，就kill掉他们的进程然后将它们重启
#
use strict;
use bytes;

# ==========================================================================
#
# These are the elements you can edit to suit your installation
#
# ==========================================================================

use constant DBG_ID => "zmwatch"; # Tag that appears in debug to identify source
use constant DBG_LEVEL => 0; # 0 is errors, warnings and info only, > 0 for debug

use constant START_DELAY => 30; # To give everything else time to start

# ==========================================================================
#
# Don't change anything below here
#
# ==========================================================================

use lib '/home/stan/Work/Install/lib/perl5/site_perl/5.8.8'; # Include custom perl install path
use ZoneMinder;
use POSIX;
use DBI;
use Data::Dumper;

$| = 1;

$ENV{PATH}  = '/bin:/usr/bin';
$ENV{SHELL} = '/bin/sh' if exists $ENV{SHELL};
delete @ENV{qw(IFS CDPATH ENV BASH_ENV)};

sub Usage
{
    print( "
Usage: zmwatch.pl
");
	exit( -1 );
}

zmDbgInit( DBG_ID, level=>DBG_LEVEL );
zmDbgSetSignal();

Info( "Watchdog starting\n" );
Info( "Watchdog pausing for ".START_DELAY." seconds\n" );
sleep( START_DELAY );

my $dbh = zmDbConnect();

my $sql = "select * from Monitors";
my $sth = $dbh->prepare_cached( $sql ) or Fatal( "Can't prepare '$sql': ".$dbh->errstr() );

while( 1 )
{
	my $now = time();
	my $res = $sth->execute() or Fatal( "Can't execute: ".$sth->errstr() );
	while( my $monitor = $sth->fetchrow_hashref() )
	{
		if ( $monitor->{Function} ne 'None' )
		{
            my $restart = 0;
			if ( zmMemVerify( $monitor ) && zmMemRead( $monitor, "shared_data:valid" ) )
			{
				# Check we have got an image recently
				my $image_time = zmGetLastWriteTime( $monitor );
				next if ( !defined($image_time) ); # Can't read from shared data
				next if ( !$image_time ); # We can't get the last capture time so can't be sure it's died.

				my $max_image_delay = ($monitor->{MaxFPS}&&($monitor->{MaxFPS}>0)&&($monitor->{MaxFPS}<1))?(3/$monitor->{MaxFPS}):ZM_WATCH_MAX_DELAY;
				my $image_delay = $now-$image_time;
				Debug( "Monitor $monitor->{Id} last captured $image_delay seconds ago, max is $max_image_delay\n" );
				if ( $image_delay > $max_image_delay )
				{
				    Info( "Restarting capture daemon for ".$monitor->{Name}.", time since last capture $image_delay seconds ($now-$image_time)\n" );
                    $restart = 1;
                }
			}
			else
			{
				#Info( "Restarting capture daemon for ".$monitor->{Name}.", shared data not valid\n" );
                #$restart = 1;
			}

            if ( $restart )
            {
                my $command;
                if ( $monitor->{Type} eq 'Local' )
                {
                    $command = ZM_PATH_BIN."/zmdc.pl restart zmc -d $monitor->{Device}";
                }
                else
                {
                    $command = ZM_PATH_BIN."/zmdc.pl restart zmc -m $monitor->{Id}";
                }
                Info( qx( $command ) );
            }
            elsif ( $monitor->{Function} ne 'Monitor' )
            {
                if ( zmMemVerify( $monitor ) && zmMemRead( $monitor, "shared_data:valid" ) )
                {
                    # Check we have got an image recently
                    my $image_time = zmGetLastReadTime( $monitor );
                    next if ( !defined($image_time) ); # Can't read from shared data
                    next if ( !$image_time ); # We can't get the last capture time so can't be sure it's died.

                    my $max_image_delay = ($monitor->{MaxFPS}&&($monitor->{MaxFPS}>0)&&($monitor->{MaxFPS}<1))?(3/$monitor->{MaxFPS}):ZM_WATCH_MAX_DELAY;
                    my $image_delay = $now-$image_time;
                    Debug( "Monitor $monitor->{Id} last analysed $image_delay seconds ago, max is $max_image_delay\n" );
                    if ( $image_delay > $max_image_delay )
                    {
                        Info( "Restarting analysis daemon for ".$monitor->{Name}.", time since last analysis $image_delay seconds ($now-$image_time)\n" );
                        my $command = ZM_PATH_BIN."/zmdc.pl restart zma -m ".$monitor->{Id};
                        Info( qx( $command ) );
                    }
                }
            }
        }
	}
	sleep( ZM_WATCH_CHECK_INTERVAL );
}
Info( "Watchdog exiting\n" );
exit();
