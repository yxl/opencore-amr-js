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
#ifndef HTTP_COMPOSER_H_
#define HTTP_COMPOSER_H_


///////////////////////////////////////////////////////////////////////////////////////

#include "oscl_base.h"
#include "oscl_mem.h"
#include "oscl_str_ptr_len.h"

enum HTTPVersion
{
    HTTP_V1_0 = 0,
    HTTP_V1_1
};

// HTTPMethod enum
enum HTTPMethod
{
    HTTP_METHOD_GET = 0,			// both HTTP 1.0 and HTTP 1.1
    HTTP_METHOD_HEAD,			// both HTTP 1.0 and HTTP 1.1
    HTTP_METHOD_POST,			// both HTTP 1.0 and HTTP 1.1
    HTTP_METHOD_DELETE,			// both HTTP 1.0 and HTTP 1.1
    HTTP_METHOD_LINK,			// HTTP 1.0 only
    HTTP_METHOD_UNLINK,			// HTTP 1.0 only
    HTTP_METHOD_OPTIONS,		// HTTP 1.1 only
    HTTP_METHOD_PUT,			// HTTP 1.1 only
    HTTP_METHOD_TRACE,			// HTTP 1.1 only
    HTTP_METHOD_CONNECT,		// HTTP 1.1 only
};

// forward declaration
struct HTTPMemoryFragment;
class StringKeyValueStore;

class HTTPComposer
{
    public:

        /**
         * HTTP request line:  Method SP Request-URI SP HTTP-Version CRLF
         * set three basic elements for a http request: method, uri and version
         * uri must be set; if URI is set, but method or version is not set, then
         * the method or version will default to GET or version 1.1
         **/
        void setMethod(const HTTPMethod aMethod)
        {
            iMethod = aMethod;
        }
        void setVersion(const HTTPVersion aVersion)
        {
            iVersion = aVersion;
        }
        /**
         * Set URI. Note that composer doesn't do any memory allocation and memory copy for the input URI, and just save the pointer
         * for the URI string. So it is composer user's reponsiblity to keep URI valid during request composing.
         */
        OSCL_IMPORT_REF void setURI(const StrPtrLen aURI);

        /**
         * set/remove a standard field or add a new extension field for the headers
         * if aNewFieldValue=NULL, that means removing the existing field with the field key specified by aNewFieldName
         * The flag aNeedReplaceOldValue tells the library whether to replace the old value with the new value for the same field, if there is,
         * For HTTP, there is a typical situation where multiple same fields can be set at the same time, for example, field "Pragma". The
         * user can set all the implementation specifics for one "Pragma", or multiple "Pragma". So by default, if we run into the same
         * field, we won't replace the old value with the new value unless the user requires to do it.		 *
         * @param aNewFieldName, input field name
         * @param aNewFieldValue, input field value name
         * @param aNeedReplaceOldValue, flag of whether the new value replaces the old value
         * @return true for success
         **/
        OSCL_IMPORT_REF bool setField(const StrCSumPtrLen &aNewFieldName, const StrPtrLen *aNewFieldValue, const bool aNeedReplaceOldValue = false);
        OSCL_IMPORT_REF bool setField(const StrCSumPtrLen &aNewFieldName, const char *aNewFieldValue, const bool aNeedReplaceOldValue = false);

        /**
         * get the current total length based on the current settings, and the length includes the length for
         * request-line + general header + request header + entity header. Note that this length doesn't include entity body length,
         * since entity body is controlled by the user.
         * This API is used for the user to get the length after setting everything done (using above set APIs), and help the user
         * provide the buffer with an accurate size to hold the HTTP request with optional entity body.
         * Note that this API can get called in multiple times, and any time.
         * @param usingAbsoluteURI, flag to show absolute uri is used in the first request line. For MS http streaming, it uses relative uri + Host field to
         *        identify the exact source
         * @return the current total length or COMPOSE_BAD_URI from the following COMPOSE_RETURN_CODES.
         **/
        OSCL_IMPORT_REF int32 getCurrentRequestLength(const bool usingAbsoluteURI = true);

        /**
         * compose a HTTP request with the given message buffer, and the optional entity body.
         * The reason for the request buffer provided by the user is, a http request could contain entity body (e.g. POST method),
         * which is controlled by the user. And the entity body can be anything. So it doesn't make much sense for the composer to
         * allocate a buffer for holding a HTTP request, since in general, the composer has no idea about how big a request is.
         * To avoid extra memory copy, if the HTTP request does have entity body, the input buffer should already have the entity
         * body in place, i.e. the entity body is copied to the input buffer from an offset, which is header length, retrieved from
         * the above getCurrentRequestLength() function. The following "aEntityBodyLength" argument just shows whether there is an
         * enity body put into the input buffer, and whether this length matches the version from "Content-Length" (which has to be
         * be set)
         * @param aNewFielaComposedMessageBuffer, input message buffer
         * @param aEntityBodyLength, entity body length, 0 means no entity body
         * @return following enum codes
         **/
        OSCL_IMPORT_REF int32 compose(OsclMemoryFragment &aComposedMessageBuffer, const bool usingAbsoluteURI = true, const uint32 aEntityBodyLength = 0);

        enum COMPOSE_RETURN_CODES
        {
            COMPOSE_SUCCESS = 0,

            // error
            COMPOSE_GENERAL_ERROR    = -1,
            COMPOSE_BUFFER_TOO_SMALL = -2,
            COMPOSE_URI_NOT_SET      = -3,
            COMPOSE_CONTENT_TYPE_NOT_SET_FOR_ENTITY_BODY   = -4,
            COMPOSE_CONTENT_LENGTH_NOT_SET_FOR_ENTITY_BODY = -5,
            COMPOSE_CONTENT_LENGTH_NOT_MATCH_ENTITY_BODY_LENGTH = -6,
            COMPOSE_BAD_URI = -7  // input uri is relative uri, but request using absolute uri, or
            // request using relative uri, but relative uri isn't able to obtained from input uri
        };

        /**
         * reset the composer in order to compose a new HTTP request. Currently there are two cases in consideration.
         * One is reset the everything in the composer to compose brand-new HTTP request with probably new header fields,
         * or new method, etc. In this case, all the previous settings (method, version, uri, and all header fields) get
         * reset. Another useful case is the new HTTP request only has a change on URI, which is highly likely, and so
         * method, version and all header fields are kept the same. So in this case, we don't need to reset everything.
         * This offers a possiblity for fast composing a new HTTP request.
         * @param aKeepAllSettingsExceptURI, flag to show whether to keep all settings except URI
         **/
        OSCL_IMPORT_REF void reset(const bool aKeepAllSettingsExceptURI = false);

        // factory method
        OSCL_IMPORT_REF static HTTPComposer *create();

        // destructor
        OSCL_IMPORT_REF ~HTTPComposer();

    private:
        // sanity check for compose API
        int32 santityCheckForCompose(HTTPMemoryFragment &aComposedMessageBuffer, const bool usingAbsoluteURI = true, const uint32 aEntityBodyLength = 0);
        // compose the first request/status line of a HTTP request
        void composeFirstLine(HTTPMemoryFragment &aComposedMessageBuffer, const bool usingAbsoluteURI = true);
        // compose all headers of a HTTP request
        bool composeHeaders(HTTPMemoryFragment &aComposedMessageBuffer);

        // called by create(), construct the composer
        bool construct();

        // constructor
        HTTPComposer() : iKeyValueStore(NULL)
        {
            ;
        }

    private:
        HTTPMethod  iMethod;
        HTTPVersion iVersion;
        StrPtrLen   iURI;
        StrPtrLen   iRelativeURI;

        uint32 iHeaderLength;
        uint32 iFirstLineLength; // length of the request/response line in HTTP header
        uint32 iEntityBodyLength;

        // field key-value store to handle key-value operations
        StringKeyValueStore *iKeyValueStore;
};

#endif // HTTP_COMPOSER_H_

