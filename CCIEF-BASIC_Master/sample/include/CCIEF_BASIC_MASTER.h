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

#ifndef		__CCIEF_BASIC_MASTER_H__
#define		__CCIEF_BASIC_MASTER_H__

#include <stdint.h>

/*[ Definition for sample code ]*/
#define	CCIEF_BASIC_MASTER_ERR_OK							0
#define	CCIEF_BASIC_MASTER_ERR_NG							(-1)
#define CCIEF_BASIC_MASTER_ERR_DEVICE_RANGE					(-100)
#define CCIEF_BASIC_MASTER_ERR_MASTER_DUPLICATION			(-200)
#define CCIEF_BASIC_MASTER_ERR_SLAVE_DUPLICATION			(-300)

/*[ Definition for CCIEF-BASIC environment ]*/
#define	CCIEF_BASIC_PROTCOL_VERSION							1			/* Protocol Version */
#define	CCIEF_BASIC_PORT_NUMBER_CYCLIC						61450		/* Port number of Cyclic */
#define	CCIEF_BASIC_MAX_FRAME_SIZE							6000		/* Size of MAX Packet [byte] */
#define	CCIEF_BASIC_RX_RY_SIZE								8			/* Size of RX/RY [byte] */
#define	CCIEF_BASIC_RWW_RWR_SIZE							64			/* Size of RWw/RWr [byte] */
#define	CCIEF_BASIC_MAX_SLAVE_NUMBER						64			/* Number of MAX slaves */
#define	CCIEF_BASIC_MAX_GROUP_NUMBER						64			/* Number of MAX groups */
#define	CCIEF_BASIC_MAX_NUMBER_OF_OCCUPIED_STATIONS_GROUP	16			/* Number of MAX occupied stations for group */
#define	CCIEF_BASIC_SLMP_COMMAND_CYCLIC_DATA				0x0E70		/* Cyclic Data(0E70h) of SLMP Command */
#define	CCIEF_BASIC_MASTER_PERSUASION_TIME					2500		/* Master persuasion time(2500ms) [ms] */
#define	CCIEF_BASIC_CYCLIC_TRANSMISSION_TIMEOUT				500			/* Default cyclic transmission timeout [ms] */
#define	CCIEF_BASIC_CYCLIC_TRANSMISSION_TIMEOUT_COUNT		3			/* Default count of cyclic transmission timeout */
#define	CCIEF_BASIC_MAX_CONSTANT_LINK_SCAN_TIME				2000		/* Constant link scan time MAX [ms] */
#define	CCIEF_BASIC_UNIT_INFO_APPLICATION_STOP				0x0000		/* Stopping application for setting the unit info */
#define	CCIEF_BASIC_UNIT_INFO_APPLICATION_RUNNING			0x0001		/* Running application for setting the unit info */

/*[ Definition for CCIEF-BASIC End Code ]*/
#define	CCIEF_BASIC_END_OK									0x0000		/* Processing success. */
#define	CCIEF_BASIC_END_DUPLICATE_MASTER					0xCFE0		/* Duplicate of the master. */
#define	CCIEF_BASIC_END_NUMBER_OF_OCCUPIED_STATIONS			0xCFE1		/* Invalid number of occupied stations. */
#define	CCIEF_BASIC_END_SLAVE								0xCFF0		/* Error of the slave. */
#define CCIEF_BASIC_END_DISCONNECTED_REQUEST				0xCFFF		/* Disconnected request from Slave. */

/*[ Definition for CCIEF-BASIC sample code ]*/
#define	CCIEF_BASIC_BIT_ON									1			/* Bit ON */
#define	CCIEF_BASIC_BIT_OFF									0			/* Bit OFF */
#define	CCIEF_BASIC_CYCLIC_STATE_OFF						0			/* Cyclic state OFF. */
#define	CCIEF_BASIC_CYCLIC_STATE_ON							1			/* Cyclic state ON. */
#define	CCIEF_BASIC_CYCLIC_START							1			/* Start the cyclic of the slave. */
#define	CCIEF_BASIC_CYCLIC_STOP								0			/* Stop the cyclic of the slave. */
#define	CCIEF_BASIC_SLAVE_ID_OCCUPIED_STATIONS				0xFFFFFFFF	/* Slave Id of occupied stations. */
#define	CCIEF_BASIC_CONSTANT_LINK_SCAN_NOT_USE				0			/* Not using the constant link scan. */
#define	CCIEF_BASIC_CONSTANT_LINK_SCAN_USE					1			/* Using the constant link scan. */
#define	CCIEF_BASIC_DEVICE_TYPE_RX							1			/* Type of device for RX. */
#define	CCIEF_BASIC_DEVICE_TYPE_RY							2			/* Type of device for RY. */
#define	CCIEF_BASIC_DEVICE_TYPE_RWW							3			/* Type of device for RWw. */
#define	CCIEF_BASIC_DEVICE_TYPE_RWR							4			/* Type of device for RWr. */
#define	CCIEF_BASIC_SLAVE_DUPLICATION_NOT_DETECT			0			/* Slave dupsication not detect. */
#define	CCIEF_BASIC_SLAVE_DUPLICATION_DETECT				1			/* Slave dupsication detect. */
#define	CCIEF_BASIC_SLAVE_RESPONSE_NOT_RECEIVED				0			/* Response not receive. */
#define	CCIEF_BASIC_SLAVE_RESPONSE_RECEIVED					1			/* Response receive. */
#define CCIEF_BASIC_PACKET_COUNT_MAX                                                    64                      /* Number of packets to processed per main loop */

/*[ Definition for CCIEF-BASIC state ]*/
/* Master */
#define	CCIEF_BASIC_STATE_MASTER_INITAL						0			/* Initialization of the master. */
#define	CCIEF_BASIC_STATE_MASTER_WAITING					1			/* Waiting for the cyclic operation. */
#define	CCIEF_BASIC_STATE_MASTER_PERSUASION					2			/* Persuasion of the master. */
#define	CCIEF_BASIC_STATE_MASTER_LINK_SCAN_END				3			/* End of the link scan. */
#define	CCIEF_BASIC_STATE_MASTER_LINK_SCAN					4			/* Running the link scan. */
/* Slave */
#define	CCIEF_BASIC_STATE_SLAVE_INITAL						0			/* Initialization of the slave. */
#define	CCIEF_BASIC_STATE_SLAVE_DISCONNECT					1			/* Disconnection of the cyclic network. */
#define	CCIEF_BASIC_STATE_SLAVE_CONNECTING					2			/* Connecting for the cyclic network. */
#define	CCIEF_BASIC_STATE_SLAVE_CYCLIC_STOP					3			/* Stop the cyclic operation. */
#define	CCIEF_BASIC_STATE_SLAVE_CYCLIC_END					4			/* End of the cyclic operation. */
#define	CCIEF_BASIC_STATE_SLAVE_CYCLIC						5			/* Running the cyclic operation. */

/*[ Definition for CCIEF-BASIC event ]*/
/* Master */
#define	CCIEF_BASIC_EVENT_MASTER_INITAL						0			/* Initialization of the master. */
#define	CCIEF_BASIC_EVENT_MASTER_PARAMETER_GET				1			/* Getting the parameter. */
#define	CCIEF_BASIC_EVENT_MASTER_PARAMETER_CHANGE			2			/* Changing the parameter. */
#define	CCIEF_BASIC_EVENT_MASTER_PERSUASION_TIMEOUT			3			/* Timeout of monitoring the persuasion frame. */
#define	CCIEF_BASIC_EVENT_MASTER_LINK_SCAN_START			4			/* Starting the link scan. */
#define	CCIEF_BASIC_EVENT_MASTER_LINK_SCAN_END				5			/* End of the link scan. */
#define	CCIEF_BASIC_EVENT_MASTER_CYCLIC_DATA_RECV			6			/* Receiving the cyclic data from the master. */
#define	CCIEF_BASIC_EVENT_MASTER_DUPLICATION_ERR_RECV		7			/* Receiving the error of the duplication master from the slave. */
/* Slave */
#define	CCIEF_BASIC_EVENT_SLAVE_INITAL						0			/* Initialization of the slave. */
#define	CCIEF_BASIC_EVENT_SLAVE_PARAMETER_CHANGE			1			/* Changing the parameter. */
#define	CCIEF_BASIC_EVENT_SLAVE_LINK_SCAN_START				2			/* Starting the link scan. */
#define	CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_RECV			3			/* Receiving the cyclic data from the slave. */
#define	CCIEF_BASIC_EVENT_SLAVE_CYCLIC_DATA_TIMEOUT			4			/* Timeout the response of the cyclic data. */
#define	CCIEF_BASIC_EVENT_SLAVE_CYCLIC_END					5			/* The end of the cyclic data of all slaves. */
#define	CCIEF_BASIC_EVENT_SLAVE_DUPLICATION_MASTER_ERR		6			/* Receiving the error of the duplication master from the slave. */

/*[ Structure for sample code ]*/
typedef struct 
{
	uint8_t		ucGroupNumber;						/* Group number */
	uint16_t	usCyclicTransmissionTimeout;		/* Cyclic transmission timeout */
	uint16_t	usCyclicTransmissionTimeoutCount;	/* Count of cyclic transmission timeout */
	uint16_t	usConstantLinkScanTime;				/* Constant link scan time */
} CCIEF_BASIC_GROUP_PARAMETER;

typedef struct 
{
	uint32_t	ulIpAddress;				/* Slave ip address */
	uint16_t	usOccupiedStationNumber;	/* Number of occupied stations */
	uint8_t		ucGroupNumber;				/* Group number */
} CCIEF_BASIC_SLAVE_PARAMETER;

typedef struct 
{
	int							iTotalGroupNumber;						/* Total number of the groups */
	CCIEF_BASIC_GROUP_PARAMETER	Group[CCIEF_BASIC_MAX_GROUP_NUMBER];	/* Parameter of the groups */
	int							iTotalSlaveNumber;						/* Total number of the slaves */
	CCIEF_BASIC_SLAVE_PARAMETER	Slave[CCIEF_BASIC_MAX_SLAVE_NUMBER];	/* Parameter of the slaves */
} CCIEF_BASIC_MASTER_PARAMETER;

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
	uint16_t						usProtocolVersion;			/* Protocol version */
	uint16_t						usEndCode;					/* Error code of the slave */
	uint32_t						ulId;						/* Id of the slave */
	uint8_t							ucGroupNumber;				/* Group number of the slave */
	uint16_t						usFrameSequenceNumber;		/* Frame sequence number */
	int								usOccupiedStationNumber;	/* Number of occupied stations */
	int								iState;						/* State of the slave */
	CCIEF_BASIC_SLAVE_NOTIFY_INFO	NotifyInfo;					/* Notify information from the slave */
	int								iCyclicState;				/* Cyclic state */
	int								iStationNumber;				/* Number of stations */
} CCIEF_BASIC_SLAVE_INFO;

typedef struct 
{
	uint16_t	usProtocolVersion;				/* Protocol version */
	uint32_t	ulId;							/* Id of the master */
	uint8_t		ucGroupNumber;					/* Group number of the slave */
	int			iTotalSlaveNumber;				/* Total number of the slaves */
	int			usTotalOccupiedStationNumber;	/* Total number of occupied stations */
	int			iState;							/* State of Master */
	uint16_t	usUnitInfo;						/* Information of the unit */
	uint16_t	usFrameSequenceNumber;			/* Frame sequence number */
	uint16_t	usParameterId;					/* Parameter id */
	int64_t		llTimeData;						/* Data of time */
	int64_t		llLinkScanTimeCurrent;			/* Current link scan time[us] */
	int64_t		llLinkScanTimeMinimum;			/* Minimum link scan time[us] */
	int64_t		llLinkScanTimeMaximum;			/* Maximum link scan time[us] */
} CCIEF_BASIC_GROUP_INFO;

/*[ Definition of callback function ]*/
typedef void(*CCIEF_BASIC_MASTER_CALLBACK_CYCLIC_LINK_SCAN_END)( uint8_t ucGroupNumber );

/* Definition of function of sample program */
extern int ccief_basic_master_initialize( uint32_t ulIpAddress, uint32_t ulSubnetMask, CCIEF_BASIC_MASTER_PARAMETER *pParameter,
										CCIEF_BASIC_MASTER_CALLBACK_CYCLIC_LINK_SCAN_END pCyclicLinkScanEndFunc );
extern void ccief_basic_master_terminate( void );
extern int ccief_basic_master_main( void );
extern int ccief_basic_master_start_cyclic( int iSlaveNumber );
extern int ccief_basic_master_stop_cyclic( int iSlaveNumber );
extern int ccief_basic_master_get_rx( int iNumber, int *piValue );
extern int ccief_basic_master_set_ry( int iNumber, int iValue );
extern int ccief_basic_master_set_rww( int iNumber, uint16_t usValue );
extern int ccief_basic_master_get_rwr( int iNumber, uint16_t *pusValue );
extern uint16_t *ccief_basic_master_get_pointer( int iDeviceType );
extern void ccief_basic_master_set_unit_info( uint16_t usUnitInfo );
extern int ccief_basic_master_get_slave_info( int iSlaveNumber, CCIEF_BASIC_SLAVE_INFO *pSlaveInfo );
extern int ccief_basic_master_get_group_info( int iGroupNumber, CCIEF_BASIC_GROUP_INFO *pGroupInfo );

#endif
/*EOF*/