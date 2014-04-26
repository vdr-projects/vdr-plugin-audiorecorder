/*
 * postproc.h
 */

#ifndef __POSTPROC_H
#define __POSTPROC_H

#include "postdata.h"
#include "convert.h"
#include "mpa-frame.h"
#include "a-tools.h"

#include <vdr/thread.h>

#include <list>


class cPostproc : public cThread {
private:
        bool active;
        
        static std::list<cPostData> postlist;
        static cMutex mutex;
        std::list<cPostData>::iterator postdata;
        
        abuffer file_buf;
        mpeg_audio_frame mpa_frame;
        cConvert *convert;
        
        bool reencode(void);
        void fade_in(void);
        void fade_out(void);
        float get_volume(int _frame, int _frames);
        void set_tag(void);
        void rename_file(void);
        int test_and_create(const char *dirname);
protected:
        virtual void Action(void);
        virtual void Activate(bool on);
public:
        cPostproc(void);
        ~cPostproc();
        
        static void add_track(const cPostData *track);
        static int get_num_queued(void);
};

#endif /* __POSTPROC_H */
