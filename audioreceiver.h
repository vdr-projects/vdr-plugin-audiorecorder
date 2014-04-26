/*
 * audioreceiver.h
 */

#ifndef __AUDIORECEIVER_H
#define __AUDIORECEIVER_H

#include "postdata.h"
#include "rds.h"
#include "mpa-frame.h"
#include "recstat.h"
#include "a-tools.h"

#include <vdr/receiver.h>
#include <vdr/ringbuffer.h>
#include <vdr/channels.h>

#include <string>
#include <fstream>


class cAudioReceiver : public cReceiver, cThread {
private:
        bool active;
        int device_number;

        std::ofstream outfile;

        cRingBufferLinear *buffer;
        const cChannel *channel;

        cPostData *postdata;
        cRds *rds;

        eRecStat recstat;
        mpeg_audio_frame mpa_frame;
        bool pes_sync;

        void set_recstat_rds(void);
        void control_track(void);
protected:
        virtual void Receive(uchar *data, int length);

        virtual void Action(void);
        virtual void Activate(bool on);
public:
        cAudioReceiver(const cChannel* _channel);
        ~cAudioReceiver();

        void set_device_number(int _device_number) { device_number =
                _device_number; }
        int get_device_number(void) { return device_number; }
        bool is_attached(int attached_device_number);
        /*
        * returns true if the receiver is attached to the given device.
        * if attached_device_number == -1, true is returned if the
        * receiver is attached to a device
        */
        bool is_recording(void);
        const cChannel *get_channel(void) { return channel; }
};

#endif /* __AUDIORECEIVER_H */
