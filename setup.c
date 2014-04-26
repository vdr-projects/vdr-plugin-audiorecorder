/*
 * setup.c
 */

#include "setup.h"
#include "mpa-frame.h"


/* default setup values: */
struct setup_values SetupValues = {
	1,  /* receiving mode on start is 'on' */
        5,  /* max. receivers */
        50, /* min. free space (in mb) */
	3,  /* default view 'by date' */
        1,  /* pause postprocessing if osd is opened */
        8,  /* max. tracks in postprocessing queue */
        1,  /* fade in mode */
        9,  /* fade in seconds */
        2,  /* fade out mode */
        12, /* fade out seconds */
        0,  /* audio codec */
        NUM_CODECS,  /* number of audio codecs */
        9,  /* bitrate */
        0   /* file_pattern */
};


const char *audio_codecs[NUM_CODECS]   = { "mp2", "libmp3lame", "mp3" };

const char *audio_codecs_translated[NUM_CODECS] = { tr("mp2"), tr("mp3"), tr("mp3") };

const char *fade_types[NUM_FADE_TYPES] = { tr("off"), tr("linear"), tr("exponential") };

const char *_bit_rates[]               = { "0", "32000", "48000", "56000", "64000", "80000", "96000", "112000", "128000", "160000", "192000", "224000", "256000", "320000", "384000", "0" };

const char *views[NUM_VIEWS]           = { tr("all"), tr("by artist"), tr("by channel"), tr("by date") };

const char *file_patterns[NUM_FILE_PATTERNS] = { tr("Artist/Title"), tr("Artist - Title"), tr("Channel/Artist/Title"), tr("Channel - Artist - Title"), tr("Channel/Artist - Title"), tr("External") };

/* --- cAudiorecorderSetup -------------------------------------------------- */

cAudiorecorderSetup::cAudiorecorderSetup(void)
:cMenuSetupPage()
{
        setupvalues = SetupValues;

        Add(new cOsdItem(tr("--- Receiver: ---"), osUnknown, false));
        Add(new cMenuEditBoolItem(tr("Receiving mode on start"),
                        &setupvalues.start_type, tr("off"), tr("on")));
        Add(new cMenuEditIntItem(tr("Max. number of receivers"),
                        &setupvalues.max_receivers, 0, 12));
        Add(new cMenuEditIntItem(tr("Min. free disc space (in mb)"),
                        &setupvalues.min_free_space, 50, 999999));

        Add(new cOsdItem(tr("--- Browser: ---"), osUnknown, false));
        Add(new cMenuEditStraItem(tr("Default view"), &setupvalues.default_view,
                        NUM_VIEWS, views));

        Add(new cOsdItem(tr("--- Postprocessing: ---"), osUnknown, false));
        Add(new cMenuEditBoolItem(tr("Pause if osd is opened"),
                        &setupvalues.pause));
        Add(new cMenuEditIntItem(tr("Max. tracks in queue"),
                        &setupvalues.max_postproc, 1, 20));
        Add(new cMenuEditStraItem(tr("Fade in mode"), &setupvalues.fade_in_mode,
                        NUM_FADE_TYPES, fade_types));
        Add(new cMenuEditIntItem(tr("Fade in seconds"), &setupvalues.fade_in,
                        0, 20));
        Add(new cMenuEditStraItem(tr("Fade out mode"), &setupvalues.fade_out_mode,
                        NUM_FADE_TYPES, fade_types));
        Add(new cMenuEditIntItem(tr("Fade out seconds"), &setupvalues.fade_out,
                        0, 20));
        Add(new cMenuEditStraItem(tr("Audio codec"), &setupvalues.audio_codec,
                        setupvalues.num_audio_codecs, audio_codecs_translated));
        Add(new cMenuEditStraItem(tr("Bitrate (if audio codec isn't <original>)"),
                        &setupvalues.bit_rate, 14, &_bit_rates[1]));
        Add(new cMenuEditStraItem(tr("File Pattern"),
                        &setupvalues.file_pattern, NUM_FILE_PATTERNS, file_patterns));
        Add(new cMenuEditBoolItem(tr("Upper Case"),
                        &setupvalues.upper, tr("No"), tr("Yes")));
        Add(new cMenuEditIntItem(tr("Copies"), &setupvalues.copies,
                        0, 20));
}


void cAudiorecorderSetup::Store(void)
{
        SetupValues = setupvalues;

        SetupStore("default_view", SetupValues.default_view);
        SetupStore("start_type", SetupValues.start_type);
        SetupStore("max_receivers", SetupValues.max_receivers);
        SetupStore("min_free_space", SetupValues.min_free_space);
        SetupStore("pause", SetupValues.pause);
        SetupStore("max_postproc", SetupValues.max_postproc);
        SetupStore("fade_in_mode", SetupValues.fade_in_mode);
        SetupStore("fade_in", SetupValues.fade_in);
        SetupStore("fade_out_mode", SetupValues.fade_out_mode);
        SetupStore("fade_out", SetupValues.fade_out);
        SetupStore("audio_codec", audio_codecs_translated[SetupValues.audio_codec]);
        SetupStore("bit_rate", SetupValues.bit_rate);
        SetupStore("file_pattern", SetupValues.file_pattern);
        SetupStore("upper", SetupValues.upper);
        SetupStore("copies", SetupValues.copies);
}
