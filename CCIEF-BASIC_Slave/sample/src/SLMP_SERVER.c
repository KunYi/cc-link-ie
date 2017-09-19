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
/* This sample program for SLMP Server Application.									*/
/*																					*/
/* If you would like to execute another process, you may change "user_service"		*/
/* function.																		*/
/************************************************************************************/

#ifdef _WIN32
#include <stdint.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#elif __linux__
#include <netinet/in.h>
#endif
#include <stdio.h>
#include <string.h>

/*[ Structure for sample code ]*/
typedef struct 
{
	int			iTimerFlag;			/* Flag for response wait timer.(0:Not start timer  1:Starting timer) */
	int			iTimerId;			/* Id for response wait timer */
	SLMP_INFO	source;				/* Information of SLMP source */
	uint32_t	ulSendAddr;			/* Address of the target */
	uint16_t	usPortNumber;		/* Port number of the target */
	uint8_t		*pucSendData;		/* Send response data */
	uint16_t	usSendDataSize;		/* Size of the send response data */
#ifdef _WIN32
	SOCKET		sock;				/* Socket for sending response */
#elif __linux__
	int			sock;				/* Socket for sending response */
#endif
} SLMP_SERVER_SEND_RESPONSE_INFO;

#ifdef _WIN32
static SOCKET sock_server;			/* SCOKET for SLMP Server */
static SOCKET sock_ccief_basic;		/* SCOKET for CC-LinkIEF Basic */
static SOCKET sock_ccief_paramset;	/* SCOKET for parameter set */
#elif __linux__
static int		sock_server;			/* SCOKET for SLMP Server */
static int		sock_ccief_basic;		/* SCOKET for CC-LinkIEF Basic */
static int		sock_ccief_paramset;	/* SCOKET for parameter set */
#endif

static uint8_t aucSendData[SLMP_SERVER_MAX_FRAME_SIZE];		/* Data for sending packet */
static uint8_t aucRecvData[SLMP_SERVER_MAX_FRAME_SIZE];		/* Data for receiving packet */
static uint8_t aucSendPacket[SLMP_SERVER_MAX_FRAME_SIZE];	/* Buffer for sending packet */
static uint8_t aucRecvPacket[SLMP_SERVER_MAX_FRAME_SIZE];	/* Buffer for receiving packet */
static uint8_t aucSendDataNodeSearch[SLMP_SERVER_MAX_FRAME_SIZE];	/* Buffer for sending packet of NodeSearch */

static SLMP_SERVER_INFO	ServerInfo;

static int				iParameterSetFlag = 0;		/* 0:Enable ParameterSet  1:Disable ParameterSet */
static unsigned long	ulParameterSetTimer = 0;	/* Timer-value of after executing ParameterSet */

static SLMP_SERVER_CALLBACK_IPADDRESS_SET_BASIC		pUserIpAddresSetFunc;
static SLMP_SERVER_CALLBACK_PARAMETER_GET			pUserSlaveParameterGetFunc;
static SLMP_SERVER_CALLBACK_PARAMETER_SET			pUserSlaveParameterSetFunc;
static SLMP_SERVER_CALLBACK_PARAMETER_SET_END		pUserSlaveParameterSetEndFunc;
static SLMP_SERVER_CALLBACK_REMOTE_RESET			pUserRemoteResetFunc;

/*[ Service Function Table ]*/
static SLMP_SERVICE service_table[] =
{
	{ SLMP_COMMAND_MEMORY_READ, slmp_server_memory_read },
	{ SLMP_COMMAND_MEMORY_WRITE, slmp_server_memory_write },
	{ SLMP_COMMAND_DEVICE_INFO_COMPARE, slmp_server_device_info_compare },
	{ SLMP_COMMAND_PARAMETER_GET, slmp_server_parameter_get },
	{ SLMP_COMMAND_PARAMETER_SET, slmp_server_parameter_set },
	{ SLMP_COMMAND_PARAMETER_SET_START, slmp_server_parameter_set_start },
	{ SLMP_COMMAND_PARAMETER_SET_END, slmp_server_parameter_set_end },
	{ SLMP_COMMAND_PARAMETER_SET_CANCEL, slmp_server_parameter_set_cancel },
	{ SLMP_COMMAND_REMOTE_READ_TYPE_NAME, slmp_server_read_type_name },
	{ SLMP_COMMAND_REMOTE_RESET, slmp_server_remote_reset },
	/* Add the other Service here. */
};

/*[ Service Function Table for CC-LinkIEF Basic]*/
static SLMP_SERVICE service_table_basic[] =
{
	{ SLMP_COMMAND_NODE_SEARCH_BASIC, slmp_server_node_search_basic },
	{ SLMP_COMMAND_IP_ADDRESS_SET_BASIC, slmp_server_ip_address_set_basic },
};

/*[ Service Function Table use in parameter set port]*/
static SLMP_SERVICE service_table_paramset[] =
{
	{ SLMP_COMMAND_COMMUNICATION_SETTING_GET, slmp_server_communication_setting_get },
};

/* Information for NodeSearch response */
SLMP_SERVER_SEND_RESPONSE_INFO	NodeSearchResponseInfo;

/* Definition of Internal API */
#ifdef _WIN32
static int slmp_server_slmp_send_response( SOCKET sock, const SLMP_INFO *source, uint32_t ulSendAddr, uint16_t usSendPortNumber,
										   uint8_t *pucSendData, uint16_t usSendDataSize );
static int slmp_server_slmp_send_err_response( SOCKET sock, const SLMP_INFO *source, uint16_t usEndCode, uint32_t ulSendAddr, uint16_t usSendPortNumber );
#elif __linux__
static int slmp_server_slmp_send_response( int sock, const SLMP_INFO *source, uint32_t ulSendAddr, uint16_t usSendPortNumber,
										   uint8_t *pucSendData, uint16_t usSendDataSize );
static int slmp_server_slmp_send_err_response( int sock, const SLMP_INFO *source, uint16_t usEndCode, uint32_t ulSendAddr, uint16_t usSendPortNumber );
#endif
static void slmp_server_node_search_send_response_timeout( int iId, void *pCallbackArg );

/************************************************************************************/
/* This is an user defined function for initialization of the SLMP Server.			*/
/************************************************************************************/
int slmp_server_initialize( SLMP_SERVER_INFO *pServerInfo, SLMP_SERVER_CALLBACK_IPADDRESS_SET_BASIC pIpAddresSetFunc,
							SLMP_SERVER_CALLBACK_PARAMETER_GET pParameterGetFunc, SLMP_SERVER_CALLBACK_PARAMETER_SET pParameterSetFunc,
							SLMP_SERVER_CALLBACK_PARAMETER_SET_END pParameterSetEndFunc, SLMP_SERVER_CALLBACK_REMOTE_RESET pRemoteResetFunc )
{
	int iErrCode = 0;

	/* Getting the information of SLMP Server */
	memcpy( &ServerInfo, pServerInfo, sizeof( ServerInfo ) );

	/* Initialization of socket */
	/* for User port */
	iErrCode = socket_initialize( &sock_server, ServerInfo.ulIpAddress, pServerInfo->usPortNumber );
	if( iErrCode !=SOCKET_ERR_OK )
	{
		return iErrCode;
	}
	/* for CC-LinkIEF Basic port */
	iErrCode = socket_initialize( &sock_ccief_basic, ServerInfo.ulIpAddress, SLMP_SERVER_PORT_NUMBER );
	if( iErrCode !=SOCKET_ERR_OK )
	{
		return iErrCode;
	}
	/* for parameter get port */
	iErrCode = socket_initialize( &sock_ccief_paramset, ServerInfo.ulIpAddress, SLMP_SERVER_PORT_NUMBER_PARAMSET );
	if( iErrCode !=SOCKET_ERR_OK )
	{
		return iErrCode;
	}

	/* Callback function for exectute user ParameterSetting */
	pUserIpAddresSetFunc = pIpAddresSetFunc;
	pUserSlaveParameterGetFunc = pParameterGetFunc;
	pUserSlaveParameterSetFunc = pParameterSetFunc;
	pUserSlaveParameterSetEndFunc = pParameterSetEndFunc;
	pUserRemoteResetFunc = pRemoteResetFunc;

	/* Sets the ParameterSet Flag Enabled */ 
	iParameterSetFlag = 0;

	/* Initialization of NodeSearch response information */
	NodeSearchResponseInfo.iTimerFlag = 0;

	return SLMP_SERVER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for termination of the SLMP Server.				*/
/************************************************************************************/
void slmp_server_terminate( void )
{
	/* Termination of socket */
	socket_terminate( sock_server );

	return;
}

/************************************************************************************/
/* This is an user defined function for main of the SLMP Server.					*/
/************************************************************************************/
int slmp_server_main( void )
{
	int						iErrCode = 0;

	/* Received at the user port */
	iErrCode = slmp_server_user_port();
	if ( iErrCode != SLMP_SERVER_ERR_OK )
	{
		return iErrCode;
	}

	/* Received at the CC-Link IEF Basic port */
	iErrCode = slmp_server_basic_port();
	if ( iErrCode != SLMP_SERVER_ERR_OK )
	{
		return iErrCode;
	}
	/* Received at the parameter set port */
	iErrCode = slmp_server_paramset_port();
	if ( iErrCode != SLMP_SERVER_ERR_OK )
	{
		return iErrCode;
	}

	return SLMP_SERVER_ERR_OK;

}

/************************************************************************************/
/* This is an user defined function for received at the user port of the SLMP		*/
/* Server.																			*/
/************************************************************************************/
int slmp_server_user_port( void )
{
	SLMP_INFO				source;			/* SLMP Infomation for received packet */
	int						iErrCode = 0;
	uint32_t				ulRecvAddr;
	uint16_t				usRecvPortNumber;
	struct in_addr			addrSender;
	int_fast8_t				Ipaddr[16];
#ifdef _WIN32
#elif __linux__
	uint32_t				ulMyNetAddress;
	uint32_t				ulOtherNetAddress;
#endif

	/* Receive a request packet. */
	iErrCode = socket_recv( sock_server, aucRecvPacket, sizeof( aucRecvPacket ), &ulRecvAddr, &usRecvPortNumber );
	if ( iErrCode != SLMP_SERVER_ERR_OK )
	{
		if ( iErrCode == SOCKET_ERR_NO_RECEIVABLE )
		{
			return SLMP_SERVER_ERR_OK;
		}
		else
		{
			return iErrCode;
		}
	}

#ifdef _WIN32
#elif __linux__
	ulMyNetAddress = ( ServerInfo.ulIpAddress & ServerInfo.ulSubnetMask );
	ulOtherNetAddress = ( ulRecvAddr & ServerInfo.ulSubnetMask );

	/* Other network broadcast break*/
	if( ulMyNetAddress != ulOtherNetAddress )
	{
		return SLMP_SERVER_ERR_OK;
	}
#endif

	/* Sets the SLMP Information for receiving. */
	source.pucData = aucRecvData;

	/* Get the SLMP Information from the request packet using the SLMP library. */
	iErrCode = SLMP_GetSlmpInfo( &source, aucRecvPacket );
	if ( iErrCode != SLMP_ERR_OK )
	{
		printf("ERR : SLMP_GetSlmpInfo  %d\n",iErrCode);
		return SLMP_SERVER_ERR_OK;
	}

	/* Check the frame type */
	if( (source.ulFrameType == SLMP_FTYPE_BIN_REQ_ST) || (source.ulFrameType == SLMP_FTYPE_BIN_REQ_MT) )
	{	/* Request */
		/* Display the received packet information. */
#ifdef _WIN32
		addrSender.S_un.S_addr = htonl(ulRecvAddr);
#elif __linux__
		addrSender.s_addr = htonl(ulRecvAddr);
#endif
		inet_ntop(AF_INET, &addrSender, Ipaddr, sizeof(Ipaddr) );
		printf("SLMP: Receive: %-15s:%-5u  Command: %04X  Subcommand: %04X\n", Ipaddr, usRecvPortNumber,source.usCommand,source.usSubCommand);

		/* Execute a request service. */
		iErrCode = slmp_server_service( sock_server, &source, ulRecvAddr, usRecvPortNumber, service_table, (sizeof( service_table ) / sizeof( service_table[0] )) );
		if ( iErrCode != SLMP_SERVER_ERR_OK )
		{
			printf("ERR : SLMP Service  %d\n",iErrCode);
			return SLMP_SERVER_ERR_OK;
		}
	}

	return SLMP_SERVER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for received at the CC-LinkIEF Basic port of	*/
/* the SLMP Server. 																*/
/************************************************************************************/
int slmp_server_basic_port( void )
{
	SLMP_INFO				source;			/* SLMP Infomation for received packet */
	int						iErrCode = 0;
	uint32_t				ulRecvAddr;
	uint16_t				usRecvPortNumber;
	int_fast8_t				Ipaddr[16];
	struct in_addr			addrSender;

	/* Receive a request packet. */
	iErrCode = socket_recv( sock_ccief_basic, aucRecvPacket, sizeof( aucRecvPacket ), &ulRecvAddr, &usRecvPortNumber );
	if ( iErrCode != SLMP_SERVER_ERR_OK )
	{
		if ( iErrCode == SOCKET_ERR_NO_RECEIVABLE )
		{
			return SLMP_SERVER_ERR_OK;
		}
		else
		{
			return iErrCode;
		}
	}

	/* Sets the SLMP Information for receiving. */
	source.pucData = aucRecvData;

	/* Get the SLMP Information from the request packet using the SLMP library. */
	iErrCode = SLMP_GetSlmpInfo( &source, aucRecvPacket );
	if ( iErrCode != SLMP_ERR_OK )
	{
		printf("ERR : SLMP_GetSlmpInfo  %d\n",iErrCode);
		return SLMP_SERVER_ERR_OK;
	}

	/* Check the frame type */
	if( (source.ulFrameType == SLMP_FTYPE_BIN_REQ_ST) || (source.ulFrameType == SLMP_FTYPE_BIN_REQ_MT) )
	{	/* Request */
		/* Display the received packet information. */
#ifdef _WIN32
		addrSender.S_un.S_addr = htonl(ulRecvAddr);
#elif __linux__
		addrSender.s_addr = htonl(ulRecvAddr);
#endif
		inet_ntop(AF_INET, &addrSender, Ipaddr, sizeof(Ipaddr) );
		printf("SLMP: Receive: %-15s:%-5u  Command: %04X  Subcommand: %04X\n", Ipaddr, usRecvPortNumber,source.usCommand,source.usSubCommand);

		/* Execute a request service. */
		iErrCode = slmp_server_service( sock_ccief_basic, &source, ulRecvAddr, usRecvPortNumber, service_table_basic, (sizeof( service_table_basic ) / sizeof( service_table_basic[0] )) );
		if ( iErrCode != SLMP_SERVER_ERR_OK )
		{
			printf("ERR : SLMP Service  %d\n",iErrCode);
			return SLMP_SERVER_ERR_OK;
		}
	}

	return SLMP_SERVER_ERR_OK;

}

/************************************************************************************/
/* This is an user defined function for received at the parameter set port of		*/
/* the SLMP Server. 																*/
/************************************************************************************/
int slmp_server_paramset_port( void )
{
	SLMP_INFO				source;			/* SLMP Infomation for received packet */
	int						iErrCode = 0;
	uint32_t				ulRecvAddr;
	uint16_t				usRecvPortNumber;
	int_fast8_t				Ipaddr[16];
	struct in_addr			addrSender;
#ifdef _WIN32
#elif __linux__
	uint32_t				ulMyNetAddress;
	uint32_t				ulOtherNetAddress;
#endif

	/* Receive a request packet. */
	iErrCode = socket_recv( sock_ccief_paramset, aucRecvPacket, sizeof( aucRecvPacket ), &ulRecvAddr, &usRecvPortNumber );
	if ( iErrCode != SLMP_SERVER_ERR_OK )
	{
		if ( iErrCode == SOCKET_ERR_NO_RECEIVABLE )
		{
			return SLMP_SERVER_ERR_OK;
		}
		else
		{
			return iErrCode;
		}
	}

#ifdef _WIN32
#elif __linux__
	ulMyNetAddress = ( ServerInfo.ulIpAddress & ServerInfo.ulSubnetMask );
	ulOtherNetAddress = ( ulRecvAddr & ServerInfo.ulSubnetMask );

	/* Other network broadcast break*/
	if( ulMyNetAddress != ulOtherNetAddress )
	{
		return SLMP_SERVER_ERR_OK;
	}
#endif

	/* Sets the SLMP Information for receiving. */
	source.pucData = aucRecvData;

	/* Get the SLMP Information from the request packet using the SLMP library. */
	iErrCode = SLMP_GetSlmpInfo( &source, aucRecvPacket );
	if ( iErrCode != SLMP_ERR_OK )
	{
		printf("ERR : SLMP_GetSlmpInfo  %d\n",iErrCode);
		return SLMP_SERVER_ERR_OK;
	}

	/* Check the frame type */
	if( (source.ulFrameType == SLMP_FTYPE_BIN_REQ_ST) || (source.ulFrameType == SLMP_FTYPE_BIN_REQ_MT) )
	{	/* Request */
		/* Display the received packet information. */
#ifdef _WIN32
		addrSender.S_un.S_addr = htonl(ulRecvAddr);
#elif __linux__
		addrSender.s_addr = htonl(ulRecvAddr);
#endif
		inet_ntop(AF_INET, &addrSender, Ipaddr, sizeof(Ipaddr) );
		printf("SLMP: Receive: %-15s:%-5u  Command: %04X  Subcommand: %04X\n", Ipaddr ,usRecvPortNumber,source.usCommand,source.usSubCommand);

		/* Execute a request service. */
		iErrCode = slmp_server_service( sock_ccief_basic, &source, ulRecvAddr, usRecvPortNumber, service_table_paramset, (sizeof( service_table_paramset ) / sizeof( service_table_paramset[0] )) );
		if ( iErrCode != SLMP_SERVER_ERR_OK )
		{
			printf("ERR : SLMP Service  %d\n",iErrCode);
			return SLMP_SERVER_ERR_OK;
		}
	}

	return SLMP_SERVER_ERR_OK;

}

/************************************************************************************/
/* This is an user defined function for setting the status of the SLMP Server.		*/
/************************************************************************************/
void slmp_server_set_status( uint16_t usStatus )
{
	ServerInfo.usStatus = usStatus;

	return;
}

/************************************************************************************/
/* This is an user defined function for sending SLMP frame.							*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_slmp_send_response( SOCKET sock, const SLMP_INFO *source, uint32_t ulSendAddr, uint16_t usSendPortNumber,
									uint8_t *pucSendData, uint16_t usSendDataSize )

#elif __linux__
int slmp_server_slmp_send_response( int sock, const SLMP_INFO *source, uint32_t ulSendAddr, uint16_t usSendPortNumber,
									uint8_t *pucSendData, uint16_t usSendDataSize )
#endif
{
	static SLMP_INFO	target;			/* SLMP Infomation for sending packet*/
	int					iErrCode = 0;
	struct in_addr		addrSender;
	int_fast8_t			Ipaddr[16];

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
	target.usEndCode = SLMP_ERR_OK;
	target.pucData = pucSendData;

	/* Make a packet stream using the SLMP library. */
	iErrCode = SLMP_MakePacketStream( target.ulFrameType, &target, aucSendPacket );
	if ( iErrCode != SLMP_ERR_OK )
	{
		printf("ERR : SLMP_MakePacketStream  %d\n", iErrCode);
		return SLMP_SERVER_ERR_OK;
	}

	/* Send the packet for service response. */
	iErrCode = socket_send( sock, aucSendPacket, usSendDataSize + SLMP_HEADER_SIZE(target.ulFrameType), ulSendAddr, usSendPortNumber );
	if( iErrCode != SOCKET_ERR_OK )
	{
		return iErrCode;
	}

	/* Display the sending packet information. */
#ifdef _WIN32
	addrSender.S_un.S_addr = htonl(ulSendAddr);
#elif __linux__
	addrSender.s_addr = htonl(ulSendAddr);
#endif
	inet_ntop(AF_INET, &addrSender, Ipaddr, sizeof(Ipaddr) );
	printf("SLMP: Send:    %-15s:%-5u  Endcode: %04X\n", Ipaddr, usSendPortNumber, SLMP_ERR_OK );

	return SLMP_SERVER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for sending SLMP Error frame.					*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_slmp_send_err_response( SOCKET sock, const SLMP_INFO *source, uint16_t usEndCode, uint32_t ulSendAddr, uint16_t usSendPortNumber )
#elif __linux__
int slmp_server_slmp_send_err_response( int sock, const SLMP_INFO *source, uint16_t usEndCode, uint32_t ulSendAddr, uint16_t usSendPortNumber )
#endif
{
	static SLMP_INFO	target;			/* SLMP Infomation for sending packet*/
	uint8_t				*pucSendData;
	uint16_t			usSendDataSize;
	int					iErrCode = 0;
	struct in_addr		addrSender;
	int_fast8_t			Ipaddr[16];

	pucSendData = aucSendData;

	/* Make a error response data. */
	iErrCode = SLMP_MakeErrorData( source, aucSendData, &usSendDataSize );
	if( iErrCode != SLMP_ERR_OK )
	{
		printf("ERR : SLMP_MakeErrorData  %d\n", iErrCode);
		return SLMP_SERVER_ERR_OK;
	}

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
	target.usEndCode = usEndCode;
	target.pucData = pucSendData;

	/* Make a packet stream using the SLMP library. */
	iErrCode = SLMP_MakePacketStream( target.ulFrameType, &target, aucSendPacket );
	if ( iErrCode != SLMP_ERR_OK )
	{
		printf("ERR : SLMP_MakePacketStream  %d\n", iErrCode);
		return SLMP_SERVER_ERR_OK;
	}

	/* Send the packet for service response. */
	iErrCode = socket_send( sock, aucSendPacket, usSendDataSize + SLMP_HEADER_SIZE(target.ulFrameType), ulSendAddr, usSendPortNumber );
	if( iErrCode != SOCKET_ERR_OK )
	{
		return iErrCode;
	}

	/* Display the sending packet information. */
#ifdef _WIN32
	addrSender.S_un.S_addr = htonl(ulSendAddr);
#elif __linux__
	addrSender.s_addr = htonl(ulSendAddr);
#endif
	inet_ntop(AF_INET, &addrSender, Ipaddr, sizeof(Ipaddr) );
	printf("SLMP: Send:    %-15s:%-5u  Endcode: %04X\n", Ipaddr, usSendPortNumber, usEndCode );

	return SLMP_SERVER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for SLMP service execute. The following is		*/
/* one of a sample in the Windows OS. Please rewrite if necessary.					*/
/* The following is one of a sample in the Linux. Please rewrite if necessary.		*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_service( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber,
						 SLMP_SERVICE *pServiceTable, int iServiceTableNumber )
#elif __linux__
int slmp_server_service( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber,
						 SLMP_SERVICE *pServiceTable, int iServiceTableNumber )
#endif
{
	uint32_t ulDataCode;
	int i, iErrCode = SLMP_SERVER_ERR_UNSUPPORT_SERVICE;

	/* Gets a data code of the SLMP packet. */
	ulDataCode = SLMP_DATA_CODE(source->ulFrameType);

	/* Search a service and execute the function. */
	for( i = 0; i < iServiceTableNumber; i ++ )
	{
		if ( source->usCommand == pServiceTable[i].usCommand )
		{
			/*[ Binary Mode ]*/
			if ( ulDataCode == SLMP_DATA_CODE_BIN )
			{
				/* Execute a service function. */
				iErrCode = pServiceTable[i].pFunc( sock, source, ulRecvAddr, usRecvPortNumber );
				break;
			}
			/*[ Data Code Error ]*/
			else
			{
				/* Send error response */
				iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_WRONG_DATA, ulRecvAddr, usRecvPortNumber );
				break;
			}
		}
	}

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for Memory Read (Command: 0613)					*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_memory_read( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_memory_read( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucRecvData;
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode;
	unsigned int i;
	unsigned int uiHeadAddress = 0;
	unsigned int uiWordLength = 0;

	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/*[ Request data of Memory Read ]*/
	pucRecvData = source->pucData;
	/* Head Address */
	uiHeadAddress = CONCAT_4BIN(pucRecvData[3], pucRecvData[2], pucRecvData[1], pucRecvData[0]);
	/* Word Length */
	uiWordLength = CONCAT_2BIN(pucRecvData[5], pucRecvData[4]);

	/* Checked a range of Memory */
	if (( ServerInfo.pusMemory == NULL) || ( ServerInfo.uiMemorySize <= (uiHeadAddress+uiWordLength) ))
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_DATA_LENGTH, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/*[ Response data of Memory Read ]*/
	/* Read data of Memory */
	usSendDataSize = 0;
	pucSendData = aucSendData;
	for( i = 0; i < uiWordLength; i ++ )
	{
		pucSendData[i * 2] = SHIFT_R0(ServerInfo.pusMemory[uiHeadAddress + i]);
		pucSendData[( i * 2 ) + 1] = SHIFT_R8(ServerInfo.pusMemory[uiHeadAddress + i]);
		usSendDataSize += 2;
	}

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for Memory Write (Command: 1613)				*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_memory_write( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_memory_write( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucRecvData;
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode;
	unsigned int i;
	unsigned int uiHeadAddress = 0;
	unsigned int uiWordLength = 0;

	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/*[ Request data of Memory Write ]*/
	pucRecvData = source->pucData;
	/* Head Address */
	uiHeadAddress = CONCAT_4BIN(pucRecvData[3], pucRecvData[2], pucRecvData[1], pucRecvData[0]);
	/* Word Length */
	uiWordLength = CONCAT_2BIN(pucRecvData[5], pucRecvData[4]);

	/* Checked a range of Memory */
	if (( ServerInfo.pusMemory == NULL) || ( ServerInfo.uiMemorySize <= (uiHeadAddress+uiWordLength) ))
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_DATA_LENGTH, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/* Write data of Memory */
	usSendDataSize = 0;
	pucSendData = aucSendData;
	for( i = 0; i < uiWordLength; i ++ )
	{
		ServerInfo.pusMemory[uiHeadAddress + i] = SHIFT_L8(pucRecvData[(( i * 2 ) + 1) + 6]) + SHIFT_R0(pucRecvData[( i * 2 ) + 6]);
	}

	usSendDataSize = 0;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for NodeSearch (Command: 0E30)					*/
/* For CC-Link IEF Basic use.														*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_node_search_basic( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_node_search_basic( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucRecvData;
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	uint32_t ulSendDelayTime;		/* Timer-value of sending a broadcast */
	int iErrCode;
	int iIndex = 0, iLength;

	/* Subcommand Check */
	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/*[ Request data of NodeSearch ]*/
	pucRecvData = source->pucData;
	/* Client MAC Address */
	/* Client IP Address Size */
	/* Client IP Address */

	/*[ Check the response wait timer ]*/
	if ( NodeSearchResponseInfo.iTimerFlag != 0 )
	{
		/* Destruction of a packet */
		return SLMP_SERVER_ERR_OK;
	}

	/*[ Response data of NodeSearch ]*/
	pucSendData = aucSendDataNodeSearch;
	/* Client MAC Address */
	memcpy( &pucSendData[0], &pucRecvData[0], 6 );
	iIndex = 6;
	/* Client IP Address Size */
	pucSendData[iIndex] = pucRecvData[6];
	iIndex += 1;
	/* Client IP Address */
	memcpy( &pucSendData[iIndex], &pucRecvData[7], pucRecvData[6] );
	iIndex += pucRecvData[6];
	/* Server MAC Address */
	memcpy( &pucSendData[iIndex], &ServerInfo.aucMacAddress, 6 );
	iIndex += 6;
	/* Server IP Address Size */
	pucSendData[iIndex] = sizeof( ServerInfo.ulIpAddress );
	iIndex += 1;
	/* Server IP Address */
	memcpy( &pucSendData[iIndex], &ServerInfo.ulIpAddress, sizeof( ServerInfo.ulIpAddress ) );
	iIndex += sizeof( ServerInfo.ulIpAddress );
	/* Server Subnet Mask */
	memcpy( &pucSendData[iIndex], &ServerInfo.ulSubnetMask, sizeof( ServerInfo.ulSubnetMask ) );
	iIndex += sizeof( ServerInfo.ulSubnetMask );
	/* Server Default Gateway IP Address(fixed value:0xFFFFFFFF) */
	pucSendData[iIndex    ] = 0xFF;
	pucSendData[iIndex + 1] = 0xFF;
	pucSendData[iIndex + 2] = 0xFF;
	pucSendData[iIndex + 3] = 0xFF;
	iIndex += 4;
	/* Server Hostname Size(fixed value:0) */
	iLength = 0x00;
	pucSendData[iIndex] = 0x00;
	iIndex += 1;
	/* Server Hostname */
	memcpy( &pucSendData[iIndex], ServerInfo.acHostname, iLength );
	iIndex += iLength;
	/* Server Vendor Code */
	memcpy( &pucSendData[iIndex], &ServerInfo.usVenderCode, 2 );
	iIndex += 2;
	/* Server Model Code */
	memcpy( &pucSendData[iIndex], &ServerInfo.ulModelCode, 4 );
	iIndex += 4;
	/* Server Machine Version */
	memcpy( &pucSendData[iIndex], &ServerInfo.usMachineVersion, 2 );
	iIndex += 2;
	/* Target Unit IP Address Size(fixed value:4[IPv4]) */
	pucSendData[iIndex] = 0x04;
	iIndex += 1;
	/* Target Unit IP Address(fixed value:0xFFFFFFFFFF) */
	pucSendData[iIndex    ] = 0xFF;
	pucSendData[iIndex + 1] = 0xFF;
	pucSendData[iIndex + 2] = 0xFF;
	pucSendData[iIndex + 3] = 0xFF;
	iIndex += 4;
	/* Target Port Number(fixed value:0xFFFF) */
	pucSendData[iIndex    ] = 0xFF;
	pucSendData[iIndex + 1] = 0xFF;
	iIndex += 2;
	/* Server Status */
	memcpy( &pucSendData[iIndex], &ServerInfo.usStatus, 2 );
	iIndex += 2;
	/* Server Port Number */
	pucSendData[iIndex    ] = SHIFT_R0(SLMP_SERVER_PORT_NUMBER);
	pucSendData[iIndex + 1] = SHIFT_R8(SLMP_SERVER_PORT_NUMBER);
	iIndex += 2;
	/* Server Protocol(fixed value:1[UDP]) */
	pucSendData[iIndex    ] = 1;
	iIndex += 1;

	usSendDataSize = (uint16_t)iIndex;

	/* Set response data information */
	memcpy( &NodeSearchResponseInfo.source, source, sizeof( NodeSearchResponseInfo.source ) );
	inet_pton( AF_INET, "255.255.255.255", &NodeSearchResponseInfo.ulSendAddr );
	NodeSearchResponseInfo.usPortNumber = usRecvPortNumber;
	NodeSearchResponseInfo.pucSendData = pucSendData;
	NodeSearchResponseInfo.usSendDataSize = usSendDataSize;
	NodeSearchResponseInfo.sock = sock;

	/* Start the response wait timer */
	ulSendDelayTime = timer_broadcast_send_wait_time( SLMP_SERVER_MAX_BROADCAST_SEND_WAIT_TIME );
	timer_start( ulSendDelayTime, &NodeSearchResponseInfo.iTimerId,
				 &slmp_server_node_search_send_response_timeout, (void *)&NodeSearchResponseInfo );

	NodeSearchResponseInfo.iTimerFlag = 1;

	return SLMP_SERVER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function of timeout for sending NodeSearch response.		*/
/************************************************************************************/
void slmp_server_node_search_send_response_timeout( int iId, void *pCallbackArg )
{
	SLMP_SERVER_SEND_RESPONSE_INFO	*pNodeSearchResponseInfo;

	pNodeSearchResponseInfo = (SLMP_SERVER_SEND_RESPONSE_INFO *)pCallbackArg;

	/* Send response data */
	(void)slmp_server_slmp_send_response( pNodeSearchResponseInfo->sock, &pNodeSearchResponseInfo->source, pNodeSearchResponseInfo->ulSendAddr, pNodeSearchResponseInfo->usPortNumber,
										  pNodeSearchResponseInfo->pucSendData, pNodeSearchResponseInfo->usSendDataSize );

	NodeSearchResponseInfo.iTimerFlag = 0;

	return;
}

/************************************************************************************/
/* This is an user defined function for IPAddressSet (Command: 0E31)				*/
/* For CC-Link IEF Basic use.														*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_ip_address_set_basic( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_ip_address_set_basic( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucRecvData;
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode;
	uint32_t ulSendAddr;
	int iIndex = 0;
	uint8_t aucClientMacAddr[6];
	int iIpAddressSize, iHostnameSize;
	uint8_t aucTemp[6];
	uint32_t ulRequestIpAddress, ulRequestSubnetMask, ulRequestDefaultGatewayIPAddress;

	/* Subcommand Check */
	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/*[ Request data of IPAddressSet ]*/
	pucRecvData = source->pucData;
	/* Server MAC Address */
	iIndex = 6 + 1 + pucRecvData[6];
	memcpy( aucTemp, &pucRecvData[iIndex], 6 );

	/*[ Check Server MAC Address ]*/
	if ( memcmp( ServerInfo.aucMacAddress, aucTemp, sizeof( aucTemp ) ) != 0 )
	{
		/* Destruction of a packet */
		return SLMP_SERVER_ERR_OK;
	}

	/* Client MAC Address */
	iIndex = 0;
	memcpy( aucClientMacAddr, &pucRecvData[iIndex], 6 );
	iIndex = 6;
	/* Client IP Address Size */
	iIpAddressSize = pucRecvData[iIndex];
	iIndex += 1;
	/* Client IP Address */
	iIndex += iIpAddressSize;
	/* Skip Server MAC Address */
	iIndex += 6;
	/* Server IP Address Size */
	iIpAddressSize = pucRecvData[iIndex];
	iIndex += 1;
	/* Server IP Address */
	memcpy( &ulRequestIpAddress, &pucRecvData[iIndex], iIpAddressSize );
	iIndex += iIpAddressSize;
	/* Server Subnet Mask */
	memcpy( &ulRequestSubnetMask, &pucRecvData[iIndex], iIpAddressSize );
	iIndex += iIpAddressSize;
	/* Server Default Gateway IP Address */
	memcpy( &ulRequestDefaultGatewayIPAddress, &pucRecvData[iIndex], iIpAddressSize );
	iIndex += iIpAddressSize;
	/* Server Hostname Size */
	iHostnameSize = pucRecvData[iIndex];
	iIndex += 1;
	/* Server Hostname */
	/* Do not support */
	iIndex += iHostnameSize;
	/* Target Unit IP Address Size */
	/* Do not support */
	iIpAddressSize = pucRecvData[iIndex];
	iIndex += 1;
	/* Target Unit IP Address */
	/* Do not support */
	iIndex += iIpAddressSize;
	/* Target Port Number */
	/* Do not support */
	iIndex += 2;
	/* Server Protocol */
	/* Do not support */
	iIndex += 1;

	/*[ Response data of IPAddressSet ]*/
	pucSendData = aucSendData;
	/* Client MAC Address */
	memcpy( &pucSendData[0], aucClientMacAddr, 6 );
	iIndex = 6;

	usSendDataSize = (uint16_t)iIndex;

	/* Execute user callback function */
	if ( pUserIpAddresSetFunc != NULL )
	{
		pUserIpAddresSetFunc( ulRequestIpAddress, ulRequestSubnetMask );
	}

	/* Sets a broadcast address */
	inet_pton( AF_INET, "255.255.255.255", &ulSendAddr );

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulSendAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for DeviceInfoCompare (Command: 0E32)			*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_device_info_compare( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_device_info_compare( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucRecvData;
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode;
	int iIndex = 0;
	uint16_t usCompareTarget;		/* Compare Target  */
	uint16_t usTempVenderCode;		/* Server Vendor Code */
	uint32_t ulTempTypeNameCode;	/* Server Model Code */
	uint16_t usTempVersion;			/* Server Machine Version */
	uint16_t usCompareResult;		/* Compare Result  */

	/* Subcommand Check */
	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/*[ Request data of DeviceInfoCompare ]*/
	pucRecvData = source->pucData;
	/* Compare Target */
	usCompareTarget = SHIFT_L8(pucRecvData[1])+SHIFT_L0(pucRecvData[0]);
	/* Vendor Code */
	usTempVenderCode = SHIFT_L8(pucRecvData[3])+SHIFT_L0(pucRecvData[2]);
	/* Model Code */
	ulTempTypeNameCode = SHIFT_L24(pucRecvData[7])+SHIFT_L16(pucRecvData[6])+
						 SHIFT_L8(pucRecvData[5])+SHIFT_L0(pucRecvData[4]);
	/* MachineVersion */
	usTempVersion = SHIFT_L8(pucRecvData[9])+SHIFT_L0(pucRecvData[8]);

	/*[ Response data of DeviceInfoCompare ]*/
	pucSendData = aucSendData;
	iIndex = 0;
	/* Compare Target */
	memcpy( &pucSendData[iIndex], &usCompareTarget, 2 );
	iIndex += 2;

	usCompareResult = 0x0000;
	/* Compare the Vendor Code */
	if ( (usCompareTarget & 0x0001) == 0x0001 )
	{
		if ( ServerInfo.usVenderCode != usTempVenderCode )
		{
			usCompareResult |= 0x0001;
		}
	}
	/* Compare the Model Code */
	if ( (usCompareTarget & 0x0002) == 0x0002 )
	{
		if ( ServerInfo.ulModelCode != ulTempTypeNameCode )
		{
			usCompareResult |= 0x0002;
		}
	}
	/* Compare the Machine Version */
	if ( (usCompareTarget & 0x0004) == 0x0004 )
	{
		if ( ServerInfo.usMachineVersion != usTempVersion )
		{
			usCompareResult |= 0x0004;

			/* Detailed Information */
			if ( ServerInfo.usMachineVersion > usTempVersion )
			{
				usCompareResult |= 0x4000;
			}

			else if ( ServerInfo.usMachineVersion < usTempVersion )
			{
				usCompareResult |= 0x8000;
			}

			else
			{
				usCompareResult |= 0xC000;
			}
		}
	}
	/* Compare Result */
	memcpy( &pucSendData[iIndex], &usCompareResult, 2 );
	iIndex += 2;

	usSendDataSize = (uint16_t)iIndex;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for ParameterGet (Command: 0E33)				*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_parameter_get( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_parameter_get( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucRecvData;
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode, iEndCode;
	int i, j;
	int iIndex = 0;
	uint16_t usGetParameter;
	uint16_t ausParameterId[3];
	uint16_t usParameterSize;
	uint8_t	*pucParameterData;

	/* Subcommand Check */
	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/*[ Request data of ParameterGet ]*/
	pucRecvData = source->pucData;
	/* Parameter Number */
	usGetParameter = SHIFT_L8(pucRecvData[1])+SHIFT_L0(pucRecvData[0]);

	if ( (usGetParameter == 0) || (3 < usGetParameter) )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_WRONG_DATA, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/* ParameterID */
	for( i = 0; i < usGetParameter; i ++ )
	{
		ausParameterId[i] = SHIFT_L8(pucRecvData[( i * 2 + 1 ) + 2])+SHIFT_L0(pucRecvData[( i * 2 ) + 2]);
	}

	/*[ Response data of ParameterGet ]*/
	pucSendData = aucSendData;
	/* Parameter Number */
	pucSendData[0] = SHIFT_R0(usGetParameter);
	pucSendData[1] = SHIFT_R8(usGetParameter);
	iIndex += 2;
	for( i = 0; i < usGetParameter; i ++ )
	{
		/* Execute user callback function */
		if ( pUserSlaveParameterGetFunc != NULL )
		{
			iEndCode = pUserSlaveParameterGetFunc( ausParameterId[i], &usParameterSize, &pucParameterData );
			if ( iEndCode != SLMP_ERR_OK )
			{
				/* Send error response */
				iErrCode = slmp_server_slmp_send_err_response( sock, source, (uint16_t)iEndCode, ulRecvAddr, usRecvPortNumber );

				return iErrCode;
			}
			/* ParameterID */
			pucSendData[iIndex] = SHIFT_R0(ausParameterId[i]);
			pucSendData[iIndex+1] = SHIFT_R8(ausParameterId[i]);
			/* Parameter Size */
			pucSendData[iIndex+2] = SHIFT_R0(usParameterSize);
			pucSendData[iIndex+3] = SHIFT_R8(usParameterSize);
			/* Parameter Data */
			for( j = 0; j < usParameterSize; j ++ )
			{
				pucSendData[iIndex + 4 + j] = pucParameterData[j];
			}
			iIndex += 4 + usParameterSize;
		}
		else
		{
			/* Send error response */
			iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_NO_EXIST_PARAM_ID, ulRecvAddr, usRecvPortNumber );

			return iErrCode;
		}
	}

	usSendDataSize = (uint16_t)iIndex;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for ParameterSet (Command: 0E34)				*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_parameter_set( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_parameter_set( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucRecvData;
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode, iEndCode;
	int i;
	int iIndex = 0;
	uint16_t usSetParameter;
	uint16_t usParameterId;
	uint16_t usParameterSize;
	static uint8_t aucParameterData[512 + 1];

	/* Subcommand Check */
	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/* Check the ParameterSet Start */ 
	if ( iParameterSetFlag == 0 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_CAN_NOT_PARAMETER_SET, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}
	else
	{
		ulParameterSetTimer = timer_get_time();
	}

	/*[ Request data of ParameterSet ]*/
	pucRecvData = source->pucData;
	/* Parameter Number */
	usSetParameter = SHIFT_L8(pucRecvData[1]) + SHIFT_L0(pucRecvData[0]);
	iIndex += 2;

	if ( (usSetParameter == 0) || (3 < usSetParameter) )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_WRONG_DATA, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}
		
	for( i = 0; i < usSetParameter; i ++ )
	{
		/* ParameterID */
		usParameterId = SHIFT_L8(pucRecvData[iIndex + 1]) + SHIFT_L0(pucRecvData[iIndex]);

		/* Parameter Size */
		usParameterSize = SHIFT_L8(pucRecvData[iIndex + 3]) + SHIFT_L0(pucRecvData[iIndex + 2]);

		/* Parameter Data */
		memcpy( aucParameterData, &pucRecvData[iIndex + 4], usParameterSize );
		aucParameterData[ usParameterSize ] = 0x00;

		/* Execute user callback function */
		if ( pUserSlaveParameterSetFunc != NULL )
		{
			iEndCode = pUserSlaveParameterSetFunc( usParameterId, usParameterSize, aucParameterData );
			if ( iEndCode != SLMP_ERR_OK )
			{
				/* Send error response */
				iErrCode = slmp_server_slmp_send_err_response( sock, source, (uint16_t)iEndCode, ulRecvAddr, usRecvPortNumber );

				return iErrCode;
			}
		}
		else
		{
			/* Send error response */
			iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_NO_EXIST_PARAM_ID, ulRecvAddr, usRecvPortNumber );

			return iErrCode;
		}

		iIndex += 4 + usParameterSize;
	}

	/*[ Response data of ParameterSet ]*/
	pucSendData = aucSendData;

	usSendDataSize = 0;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for ParameterSetStart (Command: 0E35)			*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_parameter_set_start( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_parameter_set_start( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode;
	uint32_t ulCheckTimeout;

	/* Subcommand Check */
	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/* Check the ParameterSet Flag */ 
	if ( iParameterSetFlag == 1 )
	{
		/* Check the ParameterSet Timeout */ 
		ulCheckTimeout = timer_get_time() - ulParameterSetTimer;

		if ( SLMP_SERVER_SET_PARAMETER_TIMEOUT <= ulCheckTimeout )
		{
			/* Start ParameterSet */ 
			ulParameterSetTimer = timer_get_time();
		}
		else
		{
			/* Send error response */
			iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_CAN_NOT_PARAMETER_SET, ulRecvAddr, usRecvPortNumber );

			return iErrCode;
		}
	}
	else
	{
		/* Sets the ParameterSet Flag Disabled */ 
		iParameterSetFlag = 1;
		ulParameterSetTimer = timer_get_time();
	}

	/*[ Response data of ParameterSetStart ]*/
	pucSendData = aucSendData;

	usSendDataSize = 0;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for ParameterSetEnd (Command: 0E36)				*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_parameter_set_end( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_parameter_set_end( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode, iEndCode;

	/* Subcommand Check */
	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/* Execute user callback function */
	if ( pUserSlaveParameterSetEndFunc != NULL )
	{
		iEndCode = pUserSlaveParameterSetEndFunc();
		if ( iEndCode != SLMP_ERR_OK )
		{
			/* Send error response */
			iErrCode = slmp_server_slmp_send_err_response( sock, source, (uint16_t)iEndCode, ulRecvAddr, usRecvPortNumber );

			return iErrCode;
		}
	}

	/* Sets the ParameterSet Flag Enable */ 
	iParameterSetFlag = 0;

	/*[ Response data of ParameterSetEnd ]*/
	pucSendData = aucSendData;

	usSendDataSize = 0;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for ParameterSetCancel (Command: 0E3A)			*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_parameter_set_cancel( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_parameter_set_cancel( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode;

	/* Subcommand Check */
	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/* Sets the ParameterSet Flag Enabled */ 
	iParameterSetFlag = 0;

	/*[ Response data of ParameterSetCancel ]*/
	pucSendData = aucSendData;

	usSendDataSize = 0;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for CommunicationSettingGet (Command: 0E45)		*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_communication_setting_get( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_communication_setting_get( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucRecvData;
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode;
	int iIndex = 0;
	uint16_t usGetTarget;
	uint32_t ulTimeoutValue;

	if ( source->usSubCommand != 0x0000 )
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );

		return iErrCode;
	}

	/*[ Request data of CommunicationSettingGet ]*/
	pucRecvData = source->pucData;
	/* Get Target */
	usGetTarget = SHIFT_L8(pucRecvData[1]) + SHIFT_L0(pucRecvData[0]);

	/*[ Response data of CommunicationSettingGet ]*/
	pucSendData = aucSendData;
	iIndex = 0;
	/* Get Target */
	memcpy( &pucSendData[iIndex], &usGetTarget, 2 );
	iIndex += 2;

	/* Port Number */
	if ( (usGetTarget & 0x0001) == 0x0001 )
	{
		pucSendData[iIndex    ] = SHIFT_R0(ServerInfo.usPortNumber);
		pucSendData[iIndex + 1] = SHIFT_R8(ServerInfo.usPortNumber);
	}
	else
	{
		memset( &pucSendData[iIndex], 0, 2 );
	}
	iIndex += 2;

	/* Timeout Value */
	if ( (usGetTarget & 0x0002) == 0x0002 )
	{
		ulTimeoutValue = SLMP_SERVER_TIMEOUT;
		memcpy( &pucSendData[iIndex], &ulTimeoutValue, 4 );
	}
	else
	{
		memset( &pucSendData[iIndex], 0, 4 );
	}
	iIndex += 4;

	usSendDataSize = (uint16_t)iIndex;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for ReadTypeName (Command: 0101)				*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_read_type_name( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_read_type_name( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	uint8_t *pucSendData;
	uint16_t usSendDataSize;
	int iErrCode;
	int iIndex = 0;

	/*[ Response data of ReadTypeName ]*/
	pucSendData = aucSendData;
	iIndex = 0;
	/* Type Name */
	memcpy( &pucSendData[iIndex], &ServerInfo.acTypeName, 16 );
	iIndex += 16;
	/* Type Name Code */
	pucSendData[iIndex    ] = SHIFT_R0(ServerInfo.usTypeNameCode);
	pucSendData[iIndex + 1] = SHIFT_R8(ServerInfo.usTypeNameCode);
	iIndex += 2;

	usSendDataSize = (uint16_t)iIndex;

	/* Send response data */
	iErrCode = slmp_server_slmp_send_response( sock, source, ulRecvAddr, usRecvPortNumber, pucSendData, usSendDataSize );

	return iErrCode;
}

/************************************************************************************/
/* This is an user defined function for RemoteReset (Command: 1006)					*/
/************************************************************************************/
#ifdef _WIN32
int slmp_server_remote_reset( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#elif __linux__
int slmp_server_remote_reset( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber )
#endif
{
	int iErrCode = 0, iEndCode = 0;

	/* Execute user callback function */
	if ( pUserRemoteResetFunc != NULL )
	{
		iEndCode = pUserRemoteResetFunc();
		if ( iEndCode != SLMP_ERR_OK )
		{
			/* Send error response */
			iErrCode = slmp_server_slmp_send_err_response( sock, source, (uint16_t)iEndCode, ulRecvAddr, usRecvPortNumber );

			return iErrCode;
		}
	}
	else
	{
		/* Send error response */
		iErrCode = slmp_server_slmp_send_err_response( sock, source, SLMP_ERR_COMMAND_SUBCOMMAND, ulRecvAddr, usRecvPortNumber );
	}

	return iErrCode;
}
