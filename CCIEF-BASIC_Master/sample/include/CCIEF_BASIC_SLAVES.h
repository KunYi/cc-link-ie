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

#ifndef		__CCIEF_BASIC_SLAVES_H__
#define		__CCIEF_BASIC_SLAVES_H__

#include <stdint.h>
#include "CCIEF_BASIC_MASTER.h"

/*[ Definition for CCIEF-BASIC Slaves ]*/
#define	CCIEF_BASIC_CYCLIC_STATE_SET_CYCLIC_STOP		0			/* Setting the cyclic state of stop cyclic. */
#define	CCIEF_BASIC_CYCLIC_STATE_SET_CONNECTING			1			/* Setting the cyclic state of connecting. */
#define	CCIEF_BASIC_CYCLIC_STATE_SET_CYCLIC				2			/* Setting the cyclic state of cyclic. */

/*[ Structure for CCIEF-BASIC Slaves ]*/
typedef struct
{
	CCIEF_BASIC_SLAVE_PARAMETER		*pParameter;						/* Parameter */
	int								iNumber;							/* Slave Number */
	uint32_t						ulId;								/* Id number */
	int								iStationNumber;						/* Number of stations */
	int								iGroupStationNumber;				/* Number of stations for the group */
	int								iCyclicStart;						/* Start cyclic of the user operation */
	int								iState;								/* State of Slave */
	int								iCyclicState;						/* Cyclic state */
	int								iCyclicStateSet;					/* Setting of the cyclic state */
	int								iReceiveComplete;					/* State of response receive */
	int								iDuplicateState;					/* State of slave duplication */
	uint16_t						*pusFrameSequenceNumber;			/* Frame sequence number of the master */
	uint16_t						usProtocolVersion;					/* Protocol version of the slave */
	uint16_t						usEndCode;							/* End code of the slave */
	uint16_t						usFrameSequenceNumber;				/* Frame sequence number of the slave */
	CCIEF_BASIC_SLAVE_NOTIFY_INFO	NotifyInfo;							/* Notification information of the slave */
	uint16_t						usCyclicTransmissionTimeoutCount;	/* Count of cyclic transmission timeout */
	uint16_t						usTimeoutCount;						/* Counter of timeout for the cyclic transmission timeout */
	uint16_t						*pusRWw;							/* Pointer of RWw for the packet */
	uint16_t						*pusRY;								/* Pointer of RY for the packet */
	uint16_t						*pusRWr;							/* Pointer of RWr for the packet */
	uint16_t						*pusRX;								/* Pointer of RX for the packet */
	uint16_t						*pusSlaveRWr;						/* Pointer of RWr for the slave */
	uint16_t						*pusSlaveRX;						/* Pointer of RX for the slave */
} CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO;

/* Definition of function of sample program */
extern void ccief_basic_slaves_initialize( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave );
extern void ccief_basic_slaves_execute_state( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent );

#endif
/*EOF*/