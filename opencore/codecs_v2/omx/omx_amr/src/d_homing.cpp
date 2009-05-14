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
/*
*****************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.5.0   March 2, 2001
*                                R99   Version 3.2.0
*                                REL-4 Version 4.0.0
*
*****************************************************************************
*
*      File             : d_homing.c
*
*****************************************************************************
*/

/*
*****************************************************************************
*                         MODULE INCLUDE FILE AND VERSION ID
*****************************************************************************
*/

#include "d_homing.h"
//const char d_homing_id[] = "@(#)$Id $" d_homing_h;

/*
*****************************************************************************
*                         INCLUDE FILES
*****************************************************************************
*/

#include <stdlib.h>
#include "typedef.h"
#include "cnst.h"
#include "mode.h"
#include "bits2prm.h"

#include "d_homing.tab"

/* get rid of compiler warning "`bitno' defined but never used" */
//static void* dummy[] = { (void *) bitno, (void *) dummy };


/*
*****************************************************************************
*                         PRIVATE PROGRAM CODE
*****************************************************************************
*/

/*
********************************************************************************
*
*     Function        : dhf_test
*     In              : input_frame[]  one frame of encoded serial bits
*                       mode           mode type
*                       nparms         number of parameters to check
*     Out             : none
*     Calls           : Bits2prm
*     Tables          : d_homing.tab
*     Compile Defines : none
*     Return          : 0  input frame does not match the decoder homing
*                          frame pattern (up to nparms)
*                       1  input frame matches the decoder homing frame pattern
*                          (for the first nparms parameters)
*     Information     : The encoded serial bits are converted to all parameters
*                       of the corresponding mode. These parameters are compared
*                       with all parameters of the corresponding decoder homing frame.
*
********************************************************************************
*/

static Word16 dhf_test(Word16 input_frame[], enum Mode mode, Word16 nparms)
{
    Word16 i, j;
    Word16 param[MAX_PRM_SIZE];


    /* retrieve the encoded parameters from the received serial bits */
    Bits2prm(mode, input_frame, param);

    j = 0;

    /* check if the encoded parameters matches the parameters
       of the corresponding decoder homing frame */
    for (i = 0; i < nparms; i++)
    {
        j = param[i] ^ dhf[mode][i];

        if (j)
            break;
    }

    return !j;
}


/*
*****************************************************************************
*                         PUBLIC PROGRAM CODE
*****************************************************************************
*/


/*
********************************************************************************
*
*     Function        : decoder_homing_frame_test
*     In              : input_frame[]  one frame of encoded serial bits
*                       mode           mode type
*     Out             : none
*     Calls           : dhf_test
*     Tables          : d_homing.tab
*     Compile Defines : none
*     Return          : 0  input frame does not match the decoder homing frame
*                          pattern
*                       1  input frame matches the decoder homing frame pattern
*     Information     : The encoded serial bits are converted to all parameters
*                       of the corresponding mode. These parameters are compared
*                       with all parameters of the corresponding decoder homing frame.
*
********************************************************************************
*/

Word16 decoder_homing_frame_test(Word16 input_frame[], enum Mode mode)
{
    /* perform test for COMPLETE parameter frame */
    return dhf_test(input_frame, mode, prmno[mode]);
}


/*
********************************************************************************
*
*     Function        : decoder_homing_frame_test_first
*     In              : input_frame[]  one frame of encoded serial bits
*                       mode           mode type
*     Out             : none
*     Calls           : Bits2prm
*     Tables          : d_homing.tab
*     Compile Defines : none
*     Return          : 0  input frame does not match the decoder homing frame
*                          pattern (up to and including the first subframe)
*                       1  input frame matches the decoder homing frame pattern
*                          (up to and including the first subframe)
*     Information     : The encoded serial bits are converted to all parameters
*                       of the corresponding mode. These parameters are
*                       compared with the parameters for LPC and first subframe
*                       of the decoder homing frame.
*
********************************************************************************
*/

Word16 decoder_homing_frame_test_first(Word16 input_frame[], enum Mode mode)
{
    /* perform test for FIRST SUBFRAME of parameter frame ONLY */
    return dhf_test(input_frame, mode, prmnofsf[mode]);
}
