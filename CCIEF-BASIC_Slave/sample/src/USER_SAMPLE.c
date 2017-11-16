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
#include "TIMER.h"
#include "USER_SAMPLE.h"

/************************************************************************************/
/* The following is an user defined main program. This main program is one of a		*/
/* sample in the Windows OS and Intel x86 CPU. Please rewrite if necessary.			*/
/* This main program is one of a sample in the Linux. Please rewrite if necessary.	*/
/************************************************************************************/

#include <stdio.h>
#ifdef _WIN32
#include <stdint.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <conio.h>
#include <direct.h>
#include <sys/stat.h>
#elif __linux__
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <limits.h>
#endif
#include <time.h>

/*[ Definition for sample program ]*/
#define	USER_PARAMETER_FILE_READ			1		/* Read the parameter file */
#define	USER_PARAMETER_FILE_NOT_READ		0		/* Not read the parameter file */
#define	USER_DISPLAY_INTERVAL_TIME			5000	/* Interval time for the display of the cyclic state [ms] */
#ifdef _WIN32
#elif __linux__
#define	MAX_INTERFACE						20
#define	MAX_PATH							260
#define	SOCKET_NOT_OPEN						0
#define	DIR_PROC_ROUTE						"/proc/net/route"

/*[ Structure of sample program ]*/
typedef struct 
{
	char				IPAddr[INET_ADDRSTRLEN];
	char				IPMask[INET_ADDRSTRLEN];
	char				MACAddr[18];
	char				Defo[1028];
	struct ifreq		USER_ifreq;
} USER_NETWORK_INFO;

typedef struct 
{
	char 				cIface[IF_NAMESIZE];
	struct in_addr		cDst;
	struct in_addr		cGateway;
	unsigned short int	iFlag;
	int					iRecCnt;
	int					iUse;
	short int			iMetric;
	struct in_addr		cMask;
	unsigned long int	iMTU;
	unsigned long int	iWindow;
	unsigned short int	iIRTT;
} t_RouteInfo;
#endif

typedef struct 
{
	uint8_t		aucMacAddress[6];			/* MAC Address */
	uint32_t	ulIpAddress;				/* IP Address */
	uint32_t	ulSubnetMask;				/* Subnet Mask */
	uint32_t	ulDefaultGatewayIPAddress;	/* Default Gateway IP Address */
} USER_ADAPTER_INFO;

/************************************************************************************/
/* This is an user defined data for the CCIEF-BASIC Slave sample.					*/
/************************************************************************************/
static USER_SLAVE_PARAMETER		UserSlaveParameter = { 0, 0, 0, 1, 0 };	/* Parameter of user setting */
static char						acParameterFilePath[MAX_PATH];			/* Path of the parameter file */
static int						iParameterFileRead;						/* Flag of the parameter read */

/************************************************************************************/
/* This is an user defined data for the SLMP Server sample.							*/
/************************************************************************************/
/* Memory */
static uint16_t				ausUserMemory[10000];	/* User Memory */

/************************************************************************************/
/* This is an user defined data for user sample.									*/
/************************************************************************************/
static USER_ADAPTER_INFO	AdapterInfo;			/* Information of the Network adapter */
static int					iReset;					/* Flag of user reset */
static unsigned long		ulCyclicDataCount = 0;	/* Count of the cyclic data at intervals */

/* Definition of function of sample program */
static void user_callback_recv_cyclic_data( int iCyclicState, int iOccupiedStationNumber );
static void user_callback_cyclic_disconnection( void );
static void user_callback_set_ip_address_basic( uint32_t ulIpAddress, uint32_t ulSubnetMask );
static int user_callback_parameter_get( uint16_t usId, uint16_t *pusSize, uint8_t **ppucData );
static int user_callback_parameter_set( uint16_t usId, uint16_t usSize, uint8_t *pucData );
static int user_callback_parameter_set_end( void );
static int user_callback_remote_reset( void );
static int32_t user_parameter_file_read( char *file_path, USER_SLAVE_PARAMETER *pParameter, USER_ADAPTER_INFO *pAdapterInfo );
static int32_t user_parameter_file_write( char *file_path, USER_SLAVE_PARAMETER *pParameter );
static void user_display_cyclic_information( void );
static int user_get_adapter_info( USER_ADAPTER_INFO *pGetAdapterInfo );
static int user_set_adapter_info( USER_ADAPTER_INFO *pSetAdapterInfo );

/************************************************************************************/
/* This is an user defined function for main function.								*/
/* The following is one of a sample in the Windows OS. Please rewrite if necessary.	*/
/* The following is one of a sample in the Linux. Please rewrite if necessary.		*/
/************************************************************************************/
void main( int argc, char *argv[] )
{
	CCIEF_BASIC_SLAVE_INFO Slave;
	SLMP_SERVER_INFO ServerInfo;
	int iErrCode = 0;
	int iResult;
	struct in_addr addr;
	char Ipaddr[16];
#ifdef _WIN32
	WSADATA wsaData;
	
	/* Initialize Winsock */
	iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != 0 )
	{
		printf( "WSAStartup failed with error: %d\n", iResult );
		goto end;
	}
#elif __linux__
#endif

	/* Get the Network adapter information */
	iErrCode = user_get_adapter_info( &AdapterInfo );
	if ( iErrCode != USER_ERR_OK )
	{
		goto end;
	}

	/* Get command line argument */
	if ( 1 < argc )
	{
#ifdef _WIN32
		strncpy_s( acParameterFilePath, sizeof( acParameterFilePath ), argv[1], sizeof( acParameterFilePath ) );
#elif __linux__
		strncpy( acParameterFilePath, argv[1], sizeof( acParameterFilePath ) );
#endif
		/* Read the parameter file */
		iParameterFileRead = USER_PARAMETER_FILE_READ;
	}
	else
	{
		printf( "Not found the parameter file!\n" );
		printf( "\nStarting with the default parameter.\n" );
		/* Not read the parameter file */
		iParameterFileRead = USER_PARAMETER_FILE_NOT_READ;
	}

reset:
	iReset = USER_RESET_NONE;

	/* Check the parameter refresh */
	if ( iParameterFileRead == USER_PARAMETER_FILE_READ )
	{
		/* Read the parameter file */
		iErrCode = user_parameter_file_read( acParameterFilePath, &UserSlaveParameter, &AdapterInfo );
		if ( iErrCode != USER_ERR_OK )
		{
			goto end;
		}
		/* Not read the parameter file */
		iParameterFileRead = USER_PARAMETER_FILE_NOT_READ;
	}

	/* Set ip address */
	iErrCode = user_set_adapter_info( &AdapterInfo );
	if ( iErrCode != USER_ERR_OK )
	{
		goto end;
	}

	/****************************************************************************/
	/* Initialization of the Timer.												*/
	/****************************************************************************/
	/* Initialization the timer environment */
	timer_initialize();

	/****************************************************************************/
	/* Initialization of the CCIEF-BASIC Slave.									*/
	/****************************************************************************/
	/* Initialization of the CCIEF-BASIC Slave */
	Slave.usVenderCode = USER_PROFILE_VENDOR_CODE;
	Slave.ulModelCode = USER_PROFILE_MODEL_CODE;
	Slave.usMachineVersion = USER_PROFILE_MACHINE_VERSION;
	Slave.ulIpAddress = AdapterInfo.ulIpAddress;
#ifdef _WIN32
#elif __linux__
	Slave.ulSubnetMask = AdapterInfo.ulSubnetMask;
#endif
	Slave.iOccupiedStationNumber = (int)UserSlaveParameter.usOccupiedStationNumber;
	iErrCode = ccief_basic_slave_initialize( &Slave, user_callback_recv_cyclic_data,
										   user_callback_cyclic_disconnection );
	if ( iErrCode != CCIEF_BASIC_SLAVE_ERR_OK )
	{
		printf( "\nCCIEF-BASIC Slave Station initialization error!\n" );
		goto end;
	}

	/* Show the parameter of the slave */
	printf( "\nStart CC-Link IE Field Basic Slave Station!\n" );
#ifdef _WIN32
	addr.S_un.S_addr = htonl(AdapterInfo.ulIpAddress);
#elif __linux__
	addr.s_addr = htonl(AdapterInfo.ulIpAddress);
#endif
	inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
	printf( "\n    IP Address:\t\t\t\t%s\n", Ipaddr );
#ifdef _WIN32
	addr.S_un.S_addr = htonl(AdapterInfo.ulSubnetMask);
#elif __linux__
	addr.s_addr = htonl(AdapterInfo.ulSubnetMask);
#endif
	inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
	printf( "    Subnet mask:\t\t\t%s\n", Ipaddr );
#ifdef _WIN32
	addr.S_un.S_addr = htonl(AdapterInfo.ulDefaultGatewayIPAddress);
#elif __linux__
	addr.s_addr = htonl(AdapterInfo.ulDefaultGatewayIPAddress);
#endif
	inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
	printf( "    Default GW IP address:\t\t%s\n", Ipaddr );
	printf( "    Number of Occupied Stations:\t%d\n", UserSlaveParameter.usOccupiedStationNumber );
	printf( "    Cyclic response wait time:\t\t%d[ms] (0:Not wait)\n", UserSlaveParameter.ulCyclicResponseWaitTime );

	/****************************************************************************/
	/* Initialization of the SLMP Server.										*/
	/****************************************************************************/
	/* Set the SLMP Server Information */
	ServerInfo.usVenderCode = Slave.usVenderCode;
	ServerInfo.ulModelCode = Slave.ulModelCode;
	ServerInfo.usMachineVersion = Slave.usMachineVersion;
	memcpy( &ServerInfo.aucMacAddress, AdapterInfo.aucMacAddress, sizeof( ServerInfo.aucMacAddress ) );
	ServerInfo.ulIpAddress = AdapterInfo.ulIpAddress;
	ServerInfo.ulSubnetMask = AdapterInfo.ulSubnetMask;
	ServerInfo.ulDefaultGatewayIPAddress = AdapterInfo.ulDefaultGatewayIPAddress;
	ServerInfo.usPortNumber = USER_SERVER_PORT_NUMBER;
#ifdef _WIN32
	strncpy_s( (char*)ServerInfo.acHostname, sizeof(ServerInfo.acHostname), USER_SERVER_HOSTNAME, 64 );
#elif __linux__
	strncpy( (char*)ServerInfo.acHostname, USER_SERVER_HOSTNAME, 64 );
#endif
	ServerInfo.usStatus = 0x0000;
	memcpy( &ServerInfo.acTypeName, USER_TYPE_NAME, sizeof(ServerInfo.acTypeName) );
	ServerInfo.usTypeNameCode = USER_TYPE_NAME_CODE;
	ServerInfo.pusMemory = ausUserMemory;
	ServerInfo.uiMemorySize = sizeof( ausUserMemory ) / sizeof( uint16_t );

	/* Initialization of the SLMP Server */
	iErrCode = slmp_server_initialize( &ServerInfo, &user_callback_set_ip_address_basic, &user_callback_parameter_get, &user_callback_parameter_set,
									   &user_callback_parameter_set_end, &user_callback_remote_reset );
	if ( iErrCode != SLMP_SERVER_ERR_OK )
	{
		printf( "\nSLMP Server initialization error!\n" );
		goto end;
	}

	/* Set the slave information data */
	ccief_basic_slave_set_err_code( USER_ERR_OK );
	ccief_basic_slave_set_unit_info( CCIEF_BASIC_UNIT_INFO_APPLICATION_STOP );
	ccief_basic_slave_set_unit_data( 0x00000000 );

	/* Show the information for display of the cyclic state */
	printf( "\n  Show the cyclic information at %d-second intervals.\n", ( USER_DISPLAY_INTERVAL_TIME / 1000 ));
	printf( "\n" );
	printf( "    Master | Cyclic Data | Link scan time | Time Data\n" );
	printf( "\n" );
	printf( "      Master         - The control master station.(Master ID / Group Number)\n" );
	printf( "      Cyclic Data    - Count of the receive Cyclic Data at intervals.\n" );
	printf( "      Link scan time - The link scan time at intervals.[ms]\n" );
	printf( "      Time Data      - The time from the master information data.\n" );
	printf( "\n" );

	/****************************************************************************/
	/* Main loop of sample code.												*/
	/****************************************************************************/
	while ( 1 )
	{
		/* CCIEF-BASIC Slave */
		iErrCode = ccief_basic_slave_main();
		if ( iErrCode != CCIEF_BASIC_SLAVE_ERR_OK )
		{
			break;
		}

		/* SLMP Server */
		iErrCode = slmp_server_main();
		if ( iErrCode != SLMP_SERVER_ERR_OK )
		{
			break;
		}

		/* Timer */
		timer_main();

		/* Check reset */
		if ( iReset == USER_RESET )
		{
			break;
		}

		/* Display of the cyclic information */
		user_display_cyclic_information();
	}

	/* Termination of CCIEF-BASIC Slave */
	ccief_basic_slave_terminate();

	/* Termination of SLMP Server */
	slmp_server_terminate();

	/* Termination of the timer environment */
	timer_terminate();

	/* Check the reset */
	if ( iReset == USER_RESET )
	{
		printf( "\nReset the unit!\n" );
		goto reset;
	}

	/* Exit the application */
end:
#ifdef _WIN32
	/* WinSock clean up */
	WSACleanup();
#elif __linux__
#endif
	printf( "\nApplication has exited.(please any press)\n" );
#ifdef _WIN32
	_getch();
#elif __linux__
	getchar();
#endif
	return;
}

/************************************************************************************/
/* This is an user defined function for callback function for receiving             */
/* the Cyclic Data from the master.	                                                */
/************************************************************************************/
void user_callback_recv_cyclic_data( int iCyclicState, int iOccupiedStationNumber )
{
	uint16_t *pusRWw, *pusRY, *pusRX, *pusRWr; 
	uint32_t ulTimerStart, ulTimerEnd;

	/*[ Example: Sample code is data transfer from receiving data to sending data. ]*/
	/* Check the cyclic state. */
	if ( iCyclicState == CCIEF_BASIC_CYCLIC_DATA_VALID )
	{
		/* Please write here receiving the cyclic data from the master. */

		/* Getting the start pointer of RWw */
		pusRWw = ccief_basic_slave_get_pointer( CCIEF_BASIC_DEVICE_TYPE_RWW );
		/* Getting the start pointer of RY */
		pusRY = ccief_basic_slave_get_pointer( CCIEF_BASIC_DEVICE_TYPE_RY );

		/* Getting the start pointer of RWr */
		pusRWr = ccief_basic_slave_get_pointer( CCIEF_BASIC_DEVICE_TYPE_RWR );
		/* Getting the start pointer of RX */
		pusRX = ccief_basic_slave_get_pointer( CCIEF_BASIC_DEVICE_TYPE_RX );

		/* Data transfer from RY to RX */
		memcpy( pusRX, pusRY, iOccupiedStationNumber * CCIEF_BASIC_RX_RY_SIZE );
		/* Data transfer from RWw to RWr */
		memcpy( pusRWr, pusRWw, iOccupiedStationNumber * CCIEF_BASIC_RWW_RWR_SIZE );
	}

	/* Increase the counter of Cyclic Data. */
	ulCyclicDataCount ++;

	/*[ Example: Wait for cyclic response for sample code.(for debug the slow scan time) ]*/ 
	if ( UserSlaveParameter.ulCyclicResponseWaitTime != 0 )
	{
		ulTimerStart = timer_get_time();
		while ( 1 )
		{
			ulTimerEnd = timer_get_time();
			if(ulTimerEnd < ulTimerStart)
			{
				ulTimerEnd = ulTimerEnd + (((int64_t)LONG_MAX * 1000 / CLOCKS_PER_SEC) - ulTimerStart);
				if ( UserSlaveParameter.ulCyclicResponseWaitTime < ulTimerEnd )
				{
					break;
				}
			}
			else
			{
				if ( UserSlaveParameter.ulCyclicResponseWaitTime < ( ulTimerEnd - ulTimerStart ))
				{
					break;
				}
			}
		}
	}

	/*[ Example: Set the unit information data. ]*/
	ccief_basic_slave_set_unit_info( CCIEF_BASIC_UNIT_INFO_APPLICATION_RUNNING );

	return;
}

/************************************************************************************/
/* This is an user defined function for cyclic disconnection timeout.				*/
/************************************************************************************/
void user_callback_cyclic_disconnection( void )
{
	/*[ Example: Set the unit information data. ]*/
	ccief_basic_slave_set_unit_info( CCIEF_BASIC_UNIT_INFO_APPLICATION_STOP );

	/* Please write for disconnection of cyclic operateion here */

	return;
}

/************************************************************************************/
/* This is an user defined function for callback function for set the ip address.	*/
/* For CC-Link IEF Basic use.														*/
/************************************************************************************/
void user_callback_set_ip_address_basic( uint32_t ulIpAddress, uint32_t ulSubnetMask )
{
	/* check the subnet mask */
	if ( ulSubnetMask == 0xFFFFFFFF )
	{
		/* Not setting */
		ulSubnetMask = 0;
	}

	/* set the ip address */
	AdapterInfo.ulIpAddress = ulIpAddress;
	AdapterInfo.ulSubnetMask = ulSubnetMask;
	UserSlaveParameter.ulIpAddress = ulIpAddress;
	UserSlaveParameter.ulSubnetMask = ulSubnetMask;

	/* Write the parameter file */
	(void)user_parameter_file_write( acParameterFilePath, &UserSlaveParameter );

	iReset = USER_RESET;
	return;
}

/************************************************************************************/
/* This is an user defined function for callback function for parameter get.		*/
/************************************************************************************/
int user_callback_parameter_get( uint16_t usId, uint16_t *pusSize, uint8_t **ppucData )
{
	struct in_addr addr;
	char *addrStr;
	char Ipaddr[16];

	/* Check the parameter id */
	switch ( usId )
	{
		/* Getting of the ip address */
		case USER_PARAMETER_ID_IP_ADDRESS:
#ifdef _WIN32
			addr.S_un.S_addr = htonl(UserSlaveParameter.ulIpAddress);
#elif __linux__
			addr.s_addr = htonl(UserSlaveParameter.ulIpAddress);
#endif
			inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
			addrStr = Ipaddr;
			*pusSize = (uint16_t)strlen( addrStr );
			*ppucData = (uint8_t*)addrStr;
			break;
		/* Getting of the subnet mask */
		case USER_PARAMETER_ID_SUBNET_MASK:
#ifdef _WIN32
			addr.S_un.S_addr = htonl(UserSlaveParameter.ulSubnetMask);
#elif __linux__
			addr.s_addr = htonl(UserSlaveParameter.ulSubnetMask);
#endif
			inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
			addrStr = Ipaddr;
			*pusSize = (uint16_t)strlen( addrStr );
			*ppucData = (uint8_t*)addrStr;
			break;
		/* Getting of the default gateway ip address */
		case USER_PARAMETER_ID_DEFAULT_GW_IP_ADDRESS:
#ifdef _WIN32
			addr.S_un.S_addr = htonl(UserSlaveParameter.ulDefaultGatewayIPAddress);
#elif __linux__
			addr.s_addr = htonl(UserSlaveParameter.ulDefaultGatewayIPAddress);
#endif
			inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
			addrStr = Ipaddr;
			*pusSize = (uint16_t)strlen( addrStr );
			*ppucData = (uint8_t*)addrStr;
			break;
		/* Getting of the occupied station number */
		case USER_PARAMETER_ID_OCCUPIED_STATION_NUMBER:
			*pusSize = sizeof( uint16_t );
			*ppucData = (uint8_t*)&UserSlaveParameter.usOccupiedStationNumber;
			break;
		/* Getting of the cyclic response wait time */
		case USER_PARAMETER_ID_CYCLIC_RESPONSE_WAIT_TIME:
			*pusSize = sizeof( uint16_t );
			*ppucData = (uint8_t*)&UserSlaveParameter.ulCyclicResponseWaitTime;
			break;

		/* Please write user parameters here. */

		default:
			return SLMP_ERR_NO_EXIST_PARAM_ID;
	}

	return SLMP_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for callback function for parameter set.		*/
/************************************************************************************/
int user_callback_parameter_set( uint16_t usId, uint16_t usSize, uint8_t *pucData )
{
	uint32_t	ulIpaddr;
	int			iResult;

	/* Check the parameter id */
	switch ( usId )
	{
		/* Getting of the ip address */
		case USER_PARAMETER_ID_IP_ADDRESS:
			iResult = inet_pton( AF_INET, (char*)pucData, &ulIpaddr );
			if( iResult == 1 )
			{
				UserSlaveParameter.ulIpAddress = htonl(ulIpaddr);
			}
			else
			{
				UserSlaveParameter.ulIpAddress = 0;
			}
			break;
		/* Getting of the subnet mask */
		case USER_PARAMETER_ID_SUBNET_MASK:
			inet_pton( AF_INET, (char*)pucData, &ulIpaddr );
			iResult = inet_pton( AF_INET, (char*)pucData, &ulIpaddr );
			if( iResult == 1 )
			{
				UserSlaveParameter.ulSubnetMask = htonl(ulIpaddr);
			}
			else
			{
				UserSlaveParameter.ulSubnetMask = 0;
			}
			break;
		/* Getting of the default gateway ip address */
		case USER_PARAMETER_ID_DEFAULT_GW_IP_ADDRESS:
			iResult = inet_pton( AF_INET, (char*)pucData, &ulIpaddr );
			if( iResult == 1 )
			{
				UserSlaveParameter.ulDefaultGatewayIPAddress = htonl(ulIpaddr);
			}
			else
			{
				UserSlaveParameter.ulDefaultGatewayIPAddress = 0;
			}
			break;
		/* Getting of the occupied station number */
		case USER_PARAMETER_ID_OCCUPIED_STATION_NUMBER:
			memcpy( &UserSlaveParameter.usOccupiedStationNumber, pucData, sizeof( UserSlaveParameter.usOccupiedStationNumber ) );
			break;
		/* Getting of the cyclic response wait time */
		case USER_PARAMETER_ID_CYCLIC_RESPONSE_WAIT_TIME:
			memcpy( &UserSlaveParameter.ulCyclicResponseWaitTime, pucData, sizeof( UserSlaveParameter.ulCyclicResponseWaitTime ) );
			break;

		/* Please write user parameters here. */

		default:
			return SLMP_ERR_NO_EXIST_PARAM_ID;
	}

	return SLMP_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for callback function for parameter set end.	*/
/************************************************************************************/
int user_callback_parameter_set_end( void )
{
	/* Write the parameter file */
	(void)user_parameter_file_write( acParameterFilePath, &UserSlaveParameter );

	/* Read the parameter file */
	iParameterFileRead = USER_PARAMETER_FILE_READ;

	return SLMP_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for callback function for remote reset			*/
/************************************************************************************/
int user_callback_remote_reset( void )
{
	/* Reset the operation */
	iReset = USER_RESET;

	return SLMP_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for read of the parameter file (csv format).	*/
/************************************************************************************/
int user_parameter_file_read( char *file_path, USER_SLAVE_PARAMETER *pParameter, USER_ADAPTER_INFO *pAdapterInfo )
{
	uint32_t ulIpaddr;
	FILE *fp;
#ifdef _WIN32
	errno_t error;
#elif __linux__
#endif
	static char aucLine[32];
	static char aucId[32];
	static char aucData[32];
	int iId, iResult;
	int iErrCode = USER_ERR_OK;

#ifdef _WIN32
	error = fopen_s( &fp, file_path, "r" );
	if ( error != 0 )
#elif __linux__
	fp = fopen( file_path, "r" );
	if ( fp == NULL )
#endif
	{
		printf( "Could not open file \"%s\"\n", file_path );
		return USER_ERR_NG;
	}

	/* Start analyze the parameter file */
	while ( fgets( aucLine, sizeof( aucLine ), fp ) != NULL )
	{
		memset( aucId, 0, sizeof( aucId ) );
		memset( aucData, 0, sizeof( aucData ) );
#ifdef _WIN32
		sscanf_s( aucLine, "%[^,],%[^,]", aucId, sizeof(aucId), aucData, sizeof(aucData) );
#elif __linux__
		sscanf( aucLine, "%[^,],%[^,]", aucId, aucData );
#endif
		iId = strtol( aucId, NULL, 0 );
		switch ( iId )
		{
			case USER_PARAMETER_ID_IP_ADDRESS:
				iResult = inet_pton( AF_INET, aucData, &ulIpaddr );
				if ( iResult == 1 )
				{
					pParameter->ulIpAddress = htonl(ulIpaddr);
				}
				/* Check the ip address */
				if ( pParameter->ulIpAddress != 0 )
				{
					/* Setting the adapter ip address */
					pAdapterInfo->ulIpAddress = pParameter->ulIpAddress;
				}
				break;
			case USER_PARAMETER_ID_SUBNET_MASK:
				iResult = inet_pton( AF_INET, aucData, &ulIpaddr );
				if ( iResult == 1 )
				{
					pParameter->ulSubnetMask = htonl(ulIpaddr);
				}
				/* Check the subnet mask */
				if ( pParameter->ulSubnetMask != 0 )
				{
					/* Setting the adapter subnet mask */
					pAdapterInfo->ulSubnetMask = pParameter->ulSubnetMask;
				}
				break;
			case USER_PARAMETER_ID_DEFAULT_GW_IP_ADDRESS:
				iResult = inet_pton( AF_INET, aucData, &ulIpaddr );
				if ( iResult == 1 )
				{
					pParameter->ulDefaultGatewayIPAddress = htonl(ulIpaddr);
				}
					/* Check the default gateway ip address */
				if ( pParameter->ulDefaultGatewayIPAddress != 0 )
				{
					/* Setting the adapter default gateway ip address */
					pAdapterInfo->ulDefaultGatewayIPAddress = pParameter->ulDefaultGatewayIPAddress;
				}
				break;
			case USER_PARAMETER_ID_OCCUPIED_STATION_NUMBER:
				pParameter->usOccupiedStationNumber = (uint16_t)strtoul( aucData, NULL, 0 );
				/* Check the occupied station number */
				if (( pParameter->usOccupiedStationNumber <= 0 ) || ( CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP < pParameter->usOccupiedStationNumber ))
				{
					printf( "Parameter Error: Invalid number of occupied stations! %d (range: 1-64)\n", pParameter->usOccupiedStationNumber );
					iErrCode = USER_ERR_NG;
					goto end;
				}
				break;
			case USER_PARAMETER_ID_CYCLIC_RESPONSE_WAIT_TIME:
				pParameter->ulCyclicResponseWaitTime = strtoul( aucData, NULL, 0 );
				break;

			/* Please write user parameters here. */

			default:
				break;
		}
	}

end:
	fclose( fp );

	return USER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for write of the parameter file (csv format).	*/
/************************************************************************************/
int user_parameter_file_write( char *file_path, USER_SLAVE_PARAMETER *pParameter )
{
	static char aucLine[1024];
	struct in_addr addr;
	FILE *fp;
#ifdef _WIN32
	errno_t error;
#elif __linux__
#endif
	char Ipaddr[16];

#ifdef _WIN32
	error = fopen_s( &fp, file_path, "w" );
	if ( error != 0 )
#elif __linux__
	fp = fopen( file_path, "w" );
	if ( fp == NULL )
#endif
	{
		printf( "Could not open file \"%s\"\n", file_path );
		return USER_ERR_NG;
	}

#ifdef _WIN32
	sprintf_s( aucLine, sizeof(aucLine), ",,\n" );
#elif __linux__
	sprintf( aucLine,  ",,\n" );
#endif
	fputs( aucLine, fp );
#ifdef _WIN32
	sprintf_s( aucLine, sizeof(aucLine), "CCIEF-SIM Slave Sample Parameter,,\n" );
#elif __linux__
	sprintf( aucLine, "CCIEF-SIM Slave Sample Parameter,,\n" );
#endif
	fputs( aucLine, fp );
#ifdef _WIN32
	sprintf_s( aucLine, sizeof(aucLine), ",,\n" );
#elif __linux__
	sprintf( aucLine, ",,\n" );
#endif
	fputs( aucLine, fp );
#ifdef _WIN32
	sprintf_s( aucLine,  sizeof(aucLine),"ID,DATA,COMMENT\n" );
#elif __linux__
	sprintf( aucLine,  "ID,DATA,COMMENT\n" );
#endif
	fputs( aucLine, fp );

	if ( pParameter->ulIpAddress != 0 )
	{
#ifdef _WIN32
		addr.S_un.S_addr = htonl(pParameter->ulIpAddress);
#elif __linux__
		addr.s_addr = htonl(pParameter->ulIpAddress);
#endif
		inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
#ifdef _WIN32
		sprintf_s( aucLine,  sizeof(aucLine), "1,%s,IP address\n", Ipaddr );
#elif __linux__
		sprintf( aucLine, "1,%s,IP address\n", Ipaddr );
#endif
	}
	else
	{
#ifdef _WIN32
		sprintf_s( aucLine, sizeof(aucLine), "1,0,IP address\n" );
#elif __linux__
		sprintf( aucLine, "1,0,IP address\n" );
#endif
	}
	fputs( aucLine, fp );
	if ( pParameter->ulSubnetMask != 0 )
	{
#ifdef _WIN32
		addr.S_un.S_addr = htonl(pParameter->ulSubnetMask);
#elif __linux__
		addr.s_addr = htonl(pParameter->ulSubnetMask);
#endif
		inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
#ifdef _WIN32
		sprintf_s( aucLine, sizeof(aucLine), "2,%s,Subnet mask\n", Ipaddr );
#elif __linux__
		sprintf( aucLine, "2,%s,Subnet mask\n", Ipaddr );
#endif
	}
	else
	{
#ifdef _WIN32
		sprintf_s( aucLine, sizeof(aucLine), "2,0,Subnet mask\n" );
#elif __linux__
		sprintf( aucLine, "2,0,Subnet mask\n" );
#endif
	}
	fputs( aucLine, fp );
	if ( pParameter->ulDefaultGatewayIPAddress != 0 )
	{
#ifdef _WIN32
		addr.S_un.S_addr = htonl(pParameter->ulDefaultGatewayIPAddress);
#elif __linux__
		addr.s_addr = htonl(pParameter->ulDefaultGatewayIPAddress);
#endif
		inet_ntop(AF_INET, &addr, Ipaddr, sizeof(Ipaddr) );
#ifdef _WIN32
		sprintf_s( aucLine, sizeof(aucLine), "3,%s,Default gateway IP address\n", Ipaddr );
#elif __linux__
		sprintf( aucLine, "3,%s,Default gateway IP address\n", Ipaddr );
#endif
	}
	else
	{
#ifdef _WIN32
		sprintf_s( aucLine, sizeof(aucLine), "3,0,Default gateway IP address\n" );
#elif __linux__
		sprintf( aucLine, "3,0,Default gateway IP address\n" );
#endif
	}
	fputs( aucLine, fp );
#ifdef _WIN32
	sprintf_s( aucLine, sizeof(aucLine), "4,%d,Number of occupied stations\n", pParameter->usOccupiedStationNumber );
#elif __linux__
	sprintf( aucLine, "4,%d,Number of occupied stations\n", pParameter->usOccupiedStationNumber );
#endif
	fputs( aucLine, fp );
#ifdef _WIN32
	sprintf_s( aucLine, sizeof(aucLine), "5,%lu,Cyclic response wait time\n", pParameter->ulCyclicResponseWaitTime );
#elif __linux__
	sprintf( aucLine, "5,%lu,Cyclic response wait time\n", pParameter->ulCyclicResponseWaitTime );
#endif
	fputs( aucLine, fp );

	/* Please write user parameters here. */

	fclose( fp );

	return USER_ERR_OK;
}

/************************************************************************************/
/* This is an user defined function for display the cyclic state.					*/
/************************************************************************************/
void user_display_cyclic_information( void )
{
	static CCIEF_BASIC_SLAVE_MASTER_INFO	MasterInfo;
	TIMER_TIME_DATA						TimeData;
	static uint32_t						ulDisplayTimerStart = 0;	/* Start time of the display timer */
	uint32_t							ulDisplayTimerEnd;			/* End time of the display timer */
	int64_t								llTimeData;
	uint32_t 							ulInterval;


	if ( ulDisplayTimerStart == 0 ) {
		ulDisplayTimerStart = timer_get_time();
	}

	ulDisplayTimerEnd = timer_get_time();
	if(ulDisplayTimerEnd < ulDisplayTimerStart)
	{
		ulInterval = ulDisplayTimerEnd + (((int64_t)LONG_MAX * 1000 / CLOCKS_PER_SEC) - ulDisplayTimerStart);
	}
	else
	{
		ulInterval = ulDisplayTimerEnd - ulDisplayTimerStart;
	}

	/* Check the display time. */
	if ( USER_DISPLAY_INTERVAL_TIME < ulInterval )
	{
		ulDisplayTimerStart = ulDisplayTimerEnd;

		/* Getting the master information */
		ccief_basic_slave_get_master_info( &MasterInfo );

		/* Show the current cyclic state */
		if ( MasterInfo.ulId == 0 )
		{
			printf( "CCIEF-BASIC: Master(Unknown) | Cyclic Data=0 | 0.000[ms] |\n" );
		}
		else
		{
			if ( ulCyclicDataCount == 0 )
			{
				printf( "CCIEF-BASIC: Master(0x%08lX/%d) | Cyclic Data=%lu | 0.000[ms] |\n", MasterInfo.ulId,
						ulCyclicDataCount, MasterInfo.ucGroupNumber );
			}
			else
			{
				printf( "CCIEF-BASIC: Master(0x%08lX/%d) | Cyclic Data=%lu | %01.3f[ms] | ", MasterInfo.ulId,
						MasterInfo.ucGroupNumber, ulCyclicDataCount, ((float)USER_DISPLAY_INTERVAL_TIME / ulCyclicDataCount) );
				memcpy( &llTimeData, MasterInfo.NotifyInfo.aucTimeData, sizeof( llTimeData ) );
				timer_analyze_time_data( llTimeData, &TimeData );
				printf( "%d-%02d-%02d %02d:%02d:%02d\n", TimeData.usYear, TimeData.usMonth, TimeData.usDay, TimeData.usHour,
						TimeData.usMinute, TimeData.usSecond );
			}
		}

		/* Clear the count of cyclic data */
		ulCyclicDataCount = 0;
	}

	return;
}

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

/************************************************************************************/
/* This is an user defined function for get the adapter information.				*/
/* The following is one of a sample in the Windows OS. Please rewrite if necessary.	*/
/* The following is one of a sample in the Linux. Please rewrite if necessary.		*/
/************************************************************************************/
int user_get_adapter_info( USER_ADAPTER_INFO *pGetAdapterInfo )
{
#ifdef _WIN32
	ULONG			ulOutBufLen;
	ULONG			ulIpaddr, ulSubNetMask, ulDefGateway;
	DWORD			dwRetVal;
	IP_ADAPTER_INFO *pAdapterInfo;
	IP_ADAPTER_INFO *pAdapter;
	int				iAdapterNum;
	int				iKey;
	int				iInputNumber;
	int				i;
	int				iResult;

	/* GetAdaptersInfo */
	pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof( IP_ADAPTER_INFO ));
	if ( pAdapterInfo == NULL )
	{
		printf( "Error allocating memory needed to call GetAdapterInfo\n" );
		return USER_ERR_NG;
	}
	ulOutBufLen = sizeof( IP_ADAPTER_INFO );

	if ( GetAdaptersInfo( pAdapterInfo, &ulOutBufLen ) == ERROR_BUFFER_OVERFLOW )
	{
		FREE( pAdapterInfo );
		pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC( ulOutBufLen );
		if ( pAdapterInfo == NULL )
		{
			printf( "Error allocating memory needed to call GetAdapterInfo\n" );
			return USER_ERR_NG;
		}
	}

	if (( dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen )) != NO_ERROR )
	{
		printf( "GetAdaptersInfo failed with error %d\n", dwRetVal );
		if ( pAdapterInfo )
		{
			FREE( pAdapterInfo );
		}
		return USER_ERR_NG;
	}

	pAdapter = pAdapterInfo;
	iAdapterNum = 0;

	/* Check the Multiple Adapter */
	if ( pAdapter->Next != NULL )
	{
		printf( "Found the multiple Network adapter!\n\n");
		while ( pAdapter )
		{
			printf( "%2d: \tAdapter desc: \t\t%s\n", iAdapterNum+1, pAdapter->Description );
			printf( "\tMAC address: \t\t%02X:%02X:%02X:%02X:%02X:%02X\n",
					pAdapter->Address[0], pAdapter->Address[1], pAdapter->Address[2],
					pAdapter->Address[3], pAdapter->Address[4], pAdapter->Address[5] );
			printf( "\tIP address: \t\t%s\n",
					pAdapter->IpAddressList.IpAddress.String );
			printf( "\tSubnet mask: \t\t%s\n", pAdapter->IpAddressList.IpMask.String );
			printf( "\tDefault GW IP address: \t%s\n", pAdapter->GatewayList.IpAddress.String );
			printf( "\n");
			iAdapterNum ++;
			pAdapter = pAdapter->Next;
		}
		printf( "Please select the adapter number (Press 'Enter' Key after select) [1-%d]: ", iAdapterNum );

		iInputNumber = 0;

		while ( 1 )
		{
			iKey = _getch();
			if ( iKey != 0x0d )
			{
				if (( 0x30 <= iKey ) && ( iKey <= 0x39 ))
				{
					printf( "%c",iKey);
					iInputNumber = (iInputNumber*10)+(iKey-0x30);
				}
			}
			else
			{
				printf( "\n");
				if (( iInputNumber == 0 ) || ( iAdapterNum < iInputNumber ))
				{
					printf( "Wrong number select!\n");
					iInputNumber = 0;
					printf( "Please select the adapter number (Press 'Enter' Key after select) [1-%d]: ", iAdapterNum );
				}
				else
				{
					break;
				}
			}
		}
		pAdapter = pAdapterInfo;
		for ( i = 1; i < iInputNumber; i++ )
		{
			pAdapter = pAdapter->Next;
		}
	}
	else
	{
		iAdapterNum = 1;
	}
	printf( "\n" );

	/* Conversion of endian */
	AdapterInfo.aucMacAddress[0] = pAdapter->Address[5];
	AdapterInfo.aucMacAddress[1] = pAdapter->Address[4];
	AdapterInfo.aucMacAddress[2] = pAdapter->Address[3];
	AdapterInfo.aucMacAddress[3] = pAdapter->Address[2];
	AdapterInfo.aucMacAddress[4] = pAdapter->Address[1];
	AdapterInfo.aucMacAddress[5] = pAdapter->Address[0];
	iResult = inet_pton( AF_INET, pAdapter->IpAddressList.IpAddress.String, &ulIpaddr );
	if ( iResult == 1)
	{
		AdapterInfo.ulIpAddress = htonl(ulIpaddr);
	}
	else
	{
		printf("Adapter IP address error.\n");
		return USER_ERR_NG;
	}
	iResult = inet_pton( AF_INET, pAdapter->IpAddressList.IpMask.String, &ulSubNetMask );
	if ( iResult == 1)
	{
		AdapterInfo.ulSubnetMask = htonl(ulSubNetMask);
	}
	else
	{
		printf("Adapter sub-net mask error.\n");
		return USER_ERR_NG;
	}
	iResult = inet_pton( AF_INET, pAdapter->GatewayList.IpAddress.String, &ulDefGateway );
	if ( iResult == 1)
	{
		AdapterInfo.ulDefaultGatewayIPAddress = htonl(ulDefGateway);
	}
	else
	{
		printf("Adapter default gateway IP address error.\n");
		return USER_ERR_NG;
	}

	FREE( pAdapterInfo );

	/* Copy the argument pointer */
	memcpy( pGetAdapterInfo, &AdapterInfo, sizeof(USER_ADAPTER_INFO) );

	return USER_ERR_OK;
#elif __linux__
	struct ifconf			Ifc_Get;
	struct ifreq			Ifreq_Size[MAX_INTERFACE];
	struct sockaddr_in 		get_addr[MAX_INTERFACE];
	USER_NETWORK_INFO 		User_Adapter[MAX_INTERFACE];
	USER_NETWORK_INFO 		User_Get_Info;
	unsigned char			*pMAC_addr;
	int						sock_network;
	int						i;
	int						ikey;
	int						iNet_Number=0;
	FILE					*pFile;
	t_RouteInfo				RouteInfo_Defo[MAX_INTERFACE];
	char					input_line[128];

	Ifc_Get.ifc_len = sizeof( Ifreq_Size );
	Ifc_Get.ifc_ifcu.ifcu_buf=(void *)Ifreq_Size;

	/* Socket open */
	sock_network = socket( AF_INET, SOCK_DGRAM, 0 );
	if( sock_network < SOCKET_NOT_OPEN )
	{
		perror("Socket not open");
		return USER_ERR_NG;
	}

	/* Network interface list acquisition */
	if( ioctl( sock_network, SIOCGIFCONF, &Ifc_Get ) == -1 )
	{
		perror("Error can not get the interface list");
		close( sock_network );
		return USER_ERR_NG;
	}

	/* Calculate the number that came back from the kernel */
	iNet_Number = Ifc_Get.ifc_len / ( int )sizeof( struct ifreq );
	for(i=0;i<iNet_Number;i++)
	{
		/* Interface designation */
		strncpy( User_Adapter[i].USER_ifreq.ifr_name,Ifreq_Size[i].ifr_name,IFNAMSIZ-1 );

		/* IP address (IPv4) acquisition */
		if ( ioctl( sock_network, SIOCGIFADDR, &User_Adapter[i].USER_ifreq ) == -1 )
		{
			perror("Error can not get the IP address");
			close( sock_network );
			return USER_ERR_NG;
		}
		memcpy( &get_addr[i], &User_Adapter[i].USER_ifreq.ifr_addr, sizeof( struct sockaddr_in ));
		memcpy( &User_Adapter[i].IPAddr, inet_ntoa( get_addr[i].sin_addr ), sizeof( User_Adapter[i].IPAddr ));

		/* Mask address acquisition */
		if ( ioctl( sock_network, SIOCGIFNETMASK, &User_Adapter[i].USER_ifreq ) == -1 )
		{
			perror("Error can not get the Mask address");
			close( sock_network );
			return USER_ERR_NG;
		}
		memcpy( &get_addr[i], &User_Adapter[i].USER_ifreq.ifr_netmask, sizeof( struct sockaddr_in )); 
		memcpy( &User_Adapter[i].IPMask, inet_ntoa( get_addr[i].sin_addr ), sizeof( User_Adapter[i].IPMask ));

		/* Mac address acquisition */
		if ( ioctl( sock_network, SIOCGIFHWADDR, &User_Adapter[i].USER_ifreq ) == -1 )
		{
			perror("Error can not get the Mac address");
			close( sock_network );
			return USER_ERR_NG;
		}
		pMAC_addr=(unsigned char *)&User_Adapter[i].USER_ifreq.ifr_hwaddr.sa_data;
		sprintf( User_Adapter[i].MACAddr, "%02x:%02x:%02x:%02x:%02x:%02x", *pMAC_addr, *(pMAC_addr+1), *(pMAC_addr+2), *(pMAC_addr+3), *(pMAC_addr+4), *(pMAC_addr+5));

		/* Get the default gateway from the system file */
		pFile = fopen( DIR_PROC_ROUTE , "r" );
		if( pFile == NULL )
		{
			printf("Not systemfile read");
			close( sock_network );
			return USER_ERR_NG;
		}

		while(1)
		{
			if ( fgets( User_Adapter[i].Defo, sizeof( User_Adapter[i].Defo ), pFile ) == 0) 
			{
				memset( User_Adapter[i].Defo,0x00,sizeof( User_Adapter[i].Defo ));
				fclose( pFile );
				break;
			}
			//Route information acquisition
			memset( &RouteInfo_Defo[i], 0x00, sizeof( RouteInfo_Defo[i] ));
			if ( sscanf( User_Adapter[i].Defo, "%s%x%x%d%d%d%d%x%d%d%d",
				&RouteInfo_Defo[i].cIface,
				&RouteInfo_Defo[i].cDst,
				&RouteInfo_Defo[i].cGateway,
				&RouteInfo_Defo[i].iFlag,
				&RouteInfo_Defo[i].iRecCnt,
				&RouteInfo_Defo[i].iUse,
				&RouteInfo_Defo[i].iMetric,
				&RouteInfo_Defo[i].cMask,
				&RouteInfo_Defo[i].iMTU,
				&RouteInfo_Defo[i].iWindow,
				&RouteInfo_Defo[i].iIRTT) == 0 )
			{
				printf("Not file read");
				fclose( pFile );
				close( sock_network );
				return USER_ERR_NG;
			}
			/* device name and default GW flag check */
			if (( strstr( RouteInfo_Defo[i].cIface,User_Adapter[i].USER_ifreq.ifr_name ) != NULL ) && ( RouteInfo_Defo[i].iFlag == ( RTF_UP | RTF_GATEWAY )))
			{
				memcpy( &User_Adapter[i].Defo, inet_ntoa( RouteInfo_Defo[i].cGateway ), INET_ADDRSTRLEN );
				fclose( pFile );
				break;
			}
			memset( User_Adapter[i].Defo,0x00,sizeof( User_Adapter[i].Defo ));
		}
	}
	
	
	
	for(i=0;i<iNet_Number;i++)
	{
		printf( "%2d: \tAdapter desc: \t\t%s\n", i+1, User_Adapter[i].USER_ifreq.ifr_name );
		printf( "\tMAC address: \t\t%s\n", User_Adapter[i].MACAddr );
		printf( "\tIP address: \t\t%s\n",User_Adapter[i].IPAddr );
		printf( "\tSubnet mask: \t\t%s\n", User_Adapter[i].IPMask );
		printf( "\tDefault GW IP address: \t%s\n", User_Adapter[i].Defo );
		printf( "\n");
	}
	printf( "Please select the adapter number (Press 'enter' Key after select) [1-%d]: ", iNet_Number );
	while ( 1 )
	{
		fgets( input_line,sizeof( input_line ),stdin );
		sscanf( input_line,"%d",&ikey );
		if (( 1 > ikey ) || ( iNet_Number < ikey ))
		{
			printf( "Wrong number select!\n");
			printf( "Please select the adapter number (Press 'Enter' Key after select) [1-%d]: ", iNet_Number );
		}
		else
		{
			break;
		}
	}

	User_Get_Info = User_Adapter[ikey-1];

	/* Conversion of endian */
	AdapterInfo.aucMacAddress[0] = ( uint8_t ) User_Get_Info.USER_ifreq.ifr_hwaddr.sa_data[5];
	AdapterInfo.aucMacAddress[1] = ( uint8_t ) User_Get_Info.USER_ifreq.ifr_hwaddr.sa_data[4];
	AdapterInfo.aucMacAddress[2] = ( uint8_t ) User_Get_Info.USER_ifreq.ifr_hwaddr.sa_data[3];
	AdapterInfo.aucMacAddress[3] = ( uint8_t ) User_Get_Info.USER_ifreq.ifr_hwaddr.sa_data[2];
	AdapterInfo.aucMacAddress[4] = ( uint8_t ) User_Get_Info.USER_ifreq.ifr_hwaddr.sa_data[1];
	AdapterInfo.aucMacAddress[5] = ( uint8_t ) User_Get_Info.USER_ifreq.ifr_hwaddr.sa_data[0];
	AdapterInfo.ulIpAddress = htonl( inet_addr( User_Get_Info.IPAddr ));
	AdapterInfo.ulSubnetMask = htonl( inet_addr( User_Get_Info.IPMask ));
	AdapterInfo.ulDefaultGatewayIPAddress = htonl(inet_addr( User_Get_Info.Defo ));

	close( sock_network );

	return USER_ERR_OK;
#endif
}

/************************************************************************************/
/* This is an user defined function for set the ip address.							*/
/* The following is one of a sample in the Windows OS. Please rewrite if necessary.	*/
/* The following is one of a sample in the Linux. Please rewrite if necessary. 		*/
/************************************************************************************/
int user_set_adapter_info( USER_ADAPTER_INFO *pSetAdapterInfo )
{
#ifdef _WIN32
	static char				str[2000];
	static char				szAdapterName[BUFSIZ];
	uint8_t					aucMacAddress[6];
	PIP_ADAPTER_ADDRESSES	pAdapterAddresses, pAdapterAddress;
	DWORD					dwRet, dwSize;
	int						len;
	ULONG					ulOutBufLen;
	ULONG					ulIpaddr, ulSubNetMask, ulDefGateway;
	DWORD					dwRetVal;
	IP_ADAPTER_INFO			*pAdapterInfo;
	IP_ADAPTER_INFO			*pAdapter;
	static int				iKey, iFirst = 0;
	struct in_addr			addrIpAddress, addrSubnetMask, addrDefaultGatewayIPAddress;
	char					Ipaddr[16],SubNetMask[16],Gateway[16];

	/* GetAdaptersAddresses */
	dwRet = GetAdaptersAddresses( AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &dwSize );
	if ( dwRet != ERROR_BUFFER_OVERFLOW )
	{
		printf( "Error call GetAdaptersAddresses\n" );
		return USER_ERR_NG;
	}

	pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)MALLOC( dwSize );
	if (pAdapterAddresses == NULL)
	{
		printf( "Error allocating memory needed to call GetAdaptersAddresses\n" );
		return USER_ERR_NG;
	}

	dwRet = GetAdaptersAddresses( AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAdapterAddresses, &dwSize );
	if (dwRet != ERROR_SUCCESS)
	{
		printf( "Error call GetAdaptersAddresses\n" );
		return USER_ERR_NG;
	}

	/* Conversion of endian */
	aucMacAddress[0] = pSetAdapterInfo->aucMacAddress[5];
	aucMacAddress[1] = pSetAdapterInfo->aucMacAddress[4];
	aucMacAddress[2] = pSetAdapterInfo->aucMacAddress[3];
	aucMacAddress[3] = pSetAdapterInfo->aucMacAddress[2];
	aucMacAddress[4] = pSetAdapterInfo->aucMacAddress[1];
	aucMacAddress[5] = pSetAdapterInfo->aucMacAddress[0];

	/* Get Adapter Name */
	for ( pAdapterAddress = pAdapterAddresses; pAdapterAddress; pAdapterAddress = pAdapterAddress->Next )
	{
		if ( memcmp( pAdapterAddress->PhysicalAddress, aucMacAddress, pAdapterAddress->PhysicalAddressLength ) == 0 )
		{
			len = WideCharToMultiByte(CP_ACP, 0, 
									  pAdapterAddress->FriendlyName, wcslen(pAdapterAddress->FriendlyName),
									  szAdapterName, sizeof( szAdapterName ), NULL, NULL);
			if ( len == 0 )
			{
				printf( "Error Adapter Name\n" );
				return USER_ERR_NG;
			}

			szAdapterName[len] = '\0';
			break;
		}
	}

	FREE( pAdapterAddresses );

	/* GetAdaptersInfo */
	pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof( IP_ADAPTER_INFO ));
	if ( pAdapterInfo == NULL )
	{
		printf( "Error allocating memory needed to call GetAdapterInfo\n" );
		return USER_ERR_NG;
	}
	ulOutBufLen = sizeof( IP_ADAPTER_INFO );

	if ( GetAdaptersInfo( pAdapterInfo, &ulOutBufLen ) == ERROR_BUFFER_OVERFLOW )
	{
		FREE( pAdapterInfo );
		pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC( ulOutBufLen );
		if ( pAdapterInfo == NULL )
		{
			printf( "Error allocating memory needed to call GetAdapterInfo\n" );
			return USER_ERR_NG;
		}
	}

	if (( dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen )) != NO_ERROR )
	{
		printf( "GetAdaptersInfo failed with error %d\n", dwRetVal );
		if ( pAdapterInfo )
		{
			FREE( pAdapterInfo );
		}
		return USER_ERR_NG;
	}

	pAdapter = pAdapterInfo;

	/* Check the adapter */
	if ( pAdapter != NULL )
	{
		while ( pAdapter )
		{
			if ( memcmp( pAdapter->Address, aucMacAddress, pAdapter->AddressLength ) == 0 )
			{
				inet_pton( AF_INET, pAdapter->IpAddressList.IpAddress.String, &ulIpaddr );
				inet_pton( AF_INET, pAdapter->IpAddressList.IpMask.String, &ulSubNetMask );
				inet_pton( AF_INET, pAdapter->GatewayList.IpAddress.String, &ulDefGateway );

				if (( pSetAdapterInfo->ulIpAddress == htonl(ulIpaddr))
				 && ( pSetAdapterInfo->ulSubnetMask == htonl(ulSubNetMask))
				 && ( pSetAdapterInfo->ulDefaultGatewayIPAddress == htonl(ulDefGateway)))
				{
					/* Nothing */
				}
				else
				{
					addrIpAddress.S_un.S_addr = htonl(pSetAdapterInfo->ulIpAddress);
					inet_ntop(AF_INET, &addrIpAddress, Ipaddr, sizeof(Ipaddr) );
					addrSubnetMask.S_un.S_addr = htonl(pSetAdapterInfo->ulSubnetMask);
					inet_ntop(AF_INET, &addrSubnetMask, SubNetMask, sizeof(SubNetMask) );
					addrDefaultGatewayIPAddress.S_un.S_addr = htonl(pSetAdapterInfo->ulDefaultGatewayIPAddress);
					inet_ntop(AF_INET, &addrDefaultGatewayIPAddress, Gateway, sizeof(Gateway) );
					/* Check the setting first */
					if ( iFirst == 0 )
					{
						printf( "Change the Network adapter setting ?\n" );
						printf( "\n    ""\"%s\"""\n", szAdapterName );
						printf( "\tIP address:\t\t%s --> %s\n", pAdapter->IpAddressList.IpAddress.String, Ipaddr );
						printf( "\tSubnet mask:\t\t%s --> %s\n", pAdapter->IpAddressList.IpMask.String, SubNetMask );
						printf( "\tDefault GW IP address:\t%s --> %s\n", pAdapter->GatewayList.IpAddress.String, Gateway );
						while ( 1 )
						{
							printf( "\nPlease Press Key if change the Network adapter setting Yes('Y') or No('N')\n" );
							iKey = _getch();
							if (( iKey == 'Y' ) || ( iKey == 'y' ))
							{
								break;
							}
							else if (( iKey == 'N' ) || ( iKey == 'n' ))
							{
								FREE( pAdapterInfo );

								return USER_ERR_NG;
							}
						}
					}
					/* Change the Network adapter setting */
					sprintf_s( str, sizeof(str), "\nnetsh interface ip set address ""\"%s\""" static addr=%s mask=",
							 szAdapterName, Ipaddr );
					strcat_s( str, sizeof(str), SubNetMask );
					if( addrDefaultGatewayIPAddress.S_un.S_addr != 0 )
					{
						strcat_s( str, sizeof(str), " gateway=" );
						strcat_s( str, sizeof(str), Gateway );
					}
					strcat_s( str, sizeof(str), " gwmetric=1" );
					printf( str );
					system( str );

					/* Wait for changing the ip address */
					Sleep(200);
					NotifyAddrChange( NULL, NULL );
				}
				iFirst = 1;
				break;
			}
			pAdapter = pAdapter->Next;
		}
	}

	FREE( pAdapterInfo );

	return USER_ERR_OK;
#elif __linux__
	static struct ifconf	Ifc_Get;
	struct ifreq 			Ifreq_Size[MAX_INTERFACE];
	struct in_addr			addrip_get,addrMask_get,addrdg_get;
	struct in_addr			addrip_set,addrMask_set,addrdg_set;
	struct ifreq			ifr_set;
	struct sockaddr_in		*s_in;
	struct sockaddr_in 		get_addr;
	USER_NETWORK_INFO 		User_Network[MAX_INTERFACE];
	USER_ADAPTER_INFO		Get_NetworkInfo;
	FILE					*pFile;
	t_RouteInfo				RouteInfo_Defo;
	int						sock_network;
	int						iErrCode;
	int						i;
	int						iNet_Number;
	int 					Interface_Number;
	int						NotGateway=0;
	char 					system_call[256];
	char					iKey;
	uint8_t					aucUser_MacAddress[6];
	unsigned char			*pMAC_addr;

	Interface_Number=0;

	Ifc_Get.ifc_len = sizeof( Ifreq_Size );
	Ifc_Get.ifc_ifcu.ifcu_buf=(void *)Ifreq_Size;

	sock_network = socket( AF_INET, SOCK_DGRAM, 0 );
	if( sock_network < SOCKET_NOT_OPEN )
	{
		perror("Socket not open");
		return USER_ERR_NG;
	}

	if( ioctl ( sock_network, SIOCGIFCONF, &Ifc_Get ) == -1 )
	{
		perror("Error can not get the interface list");
		close( sock_network );
		return USER_ERR_NG;
	}

	/* Calculate the number that came back from the kernel */
	iNet_Number = Ifc_Get.ifc_len / ( int )sizeof( struct ifreq );

	for(i=0;i<iNet_Number;i++)
	{
		/* Interface specification */
		strncpy ( User_Network[i].USER_ifreq.ifr_name,Ifreq_Size[i].ifr_name,IFNAMSIZ-1 );

		/* Mac address acquisition */
		if( ioctl ( sock_network, SIOCGIFHWADDR, &User_Network[i].USER_ifreq ) == -1 )
		{
			perror("Error can not get the Mac address");
			close( sock_network );
			return USER_ERR_NG;
		}

		pMAC_addr=( unsigned char *)&User_Network[i].USER_ifreq.ifr_hwaddr.sa_data;
		sprintf( User_Network[i].MACAddr, "%02x:%02x:%02x:%02x:%02x:%02x", *pMAC_addr, *( pMAC_addr+1 ), *( pMAC_addr+2 ), *( pMAC_addr+3 ), *( pMAC_addr+4 ), *( pMAC_addr+5 ));
		aucUser_MacAddress[0] = ( uint8_t ) User_Network[i].USER_ifreq.ifr_hwaddr.sa_data[5];
		aucUser_MacAddress[1] = ( uint8_t ) User_Network[i].USER_ifreq.ifr_hwaddr.sa_data[4];
		aucUser_MacAddress[2] = ( uint8_t ) User_Network[i].USER_ifreq.ifr_hwaddr.sa_data[3];
		aucUser_MacAddress[3] = ( uint8_t ) User_Network[i].USER_ifreq.ifr_hwaddr.sa_data[2];
		aucUser_MacAddress[4] = ( uint8_t ) User_Network[i].USER_ifreq.ifr_hwaddr.sa_data[1];
		aucUser_MacAddress[5] = ( uint8_t ) User_Network[i].USER_ifreq.ifr_hwaddr.sa_data[0];

		if( memcmp(pSetAdapterInfo->aucMacAddress,aucUser_MacAddress,sizeof( aucUser_MacAddress )) == 0 )
		{
			Interface_Number=i;
			break;
		}
	}

	/* IP address (IPv4) acquisition */
	if ( ioctl ( sock_network, SIOCGIFADDR, &User_Network[Interface_Number].USER_ifreq ) == -1 )
	{
		perror( "Error can not get the IP address" );
		close( sock_network );
		return USER_ERR_NG;
	}
	memcpy( &get_addr, &User_Network[Interface_Number].USER_ifreq.ifr_addr, sizeof( struct sockaddr_in ));
	memcpy( &User_Network[Interface_Number].IPAddr, inet_ntoa( get_addr.sin_addr ), sizeof(User_Network[Interface_Number].IPAddr )); 

	/* Mask address acquisition */
	if ( ioctl ( sock_network, SIOCGIFNETMASK, &User_Network[Interface_Number].USER_ifreq ) == -1 )
	{
		perror( "Error can not get the Mask address" );
		close( sock_network );
		return USER_ERR_NG;
	}
	memcpy( &get_addr, &User_Network[Interface_Number].USER_ifreq.ifr_netmask, sizeof( struct sockaddr_in )); 
	memcpy( &User_Network[Interface_Number].IPMask, inet_ntoa( get_addr.sin_addr ), sizeof( User_Network[Interface_Number].IPMask ));

	/* Get the default gateway from the system file */
	pFile = fopen(DIR_PROC_ROUTE , "r" );
	if( pFile == NULL )
	{
		printf( "Not systemfile read" );
		close( sock_network );
		return USER_ERR_NG;
	}
	while(1)
	{
		if ( fgets( User_Network[Interface_Number].Defo, sizeof( User_Network[Interface_Number].Defo ), pFile ) == 0 ) 
		{
			memset( User_Network[Interface_Number].Defo,0x00,sizeof( User_Network[Interface_Number].Defo ));
			fclose( pFile );
			NotGateway = 1;
			break;
		}
		//Route information acquisition
		memset( &RouteInfo_Defo, 0x00, sizeof( RouteInfo_Defo ));
		if ( sscanf(User_Network[Interface_Number].Defo, "%s%x%x%d%d%d%d%x%d%d%d",
				&RouteInfo_Defo.cIface,
				&RouteInfo_Defo.cDst,
				&RouteInfo_Defo.cGateway,
				&RouteInfo_Defo.iFlag,
				&RouteInfo_Defo.iRecCnt,
				&RouteInfo_Defo.iUse,
				&RouteInfo_Defo.iMetric,
				&RouteInfo_Defo.cMask,
				&RouteInfo_Defo.iMTU,
				&RouteInfo_Defo.iWindow,
				&RouteInfo_Defo.iIRTT) == 0 ) 
		{
			printf( "Not file read" );
			close( sock_network );
			fclose( pFile );
			return USER_ERR_NG;
		}

		/* device name and default GW flag check */
		if (( strstr ( RouteInfo_Defo.cIface,User_Network[Interface_Number].USER_ifreq.ifr_name ) != NULL ) && ( RouteInfo_Defo.iFlag == ( RTF_UP | RTF_GATEWAY )))
		{
			memcpy( &User_Network[Interface_Number].Defo, inet_ntoa( RouteInfo_Defo.cGateway ), INET_ADDRSTRLEN );
			fclose( pFile );
			break;
		}
		memset( User_Network[Interface_Number].Defo,0x00,sizeof( User_Network[Interface_Number].Defo ));
	}

	Get_NetworkInfo.ulIpAddress = htonl( inet_addr(User_Network[Interface_Number].IPAddr ));
	Get_NetworkInfo.ulSubnetMask = htonl( inet_addr( User_Network[Interface_Number].IPMask ));
	Get_NetworkInfo.ulDefaultGatewayIPAddress = htonl( inet_addr(User_Network[Interface_Number].Defo ));

	/* Check that the network is the same */
	if(( pSetAdapterInfo->ulIpAddress == Get_NetworkInfo.ulIpAddress )    &&
		( pSetAdapterInfo->ulSubnetMask == Get_NetworkInfo.ulSubnetMask ) &&
		( pSetAdapterInfo->ulDefaultGatewayIPAddress == Get_NetworkInfo.ulDefaultGatewayIPAddress ))
	{
		return USER_ERR_OK;
	}
	else
	{
		addrip_get.s_addr=htonl( Get_NetworkInfo.ulIpAddress );
		addrip_set.s_addr=htonl( pSetAdapterInfo->ulIpAddress );
		addrMask_get.s_addr=htonl( Get_NetworkInfo.ulSubnetMask );
		addrMask_set.s_addr=htonl( pSetAdapterInfo->ulSubnetMask );
		addrdg_get.s_addr=htonl( Get_NetworkInfo.ulDefaultGatewayIPAddress );
		addrdg_set.s_addr=htonl( pSetAdapterInfo->ulDefaultGatewayIPAddress );
		printf( "Change the Network adapter setting ?\n" );
		printf( "%s", User_Network[Interface_Number].USER_ifreq.ifr_name );
		printf( "\tIP address: \t\t%s", inet_ntoa( addrip_get ));
		printf( "--> %s\n",inet_ntoa( addrip_set ));
		printf( "\tSubnet mask: \t\t%s",inet_ntoa( addrMask_get ));
		printf( " --> %s\n",inet_ntoa( addrMask_set ));
		printf( "\tDefault GW IP address: \t%s",inet_ntoa( addrdg_get ));
		printf( " --> %s\n",inet_ntoa ( addrdg_set ));
		while ( 1 )
		{
			printf( "\nPlease Press Key if change the Network adapter setting Yes('Y') or No('N')\n" );
			iKey= ( char )getchar();
			if (( iKey == 'Y' ) || ( iKey == 'y' ))
			{
				break;
			}
			else if (( iKey == 'N' ) || ( iKey == 'n' ))
			{
				return USER_ERR_NG;
			}
		}
	}

	/* Change the IP address */
	s_in = (struct sockaddr_in *)&ifr_set.ifr_addr;
	s_in->sin_family = AF_INET;
	s_in->sin_addr.s_addr = htonl( pSetAdapterInfo->ulIpAddress );
	strncpy( ifr_set.ifr_name,User_Network[Interface_Number].USER_ifreq.ifr_name,IFNAMSIZ-1 );
	if ( ioctl( sock_network,SIOCSIFADDR,&ifr_set ) == -1 )
	{
		perror("Error can not set the IP address");
		return USER_ERR_NG;
	}
	/* Change the address mask */
	s_in->sin_addr.s_addr = htonl( pSetAdapterInfo->ulSubnetMask );
	strncpy( ifr_set.ifr_name,User_Network[Interface_Number].USER_ifreq.ifr_name,IFNAMSIZ-1 );
	if ( ioctl( sock_network,SIOCSIFNETMASK,&ifr_set ) == -1 )
	{
		perror("Error can not set the Mask address");
		return USER_ERR_NG;
	}
	if( NotGateway == 1 )
	{
		/* Nothing */
	}
	else
	{
		/* Delete the default gateway */
		sprintf( system_call,"route del default gw %s",inet_ntoa( addrdg_get ));
		system(system_call);
	}
	/* add the default GW */
	sprintf( system_call,"route add default gw %s",inet_ntoa( addrdg_set ));
	system(system_call);
	/* Wait for changing the ip address */
	sleep(1);
	return USER_ERR_OK;
#endif
}
