#ifndef LGM_ELAPSEDTIME_H
#define LGM_ELAPSEDTIME_H

#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct Lgm_ElapsedTimeInfo {

    int     ColorizeText;    // Turns on colorized text output (to terminal)
    char    ColorStart[15];  // Escape sequence for colorizing text.
    char    ColorEnd[15];    // Switch back to white.
    time_t  RunStartTime;    // Clock time that the run was started.
    time_t  RunEndTime;      // Clock time that the run ended.

    char    ElapsedTimeStr[15];  // String containing elapsed time. 
    char    CurrentTimeStr[15];  // String containg current time.


} Lgm_ElapsedTimeInfo;

void    Lgm_ElapsedTimeInit( Lgm_ElapsedTimeInfo *t, int red, int grn, int blu );
double  Lgm_SetElapsedTimeStr( Lgm_ElapsedTimeInfo *t ); // returns total number of seconds
void    Lgm_PrintElapsedTime( Lgm_ElapsedTimeInfo *t );
void    Lgm_SetCurrentTimeStr( Lgm_ElapsedTimeInfo *t );
void    Lgm_PrintCurrentTime( Lgm_ElapsedTimeInfo *t );

#endif
