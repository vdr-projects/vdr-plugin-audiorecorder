/*
 * cache.c
 */

#include "cache.h"
#include "audiorecorder.h"

#include <vdr/skins.h>
#include <vdr/tools.h>


#define CACHEFILE "cache.xml"

using namespace std;

cCache Cache;

/* --- cCache --------------------------------------------------------------- */

const cColumn *cCache::sort_order = NULL;

cCache::cCache()
:cThread()
{
        what_to_do = 0;
        active = false;
}


cCache::~cCache()
{
        Activate(false);
}


void cCache::Activate(bool on)
{
        if (on) {
                if (! active) {
                        active = true;
                        Start();
                }
        }
        else if (active) {
                active = false;
                Cancel(0);
        }
}


void cCache::Action(void)
{
        if (what_to_do & REBUILD) {
            dsyslog("[audiorecorder]: rebuilding cache thread started (%s, %s())",
                __FILE__, __func__);

            Lock();
            tracklist.clear();
            xmlcache.rebuild();
            Unlock();

            Skins.QueueMessage(mtInfo, tr("Audiorecorder: rebuilding cache finished"));
            dsyslog("[audiorecorder]: rebuilding cache thread finished (%s, %s())",
                    __FILE__, __func__);

            what_to_do ^= REBUILD;

            active = false;
        } else if (what_to_do & LOAD) {
            string path = cPluginAudiorecorder::get_recdir();
            path.append(CACHEFILE);

            Lock();
            xmlcache.load(path);
            Unlock();
            Skins.QueueMessage(mtInfo, tr("Audiorecorder: loading of cache finished"));
            dsyslog("[audiorecorder]: loading of cache finished (%s, %s())",
                    __FILE__, __func__);

            what_to_do ^= LOAD;
            active = false;
        }
}

/*
void cCache::load()
{
        string path = cPluginAudiorecorder::get_recdir();
        path.append(CACHEFILE);

        Lock();
        xmlcache.load(path);
        Unlock();
}
*/

void cCache::sort(const cColumn *columns)
{
	if (! columns)
		return;

	sort_order = columns;

        Lock();
	tracklist.sort(cCache::sort_tracklist);
        Unlock();
}


void cCache::add_track(const cTrackInfo &trackinfo, bool add_xmlcache)
{
        if (trackinfo.get_artist().empty() || trackinfo.get_title().empty())
                return;

        Lock();
        tracklist.push_back(trackinfo);

        if (add_xmlcache)
                xmlcache.add_track(trackinfo);
        Unlock();
}


int cCache::get_num_cached(void)
{
        int num;
        Lock();
        num = tracklist.size();
        Unlock();

        return num;
}


const cTrackInfo *cCache::get_next_track(bool reset)
{
	if (reset)
		track = tracklist.begin();

        if (track == tracklist.end())
		return NULL;

        cTrackInfo *trackinfo = &(*track);
	++track;

        return trackinfo;
}


bool cCache::sort_tracklist(const cTrackInfo &lhs, const cTrackInfo &rhs)
{
	for (int c = 0; sort_order[c].get_type() != colEnd; ++c) {
		switch(sort_order[c].get_type()) {
		case colArtist:
			if (lhs.get_artist() > rhs.get_artist())
				return false;
			if (lhs.get_artist() < rhs.get_artist())
				return true;
			break;
		case colTitle:
			if (lhs.get_title() > rhs.get_title())
				return false;
			if (lhs.get_title() < rhs.get_title())
				return true;
			break;
		case colArtistTitle:
			if (lhs.get_artist() > rhs.get_artist())
				return false;
			if (lhs.get_artist() < rhs.get_artist())
				return true;
			if (lhs.get_title() > rhs.get_title())
				return false;
			if (lhs.get_title() < rhs.get_title())
				return true;
			break;
		case colAlbum:
			if (lhs.get_album() > rhs.get_album())
				return false;
			if (lhs.get_album() < rhs.get_album())
				return true;
			break;
		case colTrack:
			if (lhs.get_track() > rhs.get_track())
				return false;
			if (lhs.get_track() < rhs.get_track())
				return true;
			break;
		case colYear:
			if (lhs.get_year() > rhs.get_year())
				return false;
			if (lhs.get_year() < rhs.get_year())
				return true;
			break;
		case colGenre:
			if (lhs.get_genre() > rhs.get_genre())
				return false;
			if (lhs.get_genre() < rhs.get_genre())
				return true;
			break;
		case colChannel:
			if (lhs.get_channel() > rhs.get_channel())
				return false;
			if (lhs.get_channel() < rhs.get_channel())
				return true;
			break;
		case colEvent:
			if (lhs.get_event() > rhs.get_event())
				return false;
			if (lhs.get_event() < rhs.get_event())
				return true;
			break;
		case colDate:
			/* up-to-date date's first */
			if (lhs.get_date() > rhs.get_date())
				return true;
			if (lhs.get_date() < rhs.get_date())
				return false;
			break;
		case colTime:
			/* up-to-date time's first */
			if (lhs.get_time() > rhs.get_time())
				return true;
			if (lhs.get_time() < rhs.get_time())
				return false;
			break;
		default:
			break;
		}
	}

	return false;
}
