/*
 * setup.h
 */

#ifndef __SETUP_H
#define __SETUP_H

#include <vdr/menuitems.h>


struct setup_values {
	int start_type;
        int max_receivers;
        int min_free_space;

	int default_view;

        int pause;
        int max_postproc;
        int fade_in_mode;
        int fade_in;
        int fade_out_mode;
        int fade_out;
        int audio_codec;
        int num_audio_codecs;
        int bit_rate;
        int file_pattern;
        int upper;
        int copies;
};

extern struct setup_values SetupValues;

#define NUM_CODECS  3
extern const char *audio_codecs[NUM_CODECS];
extern const char *audio_codecs_translated[NUM_CODECS];

#define NUM_FADE_TYPES 3
extern const char *fade_types[NUM_FADE_TYPES];

#define NUM_VIEWS 4
extern const char *views[NUM_VIEWS];

#define NUM_FILE_PATTERNS 6
extern const char *file_patterns[NUM_FILE_PATTERNS];

class cAudiorecorderSetup : public cMenuSetupPage {
private:
        struct setup_values setupvalues;
protected:
        virtual void Store(void);
public:
        cAudiorecorderSetup(void);
};

#endif /* __SETUP_H */
