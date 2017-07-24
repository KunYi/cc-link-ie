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

#ifndef		__SLMP_H__
#define		__SLMP_H__

#include <stdint.h>

/*[ Structure of SLMP Info  ]*/
typedef struct 
{
	uint32_t	ulFrameType;			/* Frame Type */
	uint16_t	usSerialNumber;			/* Serial Number */
	uint16_t	usNetNumber;			/* Network Number */
	uint16_t	usNodeNumber;			/* Node Number */
	uint16_t	usProcNumber;			/* Processor Number */
	uint16_t	usDataLength;			/* Data Length */
	uint16_t	usTimer;				/* Timer Value */
	uint16_t	usCommand;				/* Command */
	uint16_t	usSubCommand;			/* Sub Command */
	uint16_t	usEndCode;				/* End Code */
	uint8_t	*	pucData;				/* Data */
}SLMP_INFO;

/*[ Definition of Frame Type ]*/
#define	SLMP_FTYPE_BIN_REQ_ST							(0x5000)
#define SLMP_FTYPE_BIN_RES_ST							(0xD000)
#define	SLMP_FTYPE_BIN_REQ_MT							(0x5400)
#define SLMP_FTYPE_BIN_RES_MT							(0xD400)
#define	SLMP_FTYPE_ASCII_REQ_ST							(0x35303030) /* '5000' */
#define	SLMP_FTYPE_ASCII_RES_ST							(0x44303030) /* 'D000' */
#define	SLMP_FTYPE_ASCII_REQ_MT							(0x35343030) /* '5400' */
#define	SLMP_FTYPE_ASCII_RES_MT							(0x44343030) /* 'D400' */

/*[ Definition of Index per frame type ]*/
#define	SLMP_FTYPE_BIN_REQ_ST_INDEX						(0x00)
#define SLMP_FTYPE_BIN_RES_ST_INDEX						(0x01)
#define	SLMP_FTYPE_BIN_REQ_MT_INDEX						(0x02)
#define SLMP_FTYPE_BIN_RES_MT_INDEX						(0x03)
#define	SLMP_FTYPE_ASCII_REQ_ST_INDEX					(0x04)
#define	SLMP_FTYPE_ASCII_RES_ST_INDEX					(0x05)
#define	SLMP_FTYPE_ASCII_REQ_MT_INDEX					(0x06)
#define	SLMP_FTYPE_ASCII_RES_MT_INDEX					(0x07)

/*[ Definition of Data Code ]*/
#define	SLMP_DATA_CODE_BIN								0
#define	SLMP_DATA_CODE_ASCII							1
#define SLMP_DATA_CODE(a)\
	(((a == SLMP_FTYPE_BIN_REQ_ST) | (a == SLMP_FTYPE_BIN_RES_ST) | (a == SLMP_FTYPE_BIN_REQ_MT) | (a == SLMP_FTYPE_BIN_RES_MT)) ? SLMP_DATA_CODE_BIN :\
	 ((a == SLMP_FTYPE_ASCII_REQ_ST) | (a == SLMP_FTYPE_ASCII_RES_ST) | (a == SLMP_FTYPE_ASCII_REQ_MT) | (a == SLMP_FTYPE_ASCII_RES_MT)) ? SLMP_DATA_CODE_ASCII : -1)

/*[ Definition of Response Frame Type ]*/
#define SLMP_RESPONSE_FRAME_TYPE(a)\
	((a == SLMP_FTYPE_BIN_REQ_ST) ? SLMP_FTYPE_BIN_RES_ST :\
	((a == SLMP_FTYPE_BIN_REQ_MT) ? SLMP_FTYPE_BIN_RES_MT :\
	((a == SLMP_FTYPE_ASCII_REQ_ST) ? SLMP_FTYPE_ASCII_RES_ST :\
	((a == SLMP_FTYPE_ASCII_REQ_MT) ? SLMP_FTYPE_ASCII_RES_MT : -1))))

/*[ Definition of SLMP Frame Header Size ]*/
#define SLMP_HEADER_SIZE(a)\
	(((a == SLMP_FTYPE_BIN_REQ_ST) | (a == SLMP_FTYPE_BIN_RES_ST)) ? 11 :\
	(((a == SLMP_FTYPE_BIN_REQ_MT) | (a == SLMP_FTYPE_BIN_RES_MT)) ? 15 :\
	(((a == SLMP_FTYPE_ASCII_REQ_ST) | (a == SLMP_FTYPE_ASCII_RES_ST)) ? 22 :\
	(((a == SLMP_FTYPE_ASCII_REQ_MT) | (a == SLMP_FTYPE_ASCII_RES_MT)) ? 30 : 0))))

/*[ Definition of SLMP Commands ]*/
/* Device */
#define	SLMP_COMMAND_DEVICE_READ							(0x0401)
#define	SLMP_COMMAND_DEVICE_WRITE							(0x1401)
#define	SLMP_COMMAND_DEVICE_READ_RANDOM						(0x0403)
#define	SLMP_COMMAND_DEVICE_WRITE_RANDOM					(0x1402)
#define	SLMP_COMMAND_DEVICE_ENTRY_MONITOR_DEVICE			(0x0801)
#define	SLMP_COMMAND_DEVICE_EXECUTE_MONITOR					(0x0802)
#define	SLMP_COMMAND_DEVICE_READ_BLOCK						(0x0406)
#define SLMP_COMMAND_DEVICE_WRITE_BLOCK						(0x1406)

/* Memory */
#define	SLMP_COMMAND_MEMORY_READ							(0x0613)
#define	SLMP_COMMAND_MEMORY_WRITE							(0x1613)

/* ExtendUnit */
#define	SLMP_COMMAND_EXTEND_UNIT_READ						(0x0601)
#define	SLMP_COMMAND_EXTEND_UNIT_WRITE						(0x1601)

/* RemoteControl */
#define	SLMP_COMMAND_REMOTE_RUN								(0x1001)
#define	SLMP_COMMAND_REMOTE_STOP							(0x1002)
#define	SLMP_COMMAND_REMOTE_PAUSE							(0x1003)
#define	SLMP_COMMAND_REMOTE_LATCH_CLEAR						(0x1005)
#define	SLMP_COMMAND_REMOTE_RESET							(0x1006)
#define	SLMP_COMMAND_REMOTE_READ_TYPE_NAME					(0x0101)

/* Drive */
#define	SLMP_COMMAND_DRIVE_READ_DISK_STATE					(0x0205)
#define	SLMP_COMMAND_DRIVE_DEFRAG							(0x1207)

/* FILE */
#define SLMP_COMMAND_FILE_READ_FILE_INFO					(0x0201)
#define	SLMP_COMMAND_FILE_READ_FILE_INFO_DETAIL				(0x0202)
#define	SLMP_COMMAND_FILE_READ_FILE_INFO_FILE_NUMBER_USAGE	(0x0204)
#define	SLMP_COMMAND_FILE_CHANGE_FILE_INFO					(0x1204)
#define	SLMP_COMMAND_FILE_SEARCH							(0x0203)
#define	SLMP_COMMAND_FILE_READ_ACCESS_TYPE_A				(0x0206)
#define	SLMP_COMMAND_FILE_WRITE_ACCESS_TYPE_A				(0x1203)
#define	SLMP_COMMAND_FILE_LOCK_CONTROL						(0x0808)
#define	SLMP_COMMAND_FILE_COPY_ACCESS_TYPE_A				(0x1206)
#define	SLMP_COMMAND_FILE_COPY_ACCESS_TYPE_B				(0x1824)
#define	SLMP_COMMAND_FILE_DELETE_ACCESS_TYPE_A				(0x1205)
#define	SLMP_COMMAND_FILE_DELETE_ACCESS_TYPE_B				(0x1822)
#define	SLMP_COMMAND_FILE_READ_DEIRECTORY_FILE				(0x1810)
#define	SLMP_COMMAND_FILE_SEARCH_DIRECTORY_FILE				(0x1811)
#define	SLMP_COMMAND_FILE_CREATE_NEW_FILE_ACCESS_TYPE_A		(0x1202)
#define	SLMP_COMMAND_FILE_CREATE_NEW_FILE_ACCESS_TYPE_B		(0x1820)
#define	SLMP_COMMAND_FILE_CHANGE_FILE_STATE					(0x1825)
#define	SLMP_COMMAND_FILE_CHANGE_FILE_DATE					(0x1826)
#define	SLMP_COMMAND_FILE_OPEN_FILE							(0x1827)
#define	SLMP_COMMAND_FILE_READ_ACCESS_TYPE_B				(0x1828)
#define	SLMP_COMMAND_FILE_WRITE_ACCESS_TYPE_B				(0x1829)
#define	SLMP_COMMAND_FILE_CLOSE_FILE						(0x182A)

/* TEST */
#define	SLMP_COMMAND_SELF_TEST								(0x0619)

/* CLEAR ERROR */
#define	SLMP_COMMAND_CLEAR_ERROR_CODE						(0x1617)

/* REMOTE PASSWORD */
#define	SLMP_COMMAND_PASSWORD_LOCK							(0x1630)
#define	SLMP_COMMAND_PASSWORD_UNLOCK						(0x1631)

/* ONDEMAND */
#define	SLMP_COMMAND_ONDEMAND								(0x2101)

/* NODE CONNECT */
#define	SLMP_COMMAND_NODE_SEARCH_BASIC						(0x0E30)
#define	SLMP_COMMAND_IP_ADDRESS_SET_BASIC					(0x0E31)

/* PARAMETER SETTING */
#define	SLMP_COMMAND_DEVICE_INFO_COMPARE					(0x0E32)
#define	SLMP_COMMAND_PARAMETER_GET							(0x0E33)
#define	SLMP_COMMAND_PARAMETER_SET							(0x0E34)
#define	SLMP_COMMAND_PARAMETER_SET_START					(0x0E35)
#define	SLMP_COMMAND_PARAMETER_SET_END						(0x0E36)
#define	SLMP_COMMAND_PARAMETER_SET_CANCEL					(0x0E3A)

/* NODE MONITORING */
#define	SLMP_COMMAND_STATUS_READ							(0x0E44)
#define	SLMP_COMMAND_COMMUNICATION_SETTING_GET				(0x0E45)
#define	SLMP_COMMAND_STATUS_READ2							(0x0E53)

/* CCIEF-BASIC */
#define	SLMP_COMMAND_CYCLIC_DATA							(0x0E70)

/*[ Definition of Processor Number ]*/
#define	SLMP_CPU_ACTIVE										(0x03D0)
#define	SLMP_CPU_STANDBY									(0x03D1)
#define	SLMP_CPU_TYPE_A										(0x03D2)
#define SLMP_CPU_TYPE_B										(0x03D3)
#define	SLMP_CPU_1											(0x03E0)
#define	SLMP_CPU_2											(0x03E1)
#define	SLMP_CPU_3											(0x03E2)
#define	SLMP_CPU_4											(0x03E3)
#define	SLMP_CPU_DEFAULT									(0x03FF)

/*[ Definition of Timer Value ]*/
#define	SLMP_TIMER_WAIT_FOREVER								(0x0000)

/*[ Definition of SLMP Error Codes ]*/
#define SLMP_ERR_COMMAND_SUBCOMMAND							(0xC059)
#define SLMP_ERR_WRONG_DATA									(0xC05C)
#define SLMP_ERR_DATA_LENGTH								(0xC061)
#define SLMP_ERR_UNDER_EXECUTION							(0xCEE0)
#define SLMP_ERR_REQ_DATA_SIZE								(0xCEE1)
#define SLMP_ERR_RES_DATA_SIZE								(0xCEE2)
#define SLMP_ERR_NO_EXIST_SERVER_NO							(0xCF10)
#define SLMP_ERR_CAN_NOT_COMMUNICATION_SETTING				(0xCF20)
#define SLMP_ERR_NO_EXIST_PARAM_ID							(0xCF30)
#define SLMP_ERR_CAN_NOT_PARAMETER_SET						(0xCF31)

/*[ Definition for CCIEF-BASIC End Code ]*/
#define	SLMP_END_DUPLICATE_MASTER							(0xCFE0)
#define	SLMP_END_INVALID_NUMBER_OF_OCCUPIED_STATIONS		(0xCFE1)
#define	SLMP_END_SLAVE										(0xCFF0)
#define SLMP_END_DISCONNECTED_REQUEST						(0xCFFF)

/*[ Definition of value ]*/
#define SLMP_ERR_OK		0
#define SLMP_ERR_NG		(-1)
#ifndef	NULL
#define NULL			0
#endif

/*[ Definition of mask value ]*/
#define MASK_UPPER4BIT		(0xF0)
#define MASK_LOWER4BIT		(0x0F)

/*[ Definition of bit operation ]*/
#define SHIFT_R24(a)			((uint8_t)(((a) >> 24) & 0xFF ))
#define SHIFT_R20(a)			((uint8_t)(((a) >> 20) & 0xFF ))
#define SHIFT_R16(a)			((uint8_t)(((a) >> 16) & 0xFF ))
#define SHIFT_R12(a)			((uint8_t)(((a) >> 12) & 0xFF ))
#define SHIFT_R8(a)				((uint8_t)(((a) >>  8) & 0xFF ))
#define SHIFT_R7(a)				((uint8_t)(((a) >>  7) & 0xFF ))
#define SHIFT_R6(a)				((uint8_t)(((a) >>  6) & 0xFF ))
#define SHIFT_R5(a)				((uint8_t)(((a) >>  5) & 0xFF ))
#define SHIFT_R4(a)				((uint8_t)(((a) >>  4) & 0xFF ))
#define SHIFT_R3(a)				((uint8_t)(((a) >>  3) & 0xFF ))
#define SHIFT_R2(a)				((uint8_t)(((a) >>  2) & 0xFF ))
#define SHIFT_R1(a)				((uint8_t)(((a) >>  1) & 0xFF ))
#define SHIFT_R0(a)				((uint8_t)(((a)      ) & 0xFF ))

#define SHIFT_L0(a)				((uint8_t) (((a)      ) & 0xFF ))
#define SHIFT_L1(a)				((uint8_t) (((a) <<  1) & 0xFF ))
#define SHIFT_L2(a)				((uint8_t) (((a) <<  2) & 0xFF ))
#define SHIFT_L3(a)				((uint8_t) (((a) <<  3) & 0xFF ))
#define SHIFT_L4(a)				((uint8_t) (((a) <<  4) & 0xFF ))
#define SHIFT_L5(a)				((uint8_t) (((a) <<  5) & 0xFF ))
#define SHIFT_L6(a)				((uint8_t) (((a) <<  6) & 0xFF ))
#define SHIFT_L7(a)				((uint8_t) (((a) <<  7) & 0xFF ))
#define SHIFT_L8(a)				((uint16_t)(((a) <<  8) & 0xFFFF))
#define SHIFT_L16(a)			((uint32_t)( (a) << 16))
#define SHIFT_L24(a)			((uint32_t)( (a) << 24))

#define CONCAT_2ASCII(a, b)				(((uint16_t)(a) <<  4) | (b))
#define CONCAT_4ASCII(a, b, c, d)		(((uint16_t)(a) << 12) | ((uint16_t)(b) <<  8) | ((uint16_t)(c) << 4) | (d))
#define CONCAT_6ASCII(a, b, c, d, e, f)	(((uint16_t)(a) << 20) | ((uint16_t)(b) << 16) | ((uint16_t)(c) << 12) |\
										 ((uint16_t)(d) <<  8) | ((uint16_t)(e) <<  4) | (f))
#define CONCAT_8ASCII(a, b, c, d, e, f, g, h)	(((uint16_t)(a) << 28) | ((uint16_t)(b) << 24) | ((uint16_t)(c) << 20) |\
												 ((uint16_t)(d) << 16) | ((uint16_t)(e) << 12) | ((uint16_t)(f) << 8) |\
												 ((uint16_t)(g) << 4) | (h))
#define CONCAT_2BIN(a, b)			(((uint16_t)(a) <<  8) | (b))
#define CONCAT_3BIN(a, b, c)		(((uint16_t)(a) << 16) | ((uint16_t)(b) << 8) | (c))
#define CONCAT_4BIN(a, b, c, d)		(((uint32_t) (a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (d))

#define BIT_GET(a, b)				(((((uint8_t*)a)[(b)/8]) >> ((b)%8)) & 0x01)
#define BIT_SET(a, b, c)			((((c) & 0x01) == 0x01) ? ((((uint8_t*)a)[(b)/8]) |= (0x01 << ((b)%8))) : \
									((((uint8_t*)a)[(b)/8]) &= ~(0x01 << ((b)%8))))


/*[ Definition of Public API ]*/
int SLMP_MakePacketStream ( uint32_t ulFrameType, const SLMP_INFO *p, uint8_t *pucStream );
int SLMP_GetSlmpInfo ( SLMP_INFO *p, const uint8_t *pucStream );
int SLMP_MakeErrorData ( const SLMP_INFO *p, uint8_t *pucStream, uint16_t *pusDataSize );
uint8_t local_itoa( uint8_t ucInt );
uint8_t local_atoi( uint8_t ucInt );

#endif
/*EOF*/