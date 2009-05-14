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
 * @file pvaetestinput.h
 * @brief Test input to PVAuthorEngine unit test, using single core encoders
 */

#ifndef PVAETESTINPUT_H_INCLUDED
#define PVAETESTINPUT_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMI_MIO_CONTROL_H_INCLUDED
#include "pvmi_mio_control.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVMI_MIO_FILEINPUT_FACTORY_H_INCLUDED
#include "pvmi_mio_fileinput_factory.h"
#endif

// Forward declarations
class PVMFNodeInterface;
class PVInterface;


class AVTConfig
{
    public:
        AVTConfig()
        {
            iWidth = 0;
            iHeight = 0;
            iFps = 0;
            iFrameInterval = 0;
            iSamplingRate = 0;
            iNumChannels = 0;
            iLoopingEnable = false;

        };

        ~AVTConfig()
        {
            iWidth = 0;
            iHeight = 0;
            iFps = 0;
            iFrameInterval = 0;
            iSamplingRate = 0;
            iNumChannels = 0;
            iLoopingEnable = false;

        };

        AVTConfig(AVTConfig& arATVConfig)
        {
            iWidth = arATVConfig.iWidth;
            iHeight = arATVConfig.iHeight;
            iFps = arATVConfig.iFps;
            iFrameInterval = arATVConfig.iFrameInterval;
            iSamplingRate = arATVConfig.iSamplingRate;
            iNumChannels = arATVConfig.iNumChannels;
            iTextLogFile = arATVConfig.iTextLogFile;
            iTextConfigFile = arATVConfig.iTextConfigFile;
            iLoopingEnable = arATVConfig.iLoopingEnable;

        }

        int iWidth;
        int iHeight;
        OsclFloat iFps;
        int iFrameInterval;
        int iSamplingRate;
        int iNumChannels;
        OSCL_HeapString<OsclMemAllocator> iTextLogFile;
        OSCL_HeapString<OsclMemAllocator> iTextConfigFile;
        bool iLoopingEnable;
};

/** Enumerated list of test input types */
typedef enum
{
    INVALID_INPUT_TYPE = 0
    , YUV_FILE
    , M4V_FILE
    , H263_FILE
    , H264_FILE
    , AMR_IETF_FILE
    , AAC_ADIF_FILE
    , AAC_ADTS_FILE
    , PCM16_FILE
    , SYMBIAN_DEV_SOUND
    , YUV_WRONG_FILE //just introduced for error handling scenarios
    , TEXT_FILE
    , AMRWB_IETF_FILE
} PVAETestInputType;

#define ARRAY_SIZE	512

////////////////////////////////////////////////////////////////////////////
class PVAETestInput
{
    public:
        /** Default contructor */
        PVAETestInput();

        /** Default destructor */
        ~PVAETestInput() {};

        /**
         * Copy constructor
         * @param aInput Source object
         */
        PVAETestInput(const PVAETestInput& aInput);

        /**
         * Overloaded = operator
         * @param aInput Source object
         * @return Reference to this object with values copied from the source object
         */
        PVAETestInput& operator=(const PVAETestInput& aInput);

        /**
         * Query whether the specified input type is supported
         *
         * @param aType Input type to be supported
         * @return True if input type is supported, else false
         */
        bool IsTestInputTypeSupported(PVAETestInputType aType);

        /**
         * Creates an input node of the specified type.  DeleteInputNode() will need to
         * be called to deallocate the input node.  The default contructor will not
         * delete the input node created by this method.
         *
         * @param aType Type of input node to create
         * @return True for success, else false
         */
        int CreateInputNode(PVAETestInputType aType, const OSCL_wString& aFileName, AVTConfig iAVTConfig);

        /**
         * Deletes the input node contained by this object
         * @return True for success, else false
         */
        bool DeleteInputNode();

        PVMFNodeInterface* iNode; /** Input node */
        PVAETestInputType iType; /** Input type */
        PvmiMIOFileInputSettings iSettings; /**  settings data */
        Oscl_Vector<PVInterface*, OsclMemAllocator> iExtensions; /** Vector of extensions to the input node */
        // Media input module
        PvmiMIOControl* iMediaInput;
        uint32 iDuration;

    private:
        /**
         * Copy data from source object to this object
         * @param aInput Source object
         */
        void Copy(const PVAETestInput& aInput);

        /**
         * Creates PVMFFileInputNode of the specified type
         *
         * @param aType Type of test input
         * @return True for success, else false
         */
        bool CreateFileInputNode(PVAETestInputType aType);

        /**
         * Creates the MediaIO module object using its factory::create call and
         * also creates PVMediaIONode for it
         *
         * @param aType Type of test input
         * @return True for success, else false
         */
        bool CreateMIOInputNode(PVAETestInputType aType, const OSCL_wString& aFileName, AVTConfig iAVTConfig);

        PVLogger* iLogger; /** Logger object */
};

#endif // PVAETESTINPUT_H_INCLUDED



