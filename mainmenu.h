/*
 * mainmenu.h
 */

#ifndef __MAINMENU_H
#define __MAINMENU_H

#include "dispatcher.h"
#include "cache.h"

#include <vdr/osdbase.h>
#include <vdr/plugin.h>

#include <string>


class cMainmenu : public cOsdMenu {
private:
        bool active;
        cDispatcher *dispatcher;
public:
	cMainmenu(cDispatcher *_dispatcher);
        virtual eOSState ProcessKey(eKeys key);
};


#endif /* __MAINMENU_H */
