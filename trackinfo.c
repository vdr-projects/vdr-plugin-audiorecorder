/*
 * trackinfo.c
 */

#include "trackinfo.h"
#include "audiorecorder.h"

#include <vdr/tools.h>

#include <iostream>


using namespace std;

/* --- cTrackInfo ----------------------------------------------- */


cTrackInfo::cTrackInfo(void)
{
        track = 0;
        year = 0;
}


void cTrackInfo::clear(void)
{
        recpath.erase();
        path.erase();
	artist.erase();
	title.erase();
	album.erase();
        track = 0;
        year = 0;
	genre.erase();
	comment.erase();
	event.erase();
        recdate.erase();
        rectime.erase();
        codec = 0;
        recdir.erase();
}


void cTrackInfo::set_recpath(const string &_recpath)
{
        if (_recpath.empty() || ! recpath.empty())
                return;

        recpath = _recpath;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- recpath set to: " << recpath << " ---" << endl;
}


void cTrackInfo::set_path(int _file_pattern, int _upper)
{
        if (artist.empty() || title.empty())
                return;

        stringstream tmp;

        patch_chars(artist);
        patch_chars(title);

        if (_upper) {
            string_toupper(artist);
            string_toupper(title);
            string_toupper(album);
            string_toupper(channel);
            string_toupper(event);
        }

        switch(_file_pattern) {
            case 0 :                    // Artist/Title
                tmp << artist << "/" << title << "." << audio_codecs_translated[codec];
                break;
            case 1 :                    // Artist - Title
                tmp << artist << "-" << title << "." << audio_codecs_translated[codec];
                break;
            case 2 :                    // Station/Artist/Title
                tmp << channel << "/" << artist << "/" << title << "." << audio_codecs_translated[codec];
                break;
            case 3 :                    // Station - Artist - Title
                tmp << channel << "-" << artist << "-" << title << "." << audio_codecs_translated[codec];
                break;
            case 4 :                    // Station/Artist - Title
                tmp << channel << "/" << artist << "-" << title << "." << audio_codecs_translated[codec];
                break;
            case 5 :                    // External
                tmp << path_external();
                break;
        }

        path = tmp.str();
        partial_path = tmp.str();

        recdir.insert(0, cPluginAudiorecorder::get_recdir());

        if (!path.empty())
                path.insert(0, cPluginAudiorecorder::get_recdir());

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- path set to: " << path << " ---" << endl;
}

void cTrackInfo::set_path(const string &_path)
{
        if (_path.empty()) {
                path = "";
                partial_path = "";
        } else {
                path = _path;
                partial_path = path;
                recdir = cPluginAudiorecorder::get_recdir();
                partial_path.erase(0, recdir.length());
        }

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- path set to: " << path << " ---" << endl;
}


void cTrackInfo::set_date(const char *_date)
{
        if (! _date || ! recdate.empty())
                return;

	recdate = _date;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- date set to: " << recdate << " ---" << endl;
}


void cTrackInfo::set_time(const char *_time)
{
        if (! _time || ! rectime.empty())
                return;

	rectime = _time;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- time set to: " << rectime << " ---" << endl;
}

void cTrackInfo::set_artist(const char *_artist)
{
        if (! _artist || ! artist.empty())
                return;

        artist = _artist;
        // string_toupper(artist);

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- artist set to: " << artist << " ---" << endl;
}


void cTrackInfo::set_title(const char *_title)
{
        if (! _title || ! title.empty())
                return;

        title = _title;
        // string_toupper(title);

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- title set to: " << title << " ---" << endl;
}


void cTrackInfo::set_album(const char *_album)
{
        if (! _album || ! album.empty())
                return;

        album = _album;
        // string_toupper(album);

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- album set to: " << album << " ---" << endl;
}


void cTrackInfo::set_track(int _track)
{
        if (_track == 0 || track != 0)
                return;

        track = _track;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- track set to: " << track << " ---" << endl;
}


void cTrackInfo::set_year(int _year)
{
        if (_year == 0 || year != 0)
                return;

        year = _year;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- year set to: " << year << " ---" << endl;
}


void cTrackInfo::set_genre(const char *_genre)
{
        if (! _genre || ! genre.empty())
                return;

        genre = _genre;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- genre set to: " << genre << " ---" << endl;
}


void cTrackInfo::set_comment(const char *_comment)
{
        if (! _comment || ! comment.empty())
                return;

        comment = _comment;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- comment set to: " << comment << " ---" << endl;
}


void cTrackInfo::set_channel(const string &_channel)
{
        if (_channel.empty() || ! channel.empty())
                return;

        channel = _channel;
        // string_toupper(channel);

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- channel set to: " << channel << " ---" << endl;
}


void cTrackInfo::set_event(const string &_event)
{
        if (_event.empty() || ! event.empty())
                return;

        event = _event;
        // string_toupper(event);

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- event set to: " << event << " ---" << endl;
}


/*
void cTrackInfo::set_codec(const char *_codec)
{
        if (! _codec || ! codec.empty())
                return;

        codec = _codec;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- codec set to: " << codec << " ---" << endl;
}
*/

void cTrackInfo::set_codec(int _codec)
{
        if (_codec < 0 || _codec >= NUM_CODECS)
                return;

        codec = _codec;

        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "--- codec set to: " << codec << " ---" << endl;
}


void cTrackInfo::set_bit_rates(int _mpa_frame_bit_rate, int _bit_rate)
{
        mpa_frame_bit_rate = _mpa_frame_bit_rate;
        bit_rate = _bit_rate;
}

void cTrackInfo::string_toupper(string &str)
{
        for (string::iterator i = str.begin(); i != str.end(); ++i)
                *i = toupper((unsigned char)*i);
}

void cTrackInfo::patch_chars(string &_string)
{
        for (string::iterator i = _string.begin(); i != _string.end(); ++i) {
                if ((char)*i == '*' || (char)*i == '?')
                        *i = '_';
                else if ((char)*i == '/')
                        *i = ',';
        }
}

std::string cTrackInfo::path_external(void)
{

        char *cmdbuf = NULL;
        char *result = NULL;

        dsyslog("[audiorecorder]: LT 1 (%s, %s())",
                __FILE__, __func__);

        asprintf(&cmdbuf, "\"%s\" \"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%d\"",
                cPluginAudiorecorder::get_pscript().c_str(),
                recpath.c_str(),
                mpa_frame_bit_rate,
                artist.c_str(),
                title.c_str(),
                channel.c_str(),
                event.c_str(),
                audio_codecs_translated[codec],
                bit_rate
        );

        dsyslog("[audiorecorder]: LT 2 (%s, %s())",
                __FILE__, __func__);

        dsyslog("[audiorecorder]: executing command '%s' (%s, %s())",
                cmdbuf,
                __FILE__, __func__);

        cReadLine pipe;

        dsyslog("[audiorecorder]: LT 3 (%s, %s())",
                __FILE__, __func__);

        FILE *p = popen(cmdbuf, "r");
        dsyslog("[audiorecorder]: LT 4 (%s, %s())",
                __FILE__, __func__);

        if (p != (FILE *)NULL) {
                result = pipe.Read(p);
                dsyslog("[audiorecorder]: received '%s' (%s, %s())",
                        result,
                        __FILE__, __func__);
                pclose(p);
        } else {
                dsyslog("[audiorecorder]: ERROR: can't open pipe for command '%s' (%s, %s())",
                        cmdbuf,
                        __FILE__, __func__);
        }
        free(cmdbuf);

        dsyslog("[audiorecorder]: LT 5 (%s, %s())",
                __FILE__, __func__);
        return (result == NULL) ? "" : result;
}
