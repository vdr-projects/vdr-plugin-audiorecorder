/*
 * audiorecorder.h
 */

#ifndef __AUDIORECORDER_H
#define __AUDIORECORDER_H

#include "dispatcher.h"
#include "postproc.h"

#include <vdr/plugin.h>

#include <sstream>
#include <string>

class cPluginAudiorecorder : public cPlugin {
private:
	cDispatcher *dispatcher;
        cPostproc *postproc;

        static const char *DESCRIPTION;
        static const char *VERSION;

        static std::string recdir;
        static int debug;

        static std::string cfg;
        static std::string pscript;

        std::stringstream main_menu;

        void probe_audio_codecs(void);
public:
        cPluginAudiorecorder(void);
        virtual ~cPluginAudiorecorder();
        virtual const char *Version(void) { return VERSION; }
        virtual const char *Description(void) { return DESCRIPTION; }
        virtual const char *CommandLineHelp(void);
        virtual bool ProcessArgs(int argc, char *argv[]);
        virtual bool Initialize(void);
        virtual bool Start(void);
        virtual void Stop(void);
        virtual void Housekeeping(void);
        virtual const char *MainMenuEntry(void);
        virtual cOsdObject *MainMenuAction(void);
	virtual cString Active(void);
        virtual cMenuSetupPage *SetupMenu(void);
        virtual bool SetupParse(const char *name, const char *value);
        virtual bool Service(const char *id, void *data = NULL);
        virtual const char **SVDRPHelpPages(void);
        virtual cString SVDRPCommand(const char *command, const char *option,
                int &reply_code);

        static const char *get_version(void) { return VERSION; }
        static std::string get_recdir(void) { return recdir; }
        static int get_dbg_level(void) { return debug; }
        static std::string get_cfg(void) { return cfg; }
        static std::string get_pscript(void) { return pscript; }
};

#endif /* __AUDIORECORDER_H */
