/*
 * postdata.h
 */

#ifndef __POSTDATA_H
#define __POSTDATA_H

#include "trackinfo.h"

#include <vdr/channels.h>

#include <string>


class cPostData : public cTrackInfo {
private:
        const cChannel *channel;
        int bit_rate;
        int sample_rate;
        int channels;
        int frame_len;
        
        int len_mpa_frame;
        
        int fade_in_mode;
        int frames_fade_in;
        int fade_out_mode;
        int frames_fade_out;
        int file_pattern;
        int upper;
        int copies;
        
        void set_recpath_date_time(void);
        void set_event(void);
        
        void set_fade_values(void);
public:
	cPostData(const cChannel *_channel);
	~cPostData();
        
        void start_track(void);
        void stop_track(int _bit_rate, int _sample_rate, int _channels,
                int _len_mpa_frame);
        
        int get_bit_rate(void) const { return bit_rate; }
        int get_sample_rate(void) const { return sample_rate; }
        int get_channels(void) const { return channels; }
        
        int get_frame_len(void) const { return frame_len; }
        
        int get_len_mpa_frame(void) const { return len_mpa_frame; }
        
        int get_fade_in_mode(void) const { return fade_in_mode; }
        int get_frames_fade_in(void) const { return frames_fade_in; }
        int get_fade_out_mode(void) const { return fade_out_mode; }
        int get_frames_fade_out(void) const { return frames_fade_out; }
        int get_file_pattern(void) const { return file_pattern; }
        int get_upper(void) const { return upper; }
        int get_copies(void) const { return copies; }
};

#endif /* __POSTDATA_H */
