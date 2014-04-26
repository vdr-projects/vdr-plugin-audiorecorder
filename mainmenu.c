/*
 * mainmenu.c
 */

#include "mainmenu.h"
#include "browse.h"
#include "dispatcher.h"
#include "audiorecorder.h"

#include <sstream>


using namespace std;

/* --- cMainmenu ------------------------------------------------------------ */

cMainmenu::cMainmenu(cDispatcher *_dispatcher)
:cOsdMenu("Audiorecorder, Mainmenu")
{
        dispatcher = _dispatcher;
        active = dispatcher->is_active();

        Clear();

        Add(new cOsdItem(tr("Browse tracks"), osUser1));
        Add(new cOsdItem(active ? tr("Stop receiving") : tr("Start receiving"),
                        osUser2));
        Add(new cOsdItem(tr("Start rebuilding cache"), osUser3));

        Display();
}


eOSState cMainmenu::ProcessKey(eKeys key)
{
        eOSState state = cOsdMenu::ProcessKey(key);

        if (HasSubMenu())
                return osContinue;

        switch (state) {
        case osBack:
                state = osEnd;
                break;
        case osUser1:
                if (Cache.is_rebuilding()) {
                        Skins.Message(mtStatus, tr("Rebuilding cache, try later"));
                        break;
                }

                AddSubMenu(new cBrowse());

                break;
        case osUser2:
                if (active) {
                        Add(new cOsdItem(tr("Start receiving"), osUser2), false,
                                Get(Current()));
                        dispatcher->stop();
                        active = false;
                }
                else {
                       // Message if there are no channels to record
                       if (dispatcher->get_no_of_channels() < 1) {
                               char* tmp = NULL;
                               asprintf(&tmp, tr("no channel in %s"),
                                 cPluginAudiorecorder::get_cfg().c_str());
                               Skins.Message(mtStatus, tmp);
                               free(tmp);
                               return state;
                        }
                        Add(new cOsdItem(tr("Stop receiving"), osUser2), false,
                                Get(Current()));
                        dispatcher->start();
                        active = true;
                }
                Del(Current());
                Display();

                break;
        case osUser3:
                Cache.rebuild();
                Skins.Message(mtStatus, tr("Rebuilding cache"));

                break;
        default:
                break;
        }

        return state;
}
