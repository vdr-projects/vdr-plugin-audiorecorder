/*
 * xml-cache.c
 */

#include "xml-cache.h"
#include "cache.h"
#include "audiorecorder.h"

#include <taglib/mpegfile.h>
#include <taglib/tag.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

#include "setup.h"


using namespace std;

/* --- cXmlCache ------------------------------------------------------------ */


cXmlCache::cXmlCache(void)
:cXmlBase("cache") {}


void cXmlCache::ReadDir(int level, std::string path)
{
        DIR *dir;
        struct dirent entry, *result;
        struct stat attribute;
        string fullpath;
        string file;

        dsyslog("[audiorecorder]: level : %d - %s (%s, %s())", level, path.c_str(), __FILE__, __func__);


        dir = opendir(path.c_str());
        if (! dir)
                return;


        for (readdir_r(dir, &entry, &result); result; readdir_r(dir, &entry, &result)) {
                file = result->d_name;

                int len = file.length() - 8;
        if (len < 0)
                        len = 0;

                if (file == "." || file == ".." ||
                    file.substr(len, 8) == ".tmp.mp2")
                        continue;

                fullpath = path;

                string::reverse_iterator i = fullpath.rbegin();

                if (*i != '/') {
                        fullpath.append("/");
                }
                fullpath.append(file);

                stat(fullpath.c_str(), &attribute);

                if (! attribute.st_mode & S_IFREG)
                        continue;

                if ( attribute.st_mode & S_IFDIR) {
                    ReadDir(level + 1, fullpath);
                } else {
                    for (int i1 = 0; i1 < NUM_CODECS; i1++) {
                        std::string tmp1;
                        tmp1 = ".";
                        tmp1.append(audio_codecs[i1]);
                        if (file.substr(file.length() - tmp1.length(), tmp1.length()) == tmp1) {
                            char date_str[11], time_str[6];
                            struct tm *tm_now, tm_store;
                            tm_now = localtime_r(&attribute.st_mtime, &tm_store);
                            strftime(date_str, 11, "%Y-%m-%d", tm_now);
                            strftime(time_str, 6, "%H.%M", tm_now);
                            rebuild_track(fullpath, date_str, time_str);
                            break;
                        }
                    }
                }
        }
        closedir(dir);
}

void cXmlCache::rebuild(void)
{
        clear();

        ReadDir(0, cPluginAudiorecorder::get_recdir().c_str());

        get_document()->InsertEndChild(*get_root());

        get_document()->SaveFile();

        set_root();
}


void cXmlCache::add_track(const cTrackInfo &trackinfo, bool save)
{
        if (! get_root() || trackinfo.get_artist().empty() ||
            trackinfo.get_title().empty())
                return;

        TiXmlElement track("track");
        track.SetAttribute("path", trackinfo.get_partial_path());
        track.SetAttribute("date", trackinfo.get_date());
        track.SetAttribute("time", trackinfo.get_time());

        add_subelement(track, "artist", trackinfo.get_artist());
        add_subelement(track, "title", trackinfo.get_title());
        add_subelement(track, "album", trackinfo.get_album());

        stringstream tmp;
        if (trackinfo.get_track() != 0)
                tmp << trackinfo.get_track();

        add_subelement(track, "tracknr", tmp.str());

        tmp.str("");
        tmp.clear();
        if (trackinfo.get_year() != 0)
                tmp << trackinfo.get_year();

        add_subelement(track, "year", tmp.str());
        add_subelement(track, "genre", trackinfo.get_genre());
        add_subelement(track, "channel", trackinfo.get_channel());
        add_subelement(track, "event", trackinfo.get_event());

        get_root()->InsertEndChild(track);

        if (save) {
                get_document()->SaveFile();
        }
}


void cXmlCache::copy_to_objects(void)
{
        TiXmlElement *xml_track = get_root()->FirstChildElement("track");

        while (xml_track) {
                cTrackInfo trackinfo;

                string path = xml_track->Attribute("path");

                if (path.empty()) {
                        /* remove deleted files from the xml-cache */
                        TiXmlElement *tmp = xml_track;
                        xml_track = xml_track->NextSiblingElement("track");
                        get_root()->RemoveChild(tmp);

                        continue;
                }

                path.insert(0, cPluginAudiorecorder::get_recdir());

                trackinfo.set_path(path);

                if (access(path.c_str(), F_OK) == -1) {
                        dsyslog("[audiorecorder]: copy %s : (%s, %s())", path.c_str(), __FILE__, __func__);
                        /* remove deleted files from the xml-cache */
                        TiXmlElement *tmp = xml_track;
                        xml_track = xml_track->NextSiblingElement("track");
                        get_root()->RemoveChild(tmp);

                        continue;
                }

                if (xml_track->Attribute("date"))
                        trackinfo.set_date(xml_track->Attribute("date"));
                if (xml_track->Attribute("time"))
                        trackinfo.set_time(xml_track->Attribute("time"));

                for (TiXmlElement *element = xml_track->FirstChildElement();
                     element; element = element->NextSiblingElement()) {
                        if (element->FirstChild() == NULL)
                                continue;
                        else if (element->ValueStr() == "artist")
                                trackinfo.set_artist(
                                        element->FirstChild()->Value());
                        else if (element->ValueStr() == "title")
                                trackinfo.set_title(
                                        element->FirstChild()->Value());
                        else if (element->ValueStr() == "album")
                                trackinfo.set_album(
                                        element->FirstChild()->Value());
                        else if (element->ValueStr() == "tracknr")
                                trackinfo.set_track(
                                        atoi(element->FirstChild()->Value()));
                        else if (element->ValueStr() == "year")
                                trackinfo.set_year(
                                        atoi(element->FirstChild()->Value()));
                        else if (element->ValueStr() == "genre")
                                trackinfo.set_genre(
                                        element->FirstChild()->Value());
                        else if (element->ValueStr() == "channel")
                                trackinfo.set_channel(
                                        element->FirstChild()->Value());
                        else if (element->ValueStr() == "event")
                                trackinfo.set_event(
                                        element->FirstChild()->Value());
                }

                if (! trackinfo.get_title().empty() &&
                    ! trackinfo.get_artist().empty())
                        Cache.add_track(trackinfo, false);

                xml_track = xml_track->NextSiblingElement("track");
        }
        get_document()->SaveFile();
}


void cXmlCache::rebuild_track(const string &path, const char *date,
        const char *time)
{
        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "rebuilding track: " << path << endl;

        TagLib::MPEG::File file(path.c_str());
        TagLib::Tag *tag = file.tag();

        if (! tag)
                return;

        cTrackInfo trackinfo;

        trackinfo.set_path(path);
        trackinfo.set_date(date);
        trackinfo.set_time(time);

        if (! tag->artist().isEmpty())
                trackinfo.set_artist(tag->artist().toCString());

        if (! tag->title().isEmpty())
                trackinfo.set_title(tag->title().toCString());

        if (! tag->album().isEmpty())
                trackinfo.set_album(tag->album().toCString());

        if (tag->track() != 0)
                trackinfo.set_track(tag->track());

        if (tag->year() != 0)
                trackinfo.set_year(tag->year());

        if (! tag->genre().isEmpty())
                trackinfo.set_genre(tag->genre().toCString());

        if (! tag->comment().isEmpty()) {
                string com = tag->comment().toCString();
                trackinfo.set_comment(com.c_str());
        string::size_type f1 = com.find("recorded on \"");
        string::size_type f2 = com.find("(vdr-audiorecorder ");
                if (f1 != string::npos && f2 != string::npos) {
                	com.erase(0, 13);
                	string::size_type p1 = com.find_last_of('@');
                	string::size_type p2 = com.find_last_of('"');
                        if (p1 != string::npos) {
                                trackinfo.set_event(com.substr(0, p1));
                                if (p2 != string::npos)
                                        trackinfo.set_channel(com.substr(p1 + 1,
                                                        p2 - p1 - 1));
                        }
        }
        }

        Cache.add_track(trackinfo, false);
        add_track(trackinfo, false);
}
