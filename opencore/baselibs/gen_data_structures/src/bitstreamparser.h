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

#ifndef BITSTREAMPARSER_H_INCLUDED
#define BITSTREAMPARSER_H_INCLUDED


#include "oscl_assert.h"
#include "oscl_error_codes.h"
#include "oscl_exception.h"

#define MOST_SIG_BIT              7
#define LEAST_SIG_BIT             0
#define BITS_PER_BYTE             8
#define BITS_PER_UINT8            8
#define BITS_PER_UINT16          16
#define BITS_PER_UINT32          32
#define LEAST_SIG_3_BITS_MASK  0x07


//Macro to convert bits to bytes (rounding up to next whole byte).
#define BITS_TO_BYTES(bits) ((bits + MOST_SIG_BIT) / BITS_PER_BYTE)


class BitStreamParser
{
    public:
        //Constructor requires a pointer to the stream and the size in bytes.
        OSCL_IMPORT_REF BitStreamParser(uint8* stream, uint32 size);

        // Reset bitstream parser
        OSCL_IMPORT_REF void ResetBitStreamParser(uint8* stream, uint32 size);

        //Returns the specified number of bits from the stream (up to 32 bits).
        OSCL_IMPORT_REF uint32 ReadBits(uint8 numberOfBits);

        //Returns a 8-bit byte from the stream.
        OSCL_IMPORT_REF uint8  ReadUInt8(void);

        //Returns a 16-bit word from the stream, converting from network-byte-order to host-byte-order.
        OSCL_IMPORT_REF uint16 ReadUInt16(void);

        //Returns a 32-bit long from the stream, converting from network-byte-order to host-byte-order.
        OSCL_IMPORT_REF uint32 ReadUInt32(void);

        //Writes the specified number of bits to the stream.
        OSCL_IMPORT_REF void   WriteBits(uint8 numberOfBits, const uint8* data);

        //Writes 8 bits to the stream.
        OSCL_IMPORT_REF void   WriteUInt8(uint8 data);

        //Writes 16 bits to the stream, converting from host-byte-order to network-byte-order.
        OSCL_IMPORT_REF void   WriteUInt16(uint16 data);

        //Write 32 bits to the stream, converting from host-byte-order to network-byte-order.
        OSCL_IMPORT_REF void   WriteUInt32(uint32 data);

        //Skips over exactly one bit.  This is more efficient than NextBits(1).
        OSCL_EXPORT_REF inline void NextBit(void)
        {
            if (0 != bitpos)
            {
                bitpos--;
            }
            else
            {
                if (bytepos >= (start + size))
                {
                    OSCL_LEAVE(OsclErrOverflow);
                }
                bitpos = MOST_SIG_BIT;
                bytepos++;
            }
        }

        //Skips over the specified number if bits.
        OSCL_IMPORT_REF void   NextBits(uint32 numberOfBits);

        //Returns the byte position of the stream.
        OSCL_EXPORT_REF inline void*  GetBytePos(void) const
        {
            return bytepos;
        }

        //Returns the bit position of the current byte.
        OSCL_EXPORT_REF inline uint8  GetBitPos(void) const
        {
            return bitpos;
        }

        //The returns the number of whole bytes read (not including partial bytes).
        OSCL_EXPORT_REF inline uint32 BytesRead(void) const
        {
            return (bytepos - start);
        }

        OSCL_EXPORT_REF inline uint32 BitsRead(void) const
        {
            return (BytesRead() * BITS_PER_BYTE) + (MOST_SIG_BIT - bitpos);
        }

        //This is only 100% accurate when the bitpos is 7.
        //Otherwise, part of the first byte was already consumed.
        OSCL_EXPORT_REF inline uint32 BytesLeft(void) const
        {
            return size - BytesRead();
        }

        OSCL_EXPORT_REF inline uint32 BitsLeft(void) const
        {
            return ((BytesLeft() - 1) * BITS_PER_BYTE) + (1 + bitpos);
        }

        OSCL_EXPORT_REF inline uint32 GetSize(void) const
        {
            return size;
        }

    private:
        uint8* start;
        uint32 size;
        uint8* bytepos;	//The current byte position in the stream.
        uint8  bitpos;  //The current bit position of the current byte.  Counts down from 7 to 0.
};


#endif  //BITSTREAMPARSER_H_INCLUDED


