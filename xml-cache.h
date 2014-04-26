/*
 * xml-cache.h
 */

#ifndef __XML_CACHE_H
#define __XML_CACHE_H

#include "xml-base.h"
#include "trackinfo.h"

#include <unistd.h>

#include <string>

#include <iostream>



class cXmlCache: public cXmlBase {
private:
        void rebuild_track(const std::string &path, const char *date,
		const char *time);
protected:
        virtual void copy_to_objects(void);
public:
        cXmlCache(void);

        void ReadDir(int level, std::string path);
        void rebuild(void);
        void add_track(const cTrackInfo &trackinfo, bool save = true);
};

#endif /* __XML_CACHE_H */
