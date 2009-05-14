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
// ============================================================(Auto)=
// FILE: h245_copier.c
//
// DESC: PER Copy routines for H.245
// -------------------------------------------------------------------
//  Copyright (c) 1998- 2000, PacketVideo Corporation.
//                   All Rights Reserved.
// ===================================================================

// ========================================================
//           D E E P   C O P Y   R O U T I N E S
// ========================================================
// ****************************************************
// ** NOTE: This file is not auto-generated.         **
// ** Add copy routines as needed in the application **
// ****************************************************

#include "oscl_base.h"
#include "oscl_mem.h"
#include "per_headers.h"
#include "h245def.h"
#include "h245_encoder.h"
#include "h245_decoder.h"
#include "h245_copier.h"

//-------------------------------------------------------------
// Copy_DataType()
//
// Generic copy routine for an H.245 DataType.
//-------------------------------------------------------------
OSCL_EXPORT_REF PS_DataType
Copy_DataType(PS_DataType x)
{
    PS_OutStream outstream;
    PS_InStream  instream;
    PS_DataType y;
    uint8 *data_origin;

    /* Encode original (x) to Outstream */
    outstream = NewOutStream();
    Encode_DataType(x, outstream);

    /* Decode copy (y) from Instream */
    instream = ConvertOutstreamToInstream(outstream);
    data_origin = instream->data;  // Save the origin for later OSCL_DEFAULT_FREE
    y = (PS_DataType) OSCL_DEFAULT_MALLOC(sizeof(S_DataType));
    Decode_DataType(y, instream);

    /* Free the bitstream */
    OSCL_DEFAULT_FREE(data_origin);
    OSCL_DEFAULT_FREE(instream);

    /* Return copy */
    return(y);
}

//-------------------------------------------------------------
// Copy_H223LogicalChannelParameters()
//
// Generic copy routine for an H.245 H223LogicalChannelParameters.
//-------------------------------------------------------------
OSCL_EXPORT_REF PS_H223LogicalChannelParameters
Copy_H223LogicalChannelParameters(PS_H223LogicalChannelParameters x)
{
    PS_OutStream outstream;
    PS_InStream  instream;
    PS_H223LogicalChannelParameters y;
    uint8 *data_origin;

    /* Encode original (x) to Outstream */
    outstream = NewOutStream();
    Encode_H223LogicalChannelParameters(x, outstream);

    /* Decode copy (y) from Instream */
    instream = ConvertOutstreamToInstream(outstream);
    data_origin = instream->data;  // Save the origin for later OSCL_DEFAULT_FREE
    y = (PS_H223LogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_H223LogicalChannelParameters));
    Decode_H223LogicalChannelParameters(y, instream);

    /* Free the bitstream */
    OSCL_DEFAULT_FREE(data_origin);
    OSCL_DEFAULT_FREE(instream);

    /* Return copy */
    return(y);
}

//-------------------------------------------------------------
// Copy_MultiplexEntryDescriptor()
//
// Generic copy routine for an H.245 MultiplexEntryDescriptor.
//-------------------------------------------------------------
PS_MultiplexEntryDescriptor
Copy_MultiplexEntryDescriptor(PS_MultiplexEntryDescriptor x)
{
    PS_OutStream outstream;
    PS_InStream  instream;
    PS_MultiplexEntryDescriptor y;
    uint8 *data_origin;

    /* Encode original (x) to Outstream */
    outstream = NewOutStream();
    Encode_MultiplexEntryDescriptor(x, outstream);

    /* Decode copy (y) from Instream */
    instream = ConvertOutstreamToInstream(outstream);
    data_origin = instream->data;  // Save the origin for later OSCL_DEFAULT_FREE
    y = (PS_MultiplexEntryDescriptor) OSCL_DEFAULT_MALLOC(sizeof(S_MultiplexEntryDescriptor));
    Decode_MultiplexEntryDescriptor(y, instream);

    /* Free the bitstream */
    OSCL_DEFAULT_FREE(data_origin);
    OSCL_DEFAULT_FREE(instream);

    /* Return copy */
    return(y);
}

//-------------------------------------------------------------
// Copy_ForwardLogicalChannelParameters()
//
// Generic copy routine for an H.245 ForwardLogicalChannelParameters.
//-------------------------------------------------------------
PS_ForwardLogicalChannelParameters
Copy_ForwardLogicalChannelParameters(PS_ForwardLogicalChannelParameters x)
{
    PS_OutStream outstream;
    PS_InStream  instream;
    PS_ForwardLogicalChannelParameters y;
    uint8 *data_origin;

    /* Encode original (x) to Outstream */
    outstream = NewOutStream();
    Encode_ForwardLogicalChannelParameters(x, outstream);

    /* Decode copy (y) from Instream */
    instream = ConvertOutstreamToInstream(outstream);
    data_origin = instream->data;  // Save the origin for later OSCL_DEFAULT_FREE
    y = (PS_ForwardLogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_ForwardLogicalChannelParameters));
    Decode_ForwardLogicalChannelParameters(y, instream);

    /* Free the bitstream */
    OSCL_DEFAULT_FREE(data_origin);
    OSCL_DEFAULT_FREE(instream);

    /* Return copy */
    return(y);
}

//-------------------------------------------------------------
// Copy_ReverseLogicalChannelParameters()
//
// Generic copy routine for an H.245 ReverseLogicalChannelParameters.
//-------------------------------------------------------------
PS_ReverseLogicalChannelParameters
Copy_ReverseLogicalChannelParameters(PS_ReverseLogicalChannelParameters x)
{
    PS_OutStream outstream;
    PS_InStream  instream;
    PS_ReverseLogicalChannelParameters y;
    uint8 *data_origin;

    /* Encode original (x) to Outstream */
    outstream = NewOutStream();
    Encode_ReverseLogicalChannelParameters(x, outstream);

    /* Decode copy (y) from Instream */
    instream = ConvertOutstreamToInstream(outstream);
    data_origin = instream->data;  // Save the origin for later OSCL_DEFAULT_FREE
    y = (PS_ReverseLogicalChannelParameters) OSCL_DEFAULT_MALLOC(sizeof(S_ReverseLogicalChannelParameters));
    Decode_ReverseLogicalChannelParameters(y, instream);

    /* Free the bitstream */
    OSCL_DEFAULT_FREE(data_origin);
    OSCL_DEFAULT_FREE(instream);

    /* Return copy */
    return(y);
}

PS_TerminalCapabilitySet Copy_TerminalCapabilitySet(PS_TerminalCapabilitySet x)
{
    PS_OutStream outstream;
    PS_InStream  instream;
    PS_TerminalCapabilitySet y;
    uint8 *data_origin;

    /* Encode original (x) to Outstream */
    outstream = NewOutStream();
    Encode_TerminalCapabilitySet(x, outstream);

    /* Decode copy (y) from Instream */
    instream = ConvertOutstreamToInstream(outstream);
    data_origin = instream->data;  // Save the origin for later OSCL_DEFAULT_FREE
    y = (PS_TerminalCapabilitySet) OSCL_DEFAULT_MALLOC(sizeof(S_TerminalCapabilitySet));
    Decode_TerminalCapabilitySet(y, instream);

    /* Free the bitstream */
    OSCL_DEFAULT_FREE(data_origin);
    OSCL_DEFAULT_FREE(instream);

    /* Return copy */
    return(y);
}
