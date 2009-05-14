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
* ==============================================================================
*  Name        : pv_plugin_interfaces.h
*  Part of     :
*  Interface   :
*  Description : Defines the general plugin interfaces for the PV engines
*  Version     : (see RELEASE field in copyright header above)
*
* ==============================================================================
*/

#ifndef PV_PLUGIN_INTERFACES_H_INCLUDED
#define PV_PLUGIN_INTERFACES_H_INCLUDED


//  INCLUDES
#include "pv_common_symbian_types.h"

// CONSTANTS
/**
 * MPVDataSourceBase Event categories
 *
 * These are the UIDs of the categories that should be returned via the MAsyncEventHandler
 * interface for the async event callbacks.
 **/
const TInt KPVUidDataSrcPrime	= 0xFFFFFF08;
const TInt KPVUidDataSrcPlay	= 0xFFFFFF09;
const TInt KPVUidDataSrcPause	= 0xFFFFFF0A;
const TInt KPVUidDataSrcStop	= 0xFFFFFF0B;

/**
 * MPVDataSourceBase Error Codes
 *
 * Any System Wide codes or PV defined error codes can be returned.  If a new error code needs to
 * be defined specific to this interface, it should be added here.
 **/

/**
 * MPVDataSinkBase Event categories
 *
 * These are the UIDs of the categories that should be returned via the MAsyncEventHandler
 * interface for the async event callbacks.
 **/
const TInt KPVUidDataSinkPrime	= 0xFFFFFF0C;
const TInt KPVUidDataSinkPlay	= 0xFFFFFF0D;
const TInt KPVUidDataSinkPause	= 0xFFFFFF0E;
const TInt KPVUidDataSinkStop	= 0xFFFFFF0F;


/**
 * MPVDataSinkBase Error Codes
 *
 * Any System Wide codes or PV defined error codes can be returned.  If a new error code needs to
 * be defined specific to this interface, it should be added here.
 **/

/**
 * Uid for MPVYuvFrameBuffer interface
 */
const TUid KPVUidYUVFrameBuffer = {0xFFFFFF0d};


// MACROS
/**
 * Uuid for Querying the MPVAudioInput interface
 */
#define KPVUidAudioInputInterface             TPVUuid(0x194e8655,0x944c,0x402c,0xb0,0xc2,0xf7,0xbd,0xd5,0xe5,0x43,0x2f)
/**
 * Uuid for Querying the MPVDevSoundAudioInput interface
 */
#define KPVUidDevSoundAudioInputInterface     TPVUuid(0x9e2c416e,0x0299,0x4775,0x88,0xfa,0x42,0x53,0xbc,0xbc,0x58,0xbf)
/**
 * Uuid for Querying the MPVAudioOutput interface
 */
#define KPVUidAudioOutputInterface            TPVUuid(0xf5c5b825,0x90eb,0x4091,0xbe,0xea,0xa0,0xc3,0x9b,0xe2,0x00,0xaf)
/**
 * Uuid for Querying the MPVDevSoundAudioInput interface
 */
#define KPVUidDevSoundAudioOutputInterface    TPVUuid(0x48edb46a,0x60e4,0x4e83,0xb1,0xad,0x92,0xa8,0xd4,0x07,0x04,0x7a)
/**
 * Uuid for Querying the MPVVideoInput interface
 */
#define KPVUidVideoInputInterface             TPVUuid(0xfb320151,0x6d06,0x4bd5,0xa2,0x68,0x61,0x01,0xdb,0x25,0x1c,0x0e)
/**
 * Uuid for Querying the MPVVideoOutput interface
 */
#define KPVUidVideoOutputInterface            TPVUuid(0x0bb9d8a8,0x9623,0x4dec,0x84,0x0b,0xb9,0xf2,0x66,0xf8,0x4e,0x3d)
/**
 * Uuid for Querying the Proxied version of any interface
 */
#define KPVUidProxiedInterface                TPVUuid(0xf7076653,0x6088,0x47c6,0x88,0xc1,0xb7,0xed,0x28,0xe7,0x2b,0xea)


// DATA TYPES


// FUNCTION PROTOTYPES


// FORWARD DECLARATIONS



// CLASS DECLARATION
/**
 * MPVPluginBase Class
 *
 * Base class for all supported plugins
 **/
class MPVPluginBase
{
    public:
        virtual ~MPVPluginBase() {}

        /**
         * This API returns multimedias type supported by the data source/sink -
         * Audio, Video, Data etc.  Each supported type is indicated by a MIME type structure.
         * @return Multimedia types supported by the data source/sink.  The reference
         * is valid until the MPVPluginBase derived object is destroyed.
         **/
        IMPORT_C virtual const RArray<TPVMIMEType *>& GetMultimediaTypesL() const = 0;

        /**
         * This API is to allow for extensibility of the plugin interface.
         * It allows a caller to ask for all UUIDs associated with a particular MIME type.
         * If interfaces of the requested MIME type are found within the plugin, they are added
         * to the UUIDs array.
         *
         * Also added to the UUIDs array will be all interfaces which have the requested MIME
         * type as a base MIME type.  This functionality can be turned off.
         *
         * @param aMimeType The MIME type of the desired interfaces
         * @param aUuids An array to hold the discovered UUIDs
         * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
         **/
        IMPORT_C virtual void QueryUUID(const TPVMIMEType& aMimeType, RArray<TPVUuid>& aUuids, bool aExactUuidsOnly = false) = 0;

        /**
         * This API is to allow for extensibility of the plugin interface.
         * It allows a caller to ask for an instance of a particular interface object to be returned.
         * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
         * an interface ID that is a UUID as in DCE and a pointer to the interface object is
         * returned if it is supported.  Otherwise the returned pointer is NULL.
         * TBD:  Define the UIID, InterfacePtr structures
         * @param aUuid The UUID of the desired interface
         * @param aInterfacePtr The output pointer to the desired interface
         **/
        IMPORT_C virtual void QueryInterface(const TPVUuid& aUuid, TPVInterfacePtr& aInterfacePtr) = 0;
};

//#define USE_MMF_DATA_PATH 1

#if defined USE_MMF_DATA_PATH

#define MPVDataSourceBase MDataSource
#define MPVDataSinkBase MDataSink

#else /* USE_MMF_DATA_PATH */

// forward declaration
class MPVDataSinkBase;

/**
 * MPVDataSourceBase Class
 *
 * Base class for data sources
 **/
class MPVDataSourceBase
{
    public:
        MPVDataSourceBase(TUid aType): iDataSourceType(aType) {}
        virtual ~MPVDataSourceBase() {}

        /**
         * Method called by a MDataSink to request the data source to fill aBuffer with data.
         *
         * This is a pure virtual function that each derived class must implement.
         * This method is used when a data source is passively waiting for requests from a consumer ie a data sink
         * to fill a buffer.  The data source must call the BufferFilledL member on aConsumer when it has filled
         * the buffer with data - the data source can either make this callback synchronously or asynchronously.
         *
         * @param	"aBuffer"
         *			The buffer that needs filling with data
         *
         * @param	"aConsumer"
         *			The data sink that consumes the data. The data source needs this to make the BufferFilledL
         *			callback on aConsumer when the data source has completed filling the aBuffer.
         *
         * @param	"aMediaId"
         *			This identifies the type of media eg audio or video and the stream id.
         *			This parameter is required in cases where the source can supply data
         * 			of more than one media type and/or multiple strams of data eg a multimedia file
         */
        virtual void FillBufferL(CMMFBuffer* aBuffer, MPVDataSinkBase* aConsumer, TMediaId /*aMediaId*/) = 0;

        /**
         * Method called by a data sink to pass back an emptied buffer to the source
         *
         * This is a pure virtual function that each derived class must implement.
         * This method is used as the callback when the data source actively requests a consumer ie a data sink
         * to empty a buffer by calling the data sinks EmptyBufferL.
         * When the data source gets this callback it knows that the buffer has been emptied and can be reused
         *
         * @param	"aBuffer"
         *			The buffer that has been emptied by a data sink and is now available for reuse
         */
        virtual void BufferEmptiedL(CMMFBuffer* aBuffer) = 0;

        /**
         * Method to indicate whether the data source can create a buffer.
         *
         * This is a pure virtual function that each derived class must implement.
         *
         * @return	ETrue if the data source can create a buffer else EFalse
         */
        virtual TBool CanCreateSourceBuffer() = 0;

        /**
         * Returns a buffer created by the data source
         *
         * This is a pure virtual function that each derived class must implement.
         *
         * @param	"aMediaId"
         *			This identifies the type of media eg audio or video and the stream id.
         *			This parameter is required in cases where the source can supply data
         * 			of more than one media type and/or multiple strams of data eg a multimedia file
         *
         * @param	"aReference"
         *			This must be written to by the method to indicate whether the created buffer is
         *			a 'reference' buffer.  A 'reference' buffer is a buffer that is owned by the source
         *			and should be used in preference to the sink buffer provided the sink buffer
         *			is also not a reference buffer
         * .
         * @return	The created buffer
         */
        virtual CMMFBuffer* CreateSourceBufferL(TMediaId /*aMediaId*/, TBool &/*aReference*/)
        {
            return NULL;
        }


        /**
         * Returns a buffer created by the data source
         *
         * This is a virtual function that a derived class can implement.
         * This can be used in preference to the above CreateSourceBufferL method in cases where
         * the source buffer creation has a dependancy on the sink buffer
         *
         * @param	"aMediaId"
         *			This identifies the type of media eg audio or video and the stream id.
         *			This parameter is required in cases where the source can supply data
         * 			of more than one media type and/or multiple strams of data eg a multimedia file
         *
         * @param	"aSinkBuffer"
         *			The sink buffer the nature of which may influence the creation of the source buffer
         *
         * @param	"aReference"
         *			This must be written to by the method to indicate whether the created buffer is
         *			a 'reference' buffer.  A 'reference' buffer is a buffer that is owned by the source
         *			and should be used in preference to the sink buffer provided the sink buffer is not a reference buffer
         * .
         * @return	The created buffer
         */
        virtual CMMFBuffer* CreateSourceBufferL(TMediaId /*aMediaId*/, CMMFBuffer& /*aSinkBuffer*/, TBool &/*aReference*/)
        {
            return NULL;
        }


        /**
         * Method to 'logon' the data source to the same thread that source will be supplying data in.
         *
         * This method may be required as the thread that the data source was created in is not always
         * the same thread that the data transfer will take place in.  Therefore any thread specific
         * initialisation needs to be performed in the SourceThreadLogon rather than in the creation
         * of the data source.
         *
         * This is a virtual function that a derrived data source can implement if any thread specific
         * initialisation is required and/or the data source can create any asynchronous events.
         *
         *
         * @param	"aEventHandler"
         *			This is an MAsyncEventHandler to handle asynchronous events that occur during the
         *			transfer of multimedia data.  The event handler must be in the same thread as the data transfer
         * 			thread - hence the reason it is passed in the SourceThreadLogon as opposed to say the constructor.
         *
         *
         * @return	KErrNone if successful, otherwise a system wide error code.
         */
        virtual TInt SourceThreadLogon(MAsyncEventHandler& /*aEventHandler*/)
        {
            return KErrNone;
        }

        /**
         * Method to 'logoff' the data source from the same thread that source supplies data in.
         *
         * This method may be required as the thread that the data source is deleted in may not be
         * the same thread that the data transfer took place in.  Therefore any thread specific
         * releasing of resources needs to be performed in the SourceThreadLogoff rather than in the destructor
         *
         * This is a virtual function that a derrived data source can implement if any thread specific
         * releasing of resources is required.
         */
        virtual void SourceThreadLogoff() {};

        /**
         * Method to 'prime' the data source
         *
         * This is a virtual function that a derrived data source can implement if
         * any data source specific 'priming' is required
         */
        virtual TInt SourcePrimeL()
        {
            return 0;
        }

        /**
         * Method to 'play' the data source
         *
         * This is a virtual function that a derrived data source can implement if
         * any data source specific action is required prior to 'playing'ie the start of data transfer
         */
        virtual TInt SourcePlayL()
        {
            return 0;
        };

        /**
         * Method to 'pause' the data source
         *
         * This is a virtual function that a derrived data source can implement if
         * any data source specific action is required to 'pause'
         */
        virtual TInt SourcePauseL()
        {
            return 0;
        };

        /**
         * Method to 'stop' the data source
         *
         * This is a virtual function that a derrived data source can implement if
         * any data source specific action is required to 'stop'
         */
        virtual TInt SourceStopL()
        {
            return 0;
        };
    private:
        TUid iDataSourceType;
};


/**
 * MPVDataSourceBase Class
 *
 * Base class for data sinks
 **/
class MPVDataSinkBase
{
    public:
        MPVDataSinkBase(TUid aType): iDataSinkType(aType) {}
        virtual ~MPVDataSinkBase() {}

        /**
         * Method called by a MDataSource to request the data sink to empty aBuffer of data.
         *
         * This is a pure virtual function that each derived class must implement.
         * This method is used when a data sink is passively waiting for requests from a supplier ie a data source
         * to empty a buffer.  The data sink must call the BufferEmptiedL member on aSupplier when it has emptied
         * the buffer of it's data - the data sink can either make this callback synchronously or asynchronously.
         *
         * @param	"aBuffer"
         *			The full buffer that needs emptying of it's data
         *
         * @param	"aSupplier"
         *			The data source that supplied the data. The data sink needs this to make the BufferEmptiedL
         *			callback on aSupplier to indicate to the data source that the data sink has finished with the buffer.
         *
         * @param	"aMediaId"
         *			This identifies the type of media eg audio or video and the stream id.
         *			This parameter is required in cases where the source can supply data
         * 			of more than one media type and/or multiple strams of data
         */
        virtual void EmptyBufferL(CMMFBuffer* aBuffer, MPVDataSourceBase* aSupplier, TMediaId /*aMediaId*/) = 0;

        /**
         * Method called by a data source to pass back an filled buffer to the sink
         *
         * This is a pure virtual function that each derived class must implement.
         * This method is used as the callback when the data sink actively requests a supplier ie a data source
         * to fill a buffer by calling the data sources FillBufferL.
         * When the data sink gets this callback it knows that the buffer has been filled and is ready to be emptied
         *
         * @param	"aBuffer"
         *			The buffer that has been filled by a data source and is now available for processing
         */
        virtual void BufferFilledL(CMMFBuffer* aBuffer) = 0;

        /**
         * Method to indicate whether the data sink can create a buffer.
         *
         * This is a pure virtual function that each derived class must implement.
         *
         * @return	ETrue if the data sink can create a buffer else EFalse
         */
        virtual TBool CanCreateSinkBuffer() = 0;

        /**
         * Returns a buffer created by the data sink
         *
         * This is a pure virtual function that each derived class must implement.
         *
         * @param	"aMediaId"
         *			This identifies the type of media eg audio or video and the stream id.
         *			This parameter is required in cases where the source can supply data
         * 			of more than one media type and/or multiple strams of data.
         *
         * @param	"aReference"
         *			This must be written to by the method to indicate whether the created buffer is
         *			a 'reference' buffer.  A 'reference' buffer is a buffer that is owned by the sink
         *			and should be used in preference to the source buffer provided the source buffer
         *			is also not a reference buffer.
         * .
         * @return	The created buffer
         */
        virtual CMMFBuffer* CreateSinkBufferL(TMediaId /*aMediaId*/, TBool &/*aReference*/)
        {
            return NULL;
        }

        /**
         * Method to 'logon' the data sink to the same thread that sink will be consuming data in.
         *
         * This method may be required as the thread that the data sink was created in is not always
         * the same thread that the data transfer will take place in.  Therefore any thread specific
         * initialisation needs to be performed in the SinkThreadLogon rather than in the creation
         * of the data sink.
         *
         * This is a virtual function that a derrived data sink can implement if any thread specific
         * initialisation is required and/or the data sink can create any asynchronous events.
         *
         *
         * @param	"aEventHandler"
         *			This is an MAsyncEventHandler to handle asynchronous events that occur during the
         *			transfer of multimedia data.  The event handler must be in the same thread as the data transfer
         * 			thread - hence the reason it is passed in the SinkThreadLogon as opposed to say the constructor.
         *
         *
         * @return	KErrNone if successful, otherwise a system wide error code.
         */
        virtual TInt SinkThreadLogon(MAsyncEventHandler& /*aEventHandler*/)
        {
            return KErrNone;
        };

        /**
         * Method to 'logoff' the data sink from the same thread that sink consumes data in.
         *
         * This method may be required as the thread that the data sink is deleted in may not be
         * the same thread that the data transfer took place in.  Therefore any thread specific
         * releasing of resources needs to be performed in the SinkThreadLogoff rather than in the destructor
         *
         * This is a virtual function that a derrived data sink can implement if any thread specific
         * releasing of resources is required.
         */
        virtual void SinkThreadLogoff() {};

        /**
         * Method to 'prime' the data sink
         *
         * This is a virtual function that a derrived data sink can implement if
         * any data sink specific 'priming' is required
         */
        virtual TInt SinkPrimeL()
        {
            return 0;
        };

        /**
         * Method to 'play' the data sink
         *
         * This is a virtual function that a derrived data sink can implement if
         * any data sink specific action is required prior to 'playing'ie the start of data transfer
         */
        virtual TInt SinkPlayL()
        {
            return 0;
        };

        /**
         * Method to 'pause' the data sink
         *
         * This is a virtual function that a derrived data sink can implement if
         * any data sink specific action is required to 'pause'
         */
        virtual TInt SinkPauseL()
        {
            return 0;
        };

        /**
         * Method to 'stop' the data sink
         *
         * This is a virtual function that a derrived data sink can implement if
         * any data sink specific action is required to 'stop'
         */
        virtual TInt SinkStopL()
        {
            return 0;
        };
    private:
        TUid iDataSinkType;
};

#endif



/**
 * MPVDataSource Class
 *
 * PV extension to MDataSource that supports basic PV requirements like exposing
 * capabilities, configuration interfaces etc
 **/
class MPVDataSource : public MPVPluginBase, public MPVDataSourceBase
{
    public:
        MPVDataSource(TUid aSourceType) : MPVDataSourceBase(aSourceType) {}
        virtual ~MPVDataSource() {}
};


/**
 * MPVDataSink Class
 *
 * PV extension to MDataSource that supports basic PV requirements like exposing
 * capabilities, configuration interfaces etc
 **/
class MPVDataSink : public MPVPluginBase, public MPVDataSinkBase
{
    public:
        MPVDataSink(TUid aSinkType) : MPVDataSinkBase(aSinkType) {}
        virtual ~MPVDataSink() {}
};


/**
 * Supports the basic functionality of both PV Data Sources
 * and Sinks.
 */
class MPVDataSourceAndSink :
            public MPVPluginBase,
            public MPVDataSourceBase,
            public MPVDataSinkBase
{
    public:
        MPVDataSourceAndSink(TUid aSourceType, TUid aSinkType) :
                MPVDataSourceBase(aSourceType),
                MPVDataSinkBase(aSinkType)
        {}
        virtual ~MPVDataSourceAndSink() {}
};


/**
 * Supported interfaces for external data sinks and sources
 *
 * The following are additional interfaces that can be implemented by external data sources
 * and sinks to allow the SDKs to access a richer set of configuration APIs.
 **/


/**
 * MPVAudioInput Class
 *
 * MPVAudioInput cab be implemented by any audio data source that needs to work with
 * PV SDKs.
 **/
class MPVAudioInput
{
    public:
        //To set data format using MIME string.
        IMPORT_C virtual void SetFormatL(const TDesC8& aFormat, const TDesC8& aFmtSpecific, TInt &aMaxRequestSize) = 0;

        //To set sample rate and number of channels using MIME string.
        IMPORT_C virtual void SetConfigL(const TDesC8& aSampleRate
                                         , const TDesC8& aChannels) = 0;

        //To cancel current command.
        IMPORT_C virtual void CancelCommand() = 0;

        //To reset hardware.
        IMPORT_C virtual TInt Reset() = 0;
};


//Additional methods for DevSound audio source.
class MPVDevSoundAudioInput
{
    public:
        IMPORT_C virtual void SetPrioritySettings(const TMMFPrioritySettings &aSettings) = 0;
        IMPORT_C virtual void SetInputFormatL(const TDesC8& aFormat, MPVDataSourceBase *) = 0;
        IMPORT_C virtual TPVAudioOutputSwitch OutputSwitch() = 0;
        IMPORT_C virtual TBool FillAmrBuffersToEnd() = 0;
};


/**
 * MPVAudioOutput Class
 *
 * MPVAudioOutput can be implemented by any audio data sink that needs to work with
 * PV SDKs.
 **/
class MPVAudioOutput
{
    public:
        //To set data format using MIME string.
        IMPORT_C virtual void SetFormatL(const TDesC8& aFormat) = 0;
        //To set sample rate and number of channels using MIME string.
        IMPORT_C virtual void SetConfigL(const TDesC8& aSampleRate
                                         , const TDesC8& aChannels) = 0;
        //To cancel current command.
        IMPORT_C virtual void CancelCommand() = 0;
        //To reset hardware.
        IMPORT_C virtual TInt Reset() = 0;
};


//Additional methods for DevSound audio sink.
class MPVDevSoundAudioOutput
{
    public:
        IMPORT_C virtual void SetPrioritySettings(const TMMFPrioritySettings &aSettings) = 0;
        IMPORT_C virtual void ConcealErrorForNextBuffer() = 0;
        IMPORT_C virtual void SetOutputFormatL(const TDesC8& aFormat, const TDesC8& aFmtSpecific, MPVDataSinkBase* aConsumer, TInt &aMaxRequestSize) = 0;
        IMPORT_C virtual TPVAudioOutputSwitch OutputSwitch() = 0;
        IMPORT_C virtual TBool FillAmrBuffersToEnd() = 0;
};


/**
 * MPVVideoInput Class
 *
 * MPVVideoInput can be implemented by any video data source that needs to work with
 * PV SDKs.
 **/
class MPVVideoInput
{
    public:

        /**
         * Set the video frame format.  This must be from the list of supported formats.
         * @param "aFormat" The mime string describing the video frame format.
         * @exception Can leave with one of the system wide error codes
         */
        IMPORT_C virtual void SetFormatL(const TDesC8& aFormat) = 0;

        /**
         * Set the video frame rate.  This must be within the range of supported frame rates
         * for the current frame size.
         * @param "aFrameRate" The video frame rate to set.
         * @exception Can leave with one of the system wide error codes
         */
        IMPORT_C virtual void SetFrameRateL(TReal32 aFrameRate) = 0;

        /**
         * Set the video frame size
         * @param "aSize" The video frame size, in pixels
         * @exception Can leave with one of the system wide error codes
         */
        IMPORT_C virtual void SetVideoFrameSizeL(const TSize& aSize) = 0;

        /**
         * Get the video frame size
         * @param "aSize" The video frame size, in pixels
         * @exception Can leave with one of the system wide error codes
         */
        IMPORT_C virtual void GetVideoFrameSizeL(TSize& aSize) const = 0;
};


/**
 * MPVVideoOutput Class
 *
 * MPVVideoOutput can be implemented by any video data sink that needs to work with
 * PV SDKs.
 **/
class MPVVideoOutput
{
    public:
        /**
         * Set the video frame format.  This must be from the list of supported formats.
         * @param "aFormat" A mime string describing the video frame format.
         * @exception Can leave with one of the system wide error codes
         */
        IMPORT_C virtual void SetFormatL(const TDesC8& aFormat) = 0;

        /**
         * Set the video frame size
         * @param "aSize" The video frame size, in pixels
         * @exception Can leave with one of the system wide error codes
         */
        IMPORT_C virtual void SetVideoFrameSizeL(const TSize& aSize) = 0;

        /**
         * Get the video frame size
         * @param "aSize" The video frame size, in pixels
         * @exception Can leave with one of the system wide error codes
         */
        IMPORT_C virtual void GetVideoFrameSizeL(TSize& aSize) const = 0;
};


typedef TAny* RPvCommServer;

/**
 * MPVCommServerBase Class
 *
 * MPVCommServerBase is to be implemented by a server for COMM source and sink interfaces.
 * It could be based on a serial comms interface in which case it aggregates a single comm
 * source and sink.  In the case of sockets it could support multiple sources and sinks
 **/
typedef MPVPluginBase MPVCommServerBase;

class MPVYuvFrameBuffer
{
    public:
        virtual ~MPVYuvFrameBuffer() {}
        virtual TSize GetFrameSize()const = 0;
};


#define PV_YUV_BUFFER_DEF_WIDTH  176
#define PV_YUV_BUFFER_DEF_HEIGHT 144
class CPVMMFPointerBuffer : public CMMFDataBuffer, public MPVYuvFrameBuffer
{
    public:
        static CPVMMFPointerBuffer* NewL()
        {
            CPVMMFPointerBuffer* buf = new(ELeave) CPVMMFPointerBuffer();
            return buf;
        }
        ~CPVMMFPointerBuffer()
        {
        }
        virtual TDes8& Data()
        {
            return iPtr;
        }
        virtual const TDesC8& Data() const
        {
            return iPtr;
        }
        virtual void SetRequestSizeL(TInt aSize)
        {
            (void)(aSize);
        }
        virtual TUint BufferSize() const
        {
            return iPtr.Size();
        }
        void SetData(TUint8* aData, TInt aLength)
        {
            iPtr.Set(aData, aLength, aLength);
        }

        void SetFrameSize(const TSize& size)
        {
            iFrameSize = size;
        }

        /* MPVYuvFrameBuffer virtuals */
        virtual TSize GetFrameSize()const
        {
            return iFrameSize;
        }
    private:
        CPVMMFPointerBuffer() : CMMFDataBuffer(KPVUidYUVFrameBuffer),
                iPtr(0, 0, 0),
                iFrameSize(TSize(PV_YUV_BUFFER_DEF_WIDTH, PV_YUV_BUFFER_DEF_HEIGHT)) {};

    private:
        TPtr8 iPtr;
        TSize iFrameSize;
};
#endif //


