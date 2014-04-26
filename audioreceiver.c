/*
 * audioreceiver.c
 */

#include "audioreceiver.h"
#include "postproc.h"
#include "audiorecorder.h"


/* --- cAudioReceiver ------------------------------------------------------- */

cAudioReceiver::cAudioReceiver(const cChannel *_channel)
:cReceiver(_channel->GetChannelID(), -2, _channel->Apid(0)), cThread()
{
        channel = _channel;

        active = false;

        pes_sync = false;
	device_number = -1;
	recstat = recWait;

        buffer = NULL;
        rds = NULL;
        postdata = NULL;

        dsyslog("[audiorecorder]: receiver for channel <%s> created on pid %d "
                "(%s, %s())", channel->Name(), channel->Apid(0), __FILE__,
                __func__);
}


cAudioReceiver::~cAudioReceiver(void)
{
	Activate(false);

        DELETE(buffer);
        DELETE(rds);
        DELETE(postdata);
}


void cAudioReceiver::Receive(uchar *data, int length)
{
        int ts_header_len, header_len, data_len, rb_len;

        if (! active || data[0] != 0x47)
                return;

        /* check adaptation field */
        switch (data[3] & 0x30) {
        case 0x10:
                /* no adaptation field, payload only */
                ts_header_len = 4;
                break;
        case 0x30:
                /* adaptation field followed by payload */
                ts_header_len = data[4] + 5;
                break;
        default:
                return;
        }

        header_len = ts_header_len;

        /* check payload unit start indicator */
        if ((data[1] & 0x40) == 0x40) {
                /* ts-header is followed by a pes-header */
                header_len  += data[ts_header_len + 8] + 9;
                pes_sync = true;
        }

        if (! pes_sync)
                return;

        /* put mp2-data into the ringbuffer */
        data_len = length - header_len;
        rb_len = buffer->Put(data + header_len, data_len);

        if (data_len != rb_len) {
                buffer->ReportOverflow(data_len - rb_len);
                dsyslog("[audiorecorder]: buffer overflow (%s, %s())", __FILE__,
                        __func__);
                buffer->Clear();
        }
}


void cAudioReceiver::Activate(bool on)
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


void cAudioReceiver::Action(void)
{
        abuffer input_buf;

        dsyslog("[audiorecorder]: receiving from channel <%s> (%s, %s())",
                channel->Name(), __FILE__, __func__);

        buffer = new cRingBufferLinear(KB(100), KB(5), true, NULL);
        postdata = new cPostData(channel);
        rds = new cRds(postdata);

        while (active) {
                input_buf.data = buffer->Get(input_buf.length);
                input_buf.offset = 0;

                if (! input_buf.data) {
                        usleep(10000);
                        continue;
                }


                get_mpa_frame(&input_buf, &mpa_frame, channel->Name());

                while (active && mpa_frame.data) {
                        set_recstat_rds();

                        if (outfile.is_open())
                                outfile.write((char *)mpa_frame.data,
                                        mpa_frame.length);

                        input_buf.offset += mpa_frame.length;
                        get_mpa_frame(&input_buf, &mpa_frame, channel->Name());
                }

                buffer->Del(input_buf.offset);
        }

        if (outfile.is_open()) {
                outfile.close();

                if (! postdata->get_recpath().empty())
                        remove(postdata->get_recpath().c_str());
        }


	pes_sync = false;
        device_number = -1;
        recstat = recWait;

        DELETE(rds);
        DELETE(postdata);
        DELETE(buffer);

        dsyslog("[audiorecorder]: stopped receiving from channel <%s> "
                "(%s, %s())", channel->Name(), __FILE__, __func__);
}


bool cAudioReceiver::is_attached(int attached_device_number)
{
        if (device_number < 0)
                return false;

        if (attached_device_number != device_number &&
	    attached_device_number != -1)
		return false;

        return true;
}


bool cAudioReceiver::is_recording(void)
{
        if (recstat != recStart && recstat != recRun)
                return false;

        return true;
}


void cAudioReceiver::set_recstat_rds(void)
{
        rds->put_data(mpa_frame.data, mpa_frame.length);

        while(rds->set_next_frame()) {
                recstat = rds->decode_frame();

                control_track();
        }
}


void cAudioReceiver::control_track(void)
{
//LT
        struct stat dir;
        std::string recdir;
//
        switch (recstat) {
        case recStart:
                if (outfile.is_open())
                        outfile.close();

                postdata->start_track();
//LT
                recdir = cPluginAudiorecorder::get_recdir();
                stat(recdir.c_str(), &dir);

                if (! (dir.st_mode & S_IFDIR)) {
                        dsyslog("[audiorecorder]: %s (given with the parameter "
                        " --recdir|-r) isn't a directory", recdir.c_str());
                        return;
                }

                if (access(recdir.c_str(), R_OK | W_OK) != 0) {
                        dsyslog("[audiorecorder]: can't access %s (given with the "
                        "parameter --recdir|-r)", recdir.c_str());
                        return;
                }
//
                outfile.open(postdata->get_recpath().c_str());
//LT
               if (!outfile.is_open())
                    return;
//
                dsyslog("[audiorecorder]: started recording track (%s) on <%s> "
                        "(%s, %s())", postdata->get_recpath().c_str(),
                        channel->Name(), __FILE__, __func__);
                break;
        case recStop:
                if (outfile.is_open())
                        outfile.close();

                dsyslog("[audiorecorder]: stopped recording track (%s) on <%s> "
                        "(%s, %s())", postdata->get_recpath().c_str(),
                        channel->Name(), __FILE__, __func__);

                postdata->stop_track(mpa_frame.bit_rate, mpa_frame.sample_rate,
                        mpa_frame.channels, mpa_frame.length);

                if (postdata->get_path().empty()) {
                        remove(postdata->get_recpath().c_str());
                        dsyslog("[audiorecorder]: track (%s) removed"
                                " (%s, %s())",
                                postdata->get_recpath().c_str(),
                                __FILE__, __func__);
                } else {
                        cPostproc::add_track(postdata);
                }
                postdata->clear();

                break;
        default:
                break;
        }
}
