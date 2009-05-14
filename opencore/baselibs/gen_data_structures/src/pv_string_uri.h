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

// -*- c++ -*-
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

//                     pv_string_uri

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/*! \addtogroup gen_data_structures
 *
 * @{
 */


/*! \pv_string_uri.h
    \brief strinig uri management class

*/

#ifndef PV_STRING_URI_H_INCLUDE
#define PV_STRING_URI_H_INCLUDE



#include "oscl_stdstring.h"
#include "oscl_mem_basic_functions.h"
#include "oscl_mem.h"
#include  "oscl_base_macros.h"
#include "oscl_base.h"

#define MAX_FULL_REQUEST_SIZE 4000
#define PERSENTAGE '%'



/**
 * Pv_String_Uri is a non-templatized base class for Uri.
 * The purpose of this base class is to Escaping the illigal character in the URI
 * in the Pv_String_Uri implementation.
 * Specially it will convert %NotMatch-Char to %25.
 */


class PVStringUri
{

    private:


    public:
        /**
        * PersentageToEscapedEncoding utility function provides the Persentage Pct-Encoding to %25
        * If Two Digit after % will not match in Legal Character List Of RFC-3896
        * @param [in]/[Out] character Pointer; The complete URI
        * @param [out] URL Lenght
        *@return true if successful, otherwise false.
        */
        OSCL_IMPORT_REF static bool  PersentageToEscapedEncoding(char*, uint32 &);

        /**
        * IllegalCharactersToEscapedEncoding utility function provides the EscapedEncoding
        * for illegal character , Which are not fall in URI-Legal Character List Of RFC-3896
        * @param [in]/[Out] character Pointer; The complete URI
        * @param [out] URL Lenght
        *@return true if successful, otherwise false.
        */

        OSCL_IMPORT_REF static bool  IllegalCharactersToEscapedEncoding(char*, uint32 &);


        /**
        * DecimalToHexConverter utility function provides the Decimal To Hexa.
        *
        * @param [in] unsigned character Pointer.
        * @param [out] Return The Hexa Value for a character.
        * @return return Hexa Value for a character.
        */

        OSCL_IMPORT_REF static unsigned char* DecimalToHexConverter(unsigned char aNumberIn, uint32 & aUrlMaxConvertLenght);


    protected:



};



#endif // PV_STRING_URI_H_INCLUDE
/*! @} */

