/*
 * audiorecorder.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */


#include "audiorecorder.h"
#include "setup.h"
#include "mainmenu.h"
#include "service.h"

#include <vdr/plugin.h>

#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

extern "C" {
        #include <avcodec.h>
}

// extern cChannels Channels;

using namespace std;

/* --- cPluginAudiorecorder ------------------------------------------------- */

const char *cPluginAudiorecorder::DESCRIPTION = tr("floods your disc with music");
const char *cPluginAudiorecorder::VERSION = "2.0.0_pre1";

string cPluginAudiorecorder::recdir;
int cPluginAudiorecorder::debug = 0;
string cPluginAudiorecorder::cfg;
string cPluginAudiorecorder::pscript;

cPluginAudiorecorder::cPluginAudiorecorder(void)
{
        dispatcher = NULL;
        postproc = NULL;
}


cPluginAudiorecorder::~cPluginAudiorecorder()
{
        delete dispatcher;
        delete postproc;
}


const char *cPluginAudiorecorder::CommandLineHelp(void)
{
        return "  -r dir,   --recdir=dir    set recording-directory to 'dir'.\n"
               "  -d level, --debug=level   set debugging level (default: 0),\n"
               "                            all stuff is written to stdout.\n"
               "                            0 = off\n"
               "                            1 = only errors\n"
               "                            2 = errors and further infos\n"
               "  -p script, --postproc=script set script (default: none)\n"
               "                            for postprocessing.\n";

}


bool cPluginAudiorecorder::ProcessArgs(int argc, char *argv[])
{
        int c, option_index = 0;
//        struct stat dir;

        static struct option long_options[] = {
                { "recdir", required_argument, NULL, 'r' },
                { "debug", required_argument, NULL, 'd' },
                { "postproc", required_argument, NULL, 'p' },
                { NULL }
        };

        c = getopt_long(argc, argv, "r:v:d:p:", long_options, &option_index);

        while (c != -1) {
                switch (c) {
                case 'r':
                        recdir = optarg;
                        break;
                case 'd':
                        if (atoi(optarg) > 0)
                                debug = atoi(optarg);
                        break;
                case 'p':
                        pscript = optarg;
                        break;
                default:
                        return false;
                }

                c = getopt_long(argc, argv, "r:v:d:p:", long_options,
                        &option_index);
        }

        if (recdir.empty()) {
                cerr << endl << "audiorecorder: missing parameter --recdir|-r "
                     << endl;
                dsyslog("audiorecorder: missing parameter --recdir|-r");
                return false;
        }

        if (recdir[recdir.length() - 1] != '/')
                recdir.append("/");
//LT
/*
        stat(recdir.c_str(), &dir);

        if (! (dir.st_mode & S_IFDIR)) {
                cerr << endl << "audiorecorder: " << recdir << " (given with "
                        "the parameter --recdir|-r) isn't a directory" << endl;
                dsyslog("[audiorecorder]: %s (given with the parameter "
                        " --recdir|-r) isn't a directory", recdir.c_str());
                return false;
        }

        if (access(recdir.c_str(), R_OK | W_OK) != 0) {
                cerr << endl << "audiorecorder: can't access " << recdir <<
                        " (given with the parameter --recdir|-r)" << endl;
                dsyslog("[audiorecorder]: can't access %s (given with the "
                        "parameter --recdir|-r)", recdir.c_str());
                return false;
        }
*/
//
        dsyslog("[audiorecorder]: external path-script  : %s (%s, %s())", pscript.c_str(), __FILE__, __func__);



        return true;
}


bool cPluginAudiorecorder::Initialize(void)
{
        /* Initialize any background activities the plugin shall perform. */

        cfg.append(strdup(ConfigDirectory(PLUGIN_NAME_I18N)));
        cfg.append("/audiorecorder.conf");

        audio_codecs[0]   = "mp2";
        audio_codecs[1]   = "libmp3lame";
        audio_codecs[2]   = "mp3";

        audio_codecs_translated[0]   = tr("mp2");
        audio_codecs_translated[1]   = tr("mp3");
        audio_codecs_translated[2]   = tr("mp3");

        fade_types[0]     = tr("off");
        fade_types[1]     = tr("linear");
        fade_types[2]     = tr("exponential");

        views[0]          = tr("all");
        views[1]          = tr("by artist");
        views[2]          = tr("by channel");
        views[3]          = tr("by date");

        file_patterns[0]  = tr("Artist/Title");
        file_patterns[1]  = tr("Artist - Title");
        file_patterns[2]  = tr("Channel/Artist/Title");
        file_patterns[3]  = tr("Channel - Artist - Title");

        return true;
}


bool cPluginAudiorecorder::Start(void)
{
        /* initialize libavcodec */
        avcodec_init();
        avcodec_register_all();

        probe_audio_codecs();

        Cache.load();

        dispatcher = new cDispatcher();
        postproc = new cPostproc();

        return true;
}


void cPluginAudiorecorder::Stop(void)
{
        /* Stop any background activities the plugin shall perform. */
}


void cPluginAudiorecorder::Housekeeping(void)
{
        /* Perform any cleanup or other regular tasks. */
}


const char *cPluginAudiorecorder::MainMenuEntry(void)
{
        main_menu.str("");
        main_menu.clear();

        main_menu << tr("Audiorecorder") << " ("
                << (dispatcher->is_active() ? tr("on") : tr("off"))
                << ") " << dispatcher->get_recording_receivers()
                << "/" << dispatcher->get_attached_receivers(-1)
                << "/" << SetupValues.max_receivers
                << ", " << postproc->get_num_queued();

        return main_menu.str().c_str();
}


cOsdObject *cPluginAudiorecorder::MainMenuAction(void)
{
        return new cMainmenu(dispatcher);
}


cString cPluginAudiorecorder::Active(void)
{
        /*
	if (postproc->get_num_queued() != 0)
		return "active postprocessings";
        */
	return NULL;
}


cMenuSetupPage *cPluginAudiorecorder::SetupMenu(void)
{
        return new cAudiorecorderSetup();
}


bool cPluginAudiorecorder::SetupParse(const char *name, const char *value)
{
        if (strcmp(name, "start_type") == 0)
                SetupValues.start_type = atoi(value);
        else if (strcmp(name, "max_receivers") == 0)
                SetupValues.max_receivers = atoi(value);
        else if (strcmp(name, "min_free_space") == 0)
                SetupValues.min_free_space = atoi(value);
        else if (strcmp(name, "pause") == 0)
                SetupValues.pause = atoi(value);
        else if (strcmp(name, "max_postproc") == 0)
                SetupValues.max_postproc = atoi(value);
        else if (strcmp(name, "default_view") == 0)
                SetupValues.default_view = atoi(value);
        else if (strcmp(name, "fade_in") == 0)
                SetupValues.fade_in = atoi(value);
        else if (strcmp(name, "fade_in_mode") == 0)
                SetupValues.fade_in_mode = atoi(value);
        else if (strcmp(name, "fade_out") == 0)
                SetupValues.fade_out = atoi(value);
        else if (strcmp(name, "fade_out_mode") == 0)
                SetupValues.fade_out_mode = atoi(value);
        else if (strcmp(name, "audio_codec") == 0) {
                int c;

                for (c = 0; c < SetupValues.num_audio_codecs; ++c) {
                        if (strcmp(value, audio_codecs_translated[c]) == 0) {
                                SetupValues.audio_codec = c;
                                break;
                        }
                }
        }
        else if (strcmp(name, "bit_rate") == 0)
                SetupValues.bit_rate = atoi(value);
        else if (strcmp(name, "file_pattern") == 0)
                SetupValues.file_pattern = atoi(value);
        else if (strcmp(name, "upper") == 0)
                SetupValues.upper = atoi(value);
        else if (strcmp(name, "copies") == 0)
                SetupValues.copies = atoi(value);
        else
                return false;

        return true;
}


bool cPluginAudiorecorder::Service(const char *id, void *data)
{
        if (! id)
                return false;

        if (strcmp(id, "Audiorecorder-StatusRtpChannel-v1.0") == 0) {
                if (data) {
                        struct Audiorecorder_StatusRtpChannel_v1_0 *d;
                        d = (struct Audiorecorder_StatusRtpChannel_v1_0 *)data;
                        d->status =
                                dispatcher->get_recording_status(d->channel);
                }
                return true;
        }

        return false;
}

const char **cPluginAudiorecorder::SVDRPHelpPages(void)
{
        /* Return help text for SVDRP commands this plugin implements */
        return NULL;
}


cString cPluginAudiorecorder::SVDRPCommand(const char *command,
        const char *option, int &reply_code)
{
        /* Process SVDRP commands this plugin implements */
        return NULL;
}


void cPluginAudiorecorder::probe_audio_codecs() {
        int c;
        const char *tmp;
        AVCodec *codec = NULL;

        for (c = 1; c < SetupValues.num_audio_codecs; ++c) {
                codec = avcodec_find_encoder_by_name(audio_codecs[c]);
                if (codec)
                        continue;

                dsyslog("[audiorecorder]: your version of libavcodec (ffmpeg) "
                        "is not compiled with %s support (%s, %s())", audio_codecs[c], __FILE__, __func__);

                tmp = audio_codecs[c];
                audio_codecs[c] =
                        audio_codecs[SetupValues.num_audio_codecs - 1];
                audio_codecs[SetupValues.num_audio_codecs - 1] = tmp;
                --SetupValues.num_audio_codecs;
        }
}


VDRPLUGINCREATOR(cPluginAudiorecorder); /* Don't touch this! */
