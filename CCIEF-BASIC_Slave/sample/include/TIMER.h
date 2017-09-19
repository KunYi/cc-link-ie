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

#ifndef		__TIMER_H__
#define		__TIMER_H__

#include <stdint.h>

/*[ Definition for timer ]*/
#define	TIMER_OK				0
#define	TIMER_RESOURCE_NONE		(-1)
#define	TIMER_MAX				5		/* Number of timer MAX */

/*[ Structure for sample code ]*/
typedef struct 
{
	uint16_t	usYear;				/* Year */
	uint16_t	usMonth;			/* Month */
	uint16_t	usDay;				/* Day */
	uint16_t	usHour;				/* Hour */
	uint16_t	usMinute;			/* Minute */
	uint16_t	usSecond;			/* Second */
	uint16_t	usMilliseconds;		/* Milliseconds */
} TIMER_TIME_DATA;

/*[ Definition of callback function ]*/
typedef void (*TIMER_CALLBACK)( int iId, void *pCallbackArg );

/* Definition of function of the timer */
extern void timer_initialize( void );
extern void timer_terminate( void );
extern void timer_main( void );
extern int timer_start( uint32_t ulTime, int *piId, TIMER_CALLBACK pCallbackFunc, void *pCallbackArg );
extern void timer_stop( int iId );
extern uint32_t timer_get_time( void );
extern uint32_t timer_broadcast_send_wait_time ( uint32_t ulMaxWaitTime );
extern void timer_analyze_time_data( int64_t ullTime, TIMER_TIME_DATA *pTimeData );

#endif
/*EOF*/