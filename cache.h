/*
 * cache.h
 */

#ifndef __CACHE_H
#define __CACHE_H

#include "xml-cache.h"
#include "trackinfo.h"
#include "column.h"

#include <vdr/thread.h>

#include <list>
#include <string>

#define NOTHING 0
#define LOAD 1
#define REBUILD 2


class cCache : public cThread {
private:
        bool active;
        int what_to_do;

        std::list<cTrackInfo> tracklist;
	std::list<cTrackInfo>::iterator track;

        cXmlCache xmlcache;

	static const cColumn *sort_order;

        static bool sort_tracklist(const cTrackInfo & lhs,
                const cTrackInfo &rhs);
protected:
        virtual void Action(void);
        virtual void Activate(bool on);
public:
	cCache(void);
        ~cCache();
        void load(void) { what_to_do ^= LOAD; Activate(true); }
        void rebuild(void) { what_to_do ^= REBUILD; Activate(true); }
        bool is_rebuilding(void) { return active; }
        void sort(const cColumn *columns);

        void add_track(const cTrackInfo &trackinfo, bool add_xmlcache = true);
        int get_num_cached(void);

	const cTrackInfo *get_next_track(bool reset = false);
};

extern cCache Cache;

#endif /* __CACHE_H */
