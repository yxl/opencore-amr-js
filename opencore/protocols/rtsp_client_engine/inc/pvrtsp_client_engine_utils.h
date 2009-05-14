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
#ifndef PVRTSP_CLIENT_ENGINE_UTILS_H
#define PVRTSP_CLIENT_ENGINE_UTILS_H
//#include "pvrtsp_client_engine_utils.h"

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STDSTRING_H_INCLUDED
#include "oscl_stdstring.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif

#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif

#define MAX_LONG_TEXT_LEN 1024

typedef enum
{
    CONCATENATE,
    REPLACE_PATH,
    REPLACE_HOST,
    UNKNOWN
} URLType;

const char FWD_SLASH = '/';
const char COLON = ':';
const char DOT = '.';


/*
*Function   : bool composeURL(char *baseURL, char *relativeURL, char *completeURL)
*Parameters : char *baseURL       - input parameter, needs to be NULL terminated
*             char *relativeURL   - input parameter, needs to be NULL terminated
*             char *completeURL   - output parameter, will be NULL terminated
*             int completeURLLen  - input/output parameter
*Date       : 06/03/2002
*Description: This function composes a URL using a base URL and relative URL. It is based
*             on RFC 2396 and supports a sub-set of the exhaustive list of relative URL
*             specified in the document. It does not support relative URLs that begin
*             with a "." or those that contain a ":" in them.
*             No memory is allocated inside the function. The caller of this function
*             must take care of all memory allocation/deallocation for the parameters
*             passed to this function.
*Output     : This function returns TRUE or FALSE based on a successful
*             composition or not. In case of successful composition, the complete URL
*             is returned in the third parameter.
*/
bool composeURL(const char *baseURL, const char *relativeURL, \
                char *completeURL, unsigned int &completeURLLen);

/*
 * Function   : const char* findRelativeURL(const char *aURL)
 * Parameters : const char *aURL - input parameter
 * Date       : 10/05/2006
 * Description: This function finds the relative URL within the aURL, including the forward slash. Returns NULL if NOT found.
 * Output     : The relative URL or NULL.
 */
const char* findRelativeURL(const char *aURL);

/*
*Function   : URLType findRelativeURLType(char *relativeURL)
*Parameters : char *relativeURL   - input parameter, needs to be NULL terminated
*Date       : 06/03/2002
*Description: This function looks at the relative URL and determines what type of
*             operation needs to be performed to compose the complete URL.
*Output     : This function returns the URL type. The return values are of the
*             enumerated type URLType with values
*             CONCATENATE, REPLACE_PATH, REPLACE_HOST, UNKNOWN
*/
URLType findRelativeURLType(const char *relativeURL);

/*
 * Function   : void dropTextAfterLastSlash(char *copyOfBaseURL)
 * Parameters : char *copyOfBaseURL - input/output parameter
 * Date       : 06/03/2002
 * Description: This function drops text after the last "/" in the URL(if any).
 * Output     : The modified URL is pointed to by the input parameter.
 */
void dropTextAfterLastSlash(char *copyOfBaseURL);

/*
 * Function   : void dropTextAfterFirstSlash(char *copyOfBaseURL)
 * Parameters : char *copyOfBaseURL - input/output parameter
 * Date       : 06/03/2002
 * Description: This function drops text after the first "/" in the URL(if any).
 * Output     : The modified URL is pointed to by the input parameter.
 */
void dropTextAfterFirstSlash(char *copyOfBaseURL);

/*
 * Function   : OSCL_String& generatePseudoUUID(void)
 * Parameters : OSCL_String& aUUID - input/output parameter
 * Date       : 03/27/2006
 * Description: This function generate a pseudo UUID string.
 * Output     : The pseudo UUID string like 6e31c837-b458-4c27-b290-73805cb08da1.
 */
bool generatePseudoUUID(OSCL_String& aUUID);

#endif	//PVRTSP_CLIENT_ENGINE_UTILS_H
