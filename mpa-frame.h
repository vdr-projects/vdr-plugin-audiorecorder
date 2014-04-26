/*
 * mpa-frame.h
 */

#ifndef __MPA_FRAME__H
#define __MPA_FRAME__H

#include "a-tools.h"


typedef struct mpeg_audio_frame {
        uchar *data;
        int length;
        int bit_rate;
        int sample_rate;
        int channels;
} mpeg_audio_frame;

extern const int bit_rates[];

void get_mpa_frame(abuffer *buf, mpeg_audio_frame *mpa_frame,
        const char *description);

#endif /* __MPA_FRAME_H */
