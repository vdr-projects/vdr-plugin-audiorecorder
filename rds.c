/*
 * rds.c
 */

#include "rds.h"
#include "audiorecorder.h"

#include <vdr/tools.h>

#include <iostream>
#include <string.h>


using namespace std;

/*
 * rt-translation: 0x80..0xff
 * this table is taken from the vdr-radio-plugin, thx a lot ...
 */
const uchar rt_trans[128] = {
        0xe1, 0xe0, 0xe9, 0xe8, 0xed, 0xec, 0xf3, 0xf2,
        0xfa, 0xf9, 0xd1, 0xc7, 0x8c, 0xdf, 0x8e, 0x8f,
        0xe2, 0xe4, 0xea, 0xeb, 0xee, 0xef, 0xf4, 0xf6,
        0xfb, 0xfc, 0xf1, 0xe7, 0x9c, 0x9d, 0x9e, 0x9f,
        0xaa, 0xa1, 0xa9, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xa3, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xba, 0xb9, 0xb2, 0xb3, 0xb1, 0xa1, 0xb6, 0xb7,
        0xb5, 0xbf, 0xf7, 0xb0, 0xbc, 0xbd, 0xbe, 0xa7,
        0xc1, 0xc0, 0xc9, 0xc8, 0xcd, 0xcc, 0xd3, 0xd2,
        0xda, 0xd9, 0xca, 0xcb, 0xcc, 0xcd, 0xd0, 0xcf,
        0xc2, 0xc4, 0xca, 0xcb, 0xce, 0xcf, 0xd4, 0xd6,
        0xdb, 0xdc, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xc3, 0xc5, 0xc6, 0xe3, 0xe4, 0xdd, 0xd5, 0xd8,
        0xde, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xf0,
        0xe3, 0xe5, 0xe6, 0xf3, 0xf4, 0xfd, 0xf5, 0xf8,
        0xfe, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };


/* map rds pty-codes 10-15 and 24-28 to id3v1 tags */
const char *ptys[29] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, "Pop", "Rock", "Easy Listening", "Classical",
        "Classical", "Other", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, "Jazz", "Country", "Folklore", "Oldies", "Folk" };

/* --- cRds ----------------------------------------------------------------- */


cRds::cRds(cPostData *_postdata)
{
        postdata = _postdata;
        recstat = recWait;
        
        buf.data = new uchar[RDS_BUF_SIZE];
        buf.length = 0;
        buf.offset = 0;
        
        rt_length = 0;
        lb0xfd = false;
        
        last_tb = -1;
        last_rb = -1;
}


cRds::~cRds()
{
        delete[] buf.data;
}


void cRds::put_data(uchar *data, int length)
{
        int c, pos, len;
        
        pos = length - 1;
        len = data[pos - 1]; /* length of rds data */
        
        if (data[pos] != 0xfd || len == 0)
                return;
        
        if (buf.length + len >= RDS_BUF_SIZE) {
                dsyslog("[audiorecorder]: buffer overflow <%s> (%s, %s())",
                        postdata->get_channel().c_str(), __FILE__, __func__);
                buf.length = 0;
                buf.offset = 0;
        }
                
        /* reverse rds data */
        for (c = 2; c < len + 2; ++c) {
                
                /* byte stuffing */
                int bs = data[pos - c];
                
                if (bs == 0xfd) {
                        lb0xfd = true;
                        continue;
                }
                
                if (lb0xfd) {
                        switch (bs) {
                        case 0x00:
                                bs = 0xfd;
                                break;
                        case 0x01:
                                bs = 0xfe;
                                break;
                        default:
                                bs = 0xff;
                        }
                        
                        lb0xfd = false;
                }
                
                /* copy rds value on the buffer */
                buf.data[buf.length] = bs;
                ++buf.length;
        }
}


bool cRds::set_next_frame(void)
{
        int offset;
        
        offset = buf.offset;
        rds_frame.data = NULL;
        rds_frame.length = 0;
        
        for (; buf.offset < buf.length - 4; ++buf.offset) {
                if (buf.data[buf.offset] == 0xfe) {
                        /* rds start marker found */
                        rds_frame.length = buf.data[buf.offset + 4] + 8;
                        
                        if (buf.offset + rds_frame.length > buf.length)
                                break;
                        
                        rds_frame.data = buf.data + buf.offset;
                        
                        /* check rds end marker */
                        if (rds_frame.data[rds_frame.length - 1] != 0xff)
                                dsyslog("[audiorecorder]: no rds end marker "
                                        "found <%s> (%s, %s())", 
                                        postdata->get_channel().c_str(),
                                        __FILE__, __func__);
                        
                        break;
                }
        }
        
        if (buf.offset != offset && cPluginAudiorecorder::get_dbg_level() > 0)
                cout << "skipped " << (buf.offset - offset) << " byte(s) <"
                        << postdata->get_channel() << "> (" << __FILE__
                        << ", " << __func__ << "())" << endl;
        
        if (! rds_frame.data) {
                delete_data(buf.offset);
                return false;
        }
        
        buf.offset += rds_frame.length;
        
        return true;
}


void cRds::delete_data(int length)
{
        /* clear the buffer */
        
        if (length < 1)
                return;
        
        buf.length -= length;
        buf.offset -= length;
        memmove(buf.data, buf.data + length, buf.length);
}


eRecStat cRds::decode_frame(void)
{
        /* reset recording status */
        switch (recstat) {
        case recStart:
                recstat = recRun;
                break;
        case recStop:
                recstat = recOff;
                break;
        default:
                break;
        }
        
        
        if (rds_frame.data[5] == mecRT)
                decode_radiotext();
        else if (rds_frame.data[5] == mecODA && rds_frame.data[7] == 0x4b &&
                 rds_frame.data[8] == 0xd7)
                decode_rtp();
        else if (rds_frame.data[5] == mecPTY) {
                int pty = rds_frame.data[8];
                
                if (recstat == recRun && postdata->get_genre().empty()) {
                        if ((pty > 9 && pty < 16) || (pty > 23 && pty < 29))
                                postdata->set_genre(ptys[pty]);
                }
                
                if (cPluginAudiorecorder::get_dbg_level() > 1)
                        cout << "pty-code <" << postdata->get_channel() <<
                                ">: " << pty << endl;
        }
        
        return recstat;
}


void cRds::decode_radiotext(void)
{
        int c, rt_ab_flag;
        
        rt_length = rds_frame.data[8] - 1;
        rt_ab_flag = rds_frame.data[9] & 0x01;
        
        for (c = 0; c < rt_length; ++c) {
                if (rds_frame.data[c + 10] >= 0x80)
                        rds_frame.data[c + 10] =
                                rt_trans[(rds_frame.data[c + 10] - 0x80)];
                
                radiotext[c] = rds_frame.data[c + 10];
        }
        
        radiotext[rt_length] = '\0';
        
        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "radiotext (" << rt_ab_flag << ") <" <<
                        postdata->get_channel() << ">: " << radiotext << endl;
}


void cRds::decode_rtp(void)
{
        int rb, tb;
        
        bool toggle_tb = false;
        bool toggle_rb = false;
        
        tb = (rds_frame.data[10] >> 4) & 0x01;
        if (last_tb == -1)
                last_tb = tb;
        
        rb = (rds_frame.data[10] >> 3) & 0x01;
        if (last_rb == -1)
                last_rb = rb;
        
        if (cPluginAudiorecorder::get_dbg_level() > 1)
                cout << "rtp-data <" << postdata->get_channel() << ">: toggle "
                        "bit: " << tb << ", running bit: " << rb << endl;
        
        if (last_tb != tb)
                toggle_tb = true;
        
        if (last_rb != rb)
                toggle_rb = true;
        
        last_tb = tb;
        last_rb = rb;
        
        if (recstat == recWait) {
                if (! toggle_tb && ! toggle_rb)
                        return;
                                
                /* ready to record */
                recstat = recOff;
        }
        
        if (rb == 1) {
                /* running bit is on */
                if (recstat == recOff) {
                        recstat = recStart;
                        rt_length = 0;
                }
                else if (toggle_tb) {
                        recstat = recStop;
                        return;
                }
        }
        else {
                /* running bit is off */
                if (recstat == recRun)
                        recstat = recStop;
                
                return;
        }
        
        if (recstat == recRun && rt_length > 0)
                decode_rtp_items();
}


void cRds::decode_rtp_items(void)
{
        int c, t[2], s[2], l[2];
        
        /* tag 1 */
        t[0] = ((rds_frame.data[10] << 3) & 0x38) | (rds_frame.data[11] >> 5);
        s[0] = ((rds_frame.data[11] << 1) & 0x3e) | (rds_frame.data[12] >> 7);
        l[0] = ((rds_frame.data[12] >> 1) & 0x3f) + 1;
        
        /* tag 2*/
        t[1] = ((rds_frame.data[12] << 5) & 0x20) | (rds_frame.data[13] >> 3);
        s[1] = ((rds_frame.data[13] << 3) & 0x38) | (rds_frame.data[14] >> 5);
        l[1] = (rds_frame.data[14] & 0x1f) + 1;
        
        for (c = 0; c < 2; ++c) {
                if (cPluginAudiorecorder::get_dbg_level() > 1)
                        cout << "rtp-data <" << postdata->get_channel() << ">: "
                                "type: " << t[c] << ", start:" << s[c] <<
                                ", length: " << l[c] << endl;
                
                if (t[c] < 1 || t[c] > 10)
                        continue;
                
                if (t[c] == ItemTitle) {
                        if (correct_rtp_tag(t[c], s[c], l[c]))
                                postdata->set_title(radiotext + s[c]);
                }
                else if (t[c] == ItemArtist) {
                        if (correct_rtp_tag(t[c], s[c], l[c]))
                                postdata->set_artist(radiotext + s[c]);
                }
                else if (t[c] == ItemAlbum) {
                        if (correct_rtp_tag(t[c], s[c], l[c]))
                                postdata->set_album(radiotext + s[c]);
                }
                else if (t[c] == ItemTrack)
                                postdata->set_track(atoi(radiotext + s[c]));
                else if (t[c] == ItemGenre) {
                        if (correct_rtp_tag(t[c], s[c], l[c]))
                                postdata->set_genre(radiotext + s[c]);
                }
        }
}


bool cRds::correct_rtp_tag(int &type, int &start, int &length)
{
        int original_length = length;
        
        if (start + length > rt_length) {
                length = rt_length - start;
                
                if (original_length - length > 1 || length < 0) {
                        if (cPluginAudiorecorder::get_dbg_level() > 1)
                                cout << "rtp-data <" << postdata->get_channel()
                                        << ">: got buggy tag-infos, could not "
                                        "correct the length value" << endl;
                        return false;
                }
        }
        
        /* remove ' ', '"' or "-" at the beginning of the tag */
        int end = start + length;
        for (; start < end; ++start) {
                if (radiotext[start] != ' ' && radiotext[start] != '"' &&
                    radiotext[start] != '-')
                        break;
                
                --length;
        }
        
        /* remove ' ' or '"'  at the end of the tag */
        for (; length > 0; length--) {
                if (radiotext[start + length - 1] != ' ' && radiotext[start +
                    length - 1] != '"')
                        break;
        }
        
        if (length <= 1 && cPluginAudiorecorder::get_dbg_level() > 1) {
                cout << "rtp-data <" << postdata->get_channel() << ">: got "
                        "buggy tag-infos, length is too short !" << endl;
                return false;
        }
        
        radiotext[start + length] = '\0';
        
        return true;
}
