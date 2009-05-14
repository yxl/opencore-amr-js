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
#ifndef PVSBCENCODERINTERFACE_H_INCLUDED
#define PVSBCENCODERINTERFACE_H_INCLUDED

// includes
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_e32std.h"
#endif


#define MAX_SZOF_PCM_BUFF   (512)   /* size in bytes */
#define MAX_SZOF_BS_BUFF    (1036)  /* size in bytes */

/*! \enum TPvSbcChanMode
 *  \brief Two bits to indicate the channel mode.
 *  \param CM_MONO - Mono.
 *  \param CM_DUAL_CHANNEL - Dual channel.
 *  \param CM_STEREO - Stereo.
 *  \param CM_JOINT_STEREO - Joint Stereo mode.
 */
typedef enum TPvSbcChanMode
{
    CM_MONO,
    CM_DUAL_CHANNEL,
    CM_STEREO,
    CM_JOINT_STEREO
}TPvSbcChanMode;

/*! \enum TPvSbcEncStatus
 *  \brief An enumeration type that defines the Encoder status.
 *  \param TPVSBCENC_SUCCESS = 0 - A successful function call.
 *  \param TPVSBCENC_FAIL = 1 - An unsuccessful function call.
 *  \param TPVSBCENC_INVALID_PARAMS = 2 - Invalid encoding parameters passed.
 *  \param TPVSBCENC_INSUFFICIENT_MEMORY = 3 - Memory allocation failure.
 *  \param TPVSBCENC_INSUFFICIENT_INPUT_DATA = 4 - Insufficient input data.
 */

enum TPvSbcEncStatus
{
    TPVSBCENC_SUCCESS = 0,
    TPVSBCENC_FAIL = 1,
    TPVSBCENC_INVALID_PARAMS = 2,
    TPVSBCENC_INSUFFICIENT_MEMORY = 3,
    TPVSBCENC_INSUFFICIENT_INPUT_DATA = 4
};


/*! \struct TPvSbcEncConfig
 *  \brief A structure used for providing the required information to the Encoder
 *  and obtaining the desired encoded bitstream.
 *  \param sampling_frequency - Sampling frequency in Hz.
 *  \param nrof_channels - Number of channels.
 *  \param channel_mode - Channel mode (Mono, Dual, Stereo, Joint Stereo).
 *  \param block_len - Block length.
 *  \param nrof_subbands - Number of subbands.
 *  \param bitpool - Size of the bit allocation pool.
 *  \param allocation_method - 0 for loudness and 1 for SNR.
 *  \param join - Set to 1 of enabling Joint Stereo.
 *  \param pcmbuffer - Pointer to the input PCM buffer.
 *  \param bitstream - Pointer to the output bitstream buffer.
 *  \param framelen - Frame lenght in bytes.
 *  \param state - A pointer for internal use.
 */
typedef struct  TPvSbcEncConfig
{
    uint    sampling_frequency;
    uint8   nrof_channels;
    uint    channel_mode;
    uint8   block_len;
    uint8   nrof_subbands;
    uint8   bitpool;
    uint    allocation_method;
    uint8   join;
    uint16  *pcmbuffer;
    uint8   *bitstream;
    uint    framelen;
    void    *state;
} TPvSbcEncConfig;


// PVSbcEncoderInterface pure virtual interface class
class PVSbcEncoderInterface
{
    public:
        /*! \fn ~PVSbcEncoderInterface()
         *  \brief A destructor function
         *  \return Void
         */
        OSCL_IMPORT_REF virtual ~PVSbcEncoderInterface() {};

        /*! \fn TPvSbcEncStatus Init()
         *  \brief Used to initialize the encoder.
         *  \return A status code of type TPvSbcEncStatus.
         */
        OSCL_IMPORT_REF virtual TPvSbcEncStatus Init() = 0;

        /*! \fn TPvSbcEncStatus SetInput(TPvSbcEncConfig *aEnc)
         *  \brief Used to send the input parameters to the encoder.
         *  \param aEnc - A variable of structure type TPvSbcEncConfig.
         *  \return A status code of type TPvSbcEncStatus.
         */
        OSCL_IMPORT_REF virtual TPvSbcEncStatus SetInput(TPvSbcEncConfig *aEnc) = 0;

        /*! \fn TPvSbcEncStatus Execute(unsigned short* pcmbuffer, unsigned int, inBufSize, unsigned char *bitstream, unsigned int outBufSize)
         *  \brief This is the main encoding routine of the library.
         *  \param pcmbuffer - Pointer to the pcm buffer to be encoded.
         *  \param bitstream - Pointer to the encoded bitstream.
         *  \return A status code of type TPvSbcEncStatus.
         */
        OSCL_IMPORT_REF virtual TPvSbcEncStatus Execute(uint16* pcmbuffer, uint inBufSize,
                uint8 *bitstream, uint *outBufSize) = 0;

        /*! \fn TPvSbcEncStatus Reset()
         *  \brief Used to reset the encoder.
         *  \return A status code of type TPvSbcEncStatus.
         */
        OSCL_IMPORT_REF virtual TPvSbcEncStatus Reset() = 0;
};

#endif // PVSBCENCODERINTERFACE_H_INCLUDED


