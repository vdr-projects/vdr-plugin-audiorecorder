/*
 * browse-item.h
 */

#ifndef __BROWSE_ITEM_H
#define __BROWSE_ITEM_H

#include "trackinfo.h"

#include <vdr/osdbase.h>


enum eItemType {
        itemTrack,
        itemNodeOpen,
        itemNodeClose,
};

class cBrowseItem : public cOsdItem {
private:
        const cTrackInfo *track;
        cBrowseItem *main_item;
        int column;
        eItemType type;        
        int items;
public:
        cBrowseItem(cBrowseItem *_main_item, const cTrackInfo *_track,
                int _column, eItemType _type);
        
        void increase_items(void);
        void delete_items(int del_items);
        void toggle_node(void);
        
        const cTrackInfo *get_track(void) const { return track; }
	const cBrowseItem *get_main_item(void) const { return main_item; }
        int get_column(void) const { return column; }
        int get_items(void) const { return items; }
        
        bool is_node(void) const { return (type != itemTrack); }
        bool is_open(void) const { return (type == itemNodeOpen); }
        bool is_closed(void) const { return (type == itemNodeClose); }
};

#endif /* __BROWSE_ITEM__H */
