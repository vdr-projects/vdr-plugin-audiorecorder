/*
 * external-player.h
 */

#ifndef __EXTERNAL_PLAYER_H
#define __EXTERNAL_PLAYER_H

struct MP3ServiceData {
  int result;
  union {
    const char *filename;
    } data;
  };

#endif //__EXTERNAL_PLAYER_H
