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

/*[ Definition of profile of the CCIEF-BASIC Slave (Please rewrite for user environment) ]*/
#define	USER_PROFILE_VENDOR_CODE					0x1234
#define	USER_PROFILE_MODEL_CODE						0x00000001
#define	USER_PROFILE_MACHINE_VERSION				0x0001

/*[ Definition for user environment ]*/
#define	USER_ERR_OK								0
#define	USER_ERR_NG								(-1)
#define	USER_EXIT								1

/*[ Definition of prameter for sample code (Please rewrite for user environment) ]*/
/* Group */
#define	USER_PARAMETER_GROUP_ID_TOTAL_NUMBER						1
#define	USER_PARAMETER_GROUP_ID_GROUP_NUMBER						2
#define	USER_PARAMETER_GROUP_ID_DISCONNECTION_DETECT_TIME			3
#define	USER_PARAMETER_GROUP_ID_DISCONNECTION_DETECT_TIME_COUNT		4
#define	USER_PARAMETER_GROUP_ID_CONSTANT_LINK_SCAN_TIME				5
/* Parameter */
#define	USER_PARAMETER_SLAVE_ID_TOTAL_NUMBER						1
#define	USER_PARAMETER_SLAVE_ID_IP_ADDRESS							2
#define	USER_PARAMETER_SLAVE_ID_OCCUPIED_STATION_NUMBER				3
#define	USER_PARAMETER_SLAVE_ID_GROUP_NUMBER						4

#endif
/*EOF*/