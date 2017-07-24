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
#include <stdint.h>
#include "string.h"

static unsigned int uiHeaderLength[]		= {  6,  2,  6,  2, 12,  4, 12,  4};
static unsigned int uiDataAddr[]			= { 15, 11, 19, 15, 30, 22, 38, 30};

/* A Function for making packet stream from the SLMP_Info structure */
int SLMP_MakePacketStream ( uint32_t ulFrameType, const SLMP_INFO *p, uint8_t *pucStream )
{
	int i = 0;
	int iLength = 0;
	int iIndex = 0;

	if ( (p == NULL) || (pucStream == NULL) )
	{
		return SLMP_ERR_NG;
	}

	/*[ Request : Binary Mode, Single Transmission Type ]*/
	if( (uint16_t)ulFrameType == SLMP_FTYPE_BIN_REQ_ST )
	{
		iIndex = SLMP_FTYPE_BIN_REQ_ST_INDEX;
		iLength = (p->usDataLength)-uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		pucStream[ 0] = SHIFT_R8(SLMP_FTYPE_BIN_REQ_ST);
		pucStream[ 1] = SHIFT_R0(SLMP_FTYPE_BIN_REQ_ST);
		pucStream[ 2] = SHIFT_R0(p->usNetNumber);
		pucStream[ 3] = SHIFT_R0(p->usNodeNumber);
		pucStream[ 4] = SHIFT_R0(p->usProcNumber);
		pucStream[ 5] = SHIFT_R8(p->usProcNumber);
		pucStream[ 6] = 0x00;
		pucStream[ 7] = SHIFT_R0(p->usDataLength);
		pucStream[ 8] = SHIFT_R8(p->usDataLength);
		pucStream[ 9] = SHIFT_R0(p->usTimer);
		pucStream[10] = SHIFT_R8(p->usTimer);
		pucStream[11] = SHIFT_R0(p->usCommand);
		pucStream[12] = SHIFT_R8(p->usCommand);
		pucStream[13] = SHIFT_R0(p->usSubCommand);
		pucStream[14] = SHIFT_R8(p->usSubCommand);
		for( i=0; i<iLength; i++ )
		{
			pucStream[uiDataAddr[iIndex]+i] = p->pucData[i];
		}
		return SLMP_ERR_OK;
	}

	/*[ Response : Binary Mode, Single Transmission Type ]*/
	else if( (uint16_t)ulFrameType == SLMP_FTYPE_BIN_RES_ST )
	{
		iIndex = SLMP_FTYPE_BIN_RES_ST_INDEX;
		iLength = (p->usDataLength)-uiHeaderLength[iIndex]+2;
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		pucStream[ 0] = SHIFT_R8(SLMP_FTYPE_BIN_RES_ST);
		pucStream[ 1] = SHIFT_R0(SLMP_FTYPE_BIN_RES_ST);
		pucStream[ 2] = SHIFT_R0(p->usNetNumber);
		pucStream[ 3] = SHIFT_R0(p->usNodeNumber);
		pucStream[ 4] = SHIFT_R0(p->usProcNumber);
		pucStream[ 5] = SHIFT_R8(p->usProcNumber);
		pucStream[ 6] = 0x00;
		pucStream[ 7] = SHIFT_R0(p->usDataLength+2);
		pucStream[ 8] = SHIFT_R8(p->usDataLength+2);
		pucStream[ 9] = SHIFT_R0(p->usEndCode);
		pucStream[10] = SHIFT_R8(p->usEndCode);

		for( i=0; i<iLength+2; i++ )
		{
			pucStream[uiDataAddr[iIndex]+i] = p->pucData[i];
		}
		return SLMP_ERR_OK;
	}

	/*[ Request : Binary Mode, Multiple Transmission Type ]*/
	else if( (uint16_t)ulFrameType == SLMP_FTYPE_BIN_REQ_MT )
	{
		iIndex = SLMP_FTYPE_BIN_REQ_MT_INDEX;
		iLength = (p->usDataLength)-uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		pucStream[ 0] = SHIFT_R8(SLMP_FTYPE_BIN_REQ_MT);
		pucStream[ 1] = SHIFT_R0(SLMP_FTYPE_BIN_REQ_MT);
		pucStream[ 2] = SHIFT_R0(p->usSerialNumber);
		pucStream[ 3] = SHIFT_R8(p->usSerialNumber);
		pucStream[ 4] = 0x00;
		pucStream[ 5] = 0x00;
		pucStream[ 6] = SHIFT_R0(p->usNetNumber);
		pucStream[ 7] = SHIFT_R0(p->usNodeNumber);
		pucStream[ 8] = SHIFT_R0(p->usProcNumber);
		pucStream[ 9] = SHIFT_R8(p->usProcNumber);
		pucStream[10] = 0x00;
		pucStream[11] = SHIFT_R0(p->usDataLength);
		pucStream[12] = SHIFT_R8(p->usDataLength);
		pucStream[13] = SHIFT_R0(p->usTimer);
		pucStream[14] = SHIFT_R8(p->usTimer);
		pucStream[15] = SHIFT_R0(p->usCommand);
		pucStream[16] = SHIFT_R8(p->usCommand);
		pucStream[17] = SHIFT_R0(p->usSubCommand);
		pucStream[18] = SHIFT_R8(p->usSubCommand);

		for( i=0; i<iLength; i++ )
		{
			pucStream[uiDataAddr[iIndex]+i] = p->pucData[i];
		}
		return SLMP_ERR_OK;
	}

	/*[ Response : Binary Mode, Multiple Transmission Type ]*/
	else if( (uint16_t)ulFrameType == SLMP_FTYPE_BIN_RES_MT )
	{
		iIndex = SLMP_FTYPE_BIN_RES_MT_INDEX;
		iLength = (p->usDataLength)-uiHeaderLength[iIndex]+2;
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		pucStream[ 0] = SHIFT_R8(SLMP_FTYPE_BIN_RES_MT);
		pucStream[ 1] = SHIFT_R0(SLMP_FTYPE_BIN_RES_MT);
		pucStream[ 2] = SHIFT_R0(p->usSerialNumber);
		pucStream[ 3] = SHIFT_R8(p->usSerialNumber);
		pucStream[ 4] = 0x00;
		pucStream[ 5] = 0x00;
		pucStream[ 6] = SHIFT_R0(p->usNetNumber);
		pucStream[ 7] = SHIFT_R0(p->usNodeNumber);
		pucStream[ 8] = SHIFT_R0(p->usProcNumber);
		pucStream[ 9] = SHIFT_R8(p->usProcNumber);
		pucStream[10] = 0x00;
		pucStream[11] = SHIFT_R0(p->usDataLength+2);
		pucStream[12] = SHIFT_R8(p->usDataLength+2);
		pucStream[13] = SHIFT_R0(p->usEndCode);
		pucStream[14] = SHIFT_R8(p->usEndCode);

		for( i=0; i<iLength+2; i++ )
		{
			pucStream[uiDataAddr[iIndex]+i] = p->pucData[i];
		}
		return SLMP_ERR_OK;
	}

	/*[ Request : ASCII Mode, Single Transmission Type ]*/
	else if( ulFrameType == SLMP_FTYPE_ASCII_REQ_ST )
	{
		iIndex = SLMP_FTYPE_ASCII_REQ_ST_INDEX;
		iLength = (p->usDataLength)-uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		pucStream[ 0] = SHIFT_R24(SLMP_FTYPE_ASCII_REQ_ST);
		pucStream[ 1] = SHIFT_R16(SLMP_FTYPE_ASCII_REQ_ST);
		pucStream[ 2] = SHIFT_R8 (SLMP_FTYPE_ASCII_REQ_ST);
		pucStream[ 3] = SHIFT_R0 (SLMP_FTYPE_ASCII_REQ_ST);
		pucStream[ 4] = local_itoa(SHIFT_R4(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[ 5] = local_itoa(SHIFT_R0(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[ 6] = local_itoa(SHIFT_R4(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[ 7] = local_itoa(SHIFT_R0(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[ 8] = local_itoa(SHIFT_R12(p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[ 9] = local_itoa(SHIFT_R8 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[10] = local_itoa(SHIFT_R4 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[11] = local_itoa(SHIFT_R0 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[12] = local_itoa(0x00);
		pucStream[13] = local_itoa(0x00);
		pucStream[14] = local_itoa(SHIFT_R12(p->usDataLength) & MASK_LOWER4BIT);
		pucStream[15] = local_itoa(SHIFT_R8 (p->usDataLength) & MASK_LOWER4BIT);
		pucStream[16] = local_itoa(SHIFT_R4 (p->usDataLength) & MASK_LOWER4BIT);
		pucStream[17] = local_itoa(SHIFT_R0 (p->usDataLength) & MASK_LOWER4BIT);
		pucStream[18] = local_itoa(SHIFT_R12(p->usTimer) & MASK_LOWER4BIT);
		pucStream[19] = local_itoa(SHIFT_R8 (p->usTimer) & MASK_LOWER4BIT);
		pucStream[20] = local_itoa(SHIFT_R4 (p->usTimer) & MASK_LOWER4BIT);
		pucStream[21] = local_itoa(SHIFT_R0 (p->usTimer) & MASK_LOWER4BIT);
		pucStream[22] = local_itoa(SHIFT_R12(p->usCommand) & MASK_LOWER4BIT);
		pucStream[23] = local_itoa(SHIFT_R8 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[24] = local_itoa(SHIFT_R4 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[25] = local_itoa(SHIFT_R0 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[26] = local_itoa(SHIFT_R12(p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[27] = local_itoa(SHIFT_R8 (p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[28] = local_itoa(SHIFT_R4 (p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[29] = local_itoa(SHIFT_R0 (p->usSubCommand) & MASK_LOWER4BIT);

		for( i=0; i<iLength; i++ )
		{
			pucStream[uiDataAddr[iIndex]+i] = p->pucData[i];
		}
		return SLMP_ERR_OK;
	}

	/*[ Response : ASCII Mode, Single Transmission Type ]*/
	else if( ulFrameType == SLMP_FTYPE_ASCII_RES_ST )
	{
		iIndex = SLMP_FTYPE_ASCII_RES_ST_INDEX;
		iLength = (p->usDataLength)-uiHeaderLength[iIndex]+4;
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		pucStream[ 0] = SHIFT_R24(SLMP_FTYPE_ASCII_RES_ST);
		pucStream[ 1] = SHIFT_R16(SLMP_FTYPE_ASCII_RES_ST);
		pucStream[ 2] = SHIFT_R8 (SLMP_FTYPE_ASCII_RES_ST);
		pucStream[ 3] = SHIFT_R0 (SLMP_FTYPE_ASCII_RES_ST);
		pucStream[ 4] = local_itoa(SHIFT_R4(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[ 5] = local_itoa(SHIFT_R0(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[ 6] = local_itoa(SHIFT_R4(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[ 7] = local_itoa(SHIFT_R0(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[ 8] = local_itoa(SHIFT_R12(p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[ 9] = local_itoa(SHIFT_R8 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[10] = local_itoa(SHIFT_R4 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[11] = local_itoa(SHIFT_R0 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[12] = local_itoa(0x00);
		pucStream[13] = local_itoa(0x00);
		pucStream[14] = local_itoa(SHIFT_R12(p->usDataLength+4) & MASK_LOWER4BIT);
		pucStream[15] = local_itoa(SHIFT_R8 (p->usDataLength+4) & MASK_LOWER4BIT);
		pucStream[16] = local_itoa(SHIFT_R4 (p->usDataLength+4) & MASK_LOWER4BIT);
		pucStream[17] = local_itoa(SHIFT_R0 (p->usDataLength+4) & MASK_LOWER4BIT);
		pucStream[18] = local_itoa(SHIFT_R12(p->usEndCode) & MASK_LOWER4BIT);
		pucStream[19] = local_itoa(SHIFT_R8 (p->usEndCode) & MASK_LOWER4BIT);
		pucStream[20] = local_itoa(SHIFT_R4 (p->usEndCode) & MASK_LOWER4BIT);
		pucStream[21] = local_itoa(SHIFT_R0 (p->usEndCode) & MASK_LOWER4BIT);

		for( i=0; i<iLength+4; i++ )
		{
			pucStream[uiDataAddr[iIndex]+i] = p->pucData[i];
		}
		return SLMP_ERR_OK;
	}

	/*[ Request : ASCII Mode, Multiple Transmission Type ]*/
	else if( ulFrameType == SLMP_FTYPE_ASCII_REQ_MT )
	{
		iIndex = SLMP_FTYPE_ASCII_REQ_MT_INDEX;
		iLength = (p->usDataLength)-uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		pucStream[ 0] = SHIFT_R24(SLMP_FTYPE_ASCII_REQ_MT);
		pucStream[ 1] = SHIFT_R16(SLMP_FTYPE_ASCII_REQ_MT);
		pucStream[ 2] = SHIFT_R8 (SLMP_FTYPE_ASCII_REQ_MT);
		pucStream[ 3] = SHIFT_R0 (SLMP_FTYPE_ASCII_REQ_MT);
		pucStream[ 4] = local_itoa(SHIFT_R12(p->usSerialNumber) & MASK_LOWER4BIT);
		pucStream[ 5] = local_itoa(SHIFT_R8 (p->usSerialNumber) & MASK_LOWER4BIT);
		pucStream[ 6] = local_itoa(SHIFT_R4 (p->usSerialNumber) & MASK_LOWER4BIT);
		pucStream[ 7] = local_itoa(SHIFT_R0 (p->usSerialNumber) & MASK_LOWER4BIT);
		pucStream[ 8] = local_itoa(0x00);
		pucStream[ 9] = local_itoa(0x00);
		pucStream[10] = local_itoa(0x00);
		pucStream[11] = local_itoa(0x00);
		pucStream[12] = local_itoa(SHIFT_R4(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[13] = local_itoa(SHIFT_R0(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[14] = local_itoa(SHIFT_R4(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[15] = local_itoa(SHIFT_R0(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[16] = local_itoa(SHIFT_R12(p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[17] = local_itoa(SHIFT_R8 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[18] = local_itoa(SHIFT_R4 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[19] = local_itoa(SHIFT_R0 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[20] = local_itoa(0x00);
		pucStream[21] = local_itoa(0x00);
		pucStream[22] = local_itoa(SHIFT_R12(p->usDataLength) & MASK_LOWER4BIT);
		pucStream[23] = local_itoa(SHIFT_R8 (p->usDataLength) & MASK_LOWER4BIT);
		pucStream[24] = local_itoa(SHIFT_R4 (p->usDataLength) & MASK_LOWER4BIT);
		pucStream[25] = local_itoa(SHIFT_R0 (p->usDataLength) & MASK_LOWER4BIT);
		pucStream[26] = local_itoa(SHIFT_R12(p->usTimer) & MASK_LOWER4BIT);
		pucStream[27] = local_itoa(SHIFT_R8 (p->usTimer) & MASK_LOWER4BIT);
		pucStream[28] = local_itoa(SHIFT_R4 (p->usTimer) & MASK_LOWER4BIT);
		pucStream[29] = local_itoa(SHIFT_R0 (p->usTimer) & MASK_LOWER4BIT);
		pucStream[30] = local_itoa(SHIFT_R12(p->usCommand) & MASK_LOWER4BIT);
		pucStream[31] = local_itoa(SHIFT_R8 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[32] = local_itoa(SHIFT_R4 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[33] = local_itoa(SHIFT_R0 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[34] = local_itoa(SHIFT_R12(p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[35] = local_itoa(SHIFT_R8 (p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[36] = local_itoa(SHIFT_R4 (p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[37] = local_itoa(SHIFT_R0 (p->usSubCommand) & MASK_LOWER4BIT);

		for( i=0; i<iLength; i++ )
		{
			pucStream[uiDataAddr[iIndex]+i] = p->pucData[i];
		}
		return SLMP_ERR_OK;
	}

	/*[ Response : ASCII Mode, Multiple Transmission Type ]*/
	else if( ulFrameType == SLMP_FTYPE_ASCII_RES_MT )
	{
		iIndex = SLMP_FTYPE_ASCII_RES_MT_INDEX;
		iLength = (p->usDataLength)-uiHeaderLength[iIndex]+4;
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		pucStream[ 0] = SHIFT_R24(SLMP_FTYPE_ASCII_RES_MT);
		pucStream[ 1] = SHIFT_R16(SLMP_FTYPE_ASCII_RES_MT);
		pucStream[ 2] = SHIFT_R8 (SLMP_FTYPE_ASCII_RES_MT);
		pucStream[ 3] = SHIFT_R0 (SLMP_FTYPE_ASCII_RES_MT);
		pucStream[ 4] = local_itoa(SHIFT_R12(p->usSerialNumber) & MASK_LOWER4BIT);
		pucStream[ 5] = local_itoa(SHIFT_R8 (p->usSerialNumber) & MASK_LOWER4BIT);
		pucStream[ 6] = local_itoa(SHIFT_R4 (p->usSerialNumber) & MASK_LOWER4BIT);
		pucStream[ 7] = local_itoa(SHIFT_R0 (p->usSerialNumber) & MASK_LOWER4BIT);
		pucStream[ 8] = local_itoa(0x00);
		pucStream[ 9] = local_itoa(0x00);
		pucStream[10] = local_itoa(0x00);
		pucStream[11] = local_itoa(0x00);
		pucStream[12] = local_itoa(SHIFT_R4(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[13] = local_itoa(SHIFT_R0(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[14] = local_itoa(SHIFT_R4(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[15] = local_itoa(SHIFT_R0(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[16] = local_itoa(SHIFT_R12(p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[17] = local_itoa(SHIFT_R8 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[18] = local_itoa(SHIFT_R4 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[19] = local_itoa(SHIFT_R0 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[20] = local_itoa(0x00);
		pucStream[21] = local_itoa(0x00);
		pucStream[22] = local_itoa(SHIFT_R12(p->usDataLength+4) & MASK_LOWER4BIT);
		pucStream[23] = local_itoa(SHIFT_R8 (p->usDataLength+4) & MASK_LOWER4BIT);
		pucStream[24] = local_itoa(SHIFT_R4 (p->usDataLength+4) & MASK_LOWER4BIT);
		pucStream[25] = local_itoa(SHIFT_R0 (p->usDataLength+4) & MASK_LOWER4BIT);
		pucStream[26] = local_itoa(SHIFT_R12(p->usEndCode) & MASK_LOWER4BIT);
		pucStream[27] = local_itoa(SHIFT_R8 (p->usEndCode) & MASK_LOWER4BIT);
		pucStream[28] = local_itoa(SHIFT_R4 (p->usEndCode) & MASK_LOWER4BIT);
		pucStream[29] = local_itoa(SHIFT_R0 (p->usEndCode) & MASK_LOWER4BIT);

		for( i=0; i<iLength+4; i++ )
		{
			pucStream[uiDataAddr[iIndex]+i] = p->pucData[i];
		}
		return SLMP_ERR_OK;
	}

	return SLMP_ERR_NG;
}

/* A Function for getting SLMP_Info from packet stream */
int SLMP_GetSlmpInfo ( SLMP_INFO *p, const uint8_t *pucStream )
{
	int i = 0;
	int iIndex = 0;
	int iLength = 0;
	unsigned int uiTempLength = 0;
	unsigned int uiTemp0 = 0, uiTemp1 = 0, uiTemp2 = 0, uiTemp3 = 0;
	uint16_t	usFrameType = 0;
	uint32_t	ulFrameType = 0;

	if ( (p == NULL) || (pucStream == NULL) )
	{
		return SLMP_ERR_NG;
	}

	usFrameType = CONCAT_2BIN(pucStream[0], pucStream[1]);

	(p->ulFrameType) = usFrameType;

	/*[ Request : Binary Mode, Single Transmission Type ]*/
	if( usFrameType == SLMP_FTYPE_BIN_REQ_ST )
	{
		iIndex = SLMP_FTYPE_BIN_REQ_ST_INDEX;
		uiTempLength = CONCAT_2BIN(pucStream[ 8], pucStream[ 7]);

		iLength = uiTempLength - uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		(p->usNetNumber)	= pucStream[2];
		(p->usNodeNumber)	= pucStream[3];
		(p->usProcNumber)	= CONCAT_2BIN(pucStream[ 5], pucStream[ 4]);
		(p->usDataLength)	= (uint16_t)uiTempLength;
		(p->usTimer)		= CONCAT_2BIN(pucStream[10], pucStream[ 9]);
		(p->usCommand)		= CONCAT_2BIN(pucStream[12], pucStream[11]);
		(p->usSubCommand)	= CONCAT_2BIN(pucStream[14], pucStream[13]);

		for( i=0; i<iLength; i++ )
		{
			p->pucData[i] = pucStream[uiDataAddr[iIndex]+i];
		}

		return SLMP_ERR_OK;
	}

	/*[ Response : Binary Mode, Single Transmission Type ]*/
	else if( usFrameType == SLMP_FTYPE_BIN_RES_ST )
	{
		iIndex = SLMP_FTYPE_BIN_RES_ST_INDEX;
		uiTempLength = CONCAT_2BIN(pucStream[ 8], pucStream[ 7]);

		iLength = uiTempLength - uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		(p->usNetNumber)	= pucStream[2];
		(p->usNodeNumber)	= pucStream[3];
		(p->usProcNumber)	= CONCAT_2BIN(pucStream[ 5], pucStream[ 4]);
		(p->usDataLength)	= (uint16_t)uiTempLength;
		(p->usEndCode)		= CONCAT_2BIN(pucStream[10], pucStream[ 9]);

		for( i=0; i<iLength; i++ )
		{
			p->pucData[i] = pucStream[uiDataAddr[iIndex]+i];
		}

		return SLMP_ERR_OK;
	}

	/*[ Request : Binary Mode, Multiple Transmission Type ]*/
	else if( usFrameType == SLMP_FTYPE_BIN_REQ_MT )
	{
		iIndex = SLMP_FTYPE_BIN_REQ_MT_INDEX;
		uiTempLength = CONCAT_2BIN(pucStream[12], pucStream[11]);

		iLength = uiTempLength - uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		(p->usSerialNumber)	= CONCAT_2BIN(pucStream[3], pucStream[2]);
		(p->usNetNumber)	= pucStream[6];
		(p->usNodeNumber)	= pucStream[7];
		(p->usProcNumber)	= CONCAT_2BIN(pucStream[ 9], pucStream[ 8]);
		(p->usDataLength)	= (uint16_t)uiTempLength;
		(p->usTimer)		= CONCAT_2BIN(pucStream[14], pucStream[13]);
		(p->usCommand)		= CONCAT_2BIN(pucStream[16], pucStream[15]);
		(p->usSubCommand)	= CONCAT_2BIN(pucStream[18], pucStream[17]);

		for( i=0; i<iLength; i++ )
		{
			p->pucData[i] = pucStream[uiDataAddr[iIndex]+i];
		}

		return SLMP_ERR_OK;
	}

	/*[ Response : Binary Mode, Multiple Transmission Type ]*/
	else if( usFrameType == SLMP_FTYPE_BIN_RES_MT )
	{
		iIndex = SLMP_FTYPE_BIN_RES_MT_INDEX;
		uiTempLength = CONCAT_2BIN(pucStream[12], pucStream[11]);

		iLength = uiTempLength - uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		(p->usSerialNumber)	= CONCAT_2BIN(pucStream[3], pucStream[2]);
		(p->usNetNumber)	= pucStream[6];
		(p->usNodeNumber)	= pucStream[7];
		(p->usProcNumber)	= CONCAT_2BIN(pucStream[ 9], pucStream[ 8]);
		(p->usDataLength)	= (uint16_t)uiTempLength;
		(p->usEndCode)		= CONCAT_2BIN(pucStream[14], pucStream[13]);

		for( i=0; i<iLength; i++ )
		{
			p->pucData[i] = pucStream[uiDataAddr[iIndex]+i];
		}

		return SLMP_ERR_OK;
	}

	ulFrameType = CONCAT_4BIN(pucStream[0], pucStream[1], pucStream[2], pucStream[3]);

	(p->ulFrameType) = ulFrameType;

	/*[ Request : ASCII Mode, Single Transmission Type ]*/
	if( ulFrameType == SLMP_FTYPE_ASCII_REQ_ST )
	{
		iIndex = SLMP_FTYPE_ASCII_REQ_ST_INDEX;
		uiTemp0 = local_atoi(pucStream[14]);
		uiTemp1 = local_atoi(pucStream[15]);
		uiTemp2 = local_atoi(pucStream[16]);
		uiTemp3 = local_atoi(pucStream[17]);
		uiTempLength = CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		iLength = uiTempLength - uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		uiTemp0 = local_atoi(pucStream[ 4]);
		uiTemp1 = local_atoi(pucStream[ 5]);
		(p->usNetNumber)	= (uint16_t)CONCAT_2ASCII(uiTemp0, uiTemp1);

		uiTemp0 = local_atoi(pucStream[ 6]);
		uiTemp1 = local_atoi(pucStream[ 7]);
		(p->usNodeNumber)	= (uint16_t)CONCAT_2ASCII(uiTemp0, uiTemp1);

		uiTemp0 = local_atoi(pucStream[ 8]);
		uiTemp1 = local_atoi(pucStream[ 9]);
		uiTemp2 = local_atoi(pucStream[10]);
		uiTemp3 = local_atoi(pucStream[11]);
		(p->usProcNumber)	= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		(p->usDataLength)	= (uint16_t)uiTempLength;

		uiTemp0 = local_atoi(pucStream[18]);
		uiTemp1 = local_atoi(pucStream[19]);
		uiTemp2 = local_atoi(pucStream[20]);
		uiTemp3 = local_atoi(pucStream[21]);
		(p->usTimer)		= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		uiTemp0 = local_atoi(pucStream[22]);
		uiTemp1 = local_atoi(pucStream[23]);
		uiTemp2 = local_atoi(pucStream[24]);
		uiTemp3 = local_atoi(pucStream[25]);
		(p->usCommand)		= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		uiTemp0 = local_atoi(pucStream[26]);
		uiTemp1 = local_atoi(pucStream[27]);
		uiTemp2 = local_atoi(pucStream[28]);
		uiTemp3 = local_atoi(pucStream[29]);
		(p->usSubCommand)	= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		for( i=0; i<iLength; i++ )
		{
			p->pucData[i] = pucStream[uiDataAddr[iIndex]+i];
		}

		return SLMP_ERR_OK;
	}

	/*[ Response : ASCII Mode, Single Transmission Type ]*/
	else if( ulFrameType == SLMP_FTYPE_ASCII_RES_ST )
	{
		iIndex = SLMP_FTYPE_ASCII_RES_ST_INDEX;
		uiTemp0 = local_atoi(pucStream[14]);
		uiTemp1 = local_atoi(pucStream[15]);
		uiTemp2 = local_atoi(pucStream[16]);
		uiTemp3 = local_atoi(pucStream[17]);
		uiTempLength = CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		iLength = uiTempLength - uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		uiTemp0 = local_atoi(pucStream[ 4]);
		uiTemp1 = local_atoi(pucStream[ 5]);
		(p->usNetNumber)	= (uint16_t)CONCAT_2ASCII(uiTemp0, uiTemp1);

		uiTemp0 = local_atoi(pucStream[ 6]);
		uiTemp1 = local_atoi(pucStream[ 7]);
		(p->usNodeNumber)	= (uint16_t)CONCAT_2ASCII(uiTemp0, uiTemp1);

		uiTemp0 = local_atoi(pucStream[ 8]);
		uiTemp1 = local_atoi(pucStream[ 9]);
		uiTemp2 = local_atoi(pucStream[10]);
		uiTemp3 = local_atoi(pucStream[11]);
		(p->usProcNumber)	= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		(p->usDataLength)	= (uint16_t)uiTempLength;

		uiTemp0 = local_atoi(pucStream[18]);
		uiTemp1 = local_atoi(pucStream[19]);
		uiTemp2 = local_atoi(pucStream[20]);
		uiTemp3 = local_atoi(pucStream[21]);
		(p->usEndCode)		= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		for( i=0; i<iLength; i++ )
		{
			p->pucData[i] = pucStream[uiDataAddr[iIndex]+i];
		}

		return SLMP_ERR_OK;
	}

	/*[ Request : ASCII Mode, Multiple Transmission Type ]*/
	else if( ulFrameType == SLMP_FTYPE_ASCII_REQ_MT )
	{
		iIndex = SLMP_FTYPE_ASCII_REQ_MT_INDEX;
		uiTemp0 = local_atoi(pucStream[22]);
		uiTemp1 = local_atoi(pucStream[23]);
		uiTemp2 = local_atoi(pucStream[24]);
		uiTemp3 = local_atoi(pucStream[25]);
		uiTempLength = CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		iLength = uiTempLength - uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		uiTemp0 = local_atoi(pucStream[ 4]);
		uiTemp1 = local_atoi(pucStream[ 5]);
		uiTemp2 = local_atoi(pucStream[ 6]);
		uiTemp3 = local_atoi(pucStream[ 7]);
		(p->usSerialNumber)	= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		uiTemp0 = local_atoi(pucStream[12]);
		uiTemp1 = local_atoi(pucStream[13]);
		(p->usNetNumber)	= (uint16_t)CONCAT_2ASCII(uiTemp0, uiTemp1);

		uiTemp0 = local_atoi(pucStream[14]);
		uiTemp1 = local_atoi(pucStream[15]);
		(p->usNodeNumber)	= (uint16_t)CONCAT_2ASCII(uiTemp0, uiTemp1);

		uiTemp0 = local_atoi(pucStream[16]);
		uiTemp1 = local_atoi(pucStream[17]);
		uiTemp2 = local_atoi(pucStream[18]);
		uiTemp3 = local_atoi(pucStream[19]);
		(p->usProcNumber)	= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		(p->usDataLength)	= (uint16_t)uiTempLength;

		uiTemp0 = local_atoi(pucStream[26]);
		uiTemp1 = local_atoi(pucStream[27]);
		uiTemp2 = local_atoi(pucStream[28]);
		uiTemp3 = local_atoi(pucStream[29]);
		(p->usTimer)		= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		uiTemp0 = local_atoi(pucStream[30]);
		uiTemp1 = local_atoi(pucStream[31]);
		uiTemp2 = local_atoi(pucStream[32]);
		uiTemp3 = local_atoi(pucStream[33]);
		(p->usCommand)		= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		uiTemp0 = local_atoi(pucStream[34]);
		uiTemp1 = local_atoi(pucStream[35]);
		uiTemp2 = local_atoi(pucStream[36]);
		uiTemp3 = local_atoi(pucStream[37]);
		(p->usSubCommand)	= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		for( i=0; i<iLength; i++ )
		{
			p->pucData[i] = pucStream[uiDataAddr[iIndex]+i];
		}

		return SLMP_ERR_OK;
	}

	/*[ Response : ASCII Mode, Multiple Transmission Type ]*/
	else if( ulFrameType == SLMP_FTYPE_ASCII_RES_MT )
	{
		iIndex = SLMP_FTYPE_ASCII_RES_MT_INDEX;
		uiTemp0 = local_atoi(pucStream[22]);
		uiTemp1 = local_atoi(pucStream[23]);
		uiTemp2 = local_atoi(pucStream[24]);
		uiTemp3 = local_atoi(pucStream[25]);
		uiTempLength = CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);
		
		iLength = uiTempLength - uiHeaderLength[iIndex];
		if ( iLength < 0 )
		{
			return SLMP_ERR_NG;
		}
		else if ( iLength > 0 )
		{
			if ( p->pucData == NULL )
			{
				return SLMP_ERR_NG;
			}
		}

		uiTemp0 = local_atoi(pucStream[ 4]);
		uiTemp1 = local_atoi(pucStream[ 5]);
		uiTemp2 = local_atoi(pucStream[ 6]);
		uiTemp3 = local_atoi(pucStream[ 7]);
		(p->usSerialNumber)	= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		uiTemp0 = local_atoi(pucStream[12]);
		uiTemp1 = local_atoi(pucStream[13]);
		(p->usNetNumber)	= (uint16_t)CONCAT_2ASCII(uiTemp0, uiTemp1);

		uiTemp0 = local_atoi(pucStream[14]);
		uiTemp1 = local_atoi(pucStream[15]);
		(p->usNodeNumber)	= (uint16_t)CONCAT_2ASCII(uiTemp0, uiTemp1);

		uiTemp0 = local_atoi(pucStream[16]);
		uiTemp1 = local_atoi(pucStream[17]);
		uiTemp2 = local_atoi(pucStream[18]);
		uiTemp3 = local_atoi(pucStream[19]);
		(p->usProcNumber)	= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		(p->usDataLength)	= (uint16_t)uiTempLength;

		uiTemp0 = local_atoi(pucStream[26]);
		uiTemp1 = local_atoi(pucStream[27]);
		uiTemp2 = local_atoi(pucStream[28]);
		uiTemp3 = local_atoi(pucStream[29]);
		(p->usEndCode)		= (uint16_t)CONCAT_4ASCII(uiTemp0, uiTemp1, uiTemp2, uiTemp3);

		for( i=0; i<iLength; i++ )
		{
			p->pucData[i] = pucStream[uiDataAddr[iIndex]+i];
		}

		return SLMP_ERR_OK;
	}
	return SLMP_ERR_NG;
}

/*[ translating from integer to ascii ]*/
uint8_t local_itoa( uint8_t ucInt){
	unsigned char ucTable[] = {	'0','1','2','3','4','5','6','7','8','9',
						'A','B','C','D','E','F'};
	return ucTable[ucInt];
}

/*[ translating from ascii to integer ]*/
uint8_t local_atoi( uint8_t ucInt){
	switch(ucInt)
	{
		case 'A' :
			return 0x0A;
		case 'B' :
			return 0x0B;
		case 'C' :
			return 0x0C;
		case 'D' :
			return 0x0D;
		case 'E' :
			return 0x0E;
		case 'F' :
			return 0x0F;
		default :
			return (ucInt-'0');
	}
}

/* A Function for making error response data from the SLMP_Info structure */
int SLMP_MakeErrorData ( const SLMP_INFO *p, uint8_t *pucStream, uint16_t *pusDataSize )
{
	if ( (p == NULL) || (pucStream == NULL) )
	{
		return SLMP_ERR_NG;
	}

	/*[ Binary Mode ]*/
	if ( SLMP_DATA_CODE(p->ulFrameType) == SLMP_DATA_CODE_BIN )
	{
		pucStream[ 0] = SHIFT_R0(p->usNetNumber);
		pucStream[ 1] = SHIFT_R0(p->usNodeNumber);
		pucStream[ 2] = SHIFT_R0(p->usProcNumber);
		pucStream[ 3] = SHIFT_R8(p->usProcNumber);
		pucStream[ 4] = 0x00;
		pucStream[ 5] = SHIFT_R0(p->usCommand);
		pucStream[ 6] = SHIFT_R8(p->usCommand);
		pucStream[ 7] = SHIFT_R0(p->usSubCommand);
		pucStream[ 8] = SHIFT_R8(p->usSubCommand);
		(*pusDataSize) = 0x0009;
	}

	/*[ ASCII Mode ]*/
	else if ( SLMP_DATA_CODE(p->ulFrameType) == SLMP_DATA_CODE_ASCII )
	{
		pucStream[ 0] = local_itoa(SHIFT_R4(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[ 1] = local_itoa(SHIFT_R0(p->usNetNumber) & MASK_LOWER4BIT);
		pucStream[ 2] = local_itoa(SHIFT_R4(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[ 3] = local_itoa(SHIFT_R0(p->usNodeNumber) & MASK_LOWER4BIT);
		pucStream[ 4] = local_itoa(SHIFT_R12(p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[ 5] = local_itoa(SHIFT_R8 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[ 6] = local_itoa(SHIFT_R4 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[ 7] = local_itoa(SHIFT_R0 (p->usProcNumber) & MASK_LOWER4BIT);
		pucStream[ 8] = local_itoa(0x00);
		pucStream[ 9] = local_itoa(0x00);
		pucStream[10] = local_itoa(SHIFT_R12(p->usCommand) & MASK_LOWER4BIT);
		pucStream[11] = local_itoa(SHIFT_R8 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[12] = local_itoa(SHIFT_R4 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[13] = local_itoa(SHIFT_R0 (p->usCommand) & MASK_LOWER4BIT);
		pucStream[14] = local_itoa(SHIFT_R12(p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[15] = local_itoa(SHIFT_R8 (p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[16] = local_itoa(SHIFT_R4 (p->usSubCommand) & MASK_LOWER4BIT);
		pucStream[17] = local_itoa(SHIFT_R0 (p->usSubCommand) & MASK_LOWER4BIT);
		(*pusDataSize) = 0x0012;
	}

	/*[ Data Code Error ]*/
	else
	{
		return SLMP_ERR_NG;
	}

	return SLMP_ERR_OK;
}
