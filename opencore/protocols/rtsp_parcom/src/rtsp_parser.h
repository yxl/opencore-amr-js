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

#ifndef RTSP_PARSER_H_
#define RTSP_PARSER_H_

#include "rtsp_par_com_basic_ds.h"
#include "rtsp_par_com_message.h"

class RTSPParser
{
    private:

        RTSPParser(const RTSPParser &);
        RTSPParser & operator= (const RTSPParser &);

    public:

        typedef enum
        {
            IS_WAITING_FOR_REQUEST_MEMORY,
            IS_LOOKING_FOR_END_OF_REQUEST,
            IS_WAITING_FOR_DATA,
            IS_REQUEST_IS_READY,
            IS_WAITING_FOR_ENTITY_BODY_MEMORY,
            IS_ERROR_REQUEST_TOO_BIG,
            IS_SKIPPING_OVER_ENTITY_BODY,
            IS_STARTING_TO_FILL_OUT_ENTITY_BODY,
            IS_CONTINUING_TO_FILL_OUT_ENTITY_BODY,
            IS_ENTITY_BODY_IS_READY,
            IS_INTERNAL_ERROR,
            IS_START_LOOKING_FOR_RESYNC,
            IS_LOOKING_FOR_RESYNC,
            IS_WAITING_FOR_EMBEDDED_DATA_MEMORY,
            IS_EMBEDDED_DATA_IS_READY,
            IS_SKIPPING_OVER_EMBEDDED_DATA,
            IS_STARTING_TO_FILL_OUT_EMBEDDED_DATA,
            IS_CONTINUING_TO_FILL_OUT_EMBEDDED_DATA


        } InternalState;

        typedef enum
        {
            WAITING_FOR_DATA,
            WAITING_FOR_REQUEST_MEMORY,
            REQUEST_IS_READY,
            WAITING_FOR_ENTITY_BODY_MEMORY,
            ENTITY_BODY_IS_READY,
            ERROR_REQUEST_TOO_BIG,
            WAITING_FOR_EMBEDDED_DATA_MEMORY,
            EMBEDDED_DATA_IS_READY,
            INTERNAL_ERROR
        } ParserState;


        OSCL_IMPORT_REF RTSPParser();

    protected:

        InternalState internalState;

        char 		mainBuffer[ RTSP_PARSER_BUFFER_SIZE + 3 ];
        char *  mainBufferEntry;
        char * 	mainBufferSpace;
        int 		mainBufferSizeUsed;

        RTSPIncomingMessage * requestStruct;

        StrPtrLen dataBufferSpec;

        uint32            ebFullSizeExpected;
        uint32            ebSizeCoveredSoFar;
        uint32            ebCurrentIndex;
        uint32            ebCurrentOffset;
        OsclMemoryFragment  * entityBody;

        char            * eorptr;


        // for field repetitions
        //
        StrPtrLen         fields[ RTSP_HUGE_NUMBER_OF_FIELDS_IN_PARSER ];
        uint32            numFieldsUsed;

        void		continueProcessing();
        void		lookForEndOfRequest();
        void    lookForResync();
        void    skipOverEntityBody();
        void    startFillingOutEntityBody();
        void    dealWithLineContinuations(RTSPIncomingMessage *);
        void    dealWithFieldRepetitions(RTSPIncomingMessage *);
        void	skipOverEmbeddedData();
        void	startFillingOutEmbeddedData();

    public:


        ParserState
        OSCL_IMPORT_REF getState();


        // to get the address and size of data
        // to be written to Parser's raw buffer
        //
        OSCL_IMPORT_REF const StrPtrLen *
        getDataBufferSpec();

        // to register that data buffer has been
        // written
        //
        OSCL_IMPORT_REF bool
        registerDataBufferWritten(uint32 sizeWritten);

        // to register the RTSPIncomingMessage structure to
        // be filled out
        //
        OSCL_IMPORT_REF bool
        registerNewRequestStruct(RTSPIncomingMessage * newRequestStruct);

        // to register the RTSPEntityBody buffer
        // that the entity body should be written into
        //
        OSCL_IMPORT_REF bool
        registerEntityBody(RTSPEntityBody * newBody);

        // to register the RTSPEntityBody buffer
        // that the embedded data should be written into
        //
        OSCL_IMPORT_REF bool
        registerEmbeddedDataMemory(RTSPEntityBody * newBody);

        // to flush the currently available information // and make a request out of
        // whatever was
        // available
        OSCL_IMPORT_REF void flush(void);
        OSCL_IMPORT_REF bool parseEntityBody(RTSPEntityBody * entityBody);
};


#endif // RTSP_PARSER_H_
