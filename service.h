/*
 * service.h
 */

#ifndef __SERVICE_H
#define __SERVICE_H

#include <vdr/channels.h>


/*
 * service-id: "Audiorecorder-StatusRtpChannel-v1.0"
 * give me the channel, and i will set the actual status.
 */

struct Audiorecorder_StatusRtpChannel_v1_0 {
        const cChannel *channel;
        int status;
        /*
         * 0 = channel is unknown
         * 1 = channel is known, but no receiver is attached to this channel
         * 2 = receiver is attached, but there is no actual recording
         * 3 = actual recording
         */
};

#endif /* __SERVICE_H */
