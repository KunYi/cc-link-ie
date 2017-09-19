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

#include "CCIEF_BASIC_SLAVES.h"
#include <string.h>

/* Definition of function of sample program */
static void ccief_basic_slaves_execute_state_disconnect( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent );
static void ccief_basic_slaves_execute_state_connecting( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent );
static void ccief_basic_slaves_execute_state_cyclic_stop( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent );
static void ccief_basic_slaves_execute_state_cyclic_end( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent );
static void ccief_basic_slaves_execute_state_cyclic( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent );

/************************************************************************************/
/* This is an user defined function for initialization of the slave.				*/
/************************************************************************************/
void ccief_basic_slaves_initialize( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave )
{
	/* Set the slave state */
	pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;

	return;
}

/************************************************************************************/
/* This is an user defined function for executed according to the state				*/
/* of the slave.																	*/
/************************************************************************************/
void ccief_basic_slaves_execute_state( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent )
{
	/* Execute the state of the slave */
	switch ( pSlave->iState )
	{
		/* Disconnection of the cyclic network. */
		case CCIEF_BASIC_STATE_SLAVE_DISCONNECT:
			ccief_basic_slaves_execute_state_disconnect( pSlave, iEvent );
			break;
		/* Connecting for the cyclic network. */
		case CCIEF_BASIC_STATE_SLAVE_CONNECTING:
			ccief_basic_slaves_execute_state_connecting( pSlave, iEvent );
			break;
		/* Stop the cyclic operation. */
		case CCIEF_BASIC_STATE_SLAVE_CYCLIC_STOP:
			ccief_basic_slaves_execute_state_cyclic_stop( pSlave, iEvent );
			break;
		/* End of the cyclic operation. */
		case CCIEF_BASIC_STATE_SLAVE_CYCLIC_END:
			ccief_basic_slaves_execute_state_cyclic_end( pSlave, iEvent );
			break;
		/* Running the cyclic operation. */
		case CCIEF_BASIC_STATE_SLAVE_CYCLIC:
			ccief_basic_slaves_execute_state_cyclic( pSlave, iEvent );
			break;

		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of disconnect of the cyclic network.	*/
/************************************************************************************/
void ccief_basic_slaves_execute_state_disconnect( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent )
{
	/* Check the event */
	switch ( iEvent )
	{
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE:
			/* Nothing */
			break;
		/* Make the cyclic data */
		case CCIEF_BASIC_EVENT_SLAVE_LINK_SCAN_START:
			/* Set the cyclic state */
			pSlave->iCyclicState = CCIEF_BASIC_CYCLIC_STATE_OFF;
			/* Check the start cyclic of user operation */
			if ( pSlave->iCyclicStart == CCIEF_BASIC_CYCLIC_START )
			{
				/* Set the cyclic state to connecting */
				pSlave->iCyclicStateSet = CCIEF_BASIC_CYCLIC_STATE_SET_CONNECTING;
				/* Initialize of the timeout count */
				pSlave->usTimeoutCount = 0;
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_CONNECTING;
			}
			else
			{
				/* Set the cyclic state to stop cyclic */
				pSlave->iCyclicStateSet = CCIEF_BASIC_CYCLIC_STATE_SET_CYCLIC_STOP;
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_CYCLIC_STOP;
			}
			break;
		/* Receiving the error of the duplication master from the slave */
		case CCIEF_BASIC_EVENT_SLAVE_DUPLICATION_MASTER_ERR:
			/* Nothing */
			break;

		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of connecting for the cyclic network.	*/
/************************************************************************************/
void ccief_basic_slaves_execute_state_connecting( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent )
{
	/* Check the event */
	switch ( iEvent )
	{
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* Receiving the cyclic data from the slave */
		case CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_RECV:
			/* Check the End code of the slave */
			if ( pSlave->usEndCode == CCIEF_BASIC_END_OK )
			{
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_CYCLIC_END;
			}
			else
			{
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			}
			break;
		/* Timeout the response of the cyclic data */
		case CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_TIMEOUT:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* The end of the cyclic data of all slaves */
		case CCIEF_BASIC_EVENT_SLAVE_CYCLIC_END:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* Receiving the error of the duplication master from the slave */
		case CCIEF_BASIC_EVENT_SLAVE_DUPLICATION_MASTER_ERR:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;

		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of stop the cyclic operation.			*/
/************************************************************************************/
void ccief_basic_slaves_execute_state_cyclic_stop( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent )
{
	/* Check the event */
	switch ( iEvent )
	{
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* Timeout the response of the cyclic data */
		case CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_TIMEOUT:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* The end of the cyclic data of all slaves */
		case CCIEF_BASIC_EVENT_SLAVE_CYCLIC_END:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* Receiving the error of the duplication master from the slave */
		case CCIEF_BASIC_EVENT_SLAVE_DUPLICATION_MASTER_ERR:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;

		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of end of the cyclic operation.		*/
/************************************************************************************/
void ccief_basic_slaves_execute_state_cyclic_end( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent )
{
	/* Check the event */
	switch ( iEvent )
	{
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* Make the cyclic data */
		case CCIEF_BASIC_EVENT_SLAVE_LINK_SCAN_START:
			/* Check the start cyclic of user operation */
			if ( pSlave->iCyclicStart == CCIEF_BASIC_CYCLIC_START )
			{
				/* Set the cyclic state */
				pSlave->iCyclicState = CCIEF_BASIC_CYCLIC_STATE_ON;
				/* Set the cyclic state to cyclic */
				pSlave->iCyclicStateSet = CCIEF_BASIC_CYCLIC_STATE_SET_CYCLIC;
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_CYCLIC;
			}
			else
			{
				/* Set the cyclic state to stop cyclic */
				pSlave->iCyclicStateSet = CCIEF_BASIC_CYCLIC_STATE_SET_CYCLIC_STOP;
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_CYCLIC_STOP;
			}
			break;
		/* Receiving the error of the duplication master from the slave */
		case CCIEF_BASIC_EVENT_SLAVE_DUPLICATION_MASTER_ERR:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* Receiving the cyclic data from the slave */
		case CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_RECV:
			/* Check the response received */
			if( pSlave->iReceiveComplete == CCIEF_BASIC_SLAVE_RESPONSE_RECEIVED )
			{
				/* Already receved response */
				/* Set the error code of the slave duplication */
				pSlave->iDuplicateState = CCIEF_BASIC_SLAVE_DUPLICATION_DETECT;
			}
			break;
		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of running the cyclic operation.		*/
/************************************************************************************/
void ccief_basic_slaves_execute_state_cyclic( CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave, int iEvent )
{
	/* Check the event */
	switch ( iEvent )
	{
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;
		/* Receiving the cyclic data from the slave */
		case CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_RECV:
			/* Get the current number of frame sequence */
			/* Check the end code of the slave */
			if ( pSlave->usEndCode == CCIEF_BASIC_END_OK )
			{
				/* Check the number of frame sequence */
				if ( *pSlave->pusFrameSequenceNumber == pSlave->usFrameSequenceNumber )
				{
					/* Receive the response data from the slave */
					/* RWr */
					memcpy( pSlave->pusRWr, pSlave->pusSlaveRWr, pSlave->pParameter->usOccupiedStationNumber * CCIEF_BASIC_RWW_RWR_SIZE );
					/* RX */
					memcpy( pSlave->pusRX, pSlave->pusSlaveRX, pSlave->pParameter->usOccupiedStationNumber * CCIEF_BASIC_RX_RY_SIZE );
					/* Initialize of the timeout count */
					pSlave->usTimeoutCount = 0;
					/* Set the slave state */
					pSlave->iState = CCIEF_BASIC_STATE_SLAVE_CYCLIC_END;
					
					/* Set the receive complete flag  */
					pSlave->iReceiveComplete = CCIEF_BASIC_SLAVE_RESPONSE_RECEIVED;
				}
				else
				{
					/* Nothing */
				}
			}
			else
			{
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			}
			break;
		/* Timeout the response of the cyclic data */
		case CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_TIMEOUT:
			/* Increase of the timeout count */
			pSlave->usTimeoutCount ++;
			/* Check of the timeout count */
			if ( pSlave->usTimeoutCount < pSlave->usCyclicTransmissionTimeoutCount )
			{
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_CYCLIC_END;
			}
			else
			{
				/* Set the slave state */
				pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			}
			break;
		/* Receiving the error of the duplication master from the slave */
		case CCIEF_BASIC_EVENT_SLAVE_DUPLICATION_MASTER_ERR:
			/* Set the slave state */
			pSlave->iState = CCIEF_BASIC_STATE_SLAVE_DISCONNECT;
			break;

		default:
			break;
	}

	return;
}
