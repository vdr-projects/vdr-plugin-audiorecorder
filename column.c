/*
 * column.c
 */

#include "column.h"


/* --- cColumn -------------------------------------------------------------- */

cColumn::cColumn(void)
{
        set(colEnd);
}


void cColumn::set(eColumn _type, int _width, bool _join, bool _cut)
{
        type = _type;
	width = _width;
        if (width < 0)
                width = 0;
        
        join = _join;
        cut = _cut;
        
        filter.erase();
        last_entry.erase();
        item = NULL;
}
