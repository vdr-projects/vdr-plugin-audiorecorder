/*
 * browse.h
 */

#ifndef __BROWSE_H
#define __BROWSE_H

#include "column.h"
#include "trackinfo.h"
#include "browse-item.h"

#include <vdr/osdbase.h>
#include <vdr/plugin.h>

#include <string>


class cBrowse : public cOsdMenu {
private:
        const char *help_yellow;
	std::string help_green;
	int view, num_columns, width;
	bool expand;
	cColumn *columns;
        cPlugin *player;
        
        void set_view(int _view, bool init = true);
        void set_help_keys(void);
        void set_status(void);
        void set_title(void);
        
	void process_submenu_states(eOSState &state);        
        
	cBrowseItem *get_actual_item(void);        
        void insert_items(void);
        void delete_items(void);
        
        void set_filter(const cBrowseItem *item);
        
	bool filter_track(const cTrackInfo *track);
        
	std::string get_category(const cBrowseItem *item);
        std::string get_trackname(const cTrackInfo *track);
	std::string get_value_of_column(const cTrackInfo *track, int c);
        
	void play_all_files(const cBrowseItem *node);
        void play_file(const cTrackInfo *track, bool set_status = true);
        
        void cut_string(std::string &cut, int length);
public:
	cBrowse(void);
	~cBrowse();
        
        virtual eOSState ProcessKey(eKeys key);
};


class cBrowseAction : public cOsdMenu {
public:
        cBrowseAction(bool node, const std::string &text);
};


class cBrowseInfo : public cOsdMenu {
private:
	cOsdItem *add_item(const char *type, const std::string &text);
public:
        cBrowseInfo(const cTrackInfo *track);
};

#endif /* __BROWSE__H */
