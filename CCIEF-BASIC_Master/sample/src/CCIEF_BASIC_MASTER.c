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

#include "SLMP.h"
#include "CCIEF_BASIC_MASTER.h"
#include "CCIEF_BASIC_SLAVES.h"
#include "SOCKET.h"
#include "TIMER.h"

/************************************************************************************/
/* The following is an user defined main program. This main program is one of a		*/
/* sample in the Windows OS and Intel x86 CPU. Please rewrite if necessary.			*/
/* This main program is one of a sample in the Linux. Please rewrite if necessary.	*/
/*																					*/
/* This sample program for CCIEF-BASIC Slave Application.							*/
/*																					*/
/************************************************************************************/

#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#elif __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#endif
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#elif __linux__
#define INVALID_SOCKET		-1
#endif

/*[ Structure for Information of the CCIEF-BASIC Master ]*/
typedef struct 
{
	uint8_t								ucGroupNumber;								/* Group number */
	int									iTotalSlaveNumber;							/* Total number of the slaves */
	int									iOccupiedStationNumberTotal;				/* Total number of the occupied station number */
	int									iState;										/* State of Master */
	CCIEF_BASIC_GROUP_PARAMETER			*pGroupParameter;							/* Pointer of the group parameter */
	CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO	*pSlave[CCIEF_BASIC_MAX_SLAVE_NUMBER];		/* Pointer of information of the slaves */
	uint16_t							usFrameSequenceNumber;						/* Frame sequence number */
	int									iPersuasionTimerId;							/* Timer id for persuasion */
	int									iCyclicTimerId;								/* Timer id for cyclic time */
	uint8_t								aucCyclicData[CCIEF_BASIC_MAX_FRAME_SIZE];	/* Data for sending cyclic data */
	int									iCyclicDataSize;							/* Size of data for sending cyclic data */
	int									iConstantLinkScanUse;						/* Using the constant link scan */
	uint32_t							ulResponseWaitTime;							/* Time of waiting response[ms] */
	int64_t								llTimeData;									/* Time data of the sending latest cyclic data */
	int64_t								llLinkScanTimeStart;						/* Start time of link scan[us] */
	int64_t								llLinkScanTimeCurrent;						/* Current the link scan time[us] */
	int64_t								llLinkScanTimeMinimum;						/* Minimum the link scan time[us] */
	int64_t								llLinkScanTimeMaximum;						/* Maximum the link scan time[us] */
} CCIEF_BASIC_MASTER_GROUP_INFO;

#ifdef _WIN32
typedef struct 
{
	uint32_t						ulIpAddress;							/* Ip address */
	CCIEF_BASIC_MASTER_PARAMETER	Parameter;								/* Parameter */
	int								iGroupTotalNumber;						/* Total number of the groups */
	CCIEF_BASIC_MASTER_GROUP_INFO	*pGroup[CCIEF_BASIC_MAX_GROUP_NUMBER];	/* Pointer of information of the groups */
	uint32_t						ulId;									/* Id number */
	uint32_t						ulDirectedIpAddress;					/* Directed broadcast ip address */
	int								iOccupiedStationNumberTotal;			/* Total number of the occupied station number */
	uint16_t						usParameterId;							/* Parameter id */
	uint16_t						usUnitInfo;								/* Information of the unit */
	int								iErrCode;								/* Error code of the master */
} CCIEF_BASIC_MASTER_CYCLIC_DATA_INFO;
#elif __linux__
typedef struct 
{
	uint32_t						ulIpAddress;							/* Ip address */
	uint32_t						ulSubnetMask;							/* Slave subnet mask */
	CCIEF_BASIC_MASTER_PARAMETER	Parameter;								/* Parameter */
	int								iGroupTotalNumber;						/* Total number of the groups */
	CCIEF_BASIC_MASTER_GROUP_INFO	*pGroup[CCIEF_BASIC_MAX_GROUP_NUMBER];	/* Pointer of information of the groups */
	uint32_t						ulId;									/* Id number */
	uint32_t						ulDirectedIpAddress;					/* Directed broadcast ip address */
	int								iOccupiedStationNumberTotal;			/* Total number of the occupied station number */
	uint16_t						usParameterId;							/* Parameter id */
	uint16_t						usUnitInfo;								/* Information of the unit */
	int								iErrCode;								/* Error code of the master */
} CCIEF_BASIC_MASTER_CYCLIC_DATA_INFO;
#endif

/* Definition of external variable of sample program */
#ifdef _WIN32
static SOCKET sock;		/* sokect of CCIEF-BASIC Master */
#elif __linux__
static int sock;		/* sokect of CCIEF-BASIC Master */
#endif

static unsigned char	aucSendData[CCIEF_BASIC_MAX_FRAME_SIZE];	/* Data for sending packet */
static unsigned char	aucRecvData[CCIEF_BASIC_MAX_FRAME_SIZE];	/* Data for receiving packet */
static unsigned char	aucSendPacket[CCIEF_BASIC_MAX_FRAME_SIZE];	/* Buffer for sending packet */
static unsigned char	aucRecvPacket[CCIEF_BASIC_MAX_FRAME_SIZE];	/* Buffer for receiving packet */

static unsigned short	ausRX[(CCIEF_BASIC_RX_RY_SIZE / sizeof( uint16_t )) * CCIEF_BASIC_MAX_SLAVE_NUMBER];	/* RX for the master */
static unsigned short	ausRY[(CCIEF_BASIC_RX_RY_SIZE / sizeof( uint16_t )) * CCIEF_BASIC_MAX_SLAVE_NUMBER];	/* RY for the master */
static unsigned short	ausRWw[(CCIEF_BASIC_RWW_RWR_SIZE / sizeof( uint16_t )) * CCIEF_BASIC_MAX_SLAVE_NUMBER];	/* RWw for the master */
static unsigned short	ausRWr[(CCIEF_BASIC_RWW_RWR_SIZE / sizeof( uint16_t )) * CCIEF_BASIC_MAX_SLAVE_NUMBER];	/* RWr for the master */

static CCIEF_BASIC_MASTER_CALLBACK_CYCLIC_LINK_SCAN_END	pUserLinkScanEndFunc = NULL;	/* Callback function for end of the link scan */

static CCIEF_BASIC_MASTER_CYCLIC_DATA_INFO	Master;									/* Information of the master */
static CCIEF_BASIC_MASTER_GROUP_INFO		Group[CCIEF_BASIC_MAX_GROUP_NUMBER];	/* Information of the groups */
static CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO	Slave[CCIEF_BASIC_MAX_SLAVE_NUMBER];	/* Information of the slaves cyclic data */

/* Definition of function of sample program */
static int ccief_basic_master_check_parameter( CCIEF_BASIC_MASTER_PARAMETER *pParameter );
static int ccief_basic_master_recv( void );
static int ccief_basic_master_polling( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup );
static void ccief_basic_master_execute_state( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent );
static void ccief_basic_master_execute_state_wait_cyclic( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent );
static void ccief_basic_master_execute_state_persuasion( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent );
static void ccief_basic_master_execute_state_linkscan_end( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent );
static void ccief_basic_master_execute_state_linkscan( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent );
static void ccief_basic_master_persuasion_timer_timeout( int iId, void *pCallbackArg );
static void ccief_basic_master_cyclic_timer_timeout( int iId, void *pCallbackArg );
static int ccief_basic_master_make_cyclic_data( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup );
static int ccief_basic_master_send_cyclic_data( uint32_t ulIpAddress, uint8_t *pucData, int iDataSize );
static void ccief_basic_master_recv_cyclic_data_response( uint8_t *pucData );

/************************************************************************************/
/* This is an user defined function for initialization of CCIEF-BASIC Master.			*/
/************************************************************************************/
int ccief_basic_master_initialize( uint32_t ulIpAddress, uint32_t ulSubnetMask, CCIEF_BASIC_MASTER_PARAMETER *pParameter,
								 CCIEF_BASIC_MASTER_CALLBACK_CYCLIC_LINK_SCAN_END pCyclicLinkScanEndFunc )
{
	CCIEF_BASIC_MASTER_GROUP_INFO		*pGroup;
	CCIEF_BASIC_SLAVES_CYCLIC_DATA_INFO *pSlave;
	uint8_t ucGroupNumber;
	int i, j, iGroupCheck;
	int iErrCode;

	/* Initialize of the master */
	memset( &Master, 0, sizeof( Master ) );
	memset( &Group, 0, sizeof( Group ) );
	pGroup = (CCIEF_BASIC_MASTER_GROUP_INFO*)NULL;

	/* Getting the parameter */
	memcpy( &Master.Parameter, pParameter, sizeof( Master.Parameter ) );

	/* User callback function */
	pUserLinkScanEndFunc = pCyclicLinkScanEndFunc;

	/* Setting the master information */
	Master.ulIpAddress = ulIpAddress;
	Master.ulId = ulIpAddress;
#ifdef _WIN32
#elif __linux__
	Master.ulSubnetMask = ulSubnetMask;
#endif
	Master.ulDirectedIpAddress = ((ulIpAddress & ulSubnetMask) | ~ulSubnetMask);

	/* Check the Parameter */
	iErrCode = ccief_basic_master_check_parameter( pParameter );
	if ( iErrCode != CCIEF_BASIC_MASTER_ERR_OK )
	{
		return iErrCode;
	}

	for ( i = 0; i < pParameter->iTotalSlaveNumber; i ++ )
	{
		ucGroupNumber = pParameter->Slave[i].ucGroupNumber;
		iGroupCheck = CCIEF_BASIC_MASTER_ERR_NG;
		for ( j = 0; j < Master.iGroupTotalNumber; j ++ )
		{
			if ( ucGroupNumber == Master.pGroup[j]->ucGroupNumber )
			{
				pGroup = Master.pGroup[j];
				iGroupCheck = CCIEF_BASIC_MASTER_ERR_OK;
				break;
			}
		}
		if ( iGroupCheck == CCIEF_BASIC_MASTER_ERR_NG )
		{
			pGroup = &Group[Master.iGroupTotalNumber];
			pGroup->ucGroupNumber = ucGroupNumber;
			for ( j = 0; j < pParameter->iTotalGroupNumber; j ++ )
			{
				if ( ucGroupNumber == pParameter->Group[j].ucGroupNumber )
				{
					break;
				}
			}
			pGroup->pGroupParameter = &pParameter->Group[j];
			Master.pGroup[Master.iGroupTotalNumber] = pGroup;
			Master.iGroupTotalNumber ++;
		}
		pSlave = &Slave[i];
		pSlave->iNumber = i;
		pSlave->pParameter = &Master.Parameter.Slave[i];
		pSlave->ulId = pParameter->Slave[i].ulIpAddress;
		if ( pGroup->pGroupParameter->usCyclicTransmissionTimeoutCount == 0 )
		{
			pSlave->usCyclicTransmissionTimeoutCount = CCIEF_BASIC_CYCLIC_TRANSMISSION_TIMEOUT_COUNT;
		}
		else
		{
			pSlave->usCyclicTransmissionTimeoutCount = pGroup->pGroupParameter->usCyclicTransmissionTimeoutCount;
		}
		pSlave->iStationNumber = Master.iOccupiedStationNumberTotal;
		pSlave->iGroupStationNumber = pGroup->iOccupiedStationNumberTotal;
		pSlave->iReceiveComplete = CCIEF_BASIC_SLAVE_RESPONSE_NOT_RECEIVED;
		pSlave->iDuplicateState = CCIEF_BASIC_SLAVE_DUPLICATION_NOT_DETECT;
		pSlave->pusRWw = &ausRWw[pSlave->iStationNumber * ( CCIEF_BASIC_RWW_RWR_SIZE / sizeof( uint16_t ) )];
		pSlave->pusRY = &ausRY[pSlave->iStationNumber * ( CCIEF_BASIC_RX_RY_SIZE / sizeof( uint16_t ) )];
		pSlave->pusRWr = &ausRWr[pSlave->iStationNumber * ( CCIEF_BASIC_RWW_RWR_SIZE / sizeof( uint16_t ) )];
		pSlave->pusRX = &ausRX[pSlave->iStationNumber * ( CCIEF_BASIC_RX_RY_SIZE / sizeof( uint16_t ) )];
		pSlave->pusFrameSequenceNumber = &pGroup->usFrameSequenceNumber;
		pGroup->pSlave[pGroup->iTotalSlaveNumber] = pSlave;
		pGroup->iTotalSlaveNumber ++;
		pGroup->iOccupiedStationNumberTotal += pParameter->Slave[i].usOccupiedStationNumber;
		Master.iOccupiedStationNumberTotal += pParameter->Slave[i].usOccupiedStationNumber;
	}

	/* Calucurate of the parameter id */
	srand( (unsigned)time( NULL ) );
	Master.usParameterId = (uint16_t)rand();

	/* Initialize a socket */
	sock = INVALID_SOCKET;

	/* Initialize of the slaves */
	for ( i = 0; i < Master.Parameter.iTotalSlaveNumber; i ++ )
	{
		ccief_basic_slaves_initialize( &Slave[i] );
	}

	for ( i = 0; i < Master.iGroupTotalNumber; i ++ )
	{
		/* Set the master state */
		Master.pGroup[i]->iState = CCIEF_BASIC_STATE_MASTER_WAITING;

		/* Execute the state of the master */
		ccief_basic_master_execute_state( Master.pGroup[i], CCIEF_BASIC_EVENT_MASTER_PARAMETER_GET );
	}

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for termination of CCIEF-BASIC Master.			*/
/************************************************************************************/
void ccief_basic_master_terminate( void )
{
	/* Termination of socket */
	socket_terminate( sock );

	return;
}

/************************************************************************************/
/* This is an user defined function for main function of CCIEF-BASIC Master.			*/
/************************************************************************************/
int ccief_basic_master_main( void )
{
	int i, iErrCode = 0;

	/* Receive data */
	iErrCode = ccief_basic_master_recv();
	if ( iErrCode != CCIEF_BASIC_MASTER_ERR_OK )
	{
		return iErrCode;
	}

	/* Execute the state of the master */
	for ( i = 0; i < Master.iGroupTotalNumber; i ++ )
	{
		(void)ccief_basic_master_polling( Master.pGroup[i] );
	}

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for set of the start cyclic of the slave.		*/
/************************************************************************************/
int ccief_basic_master_start_cyclic( int iSlaveNumber )
{
	if (( iSlaveNumber < 0 ) || ( Master.Parameter.iTotalSlaveNumber <= iSlaveNumber ))
	{
		return CCIEF_BASIC_MASTER_ERR_NG;
	}

	/* Start cyclic from user operation. */
	Slave[iSlaveNumber].iCyclicStart = CCIEF_BASIC_CYCLIC_START;

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for set of the stop cyclic of the slave.		*/
/************************************************************************************/
int ccief_basic_master_stop_cyclic( int iSlaveNumber )
{
	if (( iSlaveNumber < 0 ) || ( Master.Parameter.iTotalSlaveNumber <= iSlaveNumber ))
	{
		return CCIEF_BASIC_MASTER_ERR_NG;
	}

	/* Stop cyclic from user operation. */
	Slave[iSlaveNumber].iCyclicStart = CCIEF_BASIC_CYCLIC_STOP;

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for getting the RX.								*/
/************************************************************************************/
int ccief_basic_master_get_rx( int iNumber, int *piValue )
{
	unsigned short usTemp;

	/* Check the number of device */
	if (( iNumber < 0 ) || ( sizeof( ausRX ) * 8 ) <= iNumber )
	{
		return CCIEF_BASIC_MASTER_ERR_DEVICE_RANGE;
	}

	usTemp = ausRX[iNumber / 16];
	if (( usTemp & ( 1 << ( iNumber % 16 ))) == CCIEF_BASIC_BIT_OFF )
	{
		*piValue = CCIEF_BASIC_BIT_OFF;
	}
	else
	{
		*piValue = CCIEF_BASIC_BIT_ON;
	}
	
	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for setting the RY.								*/
/************************************************************************************/
int ccief_basic_master_set_ry( int iNumber, int iValue )
{
	unsigned short usTemp;

	/* Check the number of device */
	if (( iNumber < 0 ) || ( sizeof( ausRY ) * 8 ) <= iNumber )
	{
		return CCIEF_BASIC_MASTER_ERR_DEVICE_RANGE;
	}

	/* Check the value */
	if (( iValue == CCIEF_BASIC_BIT_OFF ) || ( iValue == CCIEF_BASIC_BIT_ON ))
	{
		usTemp = ausRY[( iNumber / 16 )];
		usTemp = (uint16_t)(( usTemp & ~( 1 << ( iNumber % 16 ))) | ( iValue << ( iNumber % 16 )));
		ausRY[( iNumber / 16 )] = usTemp;
	}
	else
	{
		return CCIEF_BASIC_MASTER_ERR_NG;
	}

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for setting the RWw.							*/
/************************************************************************************/
int ccief_basic_master_set_rww( int iNumber, uint16_t usValue )
{
	int iMaxNumber;

	/* Check the number of device */
	iMaxNumber = sizeof( ausRWw ) / sizeof( uint16_t );
	if (( iNumber < 0 ) || (iMaxNumber <= iNumber ))
	{
		return CCIEF_BASIC_MASTER_ERR_DEVICE_RANGE;
	}

	ausRWw[iNumber] = usValue;
	
	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for getting the RWr.							*/
/************************************************************************************/
int ccief_basic_master_get_rwr( int iNumber, uint16_t *pusValue )
{
	int iMaxNumber;

	/* Check the number of device */
	iMaxNumber = sizeof( ausRWr ) / sizeof( uint16_t );
	if (( iNumber < 0 ) || (iMaxNumber <= iNumber ))
	{
		return CCIEF_BASIC_MASTER_ERR_DEVICE_RANGE;
	}

	*pusValue = ausRWr[iNumber];
	
	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for getting the device start pointer.			*/
/************************************************************************************/
uint16_t *ccief_basic_master_get_pointer( int iDeviceType )
{
	uint16_t *pusStartPointer;

	/* Getting the device start pointer */

	/* Check the device type */
	switch ( iDeviceType )
	{
		/* RX */
		case CCIEF_BASIC_DEVICE_TYPE_RX:
			pusStartPointer = ausRX;
			break;
		/* RY */
		case CCIEF_BASIC_DEVICE_TYPE_RY:
			pusStartPointer = ausRY;
			break;
		/* RWw */
		case CCIEF_BASIC_DEVICE_TYPE_RWW:
			pusStartPointer = ausRWw;
			break;
		/* RWr */
		case CCIEF_BASIC_DEVICE_TYPE_RWR:
			pusStartPointer = ausRWr;
			break;
		/* Error of argument */
		default:
			pusStartPointer = NULL;
			break;
	}
	
	return pusStartPointer;
}

/************************************************************************************/
/* This is an user defined function for setting the unit info.						*/
/************************************************************************************/
void ccief_basic_master_set_unit_info( uint16_t usUnitInfo )
{
	Master.usUnitInfo = usUnitInfo;

	return;
}

/************************************************************************************/
/* This is an user defined function for getting information of the slave.			*/
/************************************************************************************/
int ccief_basic_master_get_slave_info( int iSlaveNumber, CCIEF_BASIC_SLAVE_INFO *pSlaveInfo )
{

	if (( iSlaveNumber < 0 ) || ( Master.Parameter.iTotalSlaveNumber <= iSlaveNumber ))
	{
		return CCIEF_BASIC_MASTER_ERR_NG;
	}

	if ( pSlaveInfo == NULL )
	{
		return CCIEF_BASIC_MASTER_ERR_NG;
	}

	/* Getting information of the slave */
	pSlaveInfo->usProtocolVersion = Slave[iSlaveNumber].usProtocolVersion;
	pSlaveInfo->usEndCode = Slave[iSlaveNumber].usEndCode;
	pSlaveInfo->ucGroupNumber = Slave[iSlaveNumber].pParameter->ucGroupNumber;
	pSlaveInfo->ulId = Slave[iSlaveNumber].ulId;
	pSlaveInfo->usFrameSequenceNumber = Slave[iSlaveNumber].usFrameSequenceNumber;
	pSlaveInfo->usOccupiedStationNumber = Slave[iSlaveNumber].pParameter->usOccupiedStationNumber;
	pSlaveInfo->iState = Slave[iSlaveNumber].iState;
	memcpy( &pSlaveInfo->NotifyInfo, &Slave[iSlaveNumber].NotifyInfo, sizeof( pSlaveInfo->NotifyInfo ) );
	pSlaveInfo->iCyclicState = Slave[iSlaveNumber].iCyclicState;
	pSlaveInfo->iStationNumber = Slave[iSlaveNumber].iStationNumber;

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for getting information of the master.			*/
/************************************************************************************/
int ccief_basic_master_get_group_info( int iGroupNumber, CCIEF_BASIC_GROUP_INFO *pGroupInfo )
{

	if ( pGroupInfo == NULL )
	{
		return CCIEF_BASIC_MASTER_ERR_NG;
	}

	/* Getting information of the master */
	pGroupInfo->usProtocolVersion = CCIEF_BASIC_PROTCOL_VERSION;
	pGroupInfo->ulId = Master.ulId;
	pGroupInfo->ucGroupNumber = Group[iGroupNumber].ucGroupNumber;
	pGroupInfo->iTotalSlaveNumber = Group[iGroupNumber].iTotalSlaveNumber;
	pGroupInfo->usTotalOccupiedStationNumber = Group[iGroupNumber].iOccupiedStationNumberTotal;
	pGroupInfo->iState = Group[iGroupNumber].iState;
	pGroupInfo->usUnitInfo = Master.usUnitInfo;
	pGroupInfo->usFrameSequenceNumber = Group[iGroupNumber].usFrameSequenceNumber;
	pGroupInfo->usParameterId = Master.usParameterId;
	pGroupInfo->llTimeData = Group[iGroupNumber].llTimeData;
	pGroupInfo->llLinkScanTimeCurrent = Group[iGroupNumber].llLinkScanTimeCurrent;
	pGroupInfo->llLinkScanTimeMinimum = Group[iGroupNumber].llLinkScanTimeMinimum;
	pGroupInfo->llLinkScanTimeMaximum = Group[iGroupNumber].llLinkScanTimeMaximum;

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for check the parameter of CCIEF-BASIC Master.	*/
/************************************************************************************/
int ccief_basic_master_check_parameter( CCIEF_BASIC_MASTER_PARAMETER *pParameter )
{
	int	i, j, iOccupiedStationNumberTotal;
	struct in_addr addr;
	char Ipaddr[16];

	/* Check the master ip address */
	for ( i = 0; i < pParameter->iTotalSlaveNumber; i ++ )
	{
		if ( Master.ulIpAddress == pParameter->Slave[i].ulIpAddress )
		{
			printf( "Parameter Error: Duplicate master ip address in slaves!\n" );
			return CCIEF_BASIC_MASTER_ERR_NG;
		}
	}

	/* Check the total number of the slaves */
	if (( pParameter->iTotalSlaveNumber == 0 ) || ( CCIEF_BASIC_MAX_SLAVE_NUMBER < pParameter->iTotalSlaveNumber )) {
		printf( "Parameter Error: Out of range of total slave number! %d (range: 1-64)\n", pParameter->iTotalSlaveNumber );
		return CCIEF_BASIC_MASTER_ERR_NG;
	}
	
	/* Check the total number of occupied stations */
	iOccupiedStationNumberTotal = 0;
	for ( i = 0; i < pParameter->iTotalSlaveNumber; i ++ )
	{
		if (( pParameter->Slave[i].usOccupiedStationNumber < 1 ) || ( CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP < pParameter->Slave[i].usOccupiedStationNumber )) {
			printf( "Parameter Error: Invalid number of occupied stations! %d(No.%d) (range: 1-16)\n", pParameter->Slave[i].usOccupiedStationNumber, i+1 );
			return CCIEF_BASIC_MASTER_ERR_NG;
		}
		iOccupiedStationNumberTotal += pParameter->Slave[i].usOccupiedStationNumber;

	}
	if ( CCIEF_BASIC_MAX_SLAVE_NUMBER < iOccupiedStationNumberTotal )
	{
		printf( "Parameter Error: Invalid total number of occupied stations! %d (range: 1-64)\n", iOccupiedStationNumberTotal );
		return CCIEF_BASIC_MASTER_ERR_NG;
	}

	/* Check the total number of occupied stations(for group) */
	for ( i = 0; i < CCIEF_BASIC_MAX_GROUP_NUMBER; i ++ )
	{
		iOccupiedStationNumberTotal = 0;
		for ( j = 0; j < pParameter->iTotalSlaveNumber; j ++ )
		{
			if (( i + 1 ) == pParameter->Slave[j].ucGroupNumber )
			{
				iOccupiedStationNumberTotal += pParameter->Slave[j].usOccupiedStationNumber;
			}
		}
		if ( CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP < iOccupiedStationNumberTotal )
		{
			printf( "Parameter Error: Invalid total number of occupied stations! (Group Number %d)\n", ( i + 1 ));
			return CCIEF_BASIC_MASTER_ERR_NG;
		}
	}

	/* Check the slave ip address */
	for ( i = 0; i < pParameter->iTotalSlaveNumber; i ++ )
	{
		for ( j = 0; j < pParameter->iTotalSlaveNumber; j ++ )
		{
			if (( i != j ) && ( pParameter->Slave[i].ulIpAddress == pParameter->Slave[j].ulIpAddress ))
			{
#ifdef _WIN32
				addr.S_un.S_addr = htonl(pParameter->Slave[j].ulIpAddress);
#elif __linux__
				addr.s_addr = htonl(pParameter->Slave[j].ulIpAddress);	
#endif
				inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
				printf( "Parameter Error: Duplicate slaves ip address %s!\n", Ipaddr );
				return CCIEF_BASIC_MASTER_ERR_NG;
			}
		}
	}

	/* Check the total number of the group */
	if (( pParameter->iTotalGroupNumber == 0 ) || ( CCIEF_BASIC_MAX_GROUP_NUMBER < pParameter->iTotalGroupNumber )) {
		printf( "Parameter Error: Out of range of total group number! %d (range: 1-64)\n", pParameter->iTotalGroupNumber );
		return CCIEF_BASIC_MASTER_ERR_NG;
	}

	/* Check the group number */
	for ( i = 0; i < pParameter->iTotalSlaveNumber; i ++ )
	{
		if (( pParameter->Slave[i].ucGroupNumber == 0) || ( CCIEF_BASIC_MAX_GROUP_NUMBER < pParameter->Slave[i].ucGroupNumber ))
		{
			printf( "Parameter Error: Invalid group number at the slave No.%d! (range: 1-64)\n", ( i + 1 ));
			return CCIEF_BASIC_MASTER_ERR_NG;
		}
	}
	for ( i = 0; i < pParameter->iTotalSlaveNumber; i ++ )
	{
		for ( j = 0; j < pParameter->iTotalGroupNumber; j ++ )
		{
			if ( pParameter->Slave[i].ucGroupNumber == pParameter->Group[j].ucGroupNumber )
			{
				break;
			}
		}
		if ( j == pParameter->iTotalGroupNumber )
		{
			printf( "Parameter Error: Not found the parameter of group number %d!\n", pParameter->Slave[i].ucGroupNumber );
			return CCIEF_BASIC_MASTER_ERR_NG;
		}
	}

	/* Check the constant link scan time */
	for ( i = 0; i < pParameter->iTotalGroupNumber; i ++ )
	{
		if ( CCIEF_BASIC_MAX_CONSTANT_LINK_SCAN_TIME < pParameter->Group[i].usConstantLinkScanTime )
		{
			printf( "Parameter Error: Invalid constant link scan time! %d (range: 0-2000)\n", pParameter->Group[i].usConstantLinkScanTime );
			printf( "(0:no use)\n" );
			return CCIEF_BASIC_MASTER_ERR_NG;
		}
	}

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for receiving packet. The following is one of a	*/
/* sample in the Windows OS. Please rewrite if necessary.							*/
/* The following is one of a sample in the Linux. Please rewrite if necessary.		*/
/************************************************************************************/
int ccief_basic_master_recv( void )
{
	uint32_t		ulRecvAddr;
	uint16_t		usRecvPortNumber;
	struct in_addr	addr;
	SLMP_INFO		source = { 0 };	/* SLMP Infomation for received packet */
	int				iErrCode = 0;
	int				i;
	char			Ipaddr[16];
#ifdef _WIN32
#elif __linux__
	uint32_t		ulMyNetAddress;
	uint32_t		ulOtherNetAddress;
#endif

	/* Check the socket */
	if ( sock == INVALID_SOCKET ) {
		return CCIEF_BASIC_MASTER_ERR_OK;
	}

	/* Packet receiving */
	iErrCode = socket_recv( sock, aucRecvPacket, sizeof( aucRecvPacket ), &ulRecvAddr, &usRecvPortNumber );
	if ( iErrCode != SOCKET_ERR_OK )
	{
		if ( iErrCode == SOCKET_ERR_NO_RECEIVABLE )
		{
			return CCIEF_BASIC_MASTER_ERR_OK;
		}
		else
		{
			return iErrCode;
		}
	}

#ifdef _WIN32
#elif __linux__
	ulMyNetAddress = ( Master.ulIpAddress & Master.ulSubnetMask );
	ulOtherNetAddress = ( ulRecvAddr & Master.ulSubnetMask );

	/* Other network broadcast break*/
	if( ulMyNetAddress != ulOtherNetAddress )
	{
		return CCIEF_BASIC_MASTER_ERR_OK;
	}
#endif


	/* Sets the SLMP Information for receiving. */
	source.pucData = aucRecvData;

	/* Get the SLMP Information from the request packet using the SLMP library. */
	iErrCode = SLMP_GetSlmpInfo( &source, aucRecvPacket );
	if ( iErrCode != SLMP_ERR_OK )
	{
		/* Invalid SLMP Frame. */
		printf( "ERR : Invalid SLMP frame received!\n" );
		return CCIEF_BASIC_MASTER_ERR_OK;
	}

	/* Check the SLMP frame. */
	if ( source.ulFrameType == SLMP_FTYPE_BIN_RES_ST )
	{
		/* Response of the SLMP frame */
		if ( source.usEndCode == SLMP_ERR_OK )
		{
			/* Receiving response of the cyclic data. */
			ccief_basic_master_recv_cyclic_data_response( source.pucData );
		}
		else
		{
#ifdef _WIN32
			addr.S_un.S_addr = htonl(ulRecvAddr);
#elif __linux__
			addr.s_addr = htonl(ulRecvAddr);	
#endif
			inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
			printf( "ERR : EndCode %04X from %s!\n", source.usEndCode, Ipaddr );
		}
	}
	else
	{
		/* Request of the SLMP frame */
		if ( source.usCommand == SLMP_COMMAND_CYCLIC_DATA )
		{
			/* Cyclic Data(0E70h) */
			for ( i = 0; i < Master.iGroupTotalNumber; i ++ )
			{
				/* Execute the state of the master */
				ccief_basic_master_execute_state( Master.pGroup[i], CCIEF_BASIC_EVENT_MASTER_CYCLIC_DATA_RECV );
			}
		}
	}

	/* Check the error code of the master duplication */
	if ( Master.iErrCode == CCIEF_BASIC_MASTER_ERR_MASTER_DUPLICATION )
	{
		return CCIEF_BASIC_MASTER_ERR_MASTER_DUPLICATION;
	}
	
	/* Check the error code of the slave duplication */
	if ( Master.iErrCode == CCIEF_BASIC_MASTER_ERR_SLAVE_DUPLICATION )
	{
		return CCIEF_BASIC_MASTER_ERR_SLAVE_DUPLICATION;
	}

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for polling operation.							*/
/************************************************************************************/
int ccief_basic_master_polling( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup )
{
	int	i;
	int iSlaveCyclicStateOffNumber, iSlaveCyclicStateOffCyclicEndNumber;
	int iSlaveCyclicStateOnNumber, iSlaveCyclicStateOnCyclicEndNumber;
	int iErrCode = CCIEF_BASIC_MASTER_ERR_OK;

	/* Check the state of the master */
	if ( pGroup->iState == CCIEF_BASIC_STATE_MASTER_LINK_SCAN )
	{
		/* Check using the constant link scan */
		if ( pGroup->iConstantLinkScanUse == CCIEF_BASIC_CONSTANT_LINK_SCAN_NOT_USE )
		{
			/* Check the cyclic end of the slave */
			iSlaveCyclicStateOffNumber = 0;
			iSlaveCyclicStateOffCyclicEndNumber = 0;
			iSlaveCyclicStateOnNumber = 0;
			iSlaveCyclicStateOnCyclicEndNumber = 0;
			for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
			{
				if ( pGroup->pSlave[i]->iCyclicState == CCIEF_BASIC_CYCLIC_STATE_OFF )
				{
					/* OFF the slave cyclic state */
					iSlaveCyclicStateOffNumber ++;
					if ( pGroup->pSlave[i]->iState == CCIEF_BASIC_STATE_SLAVE_CYCLIC_END )
					{
						iSlaveCyclicStateOffCyclicEndNumber ++;
					}
				}
				else
				{
					/* ON the slave cyclic state */
					iSlaveCyclicStateOnNumber ++;
					if ( pGroup->pSlave[i]->iState == CCIEF_BASIC_STATE_SLAVE_CYCLIC_END )
					{
						iSlaveCyclicStateOnCyclicEndNumber ++;
					}
				}
			}
			if ((( 0 < iSlaveCyclicStateOnNumber ) && ( iSlaveCyclicStateOnNumber == iSlaveCyclicStateOnCyclicEndNumber ))
			 || (( 0 == iSlaveCyclicStateOnNumber ) && ( iSlaveCyclicStateOffNumber == iSlaveCyclicStateOffCyclicEndNumber )))
			{
				/* End of the cyclic */
				/* Execute the state of the slave */
				for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
				{
					ccief_basic_slaves_execute_state( pGroup->pSlave[i], CCIEF_BASIC_EVENT_SLAVE_CYCLIC_END );
				}
				/* Execute the state of the master */
				ccief_basic_master_execute_state( pGroup, CCIEF_BASIC_EVENT_MASTER_LINK_SCAN_END );
			}
		}
	}

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for executed according to the state				*/
/* of the master.																	*/
/************************************************************************************/
void ccief_basic_master_execute_state( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent )
{
	/* Execute the state of the master */
	switch ( pGroup->iState )
	{
		/* Waiting for the cyclic operation. */
		case CCIEF_BASIC_STATE_MASTER_WAITING:
			ccief_basic_master_execute_state_wait_cyclic( pGroup, iEvent );
			break;
		/* Persuasion of the master. */
		case CCIEF_BASIC_STATE_MASTER_PERSUASION:
			ccief_basic_master_execute_state_persuasion( pGroup, iEvent );
			break;
		/* End of the link scan. */
		case CCIEF_BASIC_STATE_MASTER_LINK_SCAN_END:
			ccief_basic_master_execute_state_linkscan_end( pGroup, iEvent );
			break;
		/* Running the link scan. */
		case CCIEF_BASIC_STATE_MASTER_LINK_SCAN:
			ccief_basic_master_execute_state_linkscan( pGroup, iEvent );
			break;
		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of waiting of the cyclic operation.	*/
/************************************************************************************/
void ccief_basic_master_execute_state_wait_cyclic( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent )
{
	/* Check the event */
	switch ( iEvent )
	{
		/* Getting the parameter */
		case CCIEF_BASIC_EVENT_MASTER_PARAMETER_GET:
			/* Initialization of the socket */
			if ( sock == INVALID_SOCKET )
			{
				Master.iErrCode = socket_initialize( &sock, Master.ulIpAddress, CCIEF_BASIC_PORT_NUMBER_CYCLIC );
				if( Master.iErrCode != CCIEF_BASIC_MASTER_ERR_OK )
				{
					return;
				}
			}
			/* Check constant link scan */
			pGroup->iConstantLinkScanUse = CCIEF_BASIC_CONSTANT_LINK_SCAN_NOT_USE;
			if ( pGroup->pGroupParameter->usConstantLinkScanTime != 0 )
			{
				/* Set waiting of the end of constant link scan */
				pGroup->iConstantLinkScanUse = CCIEF_BASIC_CONSTANT_LINK_SCAN_USE;
			}
			/* Start the master persuasion timer*/
			timer_start( CCIEF_BASIC_MASTER_PERSUASION_TIME, &pGroup->iPersuasionTimerId,
						 &ccief_basic_master_persuasion_timer_timeout, (void *)pGroup );
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_PERSUASION;
			break;
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_MASTER_PARAMETER_CHANGE:
			/* Nothing */
			break;
		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of persuasion of the master.			*/
/************************************************************************************/
void ccief_basic_master_execute_state_persuasion( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent )
{
	int	i;

	/* Check the event */
	switch ( iEvent )
	{
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_MASTER_PARAMETER_CHANGE:
			/* Execute the state of the slave */
			for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
			{
				ccief_basic_slaves_execute_state( pGroup->pSlave[i], CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE );
			}
			/* Stop the master persuasion timer*/
			timer_stop( pGroup->iPersuasionTimerId );
			/* Termination of socket */
			socket_terminate( sock );
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_WAITING;
			break;
		/* Timeout of monitoring the persuasion frame */
		case CCIEF_BASIC_EVENT_MASTER_PERSUASION_TIMEOUT:
			/* Check the constant link scan time */
			if ( pGroup->pGroupParameter->usConstantLinkScanTime != 0 )
			{
				/* Set the constant link scan time */
				pGroup->ulResponseWaitTime = pGroup->pGroupParameter->usConstantLinkScanTime;
			}
			else
			{
				/* Set the cyclic transmission timeout */
				pGroup->ulResponseWaitTime = pGroup->pGroupParameter->usCyclicTransmissionTimeout;
			}
			if ( pGroup->ulResponseWaitTime == 0 )
			{
				pGroup->ulResponseWaitTime = CCIEF_BASIC_CYCLIC_TRANSMISSION_TIMEOUT;
			}
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_LINK_SCAN_END;
			/* Initialization of the frame sequence number */
			pGroup->usFrameSequenceNumber = 0;
			/* Execute the state of the master */
			ccief_basic_master_execute_state( pGroup, CCIEF_BASIC_EVENT_MASTER_LINK_SCAN_START );
			break;
		/* Receiving the cyclic data from the master */
		case CCIEF_BASIC_EVENT_MASTER_CYCLIC_DATA_RECV:
			/* Stop the master persuasion timer*/
			timer_stop( pGroup->iPersuasionTimerId );
			/* Termination of socket */
			socket_terminate( sock );
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_WAITING;
			/* Set the error code of the master duplication */
			Master.iErrCode = CCIEF_BASIC_MASTER_ERR_MASTER_DUPLICATION;
			return;
		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of end of the link scan.				*/
/************************************************************************************/
void ccief_basic_master_execute_state_linkscan_end( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent )
{
	int	i;

	/* Check the event */
	switch ( iEvent )
	{
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_MASTER_PARAMETER_CHANGE:
			/* Execute the state of the slave */
			for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
			{
				ccief_basic_slaves_execute_state( pGroup->pSlave[i], CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE );
			}
			/* Termination of socket */
			socket_terminate( sock );
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_WAITING;
			break;
		/* Starting the link scan */
		case CCIEF_BASIC_EVENT_MASTER_LINK_SCAN_START:
			/* Execute the state of the slave */
			for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
			{
				/* Reset the receive complete flag */
				pGroup->pSlave[i]->iReceiveComplete = CCIEF_BASIC_SLAVE_RESPONSE_NOT_RECEIVED;
				
				ccief_basic_slaves_execute_state( pGroup->pSlave[i], CCIEF_BASIC_EVENT_SLAVE_LINK_SCAN_START );
			}
			/* Callback function for notification of end of the link scan to user operation */
			if ( pUserLinkScanEndFunc != NULL )
			{
				pUserLinkScanEndFunc( pGroup->ucGroupNumber );
			}
			/* Make the request data of Cyclic Data */
			pGroup->iCyclicDataSize = ccief_basic_master_make_cyclic_data( pGroup );
			/* Start the link scan time */
			pGroup->llLinkScanTimeStart = timer_get_time();
			/* Sending Cyclic Data */
			ccief_basic_master_send_cyclic_data( Master.ulDirectedIpAddress, pGroup->aucCyclicData, pGroup->iCyclicDataSize );
			/* Start the timer for cyclic time */
			timer_start( pGroup->ulResponseWaitTime, &pGroup->iCyclicTimerId, &ccief_basic_master_cyclic_timer_timeout, (void *)pGroup );
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_LINK_SCAN;
			break;
		/* Receiving the cyclic data from the master */
		case CCIEF_BASIC_EVENT_MASTER_CYCLIC_DATA_RECV:
			/* Nothing */
			break;
		/* Receiving the error of the duplication master from the slave */
		case CCIEF_BASIC_EVENT_MASTER_DUPLICATION_ERR_RECV:
			/* Execute the state of the slave */
			for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
			{
				ccief_basic_slaves_execute_state( pGroup->pSlave[i], CCIEF_BASIC_EVENT_SLAVE_DUPLICATION_MASTER_ERR );
			}
			/* Termination of socket */
			socket_terminate( sock );
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_WAITING;
			/* Set the error code of the master duplication */
			Master.iErrCode = CCIEF_BASIC_MASTER_ERR_MASTER_DUPLICATION;
			return;
		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for state of running the link scan.				*/
/************************************************************************************/
void ccief_basic_master_execute_state_linkscan( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup, int iEvent )
{
	int	i;
	int64_t	llLinkScanTimeEnd;

	/* Check the event */
	switch ( iEvent )
	{
		/* Changing the parameter */
		case CCIEF_BASIC_EVENT_MASTER_PARAMETER_CHANGE:
			/* Execute the state of the slave */
			for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
			{
				ccief_basic_slaves_execute_state( pGroup->pSlave[i], CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE );
			}
			/* Stop the cyclic timer*/
			timer_stop( pGroup->iCyclicTimerId );
			/* Termination of socket */
			socket_terminate( sock );
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_WAITING;
			break;
		/* End of the link scan */
		case CCIEF_BASIC_EVENT_MASTER_LINK_SCAN_END:
			/* Stop the cyclic timer*/
			timer_stop( pGroup->iCyclicTimerId );
			/* Stop the link scan time */
			llLinkScanTimeEnd = timer_get_time();
			pGroup->llLinkScanTimeCurrent = llLinkScanTimeEnd - pGroup->llLinkScanTimeStart;
			if (( pGroup->llLinkScanTimeMinimum == 0 ) || ( pGroup->llLinkScanTimeCurrent < pGroup->llLinkScanTimeMinimum ))
			{
				pGroup->llLinkScanTimeMinimum = pGroup->llLinkScanTimeCurrent;
			}
			if ( pGroup->llLinkScanTimeMaximum < pGroup->llLinkScanTimeCurrent )
			{
				pGroup->llLinkScanTimeMaximum = pGroup->llLinkScanTimeCurrent;
			}
			pGroup->llLinkScanTimeStart = llLinkScanTimeEnd;
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_LINK_SCAN_END;
			/* Increasing of th frame sequence number */
			pGroup->usFrameSequenceNumber ++;
			if ( pGroup->usFrameSequenceNumber == 0 )
			{
				pGroup->usFrameSequenceNumber = 1;
			}
			/* Execute the state of the master */
			ccief_basic_master_execute_state( pGroup, CCIEF_BASIC_EVENT_MASTER_LINK_SCAN_START );
			break;
		/* Receiving the cyclic data from the master */
		case CCIEF_BASIC_EVENT_MASTER_CYCLIC_DATA_RECV:
			/* Nothing */
			break;
		/* Receiving the error of the duplication master from the slave */
		case CCIEF_BASIC_EVENT_MASTER_DUPLICATION_ERR_RECV:
			/* Execute the state of the slave */
			for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
			{
				ccief_basic_slaves_execute_state( pGroup->pSlave[i], CCIEF_BASIC_EVENT_SLAVE_DUPLICATION_MASTER_ERR );
			}
			/* Stop the cyclic timer*/
			timer_stop( pGroup->iCyclicTimerId );
			/* Termination of socket */
			socket_terminate( sock );
			/* Set the master state */
			pGroup->iState = CCIEF_BASIC_STATE_MASTER_WAITING;
			/* Set the error code of the master duplication */
			Master.iErrCode = CCIEF_BASIC_MASTER_ERR_MASTER_DUPLICATION;
			return;
		default:
			break;
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for timeout of the persuasion timer.			*/
/************************************************************************************/
void ccief_basic_master_persuasion_timer_timeout( int iId, void *pCallbackArg )
{
	
	/* Execute the state of the master */
	ccief_basic_master_execute_state( (CCIEF_BASIC_MASTER_GROUP_INFO *)pCallbackArg, CCIEF_BASIC_EVENT_MASTER_PERSUASION_TIMEOUT );

	return;
}

/************************************************************************************/
/* This is an user defined function for timeout of the cyclic transmission timeout.	*/
/************************************************************************************/
void ccief_basic_master_cyclic_timer_timeout( int iId, void *pCallbackArg )
{
	int i;
	CCIEF_BASIC_MASTER_GROUP_INFO *pGroup;
	
	pGroup = (CCIEF_BASIC_MASTER_GROUP_INFO *)pCallbackArg;

	/* Execute the state of the slave */
	for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
	{
		ccief_basic_slaves_execute_state( pGroup->pSlave[i], CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_TIMEOUT );
	}

	/* Execute the state of the master */
	ccief_basic_master_execute_state( pGroup, CCIEF_BASIC_EVENT_MASTER_LINK_SCAN_END );

	return;
}

/************************************************************************************/
/* This is an user defined function for make the request data of Cyclic Data.		*/
/************************************************************************************/
#define REQUEST_OFFSET_ADDRESS_CYCLIC_INFO	0x0024

int ccief_basic_master_make_cyclic_data( CCIEF_BASIC_MASTER_GROUP_INFO *pGroup )
{
	uint8_t *pucData;
	int iIndex, iIndexCyclicState, iIndexRWw, iIndexRY;
	int i, j, iDataSize;
	static uint8_t aucCyclicStateBits[2];
	int64_t llTimeData;
	uint16_t usOccupiedStationNumber;

	pucData = pGroup->aucCyclicData;

	/* Make the request data of Cyclic Data. */
	iIndex = 0;
	/* Protocol Version */
	pucData[iIndex    ] = SHIFT_R0(CCIEF_BASIC_PROTCOL_VERSION);
	pucData[iIndex + 1] = SHIFT_R8(CCIEF_BASIC_PROTCOL_VERSION);
	iIndex += 2;
	/* Reserve */
	pucData[iIndex    ] = SHIFT_R0(0);
	pucData[iIndex + 1] = SHIFT_R8(0);
	iIndex += 2;
	/* Offset address (Cyclic information) */
	pucData[iIndex    ] = SHIFT_R0(REQUEST_OFFSET_ADDRESS_CYCLIC_INFO);
	pucData[iIndex + 1] = SHIFT_R8(REQUEST_OFFSET_ADDRESS_CYCLIC_INFO);
	iIndex += 2;
	/* Offset address (Reserved) */
	memset( &pucData[iIndex], 0, 14 );
	iIndex += 14;

	/* Master information data */
	/* Unit info */
	pucData[iIndex    ] = SHIFT_R0(Master.usUnitInfo);
	pucData[iIndex + 1] = SHIFT_R8(Master.usUnitInfo);
	iIndex += 2;
	/* Reserve */
	pucData[iIndex    ] = 0;
	pucData[iIndex + 1] = 0;
	iIndex += 2;
	/* Time data */
	llTimeData = timer_calculate_time_data();
	memcpy( &pucData[iIndex], &llTimeData, sizeof( llTimeData ) );
	pGroup->llTimeData = llTimeData;
	iIndex += 8;

	/* Cyclic information */
	/* Master id */
	memcpy( &pucData[iIndex], &Master.ulId, sizeof( Master.ulId ) );
	iIndex += 4;
	/* Group number */
	pucData[iIndex    ] = pGroup->ucGroupNumber;
	iIndex += 1;
	/* Reserve */
	pucData[iIndex    ] = 0;
	iIndex += 1;
	/* Frame sequence number */
	pucData[iIndex    ] = SHIFT_R0(pGroup->usFrameSequenceNumber);
	pucData[iIndex + 1] = SHIFT_R8(pGroup->usFrameSequenceNumber);
	iIndex += 2;
	/* Cyclic Transmission Timeout */
	/* Check the constant link scan time */
	if ( pGroup->pGroupParameter->usConstantLinkScanTime != 0 )
	{
		/* Set the constant link scan time */
		pucData[iIndex    ] = SHIFT_R0(pGroup->pGroupParameter->usConstantLinkScanTime);
		pucData[iIndex + 1] = SHIFT_R8(pGroup->pGroupParameter->usConstantLinkScanTime);
	}
	else
	{
		/* Set the cyclic transmission timeout */
		pucData[iIndex    ] = SHIFT_R0(pGroup->pGroupParameter->usCyclicTransmissionTimeout);
		pucData[iIndex + 1] = SHIFT_R8(pGroup->pGroupParameter->usCyclicTransmissionTimeout);
	}
	iIndex += 2;
	/* Count of cyclic transmission timeout */
	pucData[iIndex    ] = SHIFT_R0(pGroup->pGroupParameter->usCyclicTransmissionTimeoutCount);
	pucData[iIndex + 1] = SHIFT_R8(pGroup->pGroupParameter->usCyclicTransmissionTimeoutCount);
	iIndex += 2;
	/* Parameter id */
	pucData[iIndex    ] = SHIFT_R0(Master.usParameterId);
	pucData[iIndex + 1] = SHIFT_R8(Master.usParameterId);
	iIndex += 2;
	/* Total occupied station number of the slaves */
	pucData[iIndex    ] = SHIFT_R0(pGroup->iOccupiedStationNumberTotal);
	pucData[iIndex + 1] = SHIFT_R8(pGroup->iOccupiedStationNumberTotal);
	iIndex += 2;
	/* State of the cyclic and Slave id */
	iIndexCyclicState = iIndex;
	memset( &aucCyclicStateBits, 0, sizeof( aucCyclicStateBits ) );
	iIndex += 2;
	/* Reserve */
	pucData[iIndex    ] = 0;
	pucData[iIndex + 1] = 0;
	iIndex += 2;
	for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
	{
		/* Make the cyclic insication bits */
		if ( pGroup->pSlave[i]->iCyclicState == CCIEF_BASIC_CYCLIC_STATE_ON )
		{
			aucCyclicStateBits[( pGroup->pSlave[i]->iGroupStationNumber ) / 8] |= ( 1 << ( ( pGroup->pSlave[i]->iGroupStationNumber ) % 8 ));
		}
		/* Check the current start cyclic of user operation */
		if (( pGroup->pSlave[i]->iCyclicStateSet == CCIEF_BASIC_CYCLIC_STATE_SET_CONNECTING )
		 || ( pGroup->pSlave[i]->iCyclicStateSet == CCIEF_BASIC_CYCLIC_STATE_SET_CYCLIC ))
		{
			memcpy( &pucData[iIndex], &(pGroup->pSlave[i]->ulId), sizeof( pGroup->pSlave[i]->ulId ) );
		}
		else
		{
			memset( &pucData[iIndex], 0, sizeof( uint32_t ) );
		}
		iIndex += 4;
		/* Occupied station number */
		usOccupiedStationNumber = pGroup->pSlave[i]->pParameter->usOccupiedStationNumber;
		if ( 1 < usOccupiedStationNumber )
		{
			for ( j = 1; j < usOccupiedStationNumber; j ++ )
			{
				memset( &pucData[iIndex], 0xFF, sizeof( uint32_t ) );
				iIndex += 4;
			}
		}
	}
	memcpy( &pucData[iIndexCyclicState], aucCyclicStateBits, sizeof( aucCyclicStateBits ) );
	/* Send data */
	iIndexRWw = iIndex;
	iIndexRY = iIndex + ( CCIEF_BASIC_RWW_RWR_SIZE * pGroup->iOccupiedStationNumberTotal );
	for ( i = 0; i < pGroup->iTotalSlaveNumber; i ++ )
	{
		/* Get the cyclic insication of the slave */
		usOccupiedStationNumber = pGroup->pSlave[i]->pParameter->usOccupiedStationNumber;
		/* Set data */
		if ( pGroup->pSlave[i]->iCyclicStateSet == CCIEF_BASIC_CYCLIC_STATE_SET_CYCLIC )
		{
			/* Valid the cyclic data */
			memcpy( &pucData[iIndexRWw], pGroup->pSlave[i]->pusRWw,
					CCIEF_BASIC_RWW_RWR_SIZE * usOccupiedStationNumber );
			memcpy( &pucData[iIndexRY], pGroup->pSlave[i]->pusRY,
					CCIEF_BASIC_RX_RY_SIZE * usOccupiedStationNumber );
		}
		else
		{
			/* Invalid the cyclic data */
			memset( &pucData[iIndexRWw], 0, CCIEF_BASIC_RWW_RWR_SIZE * usOccupiedStationNumber );
			memset( &pucData[iIndexRY], 0, CCIEF_BASIC_RX_RY_SIZE * usOccupiedStationNumber );
		}
		iIndexRWw += CCIEF_BASIC_RWW_RWR_SIZE * usOccupiedStationNumber;
		iIndexRY += CCIEF_BASIC_RX_RY_SIZE * usOccupiedStationNumber;
	}

	/* Total length of the making data */
	iDataSize = iIndexRY;

	return iDataSize;
}

/************************************************************************************/
/* This is an user defined function for send packet.								*/
/************************************************************************************/
int ccief_basic_master_send_cyclic_data( uint32_t ulIpAddress, uint8_t *pucData, int iDataSize )
{
	SLMP_INFO target;	/* SLMP Infomation for sending packet*/
	int iSendPacketSize;
	int iErrCode = 0;

	/* Sets the SLMP Information for sending. */
	target.usNetNumber	= 0;						/* Network number */
	target.usNodeNumber	= 0xFF;						/* Node number  */
	target.usProcNumber = SLMP_CPU_DEFAULT;			/* Processor number */
	target.usDataLength = (uint16_t)(6 + iDataSize);/* Data length */
													/* timer(2)+command(2)+subcommand(2)+Data */
	target.usTimer = SLMP_TIMER_WAIT_FOREVER;		/* Timer value */
	target.usCommand = SLMP_COMMAND_CYCLIC_DATA;	/* Command = NodeSearch */
	target.usSubCommand = 0x0000;					/* Sub command */
	target.pucData = pucData;						/* Pointer for sending data */

	/* Make a packet stream using the SLMP library. */
	iErrCode = SLMP_MakePacketStream( SLMP_FTYPE_BIN_REQ_ST, &target, aucSendPacket );
	if( iErrCode != CCIEF_BASIC_MASTER_ERR_OK )
	{
		/* Could not be created. */
		return iErrCode;
	}

	/* Send the packet for cyclic request. */
	iSendPacketSize = 15+iDataSize;	/* subheader(2)+network number(1)+node number(1)+processor number(2)+ */
									/* multidrop node number(1)+data length(2)+fixed value(2)+command(2)+subcommand(2)+data */
	iErrCode = socket_send( sock, aucSendPacket, iSendPacketSize, ulIpAddress, CCIEF_BASIC_PORT_NUMBER_CYCLIC );
	if ( iErrCode != CCIEF_BASIC_MASTER_ERR_OK )
	{
		return iErrCode;
	}

	return CCIEF_BASIC_MASTER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for receiving the response data of Cyclic Data.	*/
/************************************************************************************/
#define RESPONSE_OFFSET_ADDRESS_SLAVE_INFO		0x0014

void ccief_basic_master_recv_cyclic_data_response( uint8_t *pucData )
{
	CCIEF_BASIC_MASTER_GROUP_INFO	*pGroup;
	uint32_t ulSlaveId;
	uint8_t	ucGroupNumber;
	uint16_t usProtocolVersion;
	int i, j, iIndex;
	uint16_t usEndCode;

	/* Get the protocol version and the group number and the slave id from received data */
	usProtocolVersion = SHIFT_L8( pucData[1] ) + SHIFT_L0( pucData[0] );
	usEndCode = SHIFT_L8( pucData[3] ) + SHIFT_L0( pucData[2] );
	/* Check the end code */
	if ( usEndCode == SLMP_END_DUPLICATE_MASTER )
	{
		for ( i = 0; i < Master.iGroupTotalNumber; i ++ )
		{
			/* Execute the state of the master */
			ccief_basic_master_execute_state( Master.pGroup[i], CCIEF_BASIC_EVENT_MASTER_DUPLICATION_ERR_RECV );
		}

		return;
	}
	iIndex = SHIFT_L8( pucData[5] ) + SHIFT_L0( pucData[4]);
	memcpy( &ulSlaveId, &pucData[iIndex], sizeof(uint32_t) );
	iIndex += 4;
	ucGroupNumber = pucData[iIndex];

	for ( i = 0; i < Master.iGroupTotalNumber; i ++ )
	{
		/* Check the group number */
		if ( Master.pGroup[i]->ucGroupNumber == ucGroupNumber )
		{
			pGroup = Master.pGroup[i];

			for ( j = 0; j < pGroup->iTotalSlaveNumber; j ++ )
			{
				/* Check the slave id */
				if ( pGroup->pSlave[j]->ulId == ulSlaveId )
				{
					/* Get the information from received data */
					/* Check the cyclic start of the slave */
					if ( pGroup->pSlave[j]->iCyclicStart == CCIEF_BASIC_CYCLIC_START )
					{
						/* Get the information from received data */
						/* Slave information data */
						memcpy( &pGroup->pSlave[j]->NotifyInfo, &pucData[RESPONSE_OFFSET_ADDRESS_SLAVE_INFO],
								sizeof( pGroup->pSlave[j]->NotifyInfo ) );

						/* Protocol version */
						pGroup->pSlave[j]->usProtocolVersion = usProtocolVersion;
						/* End code */
						pGroup->pSlave[j]->usEndCode = usEndCode;
						/* Frame sequence number */
						pGroup->pSlave[j]->usFrameSequenceNumber = SHIFT_L8( pucData[iIndex + 3] ) + SHIFT_L0( pucData[iIndex + 2] );
						/* Pointer of RWr */
						pGroup->pSlave[j]->pusSlaveRWr = (uint16_t*)&pucData[iIndex + 4];
						/* Pointer of RX */
						pGroup->pSlave[j]->pusSlaveRX = (uint16_t*)&pucData[iIndex + 4
													  + ( Master.Parameter.Slave[j].usOccupiedStationNumber * CCIEF_BASIC_RWW_RWR_SIZE )];
						/* Execute the state of the slave */
						ccief_basic_slaves_execute_state( pGroup->pSlave[j], CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_RECV );
						
						/* Check the state of slave duplication */
						if( pGroup->pSlave[j]->iDuplicateState == CCIEF_BASIC_SLAVE_DUPLICATION_DETECT )
						{
							Master.iErrCode = CCIEF_BASIC_MASTER_ERR_SLAVE_DUPLICATION;
						}
					}
				}
			}
		}
	}

	return;
}
