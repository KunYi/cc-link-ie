/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (C) 2016 CC-Link Partner Association -http://am.cc-link.org/
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include "TIMER.h"

/************************************************************************************/
/* The following is an user defined main program. This main program is one of a		*/
/* sample in the Windows OS and Intel x86 CPU. Please rewrite if necessary.			*/
/* This main program is one of a sample in the Linux. Please rewrite if necessary.	*/
/************************************************************************************/

#ifdef _WIN32
#include <stdint.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#define MS_WINDOWS
#endif

#ifdef MS_WINDOWS
#include <windows.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment( lib, "winmm.lib" )
#else
#include <sys/time.h>
#endif
#elif __linux__
#include <time.h>
#include <string.h>
#include <sys/time.h>
#endif
/*[ Structure for the timer ]*/
typedef struct 
{
	int				iId;			/* Timer id */
	int64_t			llTime;			/* Period of time [us] */
	int64_t			llStart;		/* Start of time [us] */
	TIMER_CALLBACK	pCallbackFunc;	/* Callback function */
	void			*pCallbackArg;	/* Callback argument */
} TIMER;

#ifdef _WIN32
struct timezone {
	int		tz_minuteswest;
	int		tz_dsttime;    
};
#elif __linux__
#endif

static TIMER Timer[TIMER_MAX];
static int iTimerId = 0;

/* Definition of function of the timer */
int	timer_gettimeofday( struct timeval *tv, struct timezone *tz );

/************************************************************************************/
/* This is an user defined function for initialize the timer.						*/
/************************************************************************************/
void timer_initialize( void )
{
	/* Initialize the timer environment */
	memset( &Timer, 0, sizeof( Timer ) );

	return;
}

/************************************************************************************/
/* This is an user defined function for terminate the timer.						*/
/************************************************************************************/
void timer_terminate( void )
{
	/* Terminate the timer environment */
	/* if needed */

	return;
}

/************************************************************************************/
/* This is an user defined function for main the timer.								*/
/************************************************************************************/
void timer_main( void )
{
	int i, iStopId;
	int64_t llCurrent;

	/* main loop for the timer */
	for( i = 0; i < TIMER_MAX; i++ )
	{
		if ( Timer[i].iId != 0 )
		{
			/* Get the difference time */
			llCurrent = timer_get_time();
			/* Timeout the timer */
			if ( Timer[i].llTime <= ( llCurrent - Timer[i].llStart ) )
			{
				iStopId = Timer[i].iId;
				if ( Timer[i].pCallbackFunc != NULL )
				{
					/* Execute of the callback function */
					Timer[i].pCallbackFunc( Timer[i].iId, Timer[i].pCallbackArg );
				}
				/* Initialize of the timer */
				if ( iStopId == Timer[i].iId )
				{
					Timer[i].iId = 0;
				}
			}
		}
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for starting the timer.[ms]						*/
/************************************************************************************/
int timer_start( long lTime, int *piId, TIMER_CALLBACK pCallbackFunc, void *pCallbackArg )
{
	int i;
	struct	timeval tv;

	/* Check the unusing timer */
	for( i = 0; i < TIMER_MAX; i++ )
	{
		if ( Timer[i].iId == 0 )
		{
			break;
		}
	}
	if ( i == TIMER_MAX )
	{
		/* No free timer */
		return TIMER_RESOURCE_NONE;
	}

	iTimerId ++;
	if ( iTimerId == 0 )
	{
		iTimerId = 1;
	}
	Timer[i].iId = iTimerId;

	timer_gettimeofday( &tv, (struct timezone *)NULL );

	Timer[i].llTime = (int64_t)lTime * 1000;
	Timer[i].llStart = (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
	Timer[i].pCallbackFunc = pCallbackFunc;
	Timer[i].pCallbackArg = pCallbackArg;

	if ( piId != NULL )
	{
		*piId = Timer[i].iId;
	}

	return TIMER_OK;
}

/************************************************************************************/
/* This is an user defined function for stoping the timer.							*/
/************************************************************************************/
void timer_stop( int iId )
{
	int i;

	/* Check the using timer */
	for( i = 0; i < TIMER_MAX; i++ )
	{
		if ( Timer[i].iId == iId )
		{
			/* Initialize of the timer */
			Timer[i].iId = 0;
			break;
		}
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for getting the current time.[us]				*/
/************************************************************************************/
int64_t timer_get_time( void )
{
	int64_t	llCurrent;
	struct	timeval tv;

	timer_gettimeofday( &tv, (struct timezone *)NULL );

	llCurrent = (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;

	return llCurrent;
}

/************************************************************************************/
/* This is an user defined function for calculate the Time Data[ms] (Time of UNIX).	*/
/************************************************************************************/
int64_t timer_calculate_time_data( void )
{
	time_t timer;
#ifdef _WIN32
	struct tm t_st;
#elif __linux__
	struct tm *t_st;
#endif
	struct	timeval tv;
	uint16_t usYear, usMonth, usDay;
	uint16_t usHour, usMinute, usSecond, usMilliseconds;
	int64_t llDays;
	int64_t llTime;

	/* Getting the current time */
	time(&timer);
	timer_gettimeofday( &tv, (struct timezone *)NULL );

	/* Convert to local time */
#ifdef _WIN32
	localtime_s( &t_st,&timer );
#elif __linux__
	t_st = localtime( &timer );
#endif
#ifdef _WIN32
	usYear = (uint16_t)t_st.tm_year + 1900;
	usMonth = (uint16_t)t_st.tm_mon + 1;
	usDay = (uint16_t)t_st.tm_mday;
	usHour = (uint16_t)t_st.tm_hour;
	usMinute = (uint16_t)t_st.tm_min;
	usSecond = (uint16_t)t_st.tm_sec;
#elif __linux__
	usYear = (uint16_t)t_st->tm_year + 1900;
	usMonth = (uint16_t)t_st->tm_mon + 1;
	usDay = (uint16_t)t_st->tm_mday;
	usHour = (uint16_t)t_st->tm_hour;
	usMinute = (uint16_t)t_st->tm_min;
	usSecond = (uint16_t)t_st->tm_sec;
#endif
	usMilliseconds = (uint16_t)( tv.tv_usec / 1000 );

	/* Converting the January and the February in the previous year of the month of the 13th and 14th. */
	if ( usMonth <= 2 ) {
		usYear--;
		usMonth += 12;
	}

	llDays = 365 * ( usYear - 1 )
			+ ( usYear / 4 ) - ( usYear / 100 ) + ( usYear / 400 )
			+ (( usMonth * 979 - 1033 ) / 32 ) + usDay - 1;

	llTime = (( llDays - 719162 ) * 86400 ) + ( usHour * 3600 ) + ( usMinute * 60 ) + usSecond;
	llTime = ( llTime * 1000 ) + usMilliseconds;

	return llTime;
}

/************************************************************************************/
/* This is an user defined function for analyze the Time Data (Time of UNIX).		*/
/************************************************************************************/
void timer_analyze_time_data( int64_t llTime, TIMER_TIME_DATA *pTimeData )
{
	/* Number of days table of each month (for the year-round, for a leap year) */
	static const uint8_t    aucDaysTable[2][13] = {
		{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
		{ 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
	};
	uint32_t ulElapsedDays;
	uint32_t ulTotalDays;
	uint32_t ulThisYear;
	uint32_t ulLastYearDays;
	uint32_t ulThisYearDays;
	uint32_t ulTempDays;
	uint16_t usThisMonth;
	uint32_t ulThisMinute;
	uint16_t usIndex;

	/* Calculate the number of days elapsed */
	ulElapsedDays = (uint32_t)((( llTime / 1000) / 86400 ) + 719162);

	/* It calculates the total number of days, including the day to the number of days elapsed */
	ulTotalDays = ulElapsedDays + 1;

	/* Initialized with the previous year */
	ulThisYear = ulTotalDays / 365;
	/* Calculate the year */
	while ( 1 )
	{
		/* Calculate the previous year the number of days */
		ulLastYearDays = ( ulThisYear * 365 ) + ( ulThisYear / 4 ) - ( ulThisYear / 100 ) + ( ulThisYear / 400 );
		if ( ulLastYearDays >= ulTotalDays )
		{
			ulThisYear --;
		}
		else
		{
			ulThisYear ++;
			break;
		}
	}

	/* Calculate the number of days from January 1 */
	ulThisYearDays = ulTotalDays - ulLastYearDays;

	/* Calculating the index for the number of days table of each month or the full year or a leap year */
	if (( ulThisYear % 4 == 0 ) && ( ulThisYear % 100 != 0 ) || ( ulThisYear % 400 == 0 ))
	{
		usIndex = 1;
	}
	else
	{
		usIndex = 0;
	}

	/* Calculate the month */
	ulTempDays = 0;
	for ( usThisMonth = 0; ( ulTempDays < ulThisYearDays ) && ( usThisMonth < 12 ); usThisMonth++ )
	{
		ulTempDays += aucDaysTable[ usIndex ][ usThisMonth + 1 ];
	}

	/* Results to the argument */
	pTimeData->usYear  = (uint16_t)ulThisYear;
	pTimeData->usMonth = usThisMonth;
	pTimeData->usDay = (uint16_t)( ulThisYearDays - ( ulTempDays - aucDaysTable[ usIndex ][ usThisMonth ]));
	ulThisMinute = (uint32_t)(( llTime / 1000) % 86400 );
	pTimeData->usHour = (uint16_t)( ulThisMinute / 3600 );
	pTimeData->usMinute = (uint16_t)(( ulThisMinute % 3600 ) / 60 );
	pTimeData->usSecond = (uint16_t)(( ulThisMinute % 3600 ) % 60 );
	pTimeData->usMilliseconds = (uint16_t)( llTime % 1000 );
	
	return;
}

#ifdef _WIN32
#ifdef MS_WINDOWS
/************************************************************************************/
/* This is an user defined function for "gettimeofday" function [us] (POSIX).		*/
/* The following is one of a sample in the Windows OS.								*/
/************************************************************************************/

#define EPOCHFILETIME (116444736000000000i64)

int timer_gettimeofday( struct timeval *tv, struct timezone *tz )
{
	static FILETIME			ft;
	static LARGE_INTEGER	lTime;
	__int64					val64;
	static int				tv_first = 0, tz_first = 0;
	long					lTimezone = 0;
	int						iDaylight = 0;
	/* static DWORD			dwStart; */ /* Method1 */
	/* DWORD					dwEnd; */ /* Method1 */
	static LARGE_INTEGER	start; /* Method2 */
	LARGE_INTEGER			freq, end; /* Method2 */

	if ( tv != NULL )
	{
		QueryPerformanceFrequency(&freq); /* Method2 */
		if ( tv_first == 0 )
		{
			GetSystemTimeAsFileTime( &ft );
			/* dwStart = timeGetTime(); */ /* Method1 */
			QueryPerformanceCounter(&start); /* Method2 */
			tv_first = 1;
		}
		/* dwEnd = timeGetTime(); */ /* Method1 */
		QueryPerformanceCounter(&end); /* Method2 */
		lTime.LowPart  = ft.dwLowDateTime;
		lTime.HighPart = ft.dwHighDateTime;
		/* lTime.QuadPart += (dwEnd - dwStart) * 10000; */ /* Method1 */
		lTime.QuadPart += ((end.QuadPart - start.QuadPart) * 10000000) / freq.QuadPart; /* Method2 */
		val64 = lTime.QuadPart;
		val64 = val64 - EPOCHFILETIME;
		val64 = val64 / 10;
		tv->tv_sec  = (long)( val64 / 1000000 );
		tv->tv_usec = (long)( val64 % 1000000 );
	}

	if ( tz != NULL )
	{
		if ( tz_first == 0 )
		{
			_tzset();
			tz_first = 1;
		}

		_get_timezone( &lTimezone );
		tz->tz_minuteswest = lTimezone / 60;

		_get_daylight( &iDaylight );
		tz->tz_dsttime = iDaylight;
	}

	return TIMER_OK;
}

#else
/************************************************************************************/
/* This is an user defined function for "gettimeofday" function.[us] (POSIX)		*/
/* The following is one of a sample. Please rewrite if necessary.					*/
/************************************************************************************/
int	timer_gettimeofday( struct timeval *tv, struct timezone *tz )
{
	return gettimeofday( tv, tz ); 
}
#endif
#elif __linux__
/************************************************************************************/
/* This is an user defined function for "gettimeofday" function.[us] (POSIX)		*/
/* The following is one of a sample. Please rewrite if necessary.					*/
/************************************************************************************/
int	timer_gettimeofday( struct timeval *tv, struct timezone *tz )
{
	return gettimeofday( tv, tz ); 
}
#endif
