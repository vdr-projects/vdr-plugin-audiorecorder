/*
 * recstat.h
 */

#ifndef __RECSTAT_H
#define __RECSTAT_H


enum eRecStat {
        recWait,        /* wait for the first toggle */
        recOff,         /* no active recording */
        recStart,       /* start recording */
        recRun,         /* active recording */
        recStop,        /* stop recording */        
};

#endif /* __RECSTAT_H */
