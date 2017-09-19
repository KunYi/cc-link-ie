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
#include "CCIEF_BASIC_SLAVE.h"
#include "SLMP_SERVER.h"
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
#include <stdint.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#elif __linux__
#include <netinet/in.h>
#include <string.h>
#endif
#include <stdio.h>

/*[ Structure for sample code ]*/
typedef struct 
{
	uint16_t	usVenderCode;		/* Vender code */
	uint16_t	usReserve1;			/* Reserve */
	uint32_t	ulModelCode;		/* Model code */
	uint16_t	usMachineVersion;	/* Machine version */
	uint16_t	usReserve2;			/* Reserve */
	uint16_t	usUnitInfo;			/* Information of the unit */
	uint16_t	usErrCode;			/* Error code */
	uint32_t	ulUnitData;			/* Data of the unit */
} CCIEF_BASIC_SLAVE_NOTIFY_INFO;

typedef struct 
{
	int								iFirst;							/* Flag for check the first cyclic data(0:First 1:Not First) */
	uint32_t						ulId;							/* Id of the slave */
	CCIEF_BASIC_SLAVE_NOTIFY_INFO	SlaveNotifyInfo;				/* Notify information from the slave */
	CCIEF_BASIC_MASTER_NOTIFY_INFO	MasterNotifyInfo;				/* Notify information from the master */
	uint32_t						ulMasterId;						/* Id of the master */
	uint8_t							ucGroupNumber;					/* Group number */
	uint16_t						usParameterIdNumber;			/* Parameter id number */
	uint16_t						usSlaveIdOffsetIndex;			/* Index of the slave id offset */
	uint16_t						usCyclicStateOffsetIndex;		/* Index of the cyclic state offset */
	uint16_t						usRYOffsetIndex;				/* Index of RY offset */
	uint16_t						usRWwOffsetIndex;				/* Index of RWw offset */
	int								iOccupiedStationNumber;			/* Number of occupied stations */
	int								iStationNumber;					/* Number of stations */
	int								iCyclicState;					/* Cyclic state */
	int								iDisconnectionTimer;			/* Flag for the disconnection timer (0:Stop 1:Start) */
	int								iDisconnectionTimerId;			/* Id for the disconnection timer */
} CCIEF_BASIC_SLAVE_CYCLIC_DATA_INFO;

/* Definition of external variable of sample code */
#ifdef _WIN32
static SOCKET sock_slave = 0;		/* sokect for CCIEF-BASIC Slave */
#elif __linux__
static int sock_slave = 0;			/* sokect for CCIEF-BASIC Slave */
#endif
static uint8_t aucSendData[CCIEF_BASIC_MAX_FRAME_SIZE];		/* Data for sending packet */
static uint8_t aucRecvData[CCIEF_BASIC_MAX_FRAME_SIZE];		/* Data for receiving packet */
static uint8_t aucSendPacket[CCIEF_BASIC_MAX_FRAME_SIZE];	/* Buffer for sending packet */
static uint8_t aucRecvPacket[CCIEF_BASIC_MAX_FRAME_SIZE];	/* Buffer for receiving packet */

static uint16_t ausRX[(CCIEF_BASIC_RX_RY_SIZE / sizeof( uint16_t )) * CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP];		/* RX */
static uint16_t ausRY[(CCIEF_BASIC_RX_RY_SIZE / sizeof( uint16_t)) * CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP];		/* RY */
static uint16_t ausRWw[(CCIEF_BASIC_RWW_RWR_SIZE / sizeof( uint16_t )) * CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP];	/* RWw */
static uint16_t ausRWr[(CCIEF_BASIC_RWW_RWR_SIZE / sizeof( uint16_t )) * CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP];	/* RWr */

static CCIEF_BASIC_SLAVE_CALLBACK_RECV_CYCLIC_DATA		pUserRecvCyclicDataFunc = NULL;			/* Callback function for receiving the cyclic data */
static CCIEF_BASIC_SLAVE_CALLBACK_CYCLIC_DISCONNECTION	pUserCyclicDisconnectionFunc = NULL;	/* Callback function for disconnecting of cyclic */

static CCIEF_BASIC_SLAVE_INFO				Slave;			/* Information of the slave */
static CCIEF_BASIC_SLAVE_CYCLIC_DATA_INFO	CyclicData;		/* Information of the cyclic data */

/* Definition of function of sample code */
#ifdef _WIN32
static int ccief_basic_slave_recv_cyclic_data( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
static int ccief_basic_slave_send_cyclic_data( SOCKET sock, const SLMP_INFO *source, uint32_t ulSendAddr, uint16_t usSendPortNumber );
static int ccief_basic_slave_send_cyclic_data_error( SOCKET sock, const SLMP_INFO *source, uint16_t usEndCode, uint32_t ulSendAddr, uint16_t usSendPortNumber );
#elif __linux__
static int ccief_basic_slave_recv_cyclic_data( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
static int ccief_basic_slave_send_cyclic_data( int sock, const SLMP_INFO *source, uint32_t ulSendAddr, uint16_t usSendPortNumber );
static int ccief_basic_slave_send_cyclic_data_error( int sock, const SLMP_INFO *source, uint16_t usErrCode, uint32_t ulSendAddr, uint16_t usSendPortNumber );
#endif
static void ccief_basic_slave_disconnection( void );
static void ccief_basic_slave_disconnection_timer_timeout( int iId, void *pCallbackArg );

/*[ Service Function Table ]*/
static SLMP_SERVICE service_table[] =
{
	{ SLMP_COMMAND_CYCLIC_DATA, ccief_basic_slave_recv_cyclic_data },
};

/************************************************************************************/
/* This is an user defined function for initialization of the CCIEF-BASIC Slave.	*/
/************************************************************************************/
int ccief_basic_slave_initialize( CCIEF_BASIC_SLAVE_INFO *pSlave, 
								CCIEF_BASIC_SLAVE_CALLBACK_RECV_CYCLIC_DATA pRecvCyclicDataFunc,
								CCIEF_BASIC_SLAVE_CALLBACK_CYCLIC_DISCONNECTION pCyclicDisconnectionFunc )
{
	int iErrCode = 0;

	/* Getting of the slave information */
	memcpy( &Slave, pSlave, sizeof( Slave ) );

	/* Initialization of the cyclic data information */
	memset( &CyclicData, 0, sizeof( CyclicData ) );

	/* User callback function */
	pUserRecvCyclicDataFunc = pRecvCyclicDataFunc;
	pUserCyclicDisconnectionFunc = pCyclicDisconnectionFunc;

	/* Initialization of the socket */
	iErrCode = socket_initialize( &sock_slave, Slave.ulIpAddress, CCIEF_BASIC_PORT_NUMBER );
	if( iErrCode != CCIEF_BASIC_SLAVE_ERR_OK )
	{
		return iErrCode;
	}

	/* Set of the cyclic data information */
	CyclicData.ulId = Slave.ulIpAddress;
	CyclicData.SlaveNotifyInfo.usVenderCode = pSlave->usVenderCode;
	CyclicData.SlaveNotifyInfo.ulModelCode = pSlave->ulModelCode;
	CyclicData.SlaveNotifyInfo.usMachineVersion = pSlave->usMachineVersion;

	return CCIEF_BASIC_SLAVE_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for termination of CCIEF-BASIC Slave.			*/
/************************************************************************************/
void ccief_basic_slave_terminate( void )
{
	/* Check the disconnection timer */
	if ( CyclicData.iDisconnectionTimer != 0 )
	{
		/* Stop the disconnection timer */
		timer_stop( CyclicData.iDisconnectionTimerId );
	}

	/* Termination of slave_socket */
	socket_terminate( sock_slave );

	return;
}

/************************************************************************************/
/* This is an user defined function for main function of CCIEF-BASIC Slave.			*/
/************************************************************************************/
int ccief_basic_slave_main( void )
{
	SLMP_INFO				source;			/* SLMP Infomation for received packet */
	int						iErrCode = 0;
	uint32_t				ulRecvAddr;
	uint16_t				usRecvPortNumber;
	char					Ipaddr[16];
	struct in_addr			addrSender;
#ifdef _WIN32
#elif __linux__
	uint32_t				ulMyNetAddress;
	uint32_t				ulOtherNetAddress;
#endif

	/* Receive a request packet. */
	iErrCode = socket_recv( sock_slave, aucRecvPacket, sizeof( aucRecvPacket ), &ulRecvAddr, &usRecvPortNumber );
	if ( iErrCode != SOCKET_ERR_OK )
	{
		if ( iErrCode == SOCKET_ERR_NO_RECEIVABLE )
		{
			return CCIEF_BASIC_SLAVE_ERR_OK;
		}
		else
		{
			return iErrCode;
		}
	}

#ifdef _WIN32
#elif __linux__

	ulMyNetAddress = ( Slave.ulIpAddress & Slave.ulSubnetMask );
	ulOtherNetAddress = ( ulRecvAddr & Slave.ulSubnetMask );

	/* Other network broadcast break*/
	if( ulMyNetAddress != ulOtherNetAddress )
	{
		return CCIEF_BASIC_SLAVE_ERR_OK;
	}
#endif

	/* Sets the SLMP Information for receiving. */
	source.pucData = aucRecvData;

	/* Get the SLMP Information from the request packet using the SLMP library. */
	iErrCode = SLMP_GetSlmpInfo( &source, aucRecvPacket );
	if ( iErrCode != SLMP_ERR_OK )
	{
		printf("ERR : SLMP_GetSlmpInfo  %d\n",iErrCode);
		return CCIEF_BASIC_SLAVE_ERR_OK;
	}

	/* Check the frame type */
	if( (source.ulFrameType == SLMP_FTYPE_BIN_REQ_ST) || (source.ulFrameType == SLMP_FTYPE_BIN_REQ_MT) )
	{	/* Request */
		/* Check the SLMP Command. */
		if ( source.usCommand != CCIEF_BASIC_SLMP_COMMAND_CYCLIC_DATA )
		{
			/* Display the received packet information. */
#ifdef _WIN32
			addrSender.S_un.S_addr = htonl(ulRecvAddr);
#elif __linux__
			addrSender.s_addr = htonl(ulRecvAddr);
#endif
			inet_ntop(AF_INET, &addrSender, Ipaddr, sizeof(Ipaddr) );
			printf("SLMP: Receive: %-15s:%-5u  Command: %04X  Subcommand: %04X\n", Ipaddr, usRecvPortNumber,source.usCommand,source.usSubCommand);
		}

		/* Execute a request service. */
		iErrCode = slmp_server_service( sock_slave, &source, ulRecvAddr, usRecvPortNumber, service_table, (sizeof( service_table ) / sizeof( service_table[0] )) );
		if ( iErrCode != SLMP_SERVER_ERR_OK )
		{
			printf("ERR : SLMP Service  %d\n",iErrCode);
			return CCIEF_BASIC_SLAVE_ERR_OK;
		}
	}

	return CCIEF_BASIC_SLAVE_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for setting the RX.								*/
/************************************************************************************/
int ccief_basic_slave_set_rx( int iNumber, int iValue )
{
	uint16_t usTemp;

	/* Check the number of device */
	if (( iNumber < 0 ) || ((( CCIEF_BASIC_RX_RY_SIZE * 8 ) * Slave.iOccupiedStationNumber ) <= iNumber ))
	{
		return CCIEF_BASIC_SLAVE_ERR_DEVICE_RANGE;
	}

	/* Check the value */
	if (( iValue == CCIEF_BASIC_BIT_OFF ) || ( iValue == CCIEF_BASIC_BIT_ON ))
	{
		usTemp = ausRX[( iNumber / 16 )];
		usTemp = (uint16_t)(( usTemp & ~( 1 << ( iNumber % 16 ))) | ( iValue << ( iNumber % 16 )));
		ausRX[( iNumber / 16 )] = usTemp;
	}
	else
	{
		return CCIEF_BASIC_SLAVE_ERR_NG;
	}
	
	return CCIEF_BASIC_SLAVE_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for getting the RY.								*/
/************************************************************************************/
int ccief_basic_slave_get_ry( int iNumber, int *piValue )
{
	uint16_t usTemp;

	/* Check the number of device */
	if (( iNumber < 0 ) || ((( CCIEF_BASIC_RX_RY_SIZE * 8 ) * Slave.iOccupiedStationNumber ) <= iNumber ))
	{
		return CCIEF_BASIC_SLAVE_ERR_DEVICE_RANGE;
	}

	usTemp = ausRY[( iNumber / 16 )];
	if (( usTemp & ( 1 << ( iNumber % 16 ))) == CCIEF_BASIC_BIT_OFF )
	{
		*piValue = CCIEF_BASIC_BIT_OFF;
	}
	else
	{
		*piValue = CCIEF_BASIC_BIT_ON;
	}
	
	return CCIEF_BASIC_SLAVE_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for getting the RWw.							*/
/************************************************************************************/
int ccief_basic_slave_get_rww( int iNumber, uint16_t *pusValue )
{
	int iMaxNumber;

	/* Check the number of device */
	iMaxNumber = ( CCIEF_BASIC_RWW_RWR_SIZE / sizeof( uint16_t )) * Slave.iOccupiedStationNumber;
	if (( iNumber < 0 ) || ( iMaxNumber <= iNumber ))
	{
		return CCIEF_BASIC_SLAVE_ERR_DEVICE_RANGE;
	}

	*pusValue = ausRWw[ iNumber ];
	
	return CCIEF_BASIC_SLAVE_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for setting the RWr.							*/
/************************************************************************************/
int ccief_basic_slave_set_rwr( int iNumber, uint16_t usValue )
{
	int iMaxNumber;

	/* Check the number of device */
	iMaxNumber = ( CCIEF_BASIC_RWW_RWR_SIZE / sizeof( uint16_t )) * Slave.iOccupiedStationNumber;
	if (( iNumber < 0 ) || ( iMaxNumber <= iNumber ))
	{
		return CCIEF_BASIC_SLAVE_ERR_DEVICE_RANGE;
	}

	ausRWr[ iNumber ] = usValue;
	
	return CCIEF_BASIC_SLAVE_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for getting the device start pointer.			*/
/************************************************************************************/
uint16_t *ccief_basic_slave_get_pointer( int iDeviceType )
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
void ccief_basic_slave_set_unit_info( uint16_t usUnitInfo )
{
	CyclicData.SlaveNotifyInfo.usUnitInfo = usUnitInfo;

	return;
}

/************************************************************************************/
/* This is an user defined function for setting the error code.						*/
/************************************************************************************/
void ccief_basic_slave_set_err_code( uint16_t usErrCode )
{
	CyclicData.SlaveNotifyInfo.usErrCode = usErrCode;

	return;
}

/************************************************************************************/
/* This is an user defined function for setting the unit data.						*/
/************************************************************************************/
void ccief_basic_slave_set_unit_data( uint32_t ulUnitData )
{
	CyclicData.SlaveNotifyInfo.ulUnitData = ulUnitData;

	return;
}

/************************************************************************************/
/* This is an user defined function for getting the slave status information.		*/
/************************************************************************************/
void ccief_basic_slave_get_master_info( CCIEF_BASIC_SLAVE_MASTER_INFO *pInfo )
{
	/* Getting the master information */
	pInfo->ucGroupNumber = CyclicData.ucGroupNumber;
	pInfo->ulId = CyclicData.ulMasterId;
	if ( pInfo->ulId == 0 )
	{
		memset( &pInfo->NotifyInfo, 0, sizeof( pInfo->NotifyInfo ) );
	}
	else
	{
		memcpy( &pInfo->NotifyInfo, &CyclicData.MasterNotifyInfo, sizeof( pInfo->NotifyInfo ) );
	}

	return;
}

/************************************************************************************/
/* This is an user defined function for receiving Cyclic Data request.				*/
/************************************************************************************/
#define REQUEST_OFFSET_ADDRESS_MASTER_INFO_DATA		0x0014

#ifdef _WIN32
int ccief_basic_slave_recv_cyclic_data( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int ccief_basic_slave_recv_cyclic_data( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t		*pucData;
	uint32_t	ulMasterId;							/* Id of the master */
	uint8_t		ucGroupNumber;						/* Group number */
	uint16_t	usCyclicTransmissionTimeout;		/* Cyclic transmission timeout [ms] */
	uint16_t	usCyclicTransmissionTimeoutCount;	/* Count of cyclic transmission timeout */
	uint16_t	usParameterIdNumber;				/* Parameter id number */
	uint16_t	usFrameSequenceNumber;				/* Frame sequence number */
	uint32_t	ulCyclicTransmissionTime;			/* Cyclic transmission time */
	int			iTotalOccupiedStationNumber;		/* Total number of occupied stations */
	int			iStationNumber;						/* Number of stations */
	int			iOccupiedStationNumber;				/* Number of occupied stations */
	int			iCyclicState;						/* Cyclic state */
	uint16_t	*pusCyclicState;
	uint32_t	*pulSlaveId;
	uint8_t		*pucRWw, *pucRY;
	uint32_t	ulSlaveId;
	int			i, j, iIndex;
	int			iErrCode;

	/* Get Cyclic Data pointer */
	pucData = source->pucData;

	/* Cyclic information */
	iIndex = SHIFT_L8( pucData[5] ) + SHIFT_L0( pucData[4] );
	iIndex -= 4;		/* Adjust for SLMP Library */
	/* Master id */
	memcpy( &ulMasterId, &pucData[iIndex], sizeof(uint32_t) );
	iIndex += 4;
	/* Group number */
	ucGroupNumber = pucData[iIndex];
	iIndex += 2;
	/* Check the master id */
	if (( CyclicData.iFirst != 0 ) && ( ulMasterId != CyclicData.ulMasterId ))
	{
		printf( "ERR : Found the duplicate Master!\n" );
		/* Send the response data of Cyclic Data. */
		iErrCode = ccief_basic_slave_send_cyclic_data_error( sock, source, SLMP_END_DUPLICATE_MASTER, ulRecvAddr, usRecvPortNumber );
		return iErrCode;
	}
	/* Frame sequence number */
	usFrameSequenceNumber = SHIFT_L8( pucData[iIndex + 1] ) + SHIFT_L0( pucData[iIndex] );
	iIndex += 2;
	/* Cyclic transmission timeout */
	usCyclicTransmissionTimeout = SHIFT_L8( pucData[iIndex + 1] ) + SHIFT_L0( pucData[iIndex] );
	iIndex += 2;
	if ( usCyclicTransmissionTimeout == 0 )
	{
		usCyclicTransmissionTimeout = CCIEF_BASIC_CYCLIC_TRANSMISSION_TIMEOUT;
	}
	/* Count of cyclic transmission timeout */
	usCyclicTransmissionTimeoutCount = SHIFT_L8( pucData[iIndex + 1] ) + SHIFT_L0( pucData[iIndex] );
	iIndex += 2;
	if ( usCyclicTransmissionTimeoutCount == 0 )
	{
		usCyclicTransmissionTimeoutCount = CCIEF_BASIC_CYCLIC_TRANSMISSION_TIMEOUT_COUNT;
	}
	/* Calculate the cyclic transmission timeout */
	ulCyclicTransmissionTime = usCyclicTransmissionTimeout * usCyclicTransmissionTimeoutCount;
	/* Parameter id */
	usParameterIdNumber = SHIFT_L8( pucData[iIndex + 1] ) + SHIFT_L0( pucData[iIndex] );
	iIndex += 2;

	/* Master notify information */
	memcpy( &CyclicData.MasterNotifyInfo, &pucData[REQUEST_OFFSET_ADDRESS_MASTER_INFO_DATA],
			sizeof( CyclicData.MasterNotifyInfo ) );

	/* Check the parameter id number */
	if (( CyclicData.iFirst == 0 ) || ( usParameterIdNumber != CyclicData.usParameterIdNumber ) || ( usFrameSequenceNumber == 0 ) )
	{
		/* Total occupied station number of the slaves */
		iTotalOccupiedStationNumber = SHIFT_L8( pucData[iIndex + 1] ) + SHIFT_L0( pucData[iIndex] );
		iIndex += 2;
		/* Cyclic state */
		CyclicData.usCyclicStateOffsetIndex = (uint16_t)(&pucData[iIndex] - pucData);
		iIndex += 4;
		/* Slave id */
		pulSlaveId = (uint32_t*)&pucData[iIndex];
		iStationNumber = 0;
		/* Found the slave id */
		for ( i = 0; ( i < iTotalOccupiedStationNumber ) && ( i < CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP ); i ++ )
		{
			ulSlaveId = *pulSlaveId;
			/* Check the slave id */
			if ( CyclicData.ulId == ulSlaveId )
			{
				/* Check the cyclic state */
				pusCyclicState = (uint16_t*)(pucData + CyclicData.usCyclicStateOffsetIndex);
				iCyclicState = ( (*pusCyclicState >> ( i % 16 )) & 0x0001 );
				if( iCyclicState == CCIEF_BASIC_CYCLIC_DATA_VALID )
				{	/* Valid the cyclic data */
					printf( "ERR : Slave duplication or timeout!\n" );
					return CCIEF_BASIC_SLAVE_ERR_OK;
				}
				
				/* Group number */
				CyclicData.ucGroupNumber = ucGroupNumber;
				/* Master id */
				CyclicData.ulMasterId = ulMasterId;
				/* The first receiving the cyclic data or changed the parameter */
				CyclicData.usParameterIdNumber = usParameterIdNumber;
				/* Offset of Slave id */
				CyclicData.usSlaveIdOffsetIndex = (uint16_t)((uint8_t*)pulSlaveId - pucData);
				pulSlaveId += 1;
				iStationNumber = i;
				iOccupiedStationNumber = 1;
				/* Getting the occupied station number */
				for ( j = 0; ( j < ( CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP - 1 )) && ((( i + 1 ) + j ) < iTotalOccupiedStationNumber ); j++ )
				{
					ulSlaveId = *pulSlaveId;
					pulSlaveId += 1;
					if ( ulSlaveId == CCIEF_BASIC_SLAVE_ID_OCCUPIED_STATIONS )
					{
						iOccupiedStationNumber ++;
					}
					else
					{
						break;
					}
				}
				CyclicData.iStationNumber = iStationNumber;
				CyclicData.iOccupiedStationNumber = iOccupiedStationNumber;
				/* RWw offset */
				iIndex += iTotalOccupiedStationNumber * 4;
				CyclicData.usRWwOffsetIndex =
					(uint16_t)(&pucData[iIndex + iStationNumber * CCIEF_BASIC_RWW_RWR_SIZE] - pucData);
				/* RY offset */
				iIndex += iTotalOccupiedStationNumber * CCIEF_BASIC_RWW_RWR_SIZE;
				CyclicData.usRYOffsetIndex =
					(uint16_t)(&pucData[iIndex + iStationNumber * CCIEF_BASIC_RX_RY_SIZE] - pucData);

				CyclicData.iFirst = 1;
				break;
			}
			pulSlaveId += 1;
		}
		if (( i == iTotalOccupiedStationNumber ) || ( i == CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP ))
		{
			/* Not found slave id! */
			/* Destruction of a packet */
			return CCIEF_BASIC_SLAVE_ERR_OK;
		}
	}
	else
	{
		/* The same parameters as the previous */
		/* Nothing */
	}

	/* Check the group number */
	if ( ucGroupNumber != CyclicData.ucGroupNumber )
	{
		/* Invalid group number! */
		/* Destruction of a packet */
		return CCIEF_BASIC_SLAVE_ERR_OK;
	}
	
	/* Check the disconnection timer */
	if ( CyclicData.iDisconnectionTimer != 0 )
	{
		/* Stop the disconnection timer */
		timer_stop( CyclicData.iDisconnectionTimerId );
	}
	/* Start the disconnection timer */
	timer_start( ulCyclicTransmissionTime, &CyclicData.iDisconnectionTimerId, &ccief_basic_slave_disconnection_timer_timeout, NULL );
	/* Running the disconnection timer */
	CyclicData.iDisconnectionTimer = 1;
	
	/* Check number of occupied stations */
	if ( CyclicData.iOccupiedStationNumber <= Slave.iOccupiedStationNumber )
	{	/* Valid the occupied stations number */
		/* Check the slave id */
		pulSlaveId = (uint32_t*)(pucData + CyclicData.usSlaveIdOffsetIndex);
		if ( *pulSlaveId == CyclicData.ulId )
		{
			/* Start cyclic */
			/* Check the cyclic state */
			pusCyclicState = (uint16_t*)(pucData + CyclicData.usCyclicStateOffsetIndex);
			iCyclicState = ( (*pusCyclicState >> ( CyclicData.iStationNumber % 16 )) & 0x0001 );
			CyclicData.iCyclicState = iCyclicState;

			/* Check Cyclic state */
			if ( iCyclicState == CCIEF_BASIC_CYCLIC_DATA_VALID )
			{	/* Valid the cyclic data */
				/* Receiving the cyclic data. */
				pucRWw = pucData + CyclicData.usRWwOffsetIndex;
				pucRY = pucData + CyclicData.usRYOffsetIndex;

				memcpy( ausRWw, pucRWw, CCIEF_BASIC_RWW_RWR_SIZE * CyclicData.iOccupiedStationNumber );
				memcpy( ausRY, pucRY, CCIEF_BASIC_RX_RY_SIZE * CyclicData.iOccupiedStationNumber );
			}
			else
			{	/* Invalid the cyclic data */
				/* Nothig */
			}
		}
		else
		{
			/* Not found slave id */
			/* Destruction of a packet */
			return CCIEF_BASIC_SLAVE_ERR_OK;
		}
	}
	else
	{
		/* Invalid the occupied stations number */
		printf( "ERR : Invalid the occupied stations number from the master!\n" );
		/* Send the response data of Cyclic Data. */
		iErrCode = ccief_basic_slave_send_cyclic_data_error( sock, source, SLMP_END_INVALID_NUMBER_OF_OCCUPIED_STATIONS, ulRecvAddr, usRecvPortNumber );
		return iErrCode;
	}

	/* Callback to the user function. */
	if ( pUserRecvCyclicDataFunc != NULL )
	{
		pUserRecvCyclicDataFunc( CyclicData.iCyclicState, CyclicData.iOccupiedStationNumber );
	}

	/* Send the response data of Cyclic Data. */
	iErrCode = ccief_basic_slave_send_cyclic_data( sock, source, ulRecvAddr, usRecvPortNumber );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for sending Cyclic Data response.				*/
/************************************************************************************/
#define RESPONSE_OFFSET_ADDRESS_CYCLIC_INFO	0x0028

#ifdef _WIN32
int ccief_basic_slave_send_cyclic_data( SOCKET sock, const SLMP_INFO *source, uint32_t ulSendAddr, uint16_t usSendPortNumber )
#elif __linux__
int ccief_basic_slave_send_cyclic_data( int sock, const SLMP_INFO *source, uint32_t ulSendAddr, uint16_t usSendPortNumber )
#endif
{
	SLMP_INFO	target;			/* SLMP Infomation for sending packet*/
	uint8_t		*pucData;
	uint16_t	usFrameSequenceNumber;	/* Frame sequence number */
	int			iIndex;
	uint16_t	usSendDataSize;
	int			iErrCode = 0;

	/* Get Cyclic Data pointer */
	pucData = source->pucData;
	/* Cyclic information */
	iIndex = SHIFT_L8( pucData[5] ) + SHIFT_L0( pucData[4] );
	iIndex -= 4;		/* Adjust for SLMP Library */
	/* Frame sequence number */
	iIndex += 6;
	usFrameSequenceNumber = SHIFT_L8( pucData[iIndex + 1] ) + SHIFT_L0( pucData[iIndex] );

	/* Make the Cyclic Data response data. */
	iIndex = 0;
	/* Protocol Version */
	aucSendData[iIndex    ] = SHIFT_R0(CCIEF_BASIC_PROTCOL_VERSION);
	aucSendData[iIndex + 1] = SHIFT_R8(CCIEF_BASIC_PROTCOL_VERSION);
	iIndex += 2;
	/* Error code */
	aucSendData[iIndex    ] = SHIFT_R0(0);
	aucSendData[iIndex + 1] = SHIFT_R8(0);
	iIndex += 2;
	/* Offset address (Cyclic information) */
	aucSendData[iIndex    ] = SHIFT_R0(RESPONSE_OFFSET_ADDRESS_CYCLIC_INFO);
	aucSendData[iIndex + 1] = SHIFT_R8(RESPONSE_OFFSET_ADDRESS_CYCLIC_INFO);
	iIndex += 2;
	/* Offset address (Reserved) */
	memset( &aucSendData[iIndex], 0, 14 );
	iIndex += 14;

	/* Notify information */
	memcpy( &aucSendData[iIndex], &CyclicData.SlaveNotifyInfo, sizeof( CyclicData.SlaveNotifyInfo ));

	/* Cyclic information */
	iIndex = RESPONSE_OFFSET_ADDRESS_CYCLIC_INFO;
	/* Slave id */
	memcpy( &aucSendData[iIndex], &CyclicData.ulId, sizeof(uint32_t) );
	iIndex += 4;
	/* Group number */
	aucSendData[iIndex    ] = CyclicData.ucGroupNumber;
	iIndex += 1;
	/* Reserved */
	aucSendData[iIndex    ] = 0;
	iIndex += 1;
	/* Frame sequence number */
	aucSendData[iIndex    ] = SHIFT_R0(usFrameSequenceNumber);
	aucSendData[iIndex + 1] = SHIFT_R8(usFrameSequenceNumber);
	iIndex += 2;
	/* Check cyclic state */
	if ( CyclicData.iCyclicState == CCIEF_BASIC_CYCLIC_DATA_VALID )
	{	/* Valid the cyclic data */
		/* Sending the cyclic data. */
		memcpy( &aucSendData[iIndex], ausRWr, CCIEF_BASIC_RWW_RWR_SIZE * CyclicData.iOccupiedStationNumber );
		memcpy( &aucSendData[iIndex + (CCIEF_BASIC_RWW_RWR_SIZE * CyclicData.iOccupiedStationNumber)],
				ausRX, CCIEF_BASIC_RX_RY_SIZE * CyclicData.iOccupiedStationNumber );
	}
	else
	{	/* Invalid the cyclic data */
		memset( &aucSendData[iIndex], 0, (CCIEF_BASIC_RWW_RWR_SIZE + CCIEF_BASIC_RX_RY_SIZE) * CyclicData.iOccupiedStationNumber );
	}
	iIndex += (CCIEF_BASIC_RWW_RWR_SIZE + CCIEF_BASIC_RX_RY_SIZE) * CyclicData.iOccupiedStationNumber;
	
	usSendDataSize = (uint16_t)iIndex;
	/* Sets the SLMP Information for sending. */
	target.ulFrameType = SLMP_RESPONSE_FRAME_TYPE(source->ulFrameType);
	target.usSerialNumber = source->usSerialNumber;
	target.usNetNumber = source->usNetNumber;
	target.usNodeNumber = source->usNodeNumber;
	target.usProcNumber = source->usProcNumber;
	target.usTimer = source->usTimer;
	target.usCommand = source->usCommand;
	target.usSubCommand = source->usSubCommand;
	target.usDataLength = usSendDataSize;
	target.usEndCode = CCIEF_BASIC_SLAVE_ERR_OK;
	target.pucData = aucSendData;

	/* Make a packet stream using the SLMP library. */
	iErrCode = SLMP_MakePacketStream( target.ulFrameType, &target, aucSendPacket );
	if( iErrCode != SLMP_ERR_OK )
	{
		/* Could not be created. */
		return CCIEF_BASIC_SLAVE_ERR_OK;
	}

	/* Send the packet for cyclic response. */
	iErrCode = socket_send( sock, aucSendPacket, usSendDataSize + SLMP_HEADER_SIZE(target.ulFrameType),
									 ulSendAddr, usSendPortNumber );
	if ( iErrCode != SOCKET_ERR_OK )
	{
		return iErrCode;
	}

	return CCIEF_BASIC_SLAVE_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for sending Cyclic Data error response.			*/
/************************************************************************************/
#ifdef _WIN32
int ccief_basic_slave_send_cyclic_data_error( SOCKET sock, const SLMP_INFO *source, uint16_t usEndCode, uint32_t ulSendAddr, uint16_t usSendPortNumber )
#elif __linux__
int ccief_basic_slave_send_cyclic_data_error( int sock, const SLMP_INFO *source, uint16_t usEndCode, uint32_t ulSendAddr, uint16_t usSendPortNumber )
#endif
{
	SLMP_INFO	target;					/* SLMP Infomation for sending packet*/
	uint8_t		*pucData;
	uint8_t		ucGroupNumber;			/* Group number */
	uint16_t	usFrameSequenceNumber;	/* Frame sequence number */
	int			iIndex;
	uint16_t	usSendDataSize;
	int			iErrCode = 0;
	int			iOccupiedStationNumber;

	/* Get Cyclic Data pointer */
	pucData = source->pucData;
	/* Cyclic information */
	iIndex = SHIFT_L8( pucData[5] ) + SHIFT_L0( pucData[4] );
	/* Group number */
	ucGroupNumber = pucData[iIndex];
	iIndex += 2;
	/* Frame sequence number */
	usFrameSequenceNumber = SHIFT_L8( pucData[iIndex + 1] ) + SHIFT_L0( pucData[iIndex] );
	/* Occupied station number*/
	if( CyclicData.iFirst == 1 )
	{
		/* Control master station is determined */
		iOccupiedStationNumber = CyclicData.iOccupiedStationNumber;
	}
	else
	{
		/* Control master station is undetermined */
		iOccupiedStationNumber = 1;
	}

	/* Make the Cyclic Data response data. */
	iIndex = 0;
	/* Protocol Version */
	aucSendData[iIndex    ] = SHIFT_R0(CCIEF_BASIC_PROTCOL_VERSION);
	aucSendData[iIndex + 1] = SHIFT_R8(CCIEF_BASIC_PROTCOL_VERSION);
	iIndex += 2;
	/* End code */
	aucSendData[iIndex    ] = SHIFT_R0(usEndCode);
	aucSendData[iIndex + 1] = SHIFT_R8(usEndCode);
	iIndex += 2;
	/* Offset address (Cyclic information) */
	aucSendData[iIndex    ] = SHIFT_R0(RESPONSE_OFFSET_ADDRESS_CYCLIC_INFO);
	aucSendData[iIndex + 1] = SHIFT_R8(RESPONSE_OFFSET_ADDRESS_CYCLIC_INFO);
	iIndex += 2;
	/* Offset address (Reserved) */
	memset( &aucSendData[iIndex], 0, 14 );
	iIndex += 14;

	/* Notify information */
	memcpy( &aucSendData[iIndex], &CyclicData.SlaveNotifyInfo, sizeof( CyclicData.SlaveNotifyInfo ));

	/* Cyclic information */
	iIndex = RESPONSE_OFFSET_ADDRESS_CYCLIC_INFO;
	/* Slave id */
	memcpy( &aucSendData[iIndex], &CyclicData.ulId, sizeof(uint32_t) );
	iIndex += 4;
	/* Group number */
	aucSendData[iIndex    ] = ucGroupNumber;
	iIndex += 1;
	/* Reserve */
	aucSendData[iIndex    ] = 0;
	iIndex += 1;
	/* Frame sequence number */
	aucSendData[iIndex    ] = SHIFT_R0(usFrameSequenceNumber);
	aucSendData[iIndex + 1] = SHIFT_R8(usFrameSequenceNumber);
	iIndex += 2;
	/* Cyclic data */
	memset( &aucSendData[iIndex], 0, iOccupiedStationNumber * ( CCIEF_BASIC_RWW_RWR_SIZE + CCIEF_BASIC_RX_RY_SIZE ) );
	iIndex += iOccupiedStationNumber * ( CCIEF_BASIC_RWW_RWR_SIZE + CCIEF_BASIC_RX_RY_SIZE );

	usSendDataSize = (uint16_t)iIndex;
	/* Sets the SLMP Information for sending. */
	target.ulFrameType = SLMP_RESPONSE_FRAME_TYPE(source->ulFrameType);
	target.usSerialNumber = source->usSerialNumber;
	target.usNetNumber = source->usNetNumber;
	target.usNodeNumber = source->usNodeNumber;
	target.usProcNumber = source->usProcNumber;
	target.usTimer = source->usTimer;
	target.usCommand = source->usCommand;
	target.usSubCommand = source->usSubCommand;
	target.usDataLength = usSendDataSize;
	target.usEndCode = CCIEF_BASIC_SLAVE_ERR_OK;
	target.pucData = aucSendData;

	/* Make a packet stream using the SLMP library. */
	iErrCode = SLMP_MakePacketStream( target.ulFrameType, &target, aucSendPacket );
	if( iErrCode != SLMP_ERR_OK )
	{
		/* Could not be created. */
		return CCIEF_BASIC_SLAVE_ERR_OK;
	}

	/* Send the packet for cyclic response. */
	iErrCode = socket_send( sock, aucSendPacket, usSendDataSize + SLMP_HEADER_SIZE(target.ulFrameType),
									 ulSendAddr, usSendPortNumber );
	if ( iErrCode != SOCKET_ERR_OK )
	{
		return iErrCode;
	}

	return CCIEF_BASIC_SLAVE_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for disconnection the master.					*/
/************************************************************************************/
void ccief_basic_slave_disconnection( void )
{
	/* Execute of the callback function */
	if ( pUserCyclicDisconnectionFunc != NULL )
	{
		pUserCyclicDisconnectionFunc();
	}
	/* Initialization of the cyclic data information */
	CyclicData.iFirst = 0;
	CyclicData.ulMasterId = 0;
	CyclicData.ucGroupNumber = 0;
	CyclicData.usParameterIdNumber = 0;
	/* Check the disconnection timer */
	if ( CyclicData.iDisconnectionTimer != 0 )
	{
		/* Stop the disconnection timer */
		timer_stop( CyclicData.iDisconnectionTimerId );
	}
	CyclicData.iDisconnectionTimer = 0;

	return;
}

/************************************************************************************/
/* This is an user defined function of timeout for the disconnection timer.			*/
/************************************************************************************/
void ccief_basic_slave_disconnection_timer_timeout( int iId, void *pCallbackArg )
{
	/* Disconnection the master */
	ccief_basic_slave_disconnection();

	return;
}
