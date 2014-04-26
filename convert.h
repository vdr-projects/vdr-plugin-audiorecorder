/*
 * convert.h
 */

#ifndef __CONVERT_H
#define __CONVERT_H

#include "postdata.h"
#include "mpa-frame.h"
#include "a-tools.h"

extern "C" {
#include <avcodec.h>
}


class cConvert {
private:
        AVCodec *decoder_codec, *encoder_codec;
        AVCodecContext *decoder_ctx, *encoder_ctx;
        int decoder_open, encoder_open;
        
        abuffer decoder_buf, encoder_buf, mpa_frame_buf;
        
        void init_decoder(void);
        void decode_mpa_frame(mpeg_audio_frame *mpa_frame);
        void init_encoder(const char *codec, int bit_rate, int sample_rate,
                int channels);
public:
        cConvert(const cPostData &postdata);
        ~cConvert();
        
        abuffer *reencode_mpa_frame(mpeg_audio_frame *mpa_frame,
                float volume = 1);
};

#endif /* __CONVERT_H */
