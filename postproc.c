/*
 * postproc.c
 */


#include "postproc.h"
#include "postdata.h"
#include "cache.h"
#include "setup.h"
#include "audiorecorder.h"

#include <vdr/osd.h>
#include <vdr/tools.h>

#include <taglib/mpegfile.h>
#include <taglib/tag.h>

#include <fstream>
#include <sstream>
#include <cstdio>
/* this include is needed for gcc 2.95: */
#include <vector>


using namespace std;

/* --- cPostproc -------------------------------------------------------------*/

list<cPostData> cPostproc::postlist;
cMutex cPostproc::mutex;

cPostproc::cPostproc()
:cThread()
{
        active = false;

        convert = NULL;

        file_buf.length = KB(10);
        file_buf.data = new uchar[file_buf.length];

        Activate(true);
}


cPostproc::~cPostproc()
{
        Activate(false);

        mutex.Lock();
        while (! postlist.empty()) {
                postdata = postlist.begin();
                remove(postdata->get_recpath().c_str());

                postlist.pop_front();
        }
        mutex.Unlock();

        DELETE(convert);
        delete[] file_buf.data;
}


void cPostproc::Activate(bool on)
{
        if (on) {
                if (! active) {
                        active = true;
                        Start();
                }
        }
        else if (active) {
                active = false;
                Cancel(3);
        }
}


void cPostproc::Action(void)
{
        dsyslog("[audiorecorder]: postprocessing thread started (%s, %s())",
                __FILE__, __func__);

        while (active) {
                if (postlist.empty()) {
                        usleep(10000);
                        continue;
                }

                bool tag = true;

                postdata = postlist.begin();

                if (postdata->get_frames_fade_in() > 0 ||
                    postdata->get_frames_fade_out() > 0 ||
                    strcmp(audio_codecs[postdata->get_codec()], "mp2") != 0) {
                        convert = new cConvert(*postdata);
                        tag = reencode();
                }
                else
                        rename_file();

                if (tag) {
                        set_tag();
                        Cache.add_track(*postdata);
                }

                mutex.Lock();
                postlist.pop_front();
                mutex.Unlock();

                DELETE(convert);
        }

        dsyslog("[audiorecorder]: postprocessing thread stopped (%s, %s())",
                __FILE__, __func__);
}


void cPostproc::add_track(const cPostData *track)
{
        if (track->get_path().empty() || track->get_recpath().empty())
                return;

        if (access(track->get_path().c_str(), F_OK) != -1) {
                remove(track->get_recpath().c_str());
      		dsyslog("[audiorecorder]: track (%s) already exists (%s, "
                        "%s())", track->get_path().c_str(), __FILE__,
                        __func__);
                return;
        }

        mutex.Lock();
        postlist.push_back(*track);
        mutex.Unlock();
}


int cPostproc::get_num_queued(void)
{
        int num;

        mutex.Lock();
        num = postlist.size();
        mutex.Unlock();

        return num;
}


bool cPostproc::reencode(void)
{
        ifstream infile;
        infile.open(postdata->get_recpath().c_str());
        if (! infile) {
                dsyslog("[audiorecorder]: can't open input file (%s) (%s, "
                        "%s())", postdata->get_recpath().c_str(), __FILE__,
                        __func__);
                return false;
        }

        dsyslog("[audiorecorder]: (recpath      : %s) (%s, %s())", postdata->get_recpath().c_str(), __FILE__, __func__);
        dsyslog("[audiorecorder]: (path         : %s) (%s, %s())", postdata->get_path().c_str(), __FILE__, __func__);
        dsyslog("[audiorecorder]: (recdir       : %s) (%s, %s())", postdata->get_recdir().c_str(), __FILE__, __func__);
        dsyslog("[audiorecorder]: (artist       : %s) (%s, %s())", postdata->get_artist().c_str(), __FILE__, __func__);
        dsyslog("[audiorecorder]: (title        : %s) (%s, %s())", postdata->get_title().c_str(), __FILE__, __func__);
        dsyslog("[audiorecorder]: (file_pattern : %d) (%s, %s())", postdata->get_file_pattern(), __FILE__, __func__);
        dsyslog("[audiorecorder]: (upper        : %d) (%s, %s())", postdata->get_upper(), __FILE__, __func__);
//dsyslog("[audiorecorder]: (channel : %s) (%s, %s())", postdata->get_channel(), __FILE__, __func__);

        stringstream tmp;
        std::string path;
        ofstream outfile;

        int pos, res = 0;
        std::string dir, pfad, dirpfad;

        dirpfad = postdata->get_recdir();
        pfad = postdata->get_path();
        pos  = pfad.find(dirpfad);

        if (pos >= 0) {
            dir = pfad;
            dir.erase(0, dirpfad.length());
            dsyslog("[audiorecorder]: dir : %s (%s, %s())", dir.c_str(), __FILE__,
                        __func__);

            pos = dir.find("/");

            while (pos >= 0 && res == 0) {
                if (pos == 0) {
                    dir.erase(0, 1);
                } else {
                    dirpfad.append("/");
                    dirpfad.append(dir.substr(0, pos));
                    dir.erase(0, pos + 1);

                    res = test_and_create(dirpfad.c_str());

                    dsyslog("[audiorecorder]: 5 dirpfad : %d %s (%s, %s())", res, dirpfad.c_str(), __FILE__,
                        __func__);
                }
            dsyslog("[audiorecorder]: 6 dir : %s (%s, %s())", dir.c_str(), __FILE__,
                        __func__);
                pos = dir.find("/");
            }
        }

        outfile.open(postdata->get_path().c_str());

        if (! outfile) {
                dsyslog("[audiorecorder]: can't open output file (%s) (%s, "
                        "%s())", postdata->get_path().c_str(), __FILE__,
                        __func__);
                return false;
        }

        infile.seekg(0, ios::end);
        int file_pos = infile.tellg();
        infile.seekg(0, ios::beg);

        int frames = file_pos / postdata->get_len_mpa_frame();

        dsyslog("[audiorecorder]: start reencoding (%s into %s) (%s, %s())",
                postdata->get_recpath().c_str(), postdata->get_path().c_str(),
                __FILE__, __func__);

        int frame = 0;
        file_pos = 0;
        file_buf.offset = 0;

        while (active && ! infile.eof()) {
                infile.seekg(file_pos + file_buf.offset, ios::beg);
                file_pos = infile.tellg();
                file_buf.offset = 0;

                infile.read((char *)file_buf.data, file_buf.length);

                get_mpa_frame(&file_buf, &mpa_frame,
                        postdata->get_recpath().c_str());

                while(active && mpa_frame.data) {
                        file_buf.offset += mpa_frame.length;

                        /* pause postprocessing if osd is opened */
                        while (active && SetupValues.pause == 1 &&
                               cOsd::IsOpen())
                                usleep(10000);

                        float volume = get_volume(frame, frames);
                        abuffer *encoder_buf =
                                convert->reencode_mpa_frame(&mpa_frame, volume);

                        if (encoder_buf->data) {
                                outfile.write((char *)encoder_buf->data,
                                        encoder_buf->offset);
                                /*
                                 * encoder_buf->offset is used to save the
                                 * size of the encoded frame ...
                                 */
                                ++frame;
                        }

                        get_mpa_frame(&file_buf, &mpa_frame,
                                postdata->get_recpath().c_str());
                }
        }

        infile.close();
        outfile.close();

        remove(postdata->get_recpath().c_str());

        if (! active) {
                remove(postdata->get_path().c_str());
                return false;
        }

        dsyslog("[audiorecorder]: stop reencoding (%s into %s) (%s, %s())",
                postdata->get_recpath().c_str(), postdata->get_path().c_str(),
                __FILE__, __func__);

        return true;
}


float cPostproc::get_volume(int _frame, int _frames)
{
        int fade;

        if (_frame < postdata->get_frames_fade_in()) {
                fade = postdata->get_frames_fade_in();

                if (postdata->get_fade_in_mode() == 1)
                        return (float)_frame / (float)fade;
                else if (postdata->get_fade_in_mode() == 2)
                        return (float)(_frame * _frame) / (float)(fade * fade);
        }
        else if (_frame > _frames - postdata->get_frames_fade_out()) {
                fade = postdata->get_frames_fade_out();
                _frame = _frames - _frame;

                if (postdata->get_fade_out_mode() == 1)
                        return (float)_frame / (float)fade;
                else if (postdata->get_fade_out_mode() == 2)
                        return (float)(_frame * _frame) / (float)(fade * fade);
        }

        return 1;
}


void cPostproc::set_tag()
{
        TagLib::MPEG::File file(postdata->get_path().c_str());
        TagLib::Tag *tag = file.tag();

        if (! tag)
                return;

        tag->setArtist(postdata->get_artist());
        tag->setTitle(postdata->get_title());

        if (! postdata->get_album().empty())
                tag->setAlbum(postdata->get_album());

        if (! postdata->get_genre().empty())
                tag->setGenre(postdata->get_genre());

        if (! postdata->get_channel().empty()) {
                cPlugin *plugin = cPluginManager::GetPlugin("audiorecorder");

                stringstream tmp;

                tmp << "recorded on \"" << (postdata->get_event().empty() ?
                        "" : postdata->get_event()) << "@"
                        << postdata->get_channel() << "\" (vdr-audiorecorder "
                        << plugin->Version() << ")";

                tag->setComment(tmp.str().c_str());
        }

        file.save();

        dsyslog("[audiorecorder]: tag written (%s) (%s, %s())",
                postdata->get_path().c_str(), __FILE__, __func__);
}


void cPostproc::rename_file()
{
      	rename(postdata->get_recpath().c_str(), postdata->get_path().c_str());

     	dsyslog("[audiorecorder]: renamed %s to %s (%s, %s())",
                postdata->get_recpath().c_str(), postdata->get_path().c_str(),
                __FILE__, __func__);
}

int cPostproc::test_and_create(const char *dirname)
{
    struct stat statbuf;

    int res;

    res = stat(dirname, &statbuf);

    if (res != 0) {
        if (mkdir(dirname, S_IRWXU | S_IRWXG | S_IXOTH | S_IROTH))
            return 0;
        else
            return 1;
    } else {
        if (statbuf.st_mode & S_IFDIR) {
            return 0;
        } else {
            return 1;
        }
    }
}

