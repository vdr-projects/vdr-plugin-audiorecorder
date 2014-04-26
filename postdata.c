/*
 * postdata.c
 */

#include "postdata.h"
#include "setup.h"
#include "mpa-frame.h"
#include "audiorecorder.h"

#include <vdr/epg.h>
#include <vdr/plugin.h>

#include <iostream>
#include <sstream>
#include <time.h>


using namespace std;

/* --- cPostData ------------------------------------------------------------ */

cPostData::cPostData(const cChannel *_channel)
:cTrackInfo()
{
        channel = _channel;
        set_channel(channel->Name());
}


cPostData::~cPostData()
{
}


void cPostData::start_track(void)
{
        set_recpath_date_time();
        set_event();

        file_pattern = SetupValues.file_pattern;
        upper = SetupValues.upper;
        copies = SetupValues.copies;
        dsyslog("[audiorecorder]: (file_pattern : %d) (%s, %s())", file_pattern, __FILE__, __func__);
}


void cPostData::stop_track(int _bit_rate, int _sample_rate, int _channels,
        int _len_mpa_frame)
{
        sample_rate = _sample_rate;
        channels = _channels;
        len_mpa_frame = _len_mpa_frame;

        set_codec(SetupValues.audio_codec);

        if (SetupValues.audio_codec == 0) {
                bit_rate = _bit_rate;
        }
        else {
                bit_rate = bit_rates[SetupValues.bit_rate + 1];
        }

        /* TODO: adjust this for other codecs then mp2/mp3 */
        frame_len = 144 * bit_rate / sample_rate;

        set_bit_rates(_bit_rate, bit_rate);
        set_path(file_pattern, upper);
        set_fade_values();
}


void cPostData::set_recpath_date_time(void)
{
        time_t now;
        struct tm *tm_now, tm_store;
        char path_str[20], date_str[11], time_str[6];
        stringstream tmp;

        time(&now);
        tm_now = localtime_r(&now, &tm_store);

        strftime(path_str, 20, "%Y-%m-%d.%H.%M.%S", tm_now);
 	strftime(date_str, 11, "%Y-%m-%d", tm_now);
	strftime(time_str, 6, "%H.%M", tm_now);

/*
        tmp << cPluginAudiorecorder::get_recdir()
            << *channel->GetChannelID().ToString() << "-" << path_str
            << ".tmp.mp2";
*/
        tmp << cPluginAudiorecorder::get_recdir()
            << channel->Name() << "-" << path_str
            << ".tmp.mp2";

        set_recpath(tmp.str());
        set_date(date_str);
        set_time(time_str);
}


void cPostData::set_event(void)
{
	cSchedulesLock sched_lock;
	const cSchedules *schedules = cSchedules::Schedules(sched_lock);
	if (! schedules)
                return;

    	const cSchedule *sched = schedules->GetSchedule(channel);
	if (! sched)
                return;

      	const cEvent *event = sched->GetPresentEvent();
        if (! event)
                return;

	cTrackInfo::set_event(event->Title());
}


void cPostData::set_fade_values()
{
        fade_in_mode = SetupValues.fade_in_mode;
        if (fade_in_mode == 0)
                frames_fade_in = 0;
        else
                frames_fade_in = SetupValues.fade_in * (bit_rate / 8 /
                        frame_len);

        fade_out_mode = SetupValues.fade_out_mode;
        if (fade_out_mode == 0)
                frames_fade_out = 0;
        else
                frames_fade_out = SetupValues.fade_out * (bit_rate / 8 /
                        frame_len);
}
