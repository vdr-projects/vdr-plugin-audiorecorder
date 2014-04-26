/*
 * column.h
 */

#ifndef __COLUMN_H
#define __COLUMN_H

#include "browse-item.h"

#include <string>


enum eColumn {
	colEnd,
	colArtist,
	colTitle,
        colArtistTitle,
	colAlbum,
        colTrack,
        colYear,
	colGenre,
	colChannel,
	colEvent,
	colDate,
	colTime
};


class cColumn {
private:
	eColumn type;
	int width;
	bool join;
        bool cut;
	std::string filter, last_entry;
        cBrowseItem *item;
public:
	cColumn(void);
        
        void set(eColumn _type, int _width = 0, bool _join = false,
                bool _cut = false);
        
	void set_filter(std::string &_filter) { filter = _filter; }
        void set_last_entry(std::string &_last_entry)
                { last_entry = _last_entry; }
        void del_last_entry(void) { last_entry.erase(); }
        void set_main_item(cBrowseItem *_item) { item = _item; }
        
	eColumn get_type(void) const { return type; }
	int get_width(void) const { return width; }
        bool is_joined(void) const { return join; }
        bool get_cut(void) const { return cut; }
	const std::string &get_filter(void) const { return filter; }
        const std::string &get_last_entry(void) const { return last_entry; }
        cBrowseItem *get_main_item(void) const { return item; }
};

#endif /* __COLUMN_H */	
