/*
 * browse.c
 */

#include "browse.h"
#include "cache.h"
#include "external-player.h"
#include "audiorecorder.h"
#include "setup.h"

#include <vdr/skins.h>
#include <vdr/interface.h>
#include <vdr/tools.h>

#include <sstream>


#define MAXCOLS 5
#define TAB_LEN 3
#define SERVICE "MP3-Play-v1"

using namespace std;

/* --- cBrowse -------------------------------------------------------------- */

cBrowse::cBrowse(void)
:cOsdMenu("")
{
	num_columns = MAXCOLS;
	columns = NULL;
	columns = new cColumn[num_columns + 1];

	player = cPluginManager::CallFirstService(SERVICE);

        string skin = Skins.Current()->Name();
        width = DisplayMenu()->EditableWidth();

        if (skin != "curses")
                width /= 12;

	dsyslog("[audiorecorder]: skin %s (width %d) detected (%s, %s())",
                skin.c_str(), width, __FILE__, __func__);

        set_view(SetupValues.default_view);
}


cBrowse::~cBrowse()
{
	delete[] columns;
}


eOSState cBrowse::ProcessKey(eKeys key)
{
	eOSState state = cOsdMenu::ProcessKey(key);

        if (HasSubMenu()) {
		process_submenu_states(state);

                if (state == osBack || key == kBlue || key == kOk)
                        CloseSubMenu();

		return osContinue;
        }

        set_status();
        set_help_keys();

	cBrowseItem *item = get_actual_item();

	switch (key) {
        case kRed:
		if (! item)
                        break;

                if (item->is_node())
                        AddSubMenu(new cBrowseAction(true,
                                        get_category(item)));
                else
                        AddSubMenu(new cBrowseAction(false,
                                        get_trackname(item->get_track())));

                break;
	case kGreen:
		if (help_green.empty())
			break;

		expand ? expand = false : expand = true;
		set_view(view, false);

		break;
        case kYellow:
                set_view(++view);

                break;
	case kBlue:
                if (item && ! item->is_node())
                        AddSubMenu(new cBrowseInfo(item->get_track()));

                break;
	case kOk:
		if (! item)
			break;

                if (! item->is_node())
                        play_file(item->get_track());
                else if (item->is_closed()) {
                        item->toggle_node();
                        insert_items();
                }
		else {
                        item->toggle_node();
                        delete_items();
                }
		break;
	default:
		break;
	}

	return state;
}


void cBrowse::set_view(int _view, bool init)
{
        view = _view;

        if (view == 1) {
                /* view by artist */
                num_columns = 2;

                columns[0].set(colArtist, TAB_LEN);
                columns[1].set(colTitle, width - TAB_LEN);
                columns[2].set(colEnd);

		if (init)
			expand = false;

                help_yellow = views[2];
        }
        else if (view == 2) {
                /* view by channel */
                num_columns = 3;

                columns[0].set(colChannel, TAB_LEN);
                columns[1].set(colEvent, TAB_LEN);
                columns[2].set(colArtistTitle, width - 2 * TAB_LEN);
                columns[3].set(colEnd);

		if (init)
			expand = false;

                help_yellow = views[3];
        }
        else if (view == 3) {
                /* view by date */
                num_columns = 2;

                columns[0].set(colDate, TAB_LEN);
                columns[1].set(colTime, 6, true);
                columns[2].set(colChannel, 11, true, true);
                columns[3].set(colArtistTitle, width - TAB_LEN - 17);
                columns[4].set(colEnd);

		if (init)
			expand = true;

                help_yellow = views[0];
        }
        else {
                /* all */
                view = 0;
                num_columns = 1;

                columns[0].set(colArtistTitle, width);
                columns[1].set(colEnd);

		if (init)
			expand = true;

                help_yellow = views[1];
        }

	if (init) {
		SetCols(columns[0].get_width(), columns[1].get_width(),
			columns[2].get_width(), columns[3].get_width(),
			columns[4].get_width());

        	Cache.sort(columns);
	}

        Clear();
        insert_items();

        set_status();
	set_help_keys();
        set_title();
}


void cBrowse::set_help_keys(void)
{
	/* green key */
	if (view == 0)
		help_green.erase();
	else if (expand)
                help_green = tr("collapse all");
	else
                help_green = tr("expand all");

        cBrowseItem *item = get_actual_item();

        SetHelp(tr("action"), (help_green.empty() ? NULL : help_green.c_str()) ,
		help_yellow, (item && ! item->is_node() ? "info" : NULL));
}


void cBrowse::set_status(void)
{
        /* disable status for the skin sttng: */
        if (strcmp(Skins.Current()->Name(), "sttng") == 0)
                return;

	string status(views[view]);

	cBrowseItem *item = get_actual_item();

	if (item && item->get_main_item()) {
                status.append(" ");
		status.append(get_category(item));
        }

        cut_string(status, width);
	SetStatus(status.c_str());
}


void cBrowse::set_title(void)
{
        stringstream title;
        title << tr("Audiorecorder") << ", " << tr("Browse tracks") << " (" << Cache.get_num_cached()
              << ")";

        SetTitle(title.str().c_str());
}


void cBrowse::process_submenu_states(eOSState &state)
{
	cBrowseItem *item = get_actual_item();

	switch(state) {
	case osUser1:
		play_all_files(item);
		state = osBack;

		break;
	case osUser6:
		if (item)
			play_file(item->get_track());

		state = osBack;

		break;
	default:
		break;
	}
}


cBrowseItem *cBrowse::get_actual_item(void)
{
	if (Current() < 0)
		return NULL;

	return (cBrowseItem *)Get(Current());
}


void cBrowse::insert_items(void)
{
        int pos = Current();

        /* clean up old states */
        for (int c = 0; c < num_columns; ++c) {
                columns[c].set_main_item(NULL);
                columns[c].del_last_entry();
        }

	int column = 0;
	cBrowseItem *item = get_actual_item();
        if (item && item->is_node()) {
		set_filter(item);
                column = item->get_column();
                columns[column].set_main_item(item);
                ++column;
        }

	int depth;
	if (expand)
		depth = num_columns;
	else
		depth = column + 1;

	for (const cTrackInfo *track = Cache.get_next_track(true); track;
             track = Cache.get_next_track()) {

                if (filter_track(track))
                        continue;

                for (int c = column; c < depth; ++c) {
                        eItemType type;

                        string entry = get_value_of_column(track, c);

                        if (c == num_columns - 1)
                                type = itemTrack;
			else if (expand)
				type = itemNodeOpen;
			else
				type = itemNodeClose;

			/* don't show double nodes: */
                        if (type != itemTrack &&
			    entry == columns[c].get_last_entry())
                                continue;

                        columns[c].set_last_entry(entry);
                        for (int i = c + 1; i < num_columns; ++i)
                                columns[i].del_last_entry();

                        if (type == itemNodeOpen)
                                entry.insert(0, "[-] ");
                        else if (type == itemNodeClose)
                                entry.insert(0, "[+] ");

                        string indent(c, '\t');
                        entry.insert(0, indent);

                        cBrowseItem *item, *main = NULL;
                        if (c > 0)
                                main = columns[c - 1].get_main_item();

                        item = new cBrowseItem(main, track, c, type);
                        item->SetText(entry.c_str());
                        columns[c].set_main_item(item);

                        Add(item, true, Get(Current()));
                }
	}

        SetCurrent(Get(pos));
        Display();
}


void cBrowse::delete_items(void)
{
	cBrowseItem *item = get_actual_item();
	if (! item || ! item->is_node())
		return;

        for (int c = 0; c < item->get_items(); ++c)
                Del(Current() + 1);

        item->delete_items(item->get_items());

        Display();
}


void cBrowse::set_filter(const cBrowseItem *item)
{
        for (int c = 0; c < num_columns; ++c) {
                string filter("");
                if (c <= item->get_column())
                        filter = get_value_of_column(item->get_track(), c);

                columns[c].set_filter(filter);
        }
}


bool cBrowse::filter_track(const cTrackInfo *track)
{
        for (int c = 0; c < num_columns; ++c) {

                if (columns[c].get_filter().empty())
                        continue;

                if (columns[c].get_filter() != get_value_of_column(track, c))
                        return true;
        }

        return false;
}


string cBrowse::get_category(const cBrowseItem *item)
{
	string category("");

        if (item) {
                int columns = item->get_column();

                if (item->is_node())
                        ++columns;

                for (int c = 0; c < columns; ++c) {
                        if (c != 0)
                                category.append("/");

                        category.append(get_value_of_column(item->get_track(),
                                        c));
                }
        }

	return category;
}


string cBrowse::get_trackname(const cTrackInfo *track)
{
        string trackstr("");

        if (track) {
                trackstr.append(track->get_artist());
                trackstr.append(" - ");
                trackstr.append(track->get_title());
        }

        return trackstr;
}


string cBrowse::get_value_of_column(const cTrackInfo *track, int c)
{
        string value("");
        bool join = true;

        while (join && columns[c].get_type() != colEnd) {
                stringstream col_tmp;

                switch (columns[c].get_type()) {
                case colArtist:
                        col_tmp << track->get_artist();
                        break;
                case colTitle:
                        col_tmp << track->get_title();
                        break;
                case colArtistTitle:
                        col_tmp << track->get_artist() << " - "
                                << track->get_title();
			break;
                case colAlbum:
                        col_tmp << track->get_album();
                        break;
                case colTrack:
                        col_tmp << track->get_track();
                        break;
                case colYear:
                        col_tmp << track->get_year();
                        break;
                case colGenre:
                        col_tmp << track->get_genre();
                        break;
                case colChannel:
                        col_tmp << track->get_channel();
                        break;
                case colEvent:
                        col_tmp << track->get_event();
                        break;
                case colDate:
                        col_tmp << track->get_date();
                        break;
                case colTime:
                        col_tmp << track->get_time();
                        break;
                default:
                        break;
                }

                if (col_tmp.str().empty())
                        col_tmp << tr("unknown");

                string col(col_tmp.str());

		if (columns[c].get_cut())
                        cut_string(col, columns[c].get_width() - 1);

                value.append(col);

                join = columns[c].is_joined();
                if (join)
                        value.append("\t");

                ++c;
        }

        return value;
}


void cBrowse::play_all_files(const cBrowseItem *node)
{
	if (! player) {
                Skins.Message(mtError, tr("No external player-plugin found"), 2);
		return;
        }

	if (! node || ! node->is_node())
		return;

	set_filter(node);

        string status = tr("Playing all tracks in ");
        status.append(get_category(node));
        cut_string(status, width);
        SetStatus(status.c_str());

	for (const cTrackInfo *track = Cache.get_next_track(true); track;
             track = Cache.get_next_track()) {

                if (! filter_track(track))
                        play_file(track, false);
	}
}


void cBrowse::play_file(const cTrackInfo *track, bool set_status)
{
	if (! player) {
                Skins.Message(mtError, tr("No external player-plugin found"), 2);
		return;
        }

	MP3ServiceData data;
	data.data.filename = track->get_path().c_str();
	data.result = 0;

#ifndef AUDIORECORDER_DEVEL
	player->Service(SERVICE, &data);

	if (data.result == 0) {
		dsyslog("[audiorecorder]: plugin %s could not play the file "
			"%s (%s, %s())", player->Name(),
			track->get_path().c_str(), __FILE__, __func__);
                return;
	}
#endif /* AUDIORECORDER_DEVEL */

        if (set_status) {
                string status = tr("Playing ");
                status.append(get_trackname(track));
                cut_string(status, width);
                SetStatus(status.c_str());
        }
}


void cBrowse::cut_string(string &cut, int length)
{
        if ((int)cut.length() > length) {
                cut.erase(length - 1);
                cut.append("~");
        }
}


/* --- cBrowseAction -------------------------------------------------------- */

cBrowseAction::cBrowseAction(bool node, const string &text)
:cOsdMenu(tr("Audiorecorder, Action"))
{
        string status("");
        if (node) {
                status.append(tr("Category : "));
                status.append(text);
                Add(new cOsdItem(tr("Play all tracks"), osUser1));
                Add(new cOsdItem(tr("More to come ..."), osUser7));
        }
        else {
                status.append(tr("Track: "));
                status.append(text);
                Add(new cOsdItem(tr("Play track"), osUser6));
                Add(new cOsdItem(tr("More to come ..."), osUser7));
        }

        SetStatus(status.c_str());
        SetHelp(NULL, NULL, NULL, tr("back"));
}


/* --- cBrowseInfo ---------------------------------------------------------- */

cBrowseInfo::cBrowseInfo(const cTrackInfo *track)
:cOsdMenu(tr("Audiorecorder, Info"), 11)
{
	string path = track->get_path();
	path.erase(0, cPluginAudiorecorder::get_recdir().length());
        Add(add_item(tr("File"), path));

        Add(add_item(tr("Artist"), track->get_artist()));
        Add(add_item(tr("Title"), track->get_title()));
        Add(add_item(tr("Album"), track->get_album()));
        Add(add_item(tr("Genre"), track->get_genre()));

       	stringstream tmp;
        tmp << track->get_track();
        Add(add_item(tr("Track"), tmp.str()));

        tmp.str("");
        tmp.clear();
        tmp << track->get_year();
        Add(add_item(tr("Year"), tmp.str()));

        Add(add_item(tr("Channel"), track->get_channel()));
        Add(add_item(tr("Event"), track->get_event()));
        Add(add_item(tr("Date"), track->get_date()));
        Add(add_item(tr("Time"), track->get_time()));

        SetHelp(NULL, NULL, NULL, tr("back"));
}


cOsdItem *cBrowseInfo::add_item(const char *type, const string &text)
{
	string txt = type;
	txt.append(":\t");

	if (text.empty() || text == "0")
                txt.append(tr("unknown"));
	else
		txt.append(text);

	return new cOsdItem(txt.c_str(), osUnknown, false);
}
