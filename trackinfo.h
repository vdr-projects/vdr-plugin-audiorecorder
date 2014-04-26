/*
 * trackinfo.h
 */

#ifndef __TRACKINFO_H
#define __TRACKINFO_H

#include <string>
#include "setup.h"


class cTrackInfo {
private:
        std::string recpath, path, artist, title, album, genre, comment,
                channel, event, recdate, rectime, recdir, partial_path;

        int track, year;
        int mpa_frame_bit_rate, bit_rate;
        int codec;

        void string_toupper(std::string &str);
        void patch_chars(std::string &str);
        std::string path_external(void);

public:
        cTrackInfo(void);

        void clear(void);

        void set_recpath(const std::string &_recpath);
        void set_path(int _file_pattern, int _upper);
        void set_path(const std::string &_path);
        void set_date(const char *_date);
        void set_time(const char *_time);
        void set_artist(const char *_artist);
        void set_title(const char *_title);
        void set_album(const char *_album);
        void set_track(int _track);
        void set_year(int _year);
        void set_genre(const char *_genre);
        void set_comment(const char *_comment);
        void set_channel(const std::string &_channel);
        void set_event(const std::string &_event);
        void set_codec(int _codec);
        void set_bit_rates(int _mpa_frame_bit_rate, int _bit_rate);

        std::string get_recpath(void) const { return recpath; }
        std::string get_path(void) const { return path; }
        std::string get_partial_path(void) const { return partial_path ; }
        std::string get_date(void) const { return recdate; }
        std::string get_time(void) const { return rectime; }
        std::string get_artist(void) const { return artist; }
        std::string get_title(void) const { return title; }
        std::string get_album(void) const { return album; }
        int get_track(void) const { return track; }
        int get_year(void) const { return year; }
        std::string get_genre(void) const { return genre; }
        std::string get_comment(void) const { return comment; }
        std::string get_channel(void) const { return channel; }
        std::string get_event(void) const { return event; }

        int get_codec(void) const { return codec; }

/*      std::string get_codec(void) const { return codec; }
        std::string get_codec_translated(void) const { return codec_translated; }
*/
        std::string get_recdir(void) const { return recdir; }
};
#endif /* __TRACKINFO_H */
