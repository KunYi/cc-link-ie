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

#ifndef		__SLMP_SERVER_H__
#define		__SLMP_SERVER_H__

#include "SLMP.h"

#include <stdint.h>
#ifdef _WIN32
#include <winsock2.h>
#elif __linux__
#include <sys/socket.h>
#endif

/*[ Definition of SLMP Server environment ]*/
#define	SLMP_SERVER_MAX_FRAME_SIZE					4096
#define	SLMP_SERVER_TIMEOUT							30000
#define	SLMP_SERVER_SET_PARAMETER_TIMEOUT			10000
#define	SLMP_SERVER_MAX_BROADCAST_SEND_WAIT_TIME	1500
#define SLMP_SERVER_PORT_NUMBER						61451		/* Port number for NodeConnect */
#define SLMP_SERVER_PORT_NUMBER_PARAMSET			45237		/* Port number for Parameter set */

/*[ Definition for SLMP Server Error Code ]*/
#define	SLMP_SERVER_ERR_OK								0
#define	SLMP_SERVER_ERR_NG								(-1)
#define SLMP_SERVER_ERR_UNSUPPORT_SERVICE				(-2)

/*[ Structure of Service Function ]*/
typedef struct 
{
	uint16_t	usCommand;								/* Command */
#ifdef _WIN32
	int			(*pFunc)( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr,
						  uint16_t usRecvPortNumber );	/* Function */
#elif __linux__
	int	(*pFunc)(int sock, const SLMP_INFO *source, uint32_t ulRecvAddr,uint16_t usRecvPortNumber);	/* Function */
#endif
} SLMP_SERVICE;

/*[ Structure of the SLMP Server Information ]*/
typedef struct 
{
	uint16_t		usVenderCode;				/* Vender code */
	uint32_t		ulModelCode;				/* Model code */
	uint16_t		usMachineVersion;			/* Machine version */
	uint8_t			aucMacAddress[6];			/* Mac Address */
	uint32_t		ulIpAddress;				/* Server ip address */
	uint32_t		ulSubnetMask;				/* Server subnet mask */
	uint32_t		ulDefaultGatewayIPAddress;	/* Server default gateway ip address */
	uint16_t		usPortNumber;				/* Server port number */
	uint8_t			acHostname[64];				/* Hostname */
	uint16_t		usStatus;					/* Status */
	uint8_t			acTypeName[16];				/* Type name */
	uint16_t		usTypeNameCode;				/* Type name code */
	uint16_t		*pusMemory;					/* Pointer of the user memory */
	unsigned int	uiMemorySize;				/* Size of the user memory */
} SLMP_SERVER_INFO;

/*[ Structure of a Parameter ID ]*/
typedef struct {
	uint16_t		usSize;			/* Parameter Size */
	uint8_t			aucData[512];	/* Parameter Data */
} SLMP_PARAMETER_ID;

/*[ Definition of callback function ]*/
typedef void(*SLMP_SERVER_CALLBACK_IPADDRESS_SET_BASIC)( uint32_t ulIpAddress, uint32_t ulSubnetMask );
typedef int(*SLMP_SERVER_CALLBACK_PARAMETER_GET)( uint16_t usId, uint16_t *pusSize, uint8_t **ppucData );
typedef int(*SLMP_SERVER_CALLBACK_PARAMETER_SET)( uint16_t usId, uint16_t usSize, uint8_t *pucData );
typedef int(*SLMP_SERVER_CALLBACK_PARAMETER_SET_END)( void );
typedef int(*SLMP_SERVER_CALLBACK_REMOTE_RESET)( void );

/*[ Definition of function of sample program ]*/
extern int slmp_server_initialize( SLMP_SERVER_INFO *pServerInfo, SLMP_SERVER_CALLBACK_IPADDRESS_SET_BASIC pIpAddresSetFunc,
								   SLMP_SERVER_CALLBACK_PARAMETER_GET pParameterGetFunc, SLMP_SERVER_CALLBACK_PARAMETER_SET pParameterSetFunc,
								   SLMP_SERVER_CALLBACK_PARAMETER_SET_END pParameterSetEndFunc, SLMP_SERVER_CALLBACK_REMOTE_RESET pRemoteResetFunc );
extern void slmp_server_terminate( void );
extern int slmp_server_main( void );
extern int slmp_server_user_port( void );
extern int slmp_server_basic_port( void );
extern int slmp_server_paramset_port( void );
extern void slmp_server_set_status( uint16_t usStatus );
#ifdef _WIN32
extern int slmp_server_service ( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber,
								 SLMP_SERVICE *pServiceTable, int iServiceTableNumber );
extern int slmp_server_memory_read( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_memory_write( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_node_search_basic( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_ip_address_set_basic( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_device_info_compare( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_get( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_set( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_set_start( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_set_end( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_set_cancel( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_communication_setting_get( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_read_type_name( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_remote_reset( SOCKET sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
#elif __linux__
extern int slmp_server_service ( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber,
								 SLMP_SERVICE *pServiceTable, int iServiceTableNumber );
extern int slmp_server_memory_read( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_memory_write( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_node_search_basic( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_ip_address_set_basic( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_device_info_compare( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_get( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_set( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_set_start( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_set_end( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_parameter_set_cancel( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_communication_setting_get( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_read_type_name( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
extern int slmp_server_remote_reset( int sock, const SLMP_INFO *source, uint32_t ulRecvAddr, uint16_t usRecvPortNumber );
#endif

#endif
/*EOF*/
