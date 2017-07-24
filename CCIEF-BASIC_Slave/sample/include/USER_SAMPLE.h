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

#ifndef		__USER_SAMPLE_H__
#define		__USER_SAMPLE_H__

#include <stdint.h>

/*[ Definition of CCIEF-BASIC profile for sample code (Please rewrite for user environment) ]*/
#define	USER_PROFILE_VENDOR_CODE				0x1234
#define	USER_PROFILE_MODEL_CODE					0x00010001
#define	USER_PROFILE_MACHINE_VERSION			0x0001

/*[ Definition of prameter for sample code (Please rewrite for user environment) ]*/
#define	USER_PARAMETER_ID_IP_ADDRESS						1
#define	USER_PARAMETER_ID_SUBNET_MASK						2
#define	USER_PARAMETER_ID_DEFAULT_GW_IP_ADDRESS				3
#define	USER_PARAMETER_ID_OCCUPIED_STATION_NUMBER			4
#define	USER_PARAMETER_ID_CYCLIC_RESPONSE_WAIT_TIME			5

/*[ Definition of type name for SLMP node (Please rewrite for user environment) ]*/
#define	USER_TYPE_NAME				"SampleCode      "		/* Less than 16 characters, space(20H) is stored for the remaining character */
#define	USER_TYPE_NAME_CODE			0x1234

/*[ Definition of SLMP server information for sample code (Please rewrite for user environment) ]*/
#define	USER_SERVER_PORT_NUMBER		20000
#define	USER_SERVER_HOSTNAME		"SlaveSample"

/*[ Structure of prameter for sample code (Please rewrite for user environment) ]*/
typedef struct 
{
	uint32_t	ulIpAddress;				/* Slave ip address */
	uint32_t	ulSubnetMask;				/* Subnet Mask */
	uint32_t	ulDefaultGatewayIPAddress;	/* Default Gateway IP Address */
	uint16_t	usOccupiedStationNumber;	/* Number of occupied stations */
	uint32_t	ulCyclicResponseWaitTime;	/* Wait for cyclic response time [ms] (0:Not wait) */
} USER_SLAVE_PARAMETER;

/*[ Definition for sample code ]*/
#define	USER_ERR_OK			0
#define	USER_ERR_NG			(-1)
#define	USER_RESET_NONE		0
#define	USER_RESET			1


#endif
/*EOF*/