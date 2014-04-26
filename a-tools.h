/*
 * a-tools.h:
 */


#ifndef __A_TOOLS_H
#define __A_TOOLS_H

#define KB(x) x * 1024
#define MB(x) KB(x) * 1024

#define DELETE(x) if (x != NULL) { delete x; x = NULL; }

typedef unsigned char uchar;

typedef struct abuffer {
        uchar *data;
        int length;
        int offset;
} abuffer;

#endif /* __A_TOOLS_H */
