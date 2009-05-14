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

#ifndef _PER_H_
#define _PER_H_

#include "oscl_base.h"
#include "h245msg.h"
#include "genericper.h"

class H245;
class SE;
class PVLogger;

class PER
{
    public:
        OSCL_IMPORT_REF PER();
        OSCL_IMPORT_REF ~PER();

        void SetH245(H245 *h245)
        {
            MyH245 = h245;
        }
        void SetSe(SE *se)
        {
            MySe = se;
        }

        OSCL_IMPORT_REF void Reset();

        OSCL_IMPORT_REF void Encode(PS_H245Msg pmsg);
        OSCL_IMPORT_REF void Decode(uint8* pbuffer, uint32 size);

        static void DeleteMessage(uint8, uint8, uint8*);

    private:
        bool Decode(int& nBytes, uint8* pBuffer, uint16& GetSize, uint8*& pData,
                    uint8& MsgType1, uint8& MsgType2);
        void MultiSysCtrlMessage_encode(uint16*, uint8*	*, uint8*, uint8, uint8);
        int MultiSysCtrlMessage_decode(uint8*, uint16*, uint8*	*, uint8*, uint8*);
        void RequestMessage_encode(uint8*, uint8, PS_OutStream);
        void ResponseMessage_encode(uint8*, uint8, PS_OutStream);
        void CommandMessage_encode(uint8*, uint8, PS_OutStream);
        void IndicationMessage_encode(uint8*, uint8, PS_OutStream);
        void RequestMessage_decode(uint8* *, uint8*, PS_InStream);
        void ResponseMessage_decode(uint8* *, uint8*, PS_InStream);
        void CommandMessage_decode(uint8* *, uint8*, PS_InStream);
        void IndicationMessage_decode(uint8* *, uint8*, PS_InStream);

        H245 *MyH245;
        SE   *MySe;
        PVLogger *MyLogger;

        int32 decodeSizeLeft;
};

#endif //_PER_H_
