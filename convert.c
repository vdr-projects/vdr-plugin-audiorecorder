/*
 * convert.c
 */

#include "convert.h"

#include <vdr/tools.h>

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
#endif

/* --- cConvert ------------------------------------------------------------- */

cConvert::cConvert(const cPostData &postdata)
{
        encoder_buf.data = NULL;
        encoder_buf.length = 0;
        encoder_open = -1;

        decoder_buf.data = NULL;
        decoder_buf.length = 0;
        decoder_open = -1;

        init_decoder();
        init_encoder(audio_codecs[postdata.get_codec()], postdata.get_bit_rate(),
                postdata.get_sample_rate(), postdata.get_channels());
}


cConvert::~cConvert()
{
        delete[] encoder_buf.data;
        delete[] decoder_buf.data;

        if (decoder_open > -1) {
                avcodec_close(decoder_ctx);
                av_free(decoder_ctx);
        }

        if (encoder_open > -1) {
                avcodec_close(encoder_ctx);
                av_free(encoder_ctx);
        }
}


void  cConvert::init_decoder(void)
{
        decoder_codec = avcodec_find_decoder_by_name("mp2");
        if (! decoder_codec) {
                dsyslog("[audiorecorder]: codec mp2 is not supported (%s, "
                        "%s())", __FILE__,  __func__);
                return;
        }

        decoder_ctx = avcodec_alloc_context3(NULL);
        decoder_open = avcodec_open2(decoder_ctx, decoder_codec, NULL);

        if (decoder_open < 0) {
                dsyslog("[audiorecorder]: could not open codec mp2 (%s, "
                        "%s())", __FILE__, __func__);
                return;
        }

        decoder_buf.data = new uchar[AVCODEC_MAX_AUDIO_FRAME_SIZE];

        dsyslog("[audiorecorder]: decoder initialized (%s, %s())", __FILE__,
                __func__);
}


void cConvert::init_encoder(const char *codec, int bit_rate, int sample_rate,
        int channels)
{
        encoder_codec = avcodec_find_encoder_by_name(codec);
        if (! encoder_codec) {
                dsyslog("[audiorecorder]: codec %s is not supported (%s, "
                        "%s())", codec, __FILE__,  __func__);
                return;
        }

        encoder_ctx = avcodec_alloc_context3(NULL);

        encoder_ctx->bit_rate       = bit_rate;
        encoder_ctx->sample_fmt     = AV_SAMPLE_FMT_S16P;
        encoder_ctx->sample_rate    = sample_rate;
        encoder_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
        encoder_ctx->channels       = channels;

        encoder_open = avcodec_open2(encoder_ctx, encoder_codec, NULL);

        if (encoder_open < 0) {
                dsyslog("[audiorecorder]: could not open codec %s (%s, %s())", codec, __FILE__, __func__);
                return;
        }

        encoder_buf.length = encoder_ctx->frame_size;
        encoder_buf.data = new uchar[encoder_buf.length];

        dsyslog("[audiorecorder]: encoder for %s-codec (br: %d, sr: %d, %d ch) "
                "initialized (%s, %s())", encoder_codec->name,
                encoder_ctx->bit_rate, encoder_ctx->sample_rate,
                encoder_ctx->channels, __FILE__, __func__);
}


void cConvert::decode_mpa_frame(mpeg_audio_frame *mpa_frame)
{
        if (decoder_open < 0) {
                decoder_buf.length = 0;
                return;
        }

        AVPacket avpkt;
        av_init_packet(&avpkt);
        avpkt.data = mpa_frame->data;
        avpkt.size = mpa_frame->length;
        decoder_buf.length = AVCODEC_MAX_AUDIO_FRAME_SIZE;
#ifndef AVCODEC_NEW
        int len = avcodec_decode_audio3(decoder_ctx, (short *)decoder_buf.data,
                &decoder_buf.length, &avpkt);
#else
#warning "you have enabled -DAVCODEC_NEW in Makefile"
#warning "this is still under development,"
#warning "and will compile not working code...(convert.c, decode_mpa_frame)"
// functioniert, aber es wird nur ein mp3 file mit audio level 0 erzeugt :(

        AVCodecContext *decoder_ctx;

        AVFrame *frame = avcodec_alloc_frame(); //av_frame_alloc # libav10
        int got_output;

        int len = avcodec_decode_audio4(decoder_ctx, frame, &got_output, &avpkt);

        avcodec_free_frame(&frame); // av_free_frame(&frame); // libav10
//        avcodec_close(decoder_ctx); // muss codec hier geschlossen werden?
#endif
}


abuffer *cConvert::reencode_mpa_frame(mpeg_audio_frame *mpa_frame,
        float volume)
{
        int c, len;
        short *tmp;

        if (decoder_open < 0 || encoder_open < 0)
                return &encoder_buf;

        if (strcmp(encoder_codec->name, "mp2") == 0 && volume == 1) {
                mpa_frame_buf.data = mpa_frame->data;
                mpa_frame_buf.offset = mpa_frame->length;
                return &mpa_frame_buf;
        }

        decode_mpa_frame(mpa_frame);

        if (volume != 1) {
                /* manipulate the volume */
                len = decoder_buf.length / sizeof(short);
                tmp = (short *)decoder_buf.data;

                for (c = 0; c < len; ++c) {
                        *(tmp) = (short)((float)*(tmp) * volume);
                        ++tmp;
                }
        }

#ifndef AVCODEC_NEW
        encoder_buf.offset = avcodec_encode_audio(encoder_ctx, encoder_buf.data,
                encoder_buf.length, (short *)decoder_buf.data);
        /* encoder_buf.offset is used to save the size of the encoded frame */
#else
#warning "you have enabled -DAVCODEC_NEW in Makefile"
#warning "this is still under development,"
#warning "and will compile not working code, (convert.c, reencode_mpa_frame)"
// https://www.ffmpeg.org/doxygen/1.0/group__lavc__encoding.html#gf12a9da0d33f50ff406e03572fab4763
// https://www.ffmpeg.org/doxygen/1.0/decoding__encoding_8c-source.html#l00102
        AVCodecContext *encoder_ctx;
        AVFrame *frame = avcodec_alloc_frame; // libav10 av_frame_alloc
        AVPacket avpkt;
        av_init_packet(&avpkt);
        avpkt.data = mpa_frame->data;
        avpkt.size = mpa_frame->length;
        int ret, got_output;


        encoder_buf.offset = avcodec_encode_audio2(encoder_ctx, &avpkt, frame, &got_output);
#endif
        return &encoder_buf;
}
