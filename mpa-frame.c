/*
 * mpa-frame.c
 */


#include "mpa-frame.h"
#include "audiorecorder.h"

#include <vdr/tools.h>

#include <iostream>


using namespace std;

const int sample_rates[] = { 44100, 48000, 32000, 0 };
const int bit_rates[] = { 0, 32000, 48000, 56000, 64000, 80000, 96000, 112000,
        128000, 160000, 192000, 224000, 256000, 320000, 384000, 0 };

void get_mpa_frame(abuffer *buf, mpeg_audio_frame *mpa_frame,
        const char *description)
{
        int c;
        
        mpa_frame->data = NULL;
        
        for (c = buf->offset; c < buf->length - buf->offset - 2; ++c) {
                if ((buf->data[c] == 0xff) && (buf->data[c + 1] & 0xe0) &&
                    (buf->data[c + 1] & 0x1e) == 0x1c) {
                        /* mpeg v1, layer II header found */
                        mpa_frame->bit_rate = bit_rates[buf->data[c + 2] >> 4];
                        mpa_frame->sample_rate = sample_rates[(buf->data[c + 2]
                                >> 2) & 0x03];
                        if (mpa_frame->sample_rate < 1)
                                continue;
                        mpa_frame->channels = (buf->data[c + 3] >> 6) == 3 ?
                                1 : 2;
                        int p = buf->data[c + 2] & 0x01; /* padding bit */
                        mpa_frame->length = 144 * mpa_frame->bit_rate /
                                mpa_frame->sample_rate + p;
                        
                        if (c + mpa_frame->length <= buf->length)
                                mpa_frame->data = buf->data + c;
                        
                        break;
                }
        }
        
        if (c > buf->offset && cPluginAudiorecorder::get_dbg_level() > 0)
		cout << "skipped " << c - buf->offset << " byte(s) "
			<< description << "> (" << __FILE__ << ", " << __func__
			<< "())" << endl;
        
        buf->offset = c;
}
