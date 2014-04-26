/*
 * rds.h
 */

#ifndef __RDS_H
#define __RDS_H

#include "recstat.h"
#include "postdata.h"
#include "a-tools.h"


#define RDS_BUF_SIZE 263*2

enum eMec {
        mecNone,
        mecPTY = 0x07,
        mecRT  = 0x0a,
        mecODA = 0x46
};

enum {
        ItemTitle = 1,
        ItemAlbum,
        ItemTrack,
        ItemArtist,
        ItemComposition, /* not supported */
        ItemMovement, /* not supported */
        ItemComposer, /* not supported */
        ItemBand, /* not supported */
        ItemComment,
        ItemGenre  /* not supported */
};

class cRds {
private:
        abuffer buf, rds_frame;
        char radiotext[65];
        int rt_length;
        bool lb0xfd;
        int last_tb, last_rb;
        eRecStat recstat;
        
        cPostData *postdata;
        
        void decode_radiotext(void);
        void decode_rtp(void);
        void decode_rtp_items(void);
        bool correct_rtp_tag(int &type, int &start, int &length);
        void delete_data(int length);
public:
        cRds(cPostData *_postdata);
        ~cRds();
        
        void put_data(uchar *data, int length);
        bool set_next_frame(void);
        eRecStat decode_frame(void);
};

#endif /* __RDS_H */
