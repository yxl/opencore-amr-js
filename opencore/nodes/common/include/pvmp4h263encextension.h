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
/**
 * @file pvmp4h263encextension.h
 */

#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#define PV_MP4_H263_ENC_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

// Forward declaration
template <class T> class OsclSharedPtr;

#define PVMp4H263EncExtensionUUID PVUuid(0xc4737ade,0xa002,0x4632,0x95,0x1a,0x9c,0xa7,0xd6,0x3a,0xcd,0x2b)

/**
 * Enumerated list of supported rate control types
 */
typedef enum
{
    PVMFVEN_RATE_CONTROL_CONSTANT_Q,
    PVMFVEN_RATE_CONTROL_CBR,
    PVMFVEN_RATE_CONTROL_VBR
} PVMFVENRateControlType;

////////////////////////////////////////////////////////////////////////////
class PVMp4H263EncExtensionInterface : public PVInterface
{
    public:
        /** Increment reference counter for this interface. */
        virtual void addRef() = 0;

        /** Decrement reference counter for this interface. */
        virtual void removeRef() = 0;

        /**
         * Query for a pointer to an instance of the interface specified by the UUID.
         *
         * @param uuid UUID of the interface to be queried.
         * @param iface Output parameter where a pointer to an instance of the requested
         * interface is stored if the interface is supported.
         * @return True if successful, else false.
         */
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        /**
         * Sets the number of layers in encoded output.
         * @param aNumLayers Number of layers.
         * @return True if successful, else false.
         */
        virtual bool SetNumLayers(uint32 aNumLayers) = 0;

        /**
         * Sets the bitrate of encoded output for the specified layer.
         * @param aLayer Layer ID
         * @param aBitRate Bitrate for the specified layer in bits per second.
         * @return True if successful, else false.
         */
        virtual bool SetOutputBitRate(uint32 aLayer, uint32 aBitRate) = 0;

        /**
         * Sets the frame size of encoded output for the specified layer.
         * @param aLayer Layer ID
         * @param aWidth Width for the specified layer in pixels.
         * @param aHeight Height for the specified layer in pixels.
         * @return True if successful, else false.
         */
        virtual bool SetOutputFrameSize(uint32 aLayer, uint32 aWidth, uint32 aHeight) = 0;

        /**
         * Sets the frame rate of encoded output for the specified layer.
         * @param aLayer Layer ID
         * @param aFrameRate Frame rate for the specified layer in frames per second.
         * @return True if successful, else false.
         */
        virtual bool SetOutputFrameRate(uint32 aLayer, OsclFloat aFrameRate) = 0;

        /**
         * Sets the segment size of encoded output for the specified layer
         * @param aLayer Layer ID
         * @param aSizeBytes Segment size in bytes.
         * @return True if successful, else false.
         */
        virtual bool SetSegmentTargetSize(uint32 aLayer, uint32 aSizeBytes) = 0;

        /**
         * Sets the rate control type to be used by encoder for the specified layer.
         * @param aLayer Layer ID
         * @param aRateControl Type of rate control to be used by encoder.
         * @return True if successful, else false.
         */
        virtual bool SetRateControlType(uint32 aLayer, PVMFVENRateControlType aRateControl) = 0;

        /**
         * Enable or disable data partitioning in the encoded MPEG4 output.
         * This menthod is valid only when the codec type is set to PVMFVEN_CODEC_M4V.
         *
         * @param aDataPartitioning Set to true to enable data partitioning, set to false
         * to disable data partitioning.
         * @return True if successful, else false
         */
        virtual bool SetDataPartitioning(bool aDataPartitioning) = 0;

        /**
         * Enable or disable reversible VLC in the encoded MPEG4 output.
         * This menthod is valid only when the codec type is set to PVMFVEN_CODEC_M4V.
         *
         * @param aRVLC Set to true to enable RVLC, set to false to disable
         * @return True if successful, else false
         */
        virtual bool SetRVLC(bool aRVLC) = 0;

        /**
         * Sets the I-Frame refresh rate of the encoded output.
         *
         * @param aIFrameInterval I-Frame rate in seconds per I-Frame
         * @return True if successful, else false
         */
        virtual bool SetIFrameInterval(uint32 aIFrameInterval) = 0;


        /**
         * Sets the automatic I-frame mode for scene change detection.
         *
         * @param aSCD  Set to true to enable scene detection
         * @return True is successful, else false.
         */
        virtual bool SetSceneDetection(bool aSCD) = 0;

        /**
         * Retrieves VOL header for the M4V bitstream.
         *
         * This method is valid only when codec type is set to PVMFVEN_CODEC_M4V. This method
         * would retrieve the VOL header for the M4V bitstream using encoder parameters
         * specified at the time of the method call.  The VOL header is stored in the
         * memory fragment provided by the caller.
         *
         * @param aVolHeader Memory fragment to store the VOL header
         * @return True if VOL header is successfully retrieved, else false.
         */
        virtual bool GetVolHeader(OsclRefCounterMemFrag& aVolHeader) = 0;

        /**
         * Requests the encoder to encode the next frame as an I-Frame.  This menthod is valid
         * only when the node is in EPVMFNodeStarted state. If successful, the next encoded
         * frame will be an I-Frame.
         *
         * @return True for success, else false
         */
        virtual bool RequestIFrame() = 0;

        /**
         * Sets the type of encoder use.
         *
         * @param aCodec Codec type.
         * @return True for success, else false
         */
        virtual bool SetCodec(PVMFFormatType aCodec)
        {
            OSCL_UNUSED_ARG(aCodec);
            return false;
        }

        /**
        * Set FSI buff to encoding param
        *
        * @param aFSIBuff			FSI Buffer
        * @param aFSIBuffLength		Length of FSI Buffer
        * @return True for success, else false
        */
        virtual bool SetFSIParam(uint8* aFSIBuff, int aFSIBuffLength) = 0;



};
#endif // PV_MP4_H263_ENC_EXTENSION_H_INCLUDED





