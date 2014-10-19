/*
 * browse-item.c
 */

#include "browse-item.h"

#include <string>


using namespace std;

/* --- cBrowseItem ---------------------------------------------------------- */

cBrowseItem::cBrowseItem(cBrowseItem *_main_item, const cTrackInfo *_track,
        int _column, eItemType _type)
:cOsdItem()
{
        main_item = _main_item;
        if (main_item)
                main_item->increase_items();

        track = _track;
        column = _column;
        type = _type;

        items = 0;
}


void cBrowseItem::increase_items(void)
{
        ++items;

        if (main_item)
                main_item->increase_items();
}


void cBrowseItem::delete_items(int del_items)
{
        items -= del_items;

        if (main_item)
                main_item->delete_items(del_items);

        if (items < 0)
                items = 0;
}


void cBrowseItem::toggle_node(void)
{
    string txt = Text();

        if (type == itemNodeOpen) {
                type = itemNodeClose;
                string::size_type f = txt.find_first_of("-");
                if (f != string::npos)
                        txt.replace(f, 1, "+");
    }
        else if (type == itemNodeClose) {
                type = itemNodeOpen;
                string::size_type f = txt.find_first_of("+");
                if (f != string::npos)
                        txt.replace(f, 1, "-");
    }

    SetText(txt.c_str());
}
