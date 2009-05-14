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
#ifndef PVSBCENCODER_H_INCLUDED
#define PVSBCENCODER_H_INCLUDED

// includes
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_e32std.h"
#endif

#ifndef PVSBCENCODERINTERFACE_H_INCLUDED
#include "pvsbcencoderinterface.h"
#endif

class PVSbcEncoderInterface;
class CPVSbcEncoder: public PVSbcEncoderInterface
{
    public:
        /*! \fn static CPVSbcEncoder *New()
         *  \brief A Constructor Function
         *  \return Void
         */
        static CPVSbcEncoder *New();

        /*! \fn ~CPVSbcEncoder()
         *  \brief A Destructor Function
         *  \return Void
         */
        virtual ~CPVSbcEncoder();

        /*! \fn TPvSbcEncStatus Init()
         *  \brief Used to initialize the encoder.
         *  \return A status code of type TPvSbcEncStatus.
         */
        virtual TPvSbcEncStatus Init();

        /*! \fn TPvSbcEncStatus SetInput(TPvSbcEncConfig *aEnc)
         *  \brief Used to send the input parameters to the encoder.
         *  \param aEnc - A variable of structure type TPvSbcEncConfig.
         *  \return A status code of type TPvSbcEncStatus.
         */
        virtual TPvSbcEncStatus SetInput(TPvSbcEncConfig *aEnc);

        /*! \fn TPvSbcEncStatus Execute(unsigned short* pcmbuffer, unsigned int, inBufSize, unsigned char *bitstream, unsigned int outBufSize)
         *  \brief The main encoding routine.
         *  \param pcmbuffer - Pointer to the pcm buffer to be encoded.
         *  \param inBufSize - No. of bytes in the input buffer.
         *  \param bitstream - Pointer to the encoded bitstream.
         *  \param outBufSize - No. of bytes in the output buffer.
         *  \return A status code of type TPvSbcEncStatus.
         */
        virtual TPvSbcEncStatus Execute(unsigned short* pcmbuffer, unsigned int inBufSize,
                                        unsigned char *bitstream, unsigned int *outBufSize);

        /*! \fn TPvSbcEncStatus Reset()
         *  \brief Used to reset the encoder.
         *  \return A status code of type TPvSbcEncStatus.
         */
        virtual TPvSbcEncStatus Reset();
    private:
        TPvSbcEncConfig *iEncConfig;
        bool status;

};

#endif // PVSBCENCODER_H_INCLUDED


