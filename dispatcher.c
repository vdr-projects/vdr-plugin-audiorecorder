/*
 * dispatcher.c
 */

#include "dispatcher.h"
#include "setup.h"
#include "audiorecorder.h"
#include "postproc.h"
#include "a-tools.h"

#include <vdr/device.h>
#include <vdr/channels.h>
#include <vdr/plugin.h>

#include <unistd.h>
#include <sys/statfs.h>
#include <unistd.h>


#define COUNTER 30

cRecorderChannels RecorderChannels;

using namespace std;

/* --- cDispatcher----------------------------------------------------------- */

cDispatcher::cDispatcher(void)
:cThread(), cStatus()
{
        tChannelID channel_id;
        int c;

        active = false;

        /* Load Channels to record */

        RecorderChannels.Load(cPluginAudiorecorder::get_cfg().c_str());

        if (RecorderChannels.Count() < 1) {
            dsyslog("[audiorecorder]: you must have defined at least one channel for recording in %s (%s, %s()",  cPluginAudiorecorder::get_cfg().c_str(),
                                                                          __FILE__, __func__);
            Skins.QueueMessage(mtInfo, tr("Audiorecorder: No Channels defined !"));
            return;
        }

        /* initialize one receiver for each channel */
        c = 0;

        for (cChannelList *cl = RecorderChannels.First(); cl; cl = RecorderChannels.Next(cl)) {
            if (cl->Channel()) {
                dsyslog("[audiorecorder]: Channel to be recorded # %d <%s>, <%s> %s (%s, %s())",
                                                                         c,
                                                                         cl->ChannelString(),
                                                                         cl->Comment(), cl->Channel()->Name(),
                                                                         __FILE__, __func__);

                if (cl->NewAudioReceiver())
                    c++;
            }
        }

        dsyslog("[audiorecorder]: Number of Channels : %d - Number of Receivers : %d (%s, %s))", RecorderChannels.Count(), c, __FILE__, __func__);

	if (SetupValues.start_type == 1)
        	Activate(true);
}


cDispatcher::~cDispatcher()
{
	Activate(false);

    for (cChannelList *cl = RecorderChannels.First(); cl; cl = RecorderChannels.Next(cl)) {
        if (cl->AudioReceiver()) {
            cl->DeleteAudioReceiver();
        }
        }
}


void cDispatcher::Activate(bool on)
{
        if (on) {
                if (! active) {
                        active = true;
                        Start();
                }
    } else if (active) {
                active = false;
                Cancel(3);
        }
}


void cDispatcher::Action(void)
{
        bool interrupted = false;
        counter = COUNTER;

        dsyslog("[audiorecorder]: dispatcher thread started (%s, %s())",
                __FILE__, __func__);

        while (active) {
                sleep(1);

                int num_queued = cPostproc::get_num_queued();

                if (interrupted && num_queued > (SetupValues.max_postproc / 4))
                        continue;

                interrupted = false;

                if (num_queued >= SetupValues.max_postproc) {
                        dsyslog("[audiorecorder]: max. postprocessings in queue"
                                " achieved (%d), detaching all receivers (%s, "
                                "%s())", num_queued, __FILE__, __func__);

                        interrupted = true;
                        detach_receivers(-1, 0);

                        continue;
                }

                if (! check_free_disc_space()) {
                        dsyslog("[audiorecorder]: min. free disc space on %s "
                                "reached (%d mb), recording will be stopped "
                                "(%s, %s())",
                                cPluginAudiorecorder::get_recdir().c_str(),
                                SetupValues.min_free_space, __FILE__,
                                __func__);

                        active = false;
                        continue;
                }

                int receivers = get_attached_receivers(-1);
                if (SetupValues.max_receivers > receivers) {
                        if (counter > 0)
                                counter--;
                        else
                                attach_receivers();
                }
                else if (SetupValues.max_receivers < receivers)
                        detach_receivers(-1, SetupValues.max_receivers);
        }

        detach_receivers(-1, 0);

        dsyslog("[audiorecorder]: dispatcher thread stopped (%s, %s())",
                __FILE__, __func__);
}


void cDispatcher::ChannelSwitch(const cDevice *device, int channel_number, bool LiveView)
{
	/*
	 * workaround to detach active audioreceivers if the attached device
	 * is switched to another transponder. this should be done inside of
	 * vdr, but there are situations which lead to emergency exits of vdr
	 * without this peace of code (for example if a recording starts).
         *
	 */

        int device_number;

        if (channel_number == 0) {
                counter = COUNTER;
                return;
        }

        device_number = device->DeviceNumber();

        if (get_attached_receivers(device_number) == 0)
                return;

	if (Channels.GetByNumber(channel_number)->Transponder() ==
	    get_transponder_of_first_receiver(device_number))
		return;

        detach_receivers(device_number, 0);
}


int cDispatcher::get_attached_receivers(int device_number)
{
    int count = 0;

    for (cChannelList *cl = RecorderChannels.First(); cl; cl = RecorderChannels.Next(cl)) {
        if (cl->AudioReceiver()) {
            if (cl->AudioReceiver()->is_attached(device_number)) {
			++count;
        }
        }
    }
        return count;
}


int cDispatcher::get_recording_receivers(void)
{
    int count = 0;

    for (cChannelList *cl = RecorderChannels.First(); cl; cl = RecorderChannels.Next(cl)) {
        if (cl->AudioReceiver()) {
            if (cl->AudioReceiver()->is_recording()) {
                        ++count;
        }
        }
    }
        return count;
}


int cDispatcher::get_recording_status(const cChannel *channel)
{
    for (cChannelList *cl = RecorderChannels.First(); cl; cl = RecorderChannels.Next(cl)) {
        if (cl->AudioReceiver() && cl->AudioReceiver()->get_channel() == channel) {
            if (cl->AudioReceiver()->is_recording())
                                return 3;
            else if (cl->AudioReceiver()->is_attached(-1))
                                return 2;
                        else
                                return 1;
                    }
        }

        return 0;
}

int cDispatcher::get_no_of_channels(void) {
        return RecorderChannels.Count();
}

void cDispatcher::detach_receivers(int detach_device_number,
        int devices_remaining)
{
        int device_number;
        cDevice *device = NULL;

        Lock();

    for (cChannelList *cl = RecorderChannels.First(); cl; cl = RecorderChannels.Next(cl)) {
                if (get_attached_receivers(-1) <= devices_remaining)
                        break;

        if (!cl->AudioReceiver())
                        continue;

        device_number = cl->AudioReceiver()->get_device_number();

                if (device_number < 0)
                        continue;

        if (detach_device_number != device_number && detach_device_number > 0)
                        continue;

                device = cDevice::GetDevice(device_number);

                if (! device)
                        continue;

        device->Detach(cl->AudioReceiver());
        }
        Unlock();
}


void cDispatcher::attach_receivers(void)
{
        cAudioReceiver *receiver;
        cDevice *device;
        bool needs_detach = false;

        Lock();

    for (cChannelList *cl = RecorderChannels.First(); cl; cl = RecorderChannels.Next(cl)) {
                if (get_attached_receivers(-1) >= SetupValues.max_receivers)
                        break;

        receiver = cl->AudioReceiver();

                if (! receiver || receiver->is_attached(-1))
                        continue;

        device = cDevice::GetDevice(receiver->get_channel(), 0, needs_detach);

                if (! device || (device && needs_detach))
                        continue;

#ifndef AUDIORCORDER_DEVEL
		if (device == cDevice::ActualDevice() &&
                    ! device->IsTunedToTransponder(receiver->get_channel()))
                        continue;
#endif
                if (! device->IsTunedToTransponder(receiver->get_channel()) &&
                    ! device->SwitchChannel(receiver->get_channel(), false))
                        continue;

                receiver->set_device_number(device->DeviceNumber());
                device->AttachReceiver(receiver);
        }

        Unlock();
}


int cDispatcher::get_transponder_of_first_receiver(int device_number)
{
    int transponder = 0;

    for (cChannelList *cl = RecorderChannels.First(); cl; cl = RecorderChannels.Next(cl)) {
        if (! cl->AudioReceiver())
                        continue;

        if (cl->AudioReceiver()->is_attached(device_number)) {
            transponder = cl->AudioReceiver()->get_channel()->Transponder();
                        break;
                }
        }

        return transponder;
}


bool cDispatcher::check_free_disc_space(void)
{
        struct statfs stat_fs;

        if (statfs(cPluginAudiorecorder::get_recdir().c_str(), &stat_fs) != 0)
                return false;

        if ((uint64_t)stat_fs.f_bavail * (uint64_t)stat_fs.f_bsize <
            (uint64_t)SetupValues.min_free_space * (uint64_t)1024 *
            (uint64_t)1024)
                return false;

        return true;
}

// -- cRecorderChannels --------------------------------------------------------------

bool cRecorderChannels::Load(const char *filename, bool dummy)
{
    dsyslog("[audiorecorder]: <%s>  (%s, %s()",  filename, __FILE__, __func__);

    if(cConfig < cChannelList >::Load(filename, true)) {
        SetSource(First());
        return true;
    }
    return false;
}

// -- cChannelList --------------------------------------------------------------

cChannelList::cChannelList(void)
{
    audioreceiver = NULL;
}

cChannelList::cChannelList(const char *_channelstring, const char *_comment)
{
    Set(_channelstring, _comment);
}

cChannelList::~cChannelList()
{
   if (audioreceiver) {
        delete audioreceiver;
   }
   if (channelstring) {
        free(channelstring);
   }
   if (comment) {
        free(comment);
   }
}

void cChannelList::Set(const char *_channelstring, const char *_comment)
{
    channelstring = strdup(_channelstring);
    comment = strdup(_comment);
    channel_id = tChannelID::FromString(channelstring);

    if (! channel_id.Valid()) {
        dsyslog("[audiorecorder]: channel %s is not valid (%s, %s())", channelstring, __FILE__, __func__);
    } else {
        channel = Channels.GetByChannelID(channel_id, true, true);
        if (channel) {
            dsyslog("[audiorecorder]: channel %s set (%s, %s())", channel->Name(), __FILE__, __func__);
        } else {
            dsyslog("[audiorecorder]: channel not set : %s  %d, %d, %d, %d, %d (%s, %s())", channelstring,
                                                                        channel_id.Source(),
                                                                        channel_id.Nid(),
                                                                        channel_id.Tid(),
                                                                        channel_id.Sid(),
                                                                        channel_id.Rid(),
                                                                        __FILE__, __func__);
        }
    }
}

cAudioReceiver *cChannelList::NewAudioReceiver()
{
    if (channel) {
        audioreceiver = new cAudioReceiver(channel);
        return audioreceiver;
    }
    return NULL;
}

void cChannelList::DeleteAudioReceiver(void) {
    if (audioreceiver)
        delete audioreceiver;
}

bool cChannelList::Parse(char *s)
{
    char cs[256], comment[256], *p1;

    int fields = sscanf(s, "%s", cs);
    if (fields >= 1) {
        p1 = s;
        p1 += strlen(cs) + 1;

        strncpy(comment, p1, 255);
        comment[255] = 0;

        Set(cs, comment);
        return true;
    } else {
        return false;
    }
}

