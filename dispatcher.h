/*
 * dispatcher.h
 */

#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#include "audioreceiver.h"

#include <vdr/device.h>
#include <vdr/thread.h>
#include <vdr/status.h>

class cChannelList : public cListObject {
    char *channelstring;
    char *comment;

    tChannelID channel_id;
    cChannel *channel;

    cAudioReceiver *audioreceiver;
public:
    cChannelList(void);
    cChannelList(const char *_channelstring, const char *_comment);
    void Set(const char *_channelstring, const char *_comment);
    cAudioReceiver *NewAudioReceiver();
    void DeleteAudioReceiver(void);
    bool Parse(char *s);
    const char *ChannelString(void) { return channelstring; }
    const char *Comment(void) { return comment; }
    cAudioReceiver *AudioReceiver(void) { return audioreceiver; }
    cChannel *Channel(void) { return channel; }
    ~cChannelList();
};


class cRecorderChannels : public cConfig<cChannelList> {
    cChannelList *current;
public:
    virtual bool Load(const char *filename, bool dummy=false);
    void SetSource(cChannelList *source) { current=source; }
    cChannelList *GetChannel(void) { return current; }
};

class cDispatcher : public cThread, cStatus {
private:
        bool active;
        int counter;
        // cAudioReceiver **audioreceivers;

        void attach_receivers(void);

        int get_transponder_of_first_receiver(int device_number);
	/*
	 * returns the transponder of the first attached audioreceiver on the
	 * given device
	 */

        void detach_receivers(int detach_device_number, int devices_remaining);
        /*
         * detaches all audioreceivers on the given device_number, until
         * devices_remaining are remaining.
         * if detach_device_number == -1, all audioreceivers are
         * detached.
         */

        bool check_free_disc_space(void);
protected:
        virtual void Activate(bool on);
        virtual void Action(void);

        virtual void ChannelSwitch(const cDevice *device, int channel_number);
public:
	cDispatcher(void);
	~cDispatcher();

        void stop(void) { Activate(false); }
        void start(void) { Activate(true); }
        bool is_active(void) { return active; }

        int get_recording_receivers(void);
        int get_recording_status(const cChannel *channel);
        int get_attached_receivers(int device_number);
        int get_no_of_channels(void);

        /*
         * returns the number of the attached audioreceivers on the given
         * device. if device_number == -1, the number of all attached
         * audioreceivers is returned.
         */
};

#endif /* __DISPATCHER_H */
