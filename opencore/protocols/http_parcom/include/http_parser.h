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
#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_


///////////////////////////////////////////////////////////////////////////////////////

#include "oscl_base.h"
#include "oscl_mem.h"
#include "oscl_str_ptr_len.h"
#include "http_parser_external.h" // define struct HTTPContentInfo and class RefCountHTTPEntityUnit

// forward declarations
struct HTTPContentInfoInternal;
class  HTTPParserInput;
class  HTTPParserHeaderObject;
class  HTTPParserEntityBodyObject;

class HTTPParser
{

    public:

        /**
         * the sole parsing function to parse both HTTP header and entity body. Since the input data stream is a fragment of a complete
         * HTTP response message (for client), the input data fragment is assumed arbitrary, i.e, its starting and ending position could
         * be anywhere in a complete HTTP response message. Considering HTTP header could spread among multiple input data stream
         * fragments, like the entity body case, parsing header and parsing entity body can be handled in a unified way. The output is
         * entity unit which could be a complete HTTP header (FIRST entity unit), or a complete chunk of data for partial content, or the
         * whole input data fragment. To avoid memory copy, we do inplace processing, i.e. the output entity unit is represented by a set
         * of memory fragment pointers to point to the actual memory fragments inside the input data streams that may inlude the previous
         * input data streams. Considering an entity unit may need one or mulitple input data fragments and an input data fragment could
         * constain multiple entity units, the memory for an input data fragment can only be released after all the related entity units
         * it has have been parsed and used. So the input data fragment needs to reference counted. Also the output entity unit needs to
         * be reference counted. This lets the user not worry about how to release and re-use the existing memory fragment. Each time,
         * user just need to create new memory fragment from a memory pool and pass it down to library in normal cases. Reference counter
         * will automatically take care of memory deallocation. In the case where the previous input data fragment contains multiple entity
         * units, user needs to input EMPTY fragment or the previous fragment again to let the library send out the next entity unit (since
         * each time this function only sends out ONE enity unit if it has multiple).
         * Return value: see the following enum. For PARSE_HEADER_AVAILABLE, user need to call getHTTPStatusCode() to get status code
         * @param aInputDataStream, ref-counted input data fragment
         * @param aEntityUnit, ref-counted output entity unit
         * @return following PARSE_RETURN_CODES
         *
         * NOTE: as long as the return code is non-negative, user need to check the output entity unit to see whether there is something
         * inside, and also user needs to do concatenation for the output fragments.
         **/
        OSCL_IMPORT_REF int32 parse(const OsclRefCounterMemFrag &aInputDataStream, RefCountHTTPEntityUnit &aEntityUnit);


        /**
         * If http response header is available, status code is ok (though parse() already does some sanity check for status code),
         * there still could be some headers that need further check, i.e. some headers are only supported in HTTP/1.1, such as Transfer-Encoding
         * @return following PARSE_RETURN_CODES, list the unsupported items.
         **/
        OSCL_IMPORT_REF int32 doSanityCheckForResponseHeader();


        // return codes for parse function
        enum PARSE_RETURN_CODES
        {
            PARSE_SUCCESS						 = 0,	// success with left over of the current input, don't send the new input next time
            PARSE_SUCCESS_END_OF_INPUT		     = 1,	// success with end of the current input
            PARSE_SUCCESS_END_OF_MESSAGE         = 2,	// success with end of the message (get data with the size of content-length)
            PARSE_SUCCESS_END_OF_MESSAGE_WITH_EXTRA_DATA = 3, // success with end of the message (get data with the size of content-length), but input has more extra data

            PARSE_HEADER_AVAILABLE				 = 4,	// HTTP header is parsed
            PARSE_STATUS_LINE_SHOW_NOT_SUCCESSFUL = 5,	// parse the first http status line,
            // got status code >= 300, 3xx=>redirection, 4xx=>client error, 5xx=>server error
            // note that this is return code when parser just finishes parsing the first line,
            // user can continue calling parse function to get the complete http header
            PARSE_NEED_MORE_DATA				 = 6,	// no ouput, no entity units

            // errors
            PARSE_GENERAL_ERROR	= -1,
            PARSE_SYNTAX_ERROR = -2,				// syntax is not understandable
            PARSE_HTTP_VERSION_NOT_SUPPORTED = -3,	// no HTTP version or HTTP version is different from 1.0 or 1.1.
            PARSE_TRANSFER_ENCODING_NOT_SUPPORTED = -4,
            PARSE_MEMORY_ALLOCATION_FAILURE = -5,	// memory allocation for entity units
            PARSE_HEADER_NOT_PARSED_YET = -6		// HTTP header hasn't been parsed yet, so shouldn't expect parsing entity body
        };


        // After parsing HTTP header(parse() return PARSE_HEADER_AVAILABLE), use the following functions to get the related information
        // get content info
        OSCL_IMPORT_REF void getContentInfo(HTTPContentInfo &aContentInfo);
        // get total fields inside the header
        OSCL_IMPORT_REF uint32 getTotalFieldsInHeader();
        // get a list of all field keys inside the header
        OSCL_IMPORT_REF uint32 getFieldKeyListInHeader(StrPtrLen *&aFieldKeyList);
        // get the field value with the given field key, which can be retrieved from getFieldKeyListInHeader()
        // There could be a case where one field key correponds to multiple field values, so input argument "index" is for
        // getting which value for the given field key
        OSCL_IMPORT_REF bool getField(const StrCSumPtrLen &aNewFieldName, StrPtrLen &aNewFieldValue, const uint32 index = 0);
        // get the number of field key-value pairs with the same field key. 0 => no such key, 1 or more => number of key-value pairs
        OSCL_IMPORT_REF uint32 getNumberOfFieldsByKey(const StrCSumPtrLen &aNewFieldName);
        // get the status code, 1xx, 2xx, 3xx, 4xx, 5xx
        OSCL_IMPORT_REF uint32 getHTTPStatusCode();
        /////////////////////////////////////////////////////////////////////////////////////////////

        // reset the parser to parse a new HTTP response
        OSCL_IMPORT_REF void reset();

        // factory method
        OSCL_IMPORT_REF static HTTPParser *create();

        // destructor
        OSCL_IMPORT_REF ~HTTPParser();

    private:
        int32 parseEntityBody(RefCountHTTPEntityUnit &aEntityUnit);

        // called by create() to construct the parser
        bool construct();

        // constructor
        HTTPParser()
        {
            oscl_memset(this, 0, sizeof(HTTPParser));
        }

    private:
        // Basically, this parser is decomposed into the following objects.
        // HTTPParserInput handles input data stream concatenation and fragment grouping for parsing and entity unit output
        // HTTPContentInfo contains content type, content length and content range information and does infomation parsing.
        // HTTPParserHeaderObject and HTTPParserEntityBodyObject handles header and entity body parsing
        HTTPParserInput			*iParserInput;
        HTTPContentInfoInternal *iContentInfo;
        HTTPParserHeaderObject  *iHeader;
        HTTPParserEntityBodyObject *iEntityBody;
};

#endif // HTTP_PARSER_H_

