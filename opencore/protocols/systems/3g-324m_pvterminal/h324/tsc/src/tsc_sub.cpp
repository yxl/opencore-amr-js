/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
// ----------------------------------------------------------------------
//
// This Software is an original work of authorship of PacketVideo Corporation.
// Portions of the Software were developed in collaboration with NTT  DoCoMo,
// Inc. or were derived from the public domain or materials licensed from
// third parties.  Title and ownership, including all intellectual property
// rights in and to the Software shall remain with PacketVideo Corporation
// and NTT DoCoMo, Inc.
//
// -----------------------------------------------------------------------
/*****************************************************************************/
/*  file name            : tsc_sub.c                                         */
/*  file contents        : Terminal State Control routine                    */
/*  draw                 : '96.10.04                                         */
/*---------------------------------------------------------------------------*/
/*  amendment                                                                */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
#include "tsc_sub.h"         /* Sub Information Header                    */
#include "oscl_mem.h"


/*****************************************************************************/
/*  function name        : Tsc_SendDataSet                                   */
/*  function outline     : Send Data Set procedure                           */
/*  function discription : Tsc_SendDataSet( InfType,                         */
/*                                          InfId,                           */
/*                                          InfSupplement1,                  */
/*                                          InfSupplement2,                  */
/*                                          pParameter,                      */
/*                                          Size )                           */
/*  input data           : uint32                 Information Type             */
/*                         uint32                 Information Id               */
/*                         uint32                 Information Supplement 1     */
/*                         uint32                 Information Supplement 2     */
/*                         uint8*               Information Parameter        */
/*                         uint32                 Information Parameter Size   */
/*  output data          : PS_ControlMsgHeader         Send InfHeader Pointer       */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void Tsc_SendDataSet(PS_ControlMsgHeader pInfHeader,
                     uint32    InfType,
                     uint32    InfId,
                     uint32    InfSupplement1,
                     uint32    InfSupplement2,
                     uint8*    pParameter,
                     uint32    Size)
{
    // TRANSFER.request primitive -  MsgHeader
    pInfHeader->InfType        = InfType;
    pInfHeader->InfId          = InfId;
    pInfHeader->InfSupplement1 = InfSupplement1;
    pInfHeader->InfSupplement2 = InfSupplement2;
    pInfHeader->pParameter     = pParameter;
    pInfHeader->Size           = Size;
}


/*****************************************************************************/
/*  function name        : Tsc_ChoiceDataSet                                 */
/*  function outline     : Choice Data Set procedure                         */
/*  function discription : Tsc_ChoiceDataSet( TrueTarget,                    */
/*                                            pFirstTarget,                  */
/*                                            NumberTarget )                 */
/*  input data           : uint32                 True Target                  */
/*                       : uint8*               First Target Pointer         */
/*                       : uint32                 Target Numberd               */
/*  output data          : None                                              */
/*  draw time            : '96.10.09                                         */
/*---------------------------------------------------------------------------*/
/*  amendent career      :                                                   */
/*                                                                           */
/*              Copyright (C) 1996 NTT DoCoMo                                */
/*****************************************************************************/
void Tsc_ChoiceDataSet(uint32      TrueTarget,
                       uint8*      pFirstTarget,
                       uint32      NumberTarget)
{
    uint8*              pBooleanData;
    uint32              Cnt;

    for (pBooleanData = pFirstTarget, Cnt = 0;
            Cnt < NumberTarget;
            pBooleanData++, Cnt++)
    {
        if (TrueTarget == Cnt)
        {
            *pBooleanData = true;
        }
        else
        {
            *pBooleanData = false;
        }
    }
    return;
}

