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
// ============================================================
// FILE: GenericPER.c
//
// DESCRIPTION: Generic PER encode/decode routines.  These are
//   called by the automatically generated MiniParser code.
//
// Written by Ralph Neff, PacketVideo, 2/8/2000
// (c) 2000 PacketVideo Corp.
// ============================================================

#include "per_headers.h"
#include "analyzeper.h"
#include "genericper.h"
#include "oscl_error_codes.h"
#include "oscl_mem.h"
#include "oscl_stdstring.h"
#include "pvlogger.h"

#define STREAM_ADDITION 64      /* Output stream grows in increments */
/*   of this many bytes.             */

const uint8 MaskBit[] = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };

/***********************************************************/
/*=========================================================*/
/*============ DECODING ROUTINES (Generic PER) ============*/
/*=========================================================*/
/***********************************************************/

/* --------------------------------------------- */
/* ------------- LOW LEVEL STREAM -------------- */
/* --------------------------------------------- */

// =========================================================
// ErrorMessage()
//
// This function is a generic error call for PER routines.
//
// NOTE: Removed the MessageBox called and replaced with a
//   call to the analyzer via Show245().  Since tag informaion
//   isn't easy to get (encoder/decoder autogen routines don't
//   provide tags, and deleter autogen routines don't even have
//   a tag defined!), we will fix to the "PER Decoder" tag for
//   now.  The proper thing to do would be to use a newly
//   defined "PER Error" tag.  But it's not so important to
//   do this, since the source of the error is always defined
//   in the message itself.  (RAN-MessageBox)  10/5/01
// =========================================================
void ErrorMessage(const char *msg)
{
    OSCL_UNUSED_ARG(msg);

    PVLogger *logger = PVLogger::GetLoggerObject("3g324m.h245.per");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, logger, PVLOGMSG_ERR, (0, "GenericPer::ErrorMessage - %s", msg));
}

void ErrorMessageAndLeave(const char *msg)
{
    ErrorMessage(msg);
    PVLogger *logger = PVLogger::GetLoggerObject("3g324m.h245.per");
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, logger, PVLOGMSG_ERR, (0, "GenericPer::ErrorMessageAndLeave - LEAVE"));
    OSCL_LEAVE(OsclErrCorrupt);
}

// =========================================================
// ReadBits()
//
// This function reads some number of bits from the
// input stream.
// =========================================================
uint8 ReadBits(uint32 number, PS_InStream stream)
{
    uint8 x;
    uint8 NextBit, BitSize;

    if (number > 8)
    {
        ErrorMessageAndLeave("ReadBits(): Max number (8) exceeded");
    }

    BitSize = (uint8)(stream->bitIndex + number);
    x = (uint8)(stream->data[0] & MaskBit[(int)stream->bitIndex ]);
    if (BitSize > 8)
    {
        NextBit = (uint8)(BitSize - 8);
        x <<= NextBit;
        ++stream->data;
        x |= ((stream->data[0]) & ~MaskBit[(int)stream->bitIndex ]) >> (8 - NextBit);
    }
    else if (BitSize < 8)
    {
        NextBit = BitSize;
        x >>= (8 - NextBit);
    }
    else
    {
        NextBit = 0;
        ++stream->data;
    }

    stream->bitIndex = NextBit;
    return(x);
}

// =========================================================
// ReadRemainingBits()
//
// This function advances to the next octet boundary in
// the input stream.  If the stream already points to
// an octet boundary, no action is taken.
// =========================================================
void  ReadRemainingBits(PS_InStream stream)
{
    if (stream->bitIndex)
    {
        stream->bitIndex = 0;
        ++stream->data;
    }
}

// =========================================================
// ReadOctets()
//
// This function reads one or more octets from the input
// stream.  Input arguments are:
//   uint32 number;        /* Number of octets to read */
//   uint8* octets;      /* Destination for octets */
//   uint8 reorder;      /* Reorder for little-endian machine? */
//   PS_InStream stream; /* Input stream */
// =========================================================
void  ReadOctets(uint32 number, uint8* octets, uint8 reorder, PS_InStream stream)
{
    ReadRemainingBits(stream);  /* Octet alignment */

    if (number == 0)   /* No action */
    {
        return;
    }
    else if (number <= 4 && reorder)   /* Apply reordering */
    {
        switch (number)
        {
            case 1:
                octets[0] = stream->data[0];
                break;
            case 2:
                octets[0] = stream->data[1];
                octets[1] = stream->data[0];
                break;
            case 3:
                octets[0] = stream->data[2];
                octets[1] = stream->data[1];
                octets[2] = stream->data[0];
                break;
            case 4:
                octets[0] = stream->data[3];
                octets[1] = stream->data[2];
                octets[2] = stream->data[1];
                octets[3] = stream->data[0];
                break;
        }
    }
    else
    {
        oscl_memcpy(octets, stream->data, number);  /* Straight copy */
    }

    stream->data += number;
}


/* --------------------------------------------- */
/* ------------ HIGH LEVEL ASN DATA ------------ */
/* --------------------------------------------- */


// =========================================================
// GetBoolean()
//
// This function reads a boolean from the input stream.
// =========================================================
uint8 GetBoolean(PS_InStream stream)
{
    return(ReadBits(1, stream));
}

// =========================================================
// GetInteger()                           X.691 Section 10.5
//
// This function decodes a constrained integer, given the
// lower/upper bounds.  Note that bounds are uint32, which
// means min>=0, max<=4294967295.
// Assumes ALIGNED variant of PER.
// =========================================================
uint32  GetInteger(uint32 lower, uint32 upper, PS_InStream stream)
{
    uint32 value = 0, range;
    uint8 nbits, nbytes = 0;

    if (lower > upper)
    {
        ErrorMessageAndLeave("GetInteger(): Range is negative.");
        return(lower);
    }
    range = upper - lower + 1;

    if (range == 0)     /* Special case: int32EGER(0..4294967295) */
    {
        nbytes = (uint8)(ReadBits(2, stream) + 1);
        ReadOctets(nbytes, (uint8*)&value, 1, stream);
        return(value);
    }
    else if (range == 1)            /* 0-bits */
        return(lower);
    else if (range < 256)           /* Bit-field cases */
    {
        if (range <= 2) nbits = 1;
        else if (range <= 4) nbits = 2;
        else if (range <= 8) nbits = 3;
        else if (range <= 16) nbits = 4;
        else if (range <= 32) nbits = 5;
        else if (range <= 64) nbits = 6;
        else if (range <= 128) nbits = 7;
        else nbits = 8;
        value = ReadBits(nbits, stream);
        if (lower + value > upper)
        {
            ErrorMessageAndLeave("GetInteger(): Integer exceeds range");
        }
        return(lower + value);
    }
    else                            /* One or more octets */
    {
        if (range == 256) nbytes = 1;
        else if (range <= 65536) nbytes = 2;
        else nbytes = (uint8)(ReadBits(2, stream) + 1);
        ReadOctets(nbytes, (uint8*)&value, 1, stream);
        if (lower + value > upper)
        {
            ErrorMessageAndLeave("GetInteger(): exceeds range");
        }
        return(lower + value);
    }
}

// =========================================================
// GetSignedInteger()                     X.691 Section 10.5
//
// This function decodes a constrained integer, given the
// lower/upper bounds.  The only difference from GetInteger()
// is that the bounds and returned values are int32, and
// so may be negative.
// Assumes ALIGNED variant of PER.
// =========================================================
int32   GetSignedInteger(int32 lower, int32 upper, PS_InStream stream)
{
    int32 value = 0, range;
    uint8 nbits, nbytes = 0;

    if (lower > upper)
    {
        ErrorMessageAndLeave("GetSignedInteger(): Range is negative.");
        return(lower);
    }
    range = upper - lower + 1;

    if (range == 0)
    {
        ErrorMessageAndLeave("GetSignedInteger(): Range is zero.");
        return(lower);
    }
    else if (range == 1)            /* 0-bits */
        return(lower);
    else if (range < 256)           /* Bit-field cases */
    {
        if (range <= 2) nbits = 1;
        else if (range <= 4) nbits = 2;
        else if (range <= 8) nbits = 3;
        else if (range <= 16) nbits = 4;
        else if (range <= 32) nbits = 5;
        else if (range <= 64) nbits = 6;
        else if (range <= 128) nbits = 7;
        else nbits = 8;
        value = ReadBits(nbits, stream);
        if (lower + value > upper)
        {
            ErrorMessageAndLeave("GetSignedInteger(): Integer exceeds range");
        }
        return(lower + value);
    }
    else                            /* One or more octets */
    {
        if (range == 256) nbytes = 1;
        else if (range <= 65536) nbytes = 2;
        else nbytes = (uint8)(ReadBits(2, stream) + 1);
        ReadOctets(nbytes, (uint8*)&value, 1, stream);
        if (lower + value > upper)
        {
            ErrorMessageAndLeave("GetSignedInteger(): GetInteger exceeds range");
        }
        return(lower + value);
    }
}

// =========================================================
// GetUnboundedInteger()                   X.691 Section 12
//                                         and also 10.4, 10.9.
//
// This function decodes an unbounded integer.  The structure
// definition can only accomodate a uint32 return type, so if
// the decoded value is negative, we signal an error.
// Assumes ALIGNED variant of PER.
// =========================================================
uint32  GetUnboundedInteger(PS_InStream stream)
{
    uint32 nbytes, value = 0;

    nbytes = GetLengthDet(stream);               // Length Det (10.9)
    ReadOctets(nbytes, (uint8*)&value, 1, stream);  // Value (10.4)

    // Check for negative value
    if ((nbytes == 1 && value >= 0x7f) ||
            (nbytes == 2 && value >= 0x8000) ||
            (nbytes == 3 && value >= 0x800000) ||
            (nbytes == 4 && value >= 0x80000000))
    {
        ErrorMessageAndLeave("GetUnboundedInteger: Found a negative value!");
    }

    return(value);
}

// =========================================================
// GetExtendedInteger()                   X.691 Section 12.1
//
// This function decodes a constrained integer with an
// extended range.  We assume the value is unsigned, and
// that the ALIGNED variant of PER is used.
// =========================================================
uint32  GetExtendedInteger(uint32 lower, uint32 upper, PS_InStream stream)
{
    uint32 extension_bit, value;

    extension_bit = ReadBits(1, stream);
    if (extension_bit)
    {
        value = GetUnboundedInteger(stream);
    }
    else
    {
        value = GetInteger(lower, upper, stream);
    }

    return(value);
}

// =========================================================
// GetOctetString()                      X.691 Section 16
//
// This function reads an OCTET STRING from the input stream.
// Bounds on the size, if any, are included in the input
// arguments.
// =========================================================
void GetOctetString(uint8 unbounded, uint32 min, uint32 max,
                    PS_OCTETSTRING x, PS_InStream stream)
{
    if (unbounded)         /* ====Size is unbounded==== */
    {
        x->size = (uint16) GetLengthDet(stream);
        if (x->size)
        {
            x->data = (uint8*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint8));
            ReadOctets(x->size, x->data, 0, stream);
        }
        else
        {
            x->data = NULL;
        }
    }
    else                   /* ====Size is bounded==== */
    {

        if (min > max)            /* Error handling */
        {
            ErrorMessageAndLeave("GetOctetString(): Constraint error (min>max)");
        }

        if (min == max)           /* Fixed size (no length det!) */
        {
            x->size = (uint16) min;
            if (x->size)
            {
                x->data = (uint8*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint8));
                if (x->size > 2)
                {
                    ReadOctets(x->size, x->data, 0, stream);
                }
                else if (x->size == 2)
                {
                    x->data[0] = ReadBits(8, stream);
                    x->data[1] = ReadBits(8, stream);
                }
                else if (x->size == 1)
                {
                    x->data[0] = ReadBits(8, stream);
                }
            }
            else
            {
                x->data = NULL;
            }
        }
        else                      /* Constrained size */
        {
            x->size = (uint16) GetInteger(min, max, stream);
            if (x->size)
            {
                x->data = (uint8*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint8));
                ReadOctets(x->size, x->data, 0, stream);
            }
            else
            {
                x->data = NULL;
            }
        }
    }
}

// =========================================================
// GetBitString()                      X.691 Section 16
//
// This function reads a BIT STRING from the input stream.
// Bounds on the size, if any, are included in the input
// arguments.  The BITSTRING structure (x) is returned.
// =========================================================
void GetBitString(uint8 unbounded, uint32 min, uint32 max,
                  PS_BITSTRING x, PS_InStream stream)
{
    uint32 count;
    uint8* temp;

    if (unbounded)         /* ====Size is unbounded==== */
    {
        count = GetLengthDet(stream);
        x->size = (uint16) count;
        temp = x->data = (uint8*) OSCL_DEFAULT_MALLOC((1 + x->size / 8) * sizeof(uint8));
        while (count >= 8)
        {
            temp[0] = ReadBits(8, stream);
            ++temp;
            count -= 8;
        }
        temp[0] = (uint8)(ReadBits(count, stream) << (8 - count));
    }
    else                   /* ====Size is bounded==== */
    {

        if (min > max)            /* Error handling */
        {
            ErrorMessageAndLeave("GetBitString(): Constraint error (min>max)");
        }

        if (min == max)           /* Fixed size (no length det!) */
        {
            count = min;
            x->size = (uint16) count;
            temp = x->data = (uint8*) OSCL_DEFAULT_MALLOC((1 + x->size / 8) * sizeof(uint8));
            if (x->size > 16)
            {
                ReadRemainingBits(stream);  /* Octet align */
            }
            while (count >= 8)
            {
                temp[0] = ReadBits(8, stream);
                ++temp;
                count -= 8;
            }
            temp[0] = (uint8)(ReadBits(count, stream) << (8 - count));
        }
        else                      /* Constrained size */
        {
            count = GetInteger(min, max, stream);
            x->size = (uint16) count;
            temp = x->data = (uint8*) OSCL_DEFAULT_MALLOC((1 + x->size / 8) * sizeof(uint8));
            ReadRemainingBits(stream);
            while (count >= 8)
            {
                temp[0] = ReadBits(8, stream);
                ++temp;
                count -= 8;
            }
        }
    }
}

// =========================================================
// GetCharString()                             (RAN-UII)
//
// This function decodes a character string from a stream.
// Currently it only handles the special case of DTMF user
//   input (that is, signalType in UserInputIndication).
// However, it should be straightforward to extend this
//   to handle generic restrictions on the four common
//   types (IA5String, NumericString, PrintableString,
//   and VisibleString).  See X.691 Clause 26.5 for details.
// NOTE: For now I'm adding NULL string termination so that
//   the analyzer (ShowPERCharString() ) will correctly display
//   it on the decode side.  Should correct this at some point,
//   since x->data should really only contain characters that
//   were pulled from the input stream.
// =========================================================
void GetCharString(const char *stringName,
                   uint8 unbounded, uint32 min, uint32 max, const char *from,
                   PS_int8STRING x, PS_InStream stream)
{
    if (oscl_strncmp(stringName, "IA5String", oscl_strlen("IA5String")) == 0 &&    // Validate DTMF UII case
            unbounded == 0 &&
            min == 1 &&
            max == 1 &&
            oscl_strncmp(from, "0123456789#*ABCD!", oscl_strlen("0123456789#*ABCD!")) == 0)
    {
        x->size = 2;
        x->data = (uint8*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint8));
        x->data[0] = ReadBits(8, stream);     // Read single character,
        // Normal ascii value, not octet aligned
        x->data[1] = 0;  // NULL terminate
    }
    else if (oscl_strncmp(stringName, "GeneralString", oscl_strlen("GeneralString")) == 0)   // Validate GeneralString case  (RAN-ALPHA)
    {
        x->size = (uint16)(1 + GetLengthDet(stream)); // (leave extra space for NULL)
        x->data = (uint8*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint8));
        ReadOctets(x->size - 1, x->data, 1, stream);
        x->data[x->size-1] = 0;  // NULL terminate
    }
    else
    {
        x->size = 0;
        x->data = NULL;
        ErrorMessageAndLeave("GetCharString(): Not yet implemented.");
    }
}

// =========================================================
// GetObjectID()                        X.691 Section 23
//
// This function reads an OBJECT IDENTIFIER from the input
// stream.  This is just a length determinant, followed by
// a variable number of octets.
// =========================================================
void GetObjectID(PS_OBJECTIDENT x, PS_InStream stream)
{
    x->size = (uint16) GetLengthDet(stream);
    if (x->size)
    {
        x->data = (uint8*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint8));
        ReadOctets(x->size, x->data, 0, stream);
    }
    else
    {
        x->data = NULL;
    }
}


/* --------------------------------------------- */
/* --------------- OTHER CALLS ------------------*/
/* --------------------------------------------- */

// =========================================================
// GetLengthDet()                        X.691 Section 10.9
//
// Decodes a general length determinant from the input
// stream.  Assumes ALIGNED variant of PER.
// =========================================================
uint32 GetLengthDet(PS_InStream stream)
{
    uint32 length;
    uint8 byte1, byte2;
    uint8 mask1 = 0x80;   /* 10000000 */
    uint8 mask2 = 0x40;   /* 01000000 */
    uint8 mask3 = 0x3f;   /* 00111111 */

    ReadOctets(1, (uint8*)&byte1, 0, stream);  /* Get first byte */
    if (!(byte1&mask1))       /* 0xxxxxxx ==>Single octet case */
    {
        length = (uint32) byte1;
    }
    else if (!(byte1&mask2))  /* 10xxxxxx ==>Dual octet case */
    {
        ReadOctets(1, (uint8*)&byte2, 0, stream); /* Second byte */
        length = (((uint32)(byte1 & mask3)) << 8) + (int32)byte2;
    }
    else
    {
        ErrorMessageAndLeave("GetLengthDet(): Fragmented Length Dets Not Supported.");
        return(0);
    }
    return(length);
}

// =========================================================
// GetNormSmallLength()              X.691 Section 10.9.3.4
//
// Decodes a Normally Small length determinant from the
// input stream.  (In SEQUENCE, this gets the size of an
// unknown extensions map).
// Assumes ALIGNED variant of PER.
// =========================================================
uint32 GetNormSmallLength(PS_InStream stream)
{
    uint8 value = 0;
    uint8 mask = 0x40;  /* 01000000 */

    value = ReadBits(7, stream);
    if (value&mask)
    {
        ErrorMessageAndLeave("GetNormSmallLength(): Range exceeded.");
        return(0);
    }
    return((uint32)value + 1); /* Valid range is 1...64 */
}

// =========================================================
// GetNormSmallValue()                     X.691 Section 10.6
//
// Decodes a Normally Small Nonnegative Whole Number from
// the input stream.
// (In CHOICE, gets choice index when extension is ON.)
// Assumes ALIGNED variant of PER.
// =========================================================
uint32 GetNormSmallValue(PS_InStream stream)
{
    uint8 value = 0;
    uint8 mask = 0x40;  /* 01000000 */

    value = ReadBits(7, stream);
    if (value&mask)
    {
        ErrorMessageAndLeave("GetNormSmallValue(): range exceeded.");
        return(0);
    }
    return((uint32)value);    /* Valid range is 0...63 */
}

// =========================================================
// SkipOneExtension()                X.691 Section 18.9
//
// The general length is first read, and that number of
// octets are then skipped.
// (In SEQUENCE, this skips over a single unknown extension.
//  In CHOICE, this skips an item that has been chosen from
//  the extension, assuming the choice index has already
//  been read).
// Assumes aligned variant of PER.
// =========================================================
void SkipOneExtension(PS_InStream stream)
{
    uint16 length;
    uint8 buffer[32];

    length = (uint16) GetLengthDet(stream);
    while (length > 32)      /* Pieces of at most 32 bytes */
    {
        ReadOctets(32, buffer, 0, stream);
        length -= 32;
    }
    ReadOctets(length, buffer, 0, stream);  /* Leftovers */
}

// =========================================================
// SkipAllExtensions()                X.691 Section 18.9
//
// For SEQUENCE, this skips over all unknown extensions.
// This includes reading the SigMap, including size.
// Call this routine at the location of the ext marker [...].
//
// NOTE: This routine may be obsolete.  Miniparser code
// instead uses a combination of GetUnknownSigMap() and
// SkipUnreadExtensions().
// =========================================================
void SkipAllExtensions(PS_InStream stream)
{
    uint32 size, i, num_to_skip = 0;

    size = GetNormSmallLength(stream);  /* Length of SigMap */
    /* Read sigmap, counting number of significant extensions */
    for (i = 0;i < size;++i)
    {
        num_to_skip += ReadBits(1, stream);
    }
    for (i = 0;i < num_to_skip;++i)
    {
        SkipOneExtension(stream);
    }
}

// =========================================================
// GetChoiceIndex()              X.691 Sections 22.4 - 22.8
//
// Reads the extension bit and choice index from the input
// stream, and constructs a (possibly extended) choice index.
// Assumes ALIGNED variant of PER.
// ---------------------------------------------------------
// Inputs: rootnum   (number of objects in root)
//         extmarker (0 if no extension marker, 1 otherwise)
// =========================================================
uint16 GetChoiceIndex(uint32 rootnum, uint8 extmarker, PS_InStream stream)
{
    uint8 extension;
    uint16 index;

    extension = 0;
    if (extmarker) 			/* Get extension bit */
    {
        extension = ReadBits(1, stream);
    }

    if (!extension)      /* ---Item in root--- */
    {
        if (rootnum == 1)     /* Trivial index */
        {
            return(0);
        }
        else                  /* Standard index */
        {
            index = (uint16) GetInteger(0, rootnum - 1, stream);
        }               /* ---Item in extension--- */
    }
    else                    /* Extended index */
    {
        index = (uint16)(GetNormSmallValue(stream) + rootnum);
    }

    return(index);
}

// =========================================================
// GetUnknownSigMap()           X.691 Sections 18.7 - 18.8
//
// This function reads an unknown significance map from
// the input stream.  This includes the length of the
// significance map, and an options bit corresponding to
// each of the possible extension items.  The completed
// map is returned in a special structure.
// =========================================================
PS_UnknownSigMap GetUnknownSigMap(PS_InStream stream)
{
    int32 i;
    PS_UnknownSigMap x;

    x = (PS_UnknownSigMap) OSCL_DEFAULT_MALLOC(sizeof(S_UnknownSigMap));

    x->size = (uint16) GetNormSmallLength(stream);
    x->optionFlags = (uint8*) OSCL_DEFAULT_MALLOC(x->size * sizeof(uint8));
    for (i = 0;i < x->size;++i)
    {
        x->optionFlags[i] = ReadBits(1, stream);
    }
    x->extensionsRead = 0;

    return(x);
}

// =========================================================
// SigMapValue()
//
// This function reads a single options flag from an
// unknown significance map (UnknownSigMap).  If the
// index of the desired value exceeds the size of the
// map, the return value is zero.
// =========================================================
uint8 SigMapValue(uint32 index, PS_UnknownSigMap map)
{
    if (index < map->size)
    {
        return(map->optionFlags[index]);
    }
    else
    {
        return(0);
    }
}

// =========================================================
// ExtensionPrep()
//
// This function is called immediately before reading each
// unknown extension item in a SEQUENCE.  It performs two
// actions:
//    1. Records within the unknown sigmap that another
//       extension is being read (++map->extensionsRead).
//       This is so SkipUnreadExtensions() later knows
//       how many extensions should be skipped.
//    2. Reads and discards the length wrapper from the
//       input stream.
// =========================================================
void ExtensionPrep(PS_UnknownSigMap map, PS_InStream stream)
{
    ++map->extensionsRead;
    GetLengthDet(stream);
}

// =========================================================
// SkipUnreadExtensions()
//
// This function skips any unread (and possibly unknown) items
// in the extension of a SEQUENCE.  In order for this to work,
// previously read extension items must have been counted in
// map->extensionsRead; see ExtensionPrep() for details.
//
// This routine takes the following actions:
//    1. Uses map->extensionsRead to determine how many
//       extension items remain in the input stream.
//    2. Skips remaining extension items.
//    3. Frees the UnknownSigMap.
//    4. Returns the number of extensions skipped.
// =========================================================
uint32 SkipUnreadExtensions(PS_UnknownSigMap map, PS_InStream stream)
{
    uint32 i, numExtensions;

    /* Compute the number of extensions to skip */
    numExtensions = 0;
    for (i = 0;i < map->size;++i)
    {
        numExtensions += map->optionFlags[i];
    }
    numExtensions -= map->extensionsRead;

    /* Skip remaining extensions */
    for (i = 0;i < numExtensions;++i)
    {
        SkipOneExtension(stream);
    }

    /* Free map, return number skipped */
    OSCL_DEFAULT_FREE(map->optionFlags);
    OSCL_DEFAULT_FREE(map);

    return(numExtensions);
}

// =========================================================
// SkipOneOctet()
//
// Skip a single octet.  This is used when a NULL is
// read from the extension of a CHOICE (since an empty
// octet is present)
// =========================================================
void SkipOneOctet(PS_InStream stream)
{
    uint8 temp;

    ReadOctets(1, &temp, 0, stream);
}

/***********************************************************/
/*=========================================================*/
/*============ ENCODING ROUTINES (Generic PER) ============*/
/*=========================================================*/
/***********************************************************/


/* --------------------------------------------- */
/* ------------- LOW LEVEL STREAM -------------- */
/* --------------------------------------------- */

// =========================================================
// WriteBits()
//
// This function writes some number of bits to the
// output stream.
// =========================================================
void WriteBits(uint32 number, uint8 bits, PS_OutStream stream)
{
    uint8 BitSize;
    uint8 temp;

    BitSize = (uint8)((stream->bitIndex) + number);
    temp = (uint8)(bits << (8 - number));
    (stream->buildByte) |= (temp >> (stream->bitIndex));

    if (BitSize > 8)  				          /* Appended size > 8 bits */
    {
        stream->bitIndex = 8;
        WriteRemainingBits(stream);
        stream->buildByte = (uint8)((bits) << (16 - BitSize));
        stream->bitIndex = (uint16)(BitSize - 8);
        return;
    }
    else if (BitSize < 8)  			      /* Appended size < 8 bits */
    {
        stream->bitIndex = BitSize;
        return;
    }
    else                                     /* Appended size == 8 bits */
    {
        stream->bitIndex = 8;
        WriteRemainingBits(stream);
        return;
    }

}

// =========================================================
// WriteRemainingBits()
//
// This function advances to the next octet boundary in
// the output stream.  Zero-bit padding is done, if needed.
// If the stream already points to an octet boundary, no
// action is taken.
// =========================================================
void WriteRemainingBits(PS_OutStream stream)
{
    if (stream->bitIndex)
    {
        if (stream->byteIndex + 1 > stream->size)
        {
            ExpandOutStream(stream);
        }
        stream->data[stream->byteIndex] = stream->buildByte;
        ++stream->byteIndex;
        stream->bitIndex = 0;
        stream->buildByte = 0;
    }
}

// =========================================================
// WriteOctets()
//
// This function writes one or more octets to the output
// stream.  Input arguments are:
//   uint32 number;         /* Number of octets to write */
//   uint8* octets;       /* Source for octets */
//   uint8 reorder;       /* Reorder for little-endian machine? */
//   PS_OutStream stream; /* Output stream */
// =========================================================
void WriteOctets(uint32 number, uint8* octets, uint8 reorder, PS_OutStream stream)
{
    uint8* destination;

    WriteRemainingBits(stream);  /* Octet alignment */

    /* Expand the stream, if needed */
    while (stream->byteIndex + number > stream->size)
    {
        ExpandOutStream(stream);
    }

    /* Write the octets to stream */
    destination = stream->data + stream->byteIndex;
    if (number == 0)   /* No action */
    {
        return;
    }
    else if (number <= 4 && reorder)   /* Apply reordering */
    {
        switch (number)
        {
            case 1:
                destination[0] = octets[0];
                break;
            case 2:
                destination[0] = octets[1];
                destination[1] = octets[0];
                break;
            case 3:
                destination[0] = octets[2];
                destination[1] = octets[1];
                destination[2] = octets[0];
                break;
            case 4:
                destination[0] = octets[3];
                destination[1] = octets[2];
                destination[2] = octets[1];
                destination[3] = octets[0];
                break;
        }
    }
    else
    {
        oscl_memcpy(destination, octets, number);  /* Straight copy */
    }

    stream->byteIndex = (uint16)(number + stream->byteIndex);
}

// =========================================================
// NewOutStream()
//
// This function creates a new, empty output stream.
// =========================================================
PS_OutStream NewOutStream(void)
{
    PS_OutStream x;

    x = (PS_OutStream) OSCL_DEFAULT_MALLOC(sizeof(S_OutStream));
    x->data = NULL;
    x->size = 0;
    x->byteIndex = 0;
    x->bitIndex = 0;
    x->buildByte = 0;

    return(x);
}

// =========================================================
// ExpandOutStream()
//
// This function expands an output stream by some number
// of bytes, as defined by STREAM_ADDITION.
// =========================================================
void ExpandOutStream(PS_OutStream x)
{
    uint8* tmp = x->data;

    x->data = (uint8*) OSCL_DEFAULT_MALLOC(x->size + STREAM_ADDITION);
    oscl_memcpy(x->data, tmp, x->size);
    if (tmp)
        OSCL_DEFAULT_FREE(tmp);

    x->size += STREAM_ADDITION;
}

// =========================================================
// FreeOutStream()
//
// This function frees an existing output stream.
// =========================================================
void FreeOutStream(PS_OutStream x)
{
    OSCL_DEFAULT_FREE(x->data);
    OSCL_DEFAULT_FREE(x);
}

/* ----------------------------------------------- */
/* ------------- HIGH LEVEL ASN DATA ------------- */
/* ----------------------------------------------- */

// =========================================================
// PutBoolean()
//
// This function writes a boolean to the output stream.
// =========================================================
void PutBoolean(uint32 value, PS_OutStream stream)
{
    WriteBits(1, (uint8)value, stream);
}

// =========================================================
// PutInteger()                           X.691 Section 10.5
//
// This function encodes a constrained integer, given the
// lower/upper bounds.  Note that bounds and value must be
// uint32.  This means 0<= {lower,upper,value} <=4294967295.
// Assumes ALIGNED variant of PER.
// =========================================================
void PutInteger(uint32 lower, uint32 upper, uint32 value, PS_OutStream stream)
{
    uint32 range, offset;
    uint8 nbits, nbytes = 0;
    uint8 code_nbytes;

    if (lower > upper)
    {
        ErrorMessageAndLeave("PutInteger(): Negative range.");
        return;
    }
    if (value < lower)
    {
        ErrorMessageAndLeave("PutInteger(): Value too small.");
        return;
    }
    if (value > upper)
    {
        ErrorMessageAndLeave("PutInteger(): Value too large.");
        return;
    }

    range = upper - lower + 1;
    offset = value - lower;

    if (range == 0 || range > 65536)     /* Range exceeds 2-octets */
    {
        /* (Range==0 means [0,2^32-1] case) */
        if (offset < 256) nbytes = 1; /* Send using minimum #octets */
        else if (offset < 65536) nbytes = 2;
        else if (offset < 16777216) nbytes = 3;
        else nbytes = 4;
        code_nbytes = (uint8)(nbytes - 1);
        WriteBits(2, code_nbytes, stream);              /* Send #octets */
        WriteOctets(nbytes, (uint8*)&offset, 1, stream);  /* Offset value */
        return;
    }
    else if (range == 1)            /* 0-bits */
        return;
    else if (range < 256)           /* Bit-field cases */
    {
        if (range <= 2) nbits = 1;
        else if (range <= 4) nbits = 2;
        else if (range <= 8) nbits = 3;
        else if (range <= 16) nbits = 4;
        else if (range <= 32) nbits = 5;
        else if (range <= 64) nbits = 6;
        else if (range <= 128) nbits = 7;
        else nbits = 8;
        WriteBits(nbits, (uint8)offset, stream);
        return;
    }
    else if (range == 256)          /* One octet */
    {
        WriteOctets(1, (uint8*)(&offset), 1, stream);
        return;
    }
    else                            /* range<=65536; Two octet */
    {
        WriteOctets(2, (uint8*)(&offset), 1, stream);
        return;
    }
}

// =========================================================
// PutSignedInteger()                   X.691 Section 10.5
//
// This function encodes a constrained integer, given the
// lower/upper bounds.  The only difference from
// PutInteger() is that the bounds and the values are
// of type int32, and so may be negative.
// Assumes ALIGNED variant of PER.
// =========================================================
void PutSignedInteger(int32 lower, int32 upper, int32 value, PS_OutStream stream)
{
    int32 range, offset;
    uint8 nbits, nbytes = 0;
    uint8 code_nbytes;

    if (lower > upper)
    {
        ErrorMessageAndLeave("PutSignedInteger(): Negative range.");
        return;
    }
    if (value < lower)
    {
        ErrorMessageAndLeave("PutSignedInteger(): Value too small.");
        return;
    }
    if (value > upper)
    {
        ErrorMessageAndLeave("PutSignedInteger(): Value too large.");
        return;
    }

    range = upper - lower + 1;
    offset = value - lower;

    if (range == 0 || range > 65536)     /* Range exceeds 2-octets */
    {
        /* (Range==0 means [0,2^32-1] case) */
        if (offset < 256) nbytes = 1; /* Send using minimum #octets */
        else if (offset < 65536) nbytes = 2;
        else if (offset < 16777216) nbytes = 3;
        else nbytes = 4;
        code_nbytes = (uint8)(nbytes - 1);
        WriteBits(2, code_nbytes, stream);              /* Send #octets */
        WriteOctets(nbytes, (uint8*)&offset, 1, stream);  /* Offset value */
        return;
    }
    else if (range == 1)            /* 0-bits */
        return;
    else if (range < 256)           /* Bit-field cases */
    {
        if (range <= 2) nbits = 1;
        else if (range <= 4) nbits = 2;
        else if (range <= 8) nbits = 3;
        else if (range <= 16) nbits = 4;
        else if (range <= 32) nbits = 5;
        else if (range <= 64) nbits = 6;
        else if (range <= 128) nbits = 7;
        else nbits = 8;
        WriteBits(nbits, (uint8)offset, stream);
        return;
    }
    else if (range == 256)          /* One octet */
    {
        WriteOctets(1, (uint8*)(&offset), 1, stream);
        return;
    }
    else                            /* range<=65536; Two octet */
    {
        WriteOctets(2, (uint8*)(&offset), 1, stream);
        return;
    }
}

// =========================================================
// PutUnboundedInteger()                   X.691 Section 12
//                                         and also 10.4, 10.9.
//
// This function encodes an unbounded integer, which we
// currently assume has type uint32.  We'd need to make
// special arrangements if negative values are allowed.
// Assumes ALIGNED variant of PER.
// =========================================================
void PutUnboundedInteger(uint32 value, PS_OutStream stream)
{
    uint32 nbytes;

    /* The encoding is min-octets, and so depends on value */
    if (value < 128)
        nbytes = 1;
    else if (value < 32768)
        nbytes = 2;
    else if (value < 8388608)
        nbytes = 3;
    else
        nbytes = 4;

    /* Send length det, followed by 2's complement encoding */
    PutLengthDet(nbytes, stream);
    WriteOctets(nbytes, (uint8*)(&value), 1, stream);
}

// =========================================================
// PutExtendedInteger()                   X.691 Section 12.1
//
// This function encodes a constrained integer with an
// extended range.  We assume the value is unsigned, and
// that the ALIGNED variant of PER is used.
// =========================================================
void PutExtendedInteger(uint32 lower, uint32 upper, uint32 value, PS_OutStream stream)
{
    if (value > upper) 				// Extension ON
    {
        WriteBits(1, 1, stream);
        PutUnboundedInteger(value, stream);
    }
    else 							// Extension OFF
    {
        WriteBits(1, 0, stream);
        PutInteger(lower, upper, value, stream);
    }
}

// =========================================================
// PutOctetString()                      X.691 Section 16
//
// This function writes an OCTET STRING to the output stream.
// Bounds on the size, if any, are included in the input
// arguments.  The OCTETSTRING structure (x) is freed at
// the end.
// =========================================================
void PutOctetString(uint8 unbounded, uint32 min, uint32 max,
                    PS_OCTETSTRING x, PS_OutStream stream)
{
    if (unbounded)         /* ====Size is unbounded==== */
    {
        PutLengthDet(x->size, stream);
        WriteOctets(x->size, x->data, 0, stream);
    }
    else                   /* ====Size is bounded==== */
    {

        if (min > max)            /* Error handling */
        {
            ErrorMessageAndLeave("PutOctetString(): Constraint error (min>max)");
        }
        else if ((x->size < min) || (x->size > max))
        {
            ErrorMessageAndLeave("PutOctetString(): Size out of bounds");
        }

        if (min == max)           /* Fixed size (no length det!) */
        {
            if (x->size > 2)
            {
                WriteOctets(x->size, x->data, 0, stream);
            }
            else if (x->size == 2)
            {
                WriteBits(8, x->data[0], stream);
                WriteBits(8, x->data[1], stream);
            }
            else if (x->size == 1)
            {
                WriteBits(8, x->data[0], stream);
            }
        }
        else                      /* Constrained size */
        {
            PutInteger(min, max, x->size, stream);
            WriteOctets(x->size, x->data, 0, stream);
        }
    }
}

// =========================================================
// PutBitString()                      X.691 Section 16
//
// This function writes a BIT STRING to the output stream.
// Bounds on the size, if any, are included in the input
// arguments.  The BITSTRING structure (x) is freed at
// the end.
// =========================================================
void PutBitString(uint8 unbounded, uint32 min, uint32 max,
                  PS_BITSTRING x, PS_OutStream stream)
{
    uint32 count;
    uint8* temp;

    count = x->size;
    temp = x->data;

    if (unbounded)         /* ====Size is unbounded==== */
    {
        PutLengthDet(x->size, stream);
        while (count >= 8)
        {
            WriteBits(8, temp[0], stream);
            ++temp;
            count -= 8;
        }
        WriteBits(count, (uint8)(temp[0] >> (8 - count)), stream);
    }
    else                   /* ====Size is bounded==== */
    {

        if (min > max)            /* Error handling */
        {
            ErrorMessageAndLeave("PutBitString(): Constraint error (min>max)");
        }
        else if ((x->size < min) || (x->size > max))
        {
            ErrorMessageAndLeave("PutBitString(): Size out of bounds");
        }

        if (min == max)           /* Fixed size (no length det!) */
        {
            if (x->size > 16)
            {
                WriteRemainingBits(stream);  /* Octet align */
            }
            while (count >= 8)
            {
                WriteBits(8, temp[0], stream);
                ++temp;
                count -= 8;
            }
            WriteBits(count, (uint8)(temp[0] >> (8 - count)), stream);
        }
        else                      /* Constrained size */
        {
            PutInteger(min, max, x->size, stream);
            WriteRemainingBits(stream);
            while (count >= 8)
            {
                WriteBits(8, temp[0], stream);
                ++temp;
                count -= 8;
            }
            WriteBits(count, (uint8)(temp[0] >> (8 - count)), stream);
        }
    }
}

// =========================================================
// PutCharString()                             (RAN-UII)
//
// This function encodes a character string to a stream.
// Currently it only handles the special case of DTMF user
//   input (that is, signalType in UserInputIndication).
// However, it should be straightforward to extend this
//   to handle generic restrictions on the four common
//   types (IA5String, NumericString, PrintableString,
//   and VisibleString).  See X.691 Clause 26.5 for details.
// =========================================================
void PutCharString(const char *stringName,
                   uint8 unbounded, uint32 min, uint32 max, const char *from,
                   PS_int8STRING x, PS_OutStream stream)
{
    if (oscl_strncmp(stringName, "IA5String", oscl_strlen("IA5String")) == 0 &&    // Validate DTMF UII case
            unbounded == 0 &&
            min == 1 &&
            max == 1 &&
            oscl_strncmp(from, "0123456789#*ABCD!", oscl_strlen("0123456789#*ABCD!")) == 0)
    {
        WriteBits(8, x->data[0], stream);     // Write single character,
        // Normal ascii value, not octet aligned
    }
    else if (oscl_strncmp(stringName, "GeneralString", oscl_strlen("GeneralString")) == 0)   // Validate GeneralString case (RAN-ALPHA)
        // ------------------------------------------------------------------------
        // Note on GeneralString: as far as I can tell this is treated as a
        // restricted character string type which is not a known-multiplier character
        // string.  As such it falls under clause 26.6 of X.691.  In this case, any
        // constraints passed from the PER encoder are not considered 'PER-visible'
        // and so they are not used.  Because of this, the following should be a
        // complete implementation of GeneralString (regardless of constraints).
        // It should work for alphanumeric, and anything else that would use
        // GeneralString.
        // ------------------------------------------------------------------------
        // Note also: I assume you need to install the primary graphic character
        // set described in T.51, i.e. that there is no default character set.
        // Invoking a character set means sending an escape code.  Whether you NEED
        // this to get the primary character set wasn't made clear in T.51
        // itself, though I found a hint elsewhere that suggested you have to send
        // the escape code.  The escape code is 27 40 66 15 (or in T.51 lingo,
        // "ESC 2/8 4/2 0/15").  Sending this code shouldn't hurt anything if the
        // receiving end is compliant to T.51, but if it doesn't work on the first
        // try, then one thing you guys might try is removing the escape code, in
        // case maybe the VIG is choking on it.  Comment out the relevant four lines
        // below, and also get rid of the extra '4+' in the length calculation.
        // BTW, the escape sequence breaks down into:
        //   "Designate primary T.51 character set as G0" -- 27 40 66
        //   "Invoke G0 as the current character set" -- 15
        // ------------------------------------------------------------------------
    {
        // Encode length of string, in octets
        PutLengthDet((x->size), stream);  // Assumes 4-byte T.51 escape sequence
        // Remove the '4+' if the escape sequence is removed.
        // Note: PutLengthDet() does octet alignment for us
        // Send the T.51 encoded character data
        WriteOctets(x->size, x->data, 1, stream);
    }
    else
    {
        ErrorMessageAndLeave("PutCharString(): Not fully implemented.");
    }
}

// =========================================================
// PutObjectID()                        X.691 Section 23
//
// This function writes an OBJECT IDENTIFIER to the output
// stream.  This is just a length determinant, followed by
// a variable number of octets.  The OBJECTID structure is
// freed at the end.
// =========================================================
void PutObjectID(PS_OBJECTIDENT x, PS_OutStream stream)
{
    PutLengthDet(x->size, stream);
    WriteOctets(x->size, x->data, 0, stream);
}

// =========================================================
// PutExtensionNull()
//
// This function encodes a NULL object as an extension.
// =========================================================
void PutExtensionNull(PS_OutStream stream)
{
    uint8 temp = 0;

    PutLengthDet(1, stream);        /* Length wrapper */
    WriteOctets(1, &temp, 0, stream);  /* Empty octet */
}

// =========================================================
// PutExtensionBoolean()
//
// This function encodes a boolean as an extension.
// =========================================================
void PutExtensionBoolean(uint32 value, PS_OutStream stream)
{
    PutLengthDet(1, stream);        /* Length wrapper */
    PutBoolean(value, stream);      /* Boolean */
    WriteRemainingBits(stream);     /* Complete the octet */
}

// =========================================================
// PutExtensionInteger()
//
// This function encodes an integer as an extension.
// =========================================================
void PutExtensionInteger(uint32 lower, uint32 upper, uint32 value, PS_OutStream stream)
{
    PS_OutStream tempStream;

    tempStream = NewOutStream();              /* Create a temp stream */
    PutInteger(lower, upper, value, tempStream); /* Write integer to temp */
    PutTempStream(tempStream, stream); /* Transmit contents of tempStream */
}

// =========================================================
// PutExtensionOctetString()
//
// This function encodes an octet string as an extension.
// =========================================================
void PutExtensionOctetString(uint8 unbounded,
                             uint32 min, uint32 max, PS_OCTETSTRING x, PS_OutStream stream)
{
    PS_OutStream tempStream;

    tempStream = NewOutStream();                  /* Create a temp stream */
    PutOctetString(unbounded, min, max, x, tempStream);  /* Write to temp */
    PutTempStream(tempStream, stream); /* Transmit contents of tempStream */
}


/* --------------------------------------------- */
/* ---------------- OTHER CALLS -----------------*/
/* --------------------------------------------- */

// =========================================================
// PutNormSmallValue()                  X.691 Section 10.6
//
// Sends a normally small nonnegative value.  Range is
// [0,infinity].  (In CHOICE, this sends the choice index
// when extension is ON).
// Assumes ALIGNED variant of PER.
// =========================================================
void PutNormSmallValue(uint32 value, PS_OutStream stream)
{
    uint8 sendval;
    if (value < 64)
    {
        sendval = (uint8) value;
        WriteBits(7, sendval, stream);
    }
    else
    {
        ErrorMessageAndLeave("PutNormSmallValue(): range exceeded.");
    }
}

// =========================================================
// PutChoiceIndex()              X.691 Sections 22.4 - 22.8
//
// Writes the extension bit (if needed) and the choice index
// to the bitstream.  Assumes ALIGNED variant of PER.
//
// Inputs: rootnum      (number of items in root)
//         extmarker    (0 if no extension marker, 1 otherwise)
//         index        (choice index, possibly extended)
// =========================================================
void PutChoiceIndex(uint32 rootnum, uint8 extmarker, uint32 index,
                    PS_OutStream stream)
{
    if (!extmarker)    /* ---Extension marker NOT present --- */
    {
        PutInteger(0, rootnum - 1, (uint32)index, stream);
    }
    else               /* ---Extension marker present --- */
    {
        if (index < rootnum)
        {
            WriteBits(1, 0, stream); /* Extension bit OFF */
            PutInteger(0, rootnum - 1, (uint32)index, stream);
        }
        else
        {
            WriteBits(1, 1, stream); /* Extension bit ON */
            PutNormSmallValue((uint32)(index - rootnum), stream);
        }
    }
}

// =========================================================
// PutNormSmallLength()              X.691 Section 10.9.3.4
//
// Sends a normally small length determinant.  Range is
// [1,infinity].  (For SEQUENCE, this sends the size of
// the SigMap for unknown extensions.)
// Assumes ALIGNED variant of PER.
// =========================================================
void PutNormSmallLength(uint32 value, PS_OutStream stream)
{
    uint8 sendval;
    if (value < 65)
    {
        sendval = (uint8)(value - 1);
        WriteBits(7, sendval, stream);
    }
    else
    {
        ErrorMessageAndLeave("PutNormSmallLength(): range exceeded.");
    }
}

// =========================================================
// PutLengthDet()                        X.691 Section 10.9
//
// Encodes a general length determinant to the output stream.
// (e.g. for extension wrapper).
// Assumes ALIGNED variant of PER.
// =========================================================
void PutLengthDet(uint32 value, PS_OutStream stream)
{
    uint8 byte;
    uint16 bytes;
    uint16 mask = 0x8000;   /* 10000000 00000000 */

    if (value < 128)       /* One octet with leading '0' */
    {
        byte = (uint8) value;
        WriteOctets(1, &byte, 0, stream);
    }
    else if (value < 16384)   /* Two octets with leading '10' */
    {
        bytes = (uint16)(value | mask);
        WriteOctets(2, (uint8*)&bytes, 0, stream);
    }
    else
    {
        ErrorMessageAndLeave("PutLengthDet(): Fragmented Length Dets Not Supported.");
    }
}

// =========================================================
// PutExtensionItem()
//
// This function writes an extension item, including the
// length wrapper.  It makes a generic call to one of the
// H.245-defined PER Encoder routines.
// =========================================================
void PutExtensionItem(
    void (*Func)(uint8* data, PS_OutStream stream),
    uint8* x, PS_OutStream stream)
{
    PS_OutStream tempStream;

    tempStream = NewOutStream();                  /* Create a temp stream */
    Func(x, tempStream);               /* Encode x, writing to tempStream */
    PutTempStream(tempStream, stream); /* Transmit contents of tempStream */
}

// =========================================================
// PutTempStream()
//
// This function copies the contents of a temporary output
// stream (tempStream) to the main output stream (stream),
// preceeded by a generial length determinant.
// It also frees tempStream at the end.
// =========================================================
void PutTempStream(PS_OutStream tempStream, PS_OutStream stream)
{
    WriteRemainingBits(tempStream);
    PutLengthDet(tempStream->byteIndex, stream); /* Copy to real stream */
    WriteOctets(tempStream->byteIndex, tempStream->data, 0, stream);
    FreeOutStream(tempStream);
}

/********************************************************/
/*======================================================*/
/*============ OTHER ROUTINES (Generic PER) ============*/
/*======================================================*/
/********************************************************/

// =========================================================
// ConvertOutstreamToInstream()
//
// This function creates a new InStream and copies into
// it the contents of an old OutStream.  This is useful
// for internal testing.
// Note that the outstream is destroyed in the process.
// =========================================================
PS_InStream ConvertOutstreamToInstream(PS_OutStream outstream)
{
    PS_InStream instream;

    WriteRemainingBits(outstream);
    instream = (PS_InStream) OSCL_DEFAULT_MALLOC(sizeof(S_InStream));
    instream->data = outstream->data;
    instream->bitIndex = 0;

    OSCL_DEFAULT_FREE(outstream);
    return(instream);
}

// =========================================================
// NewOctetString()
//
// This function creates a new OCTETSTRING.
// For now, the data field just contains the string
// "OctetString".
// Later, I'll add input parameters and construct the
// real thing.
// THIS ROUTINE NEEDS ATTENTION!
// =========================================================
PS_OCTETSTRING NewOctetString(void)
{
    PS_OCTETSTRING x;

    x = (PS_OCTETSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_OCTETSTRING));
    x->size = 12;
    x->data = (uint8*) OSCL_DEFAULT_MALLOC(12 * sizeof(uint8));
    oscl_strncpy((char *) x->data, "OctetString", x->size);

    return(x);
}

// =========================================================
// NewBitString()
//
// This function creates a new BITSTRING.
// For now, the data field just contains the string
// "BitString".
// Later, I'll add input parameters and construct the
// real thing.
// THIS ROUTINE NEEDS ATTENTION!
// =========================================================
PS_BITSTRING NewBitString(void)
{
    PS_BITSTRING x;

    x = (PS_BITSTRING) OSCL_DEFAULT_MALLOC(sizeof(S_BITSTRING));
    x->size = 10;
    x->data = (uint8*) OSCL_DEFAULT_MALLOC(10 * sizeof(uint8));
    oscl_strncpy((char *) x->data, "BitString", x->size);

    return(x);
}

// =========================================================
// NewCharString()
//
// This function creates a new int8STRING.
// For now, the data field just contains the string
// "CharString".
// Later, I'll add input parameters and construct the
// real thing.
// THIS ROUTINE NEEDS ATTENTION!
// =========================================================
PS_int8STRING NewCharString(void)
{
    PS_int8STRING x;

    x = (PS_int8STRING) OSCL_DEFAULT_MALLOC(sizeof(S_int8STRING));
    x->size = 11;
    x->data = (uint8*) OSCL_DEFAULT_MALLOC(11 * sizeof(uint8));
    oscl_strncpy((char *) x->data, "CharString", x->size);

    return(x);
}

// =========================================================
// NewObjectID()
//
// This function creates a new OBJECTID.
// For now, the ID just contains the string "Object ID".
// Later, I'll add input parameters and construct the
// real thing.
// THIS ROUTINE NEEDS ATTENTION!
// =========================================================
PS_OBJECTIDENT NewObjectID(void)
{
    PS_OBJECTIDENT x;

    x = (PS_OBJECTIDENT) OSCL_DEFAULT_MALLOC(sizeof(S_OBJECTIDENT));
    x->size = 10;
    x->data = (uint8*) OSCL_DEFAULT_MALLOC(10 * sizeof(uint8));
    oscl_strncpy((char *) x->data, "Object ID", x->size);

    return(x);
}

// =========================================================
// InitOctetString()
//
// This function inits a previously allocated OCTET STRING.
// For now, the data field just contains the string
// "OctetString".
// Later, I'll add input parameters and construct the
// real thing.
// THIS ROUTINE NEEDS ATTENTION!
// =========================================================
void InitOctetString(PS_OCTETSTRING x)
{
    x->size = 12;
    x->data = (uint8*) OSCL_DEFAULT_MALLOC(12 * sizeof(uint8));
    oscl_strncpy((char *) x->data, "OctetString", x->size);
}

// =========================================================
// InitBitString()
//
// This function inits a previously allocated BIT STRING.
// For now, the data field just contains the string
// "BitString".
// Later, I'll add input parameters and construct the
// real thing.
// THIS ROUTINE NEEDS ATTENTION!
// =========================================================
void InitBitString(PS_BITSTRING x)
{
    x->size = 10;
    x->data = (uint8*) OSCL_DEFAULT_MALLOC(10 * sizeof(uint8));
    oscl_strncpy((char *) x->data, "BitString", x->size);
}

// =========================================================
// InitCharString()
//
// This function inits a previously allocated int8 STRING.
// For now, the data field just contains the string
// "CharString".
// Later, I'll add input parameters and construct the
// real thing.
// THIS ROUTINE NEEDS ATTENTION!
// =========================================================
void InitCharString(PS_int8STRING x)
{
    x->size = 11;
    x->data = (uint8*) OSCL_DEFAULT_MALLOC(11 * sizeof(uint8));
    oscl_strncpy((char *) x->data, "CharString", x->size);
}

// =========================================================
// InitObjectid()
//
// This function inits a previously allocated OBJECT IDENTIFIER.
// For now, the data field just contains the string
// "ObjectID".
// Later, I'll add input parameters and construct the
// real thing.
// THIS ROUTINE NEEDS ATTENTION!
// =========================================================
void InitObjectid(PS_OBJECTIDENT x)
{
    x->size = 9;
    x->data = (uint8*) OSCL_DEFAULT_MALLOC(9 * sizeof(uint8));
    oscl_strncpy((char *) x->data, "ObjectID", x->size);
}

// =========================================================
// FreeOctetString()
//
// This frees an OCTET STRING.
// =========================================================
void FreeOctetString(PS_OCTETSTRING x)
{
    if (x->size > 0)
    {
        OSCL_DEFAULT_FREE(x->data);
    }
    OSCL_DEFAULT_FREE(x);
}

// =========================================================
// FreeBitString()
//
// This frees a BIT STRING.
// =========================================================
void FreeBitString(PS_BITSTRING x)
{
    OSCL_DEFAULT_FREE(x->data);
    OSCL_DEFAULT_FREE(x);
}

// =========================================================
// FreeCharString()
//
// This frees a int8 STRING.
// =========================================================
void FreeCharString(PS_int8STRING x)
{
    if (x->data)
    {
        OSCL_DEFAULT_FREE(x->data);
    }
    OSCL_DEFAULT_FREE(x);
}

// =========================================================
// FreeObjectID()
//
// This frees an OBJECT IDENTIFIER.
// =========================================================
void FreeObjectID(PS_OBJECTIDENT x)
{
    if (x->data)
        OSCL_DEFAULT_FREE(x->data);
    OSCL_DEFAULT_FREE(x);
}


