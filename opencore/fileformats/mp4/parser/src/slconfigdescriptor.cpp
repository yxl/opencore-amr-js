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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                        MPEG-4 SLConfigDescriptor Class                        */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/

#define IMPLEMENT_SLConfigDescriptor

#include "slconfigdescriptor.h"
#include "atomutils.h"

// Stream-in Constructor
SLConfigDescriptor::SLConfigDescriptor(MP4_FF_FILE *fp)
        : BaseDescriptor(fp)
{
    if (_success)
    {

        _OCRStreamFlag = 0;

        _pparent = NULL;

        if (!AtomUtils::read8(fp, _predefined))
            _success = false;


        // Need to add in abilty to handle case where (_predefined == 0).
        // In this situation, there are many other members that need to be rendered


        if (getSizeOfSizeField() + 2 != getSize())
        {

            if (_success && _OCRStreamFlag)
            {
                if (!AtomUtils::read16(fp, _OCRESID))
                    _success = false;
            }
            else
            {
                uint32 extra_bytes = (getSize() - (getSizeOfSizeField() + 2));

                uint8 data_8;

                if (extra_bytes > 0)
                {
                    while (extra_bytes > 0)
                    {
                        if (!AtomUtils::read8(fp, data_8))
                        {
                            _success = false;
                            break;
                        }
                        else
                        {
                            extra_bytes--;
                        }
                    }
                }
            }
        }


        if (!_success)
            _mp4ErrorCode = READ_SL_CONFIG_DESCRIPTOR_FAILED;
    }
    else
    {
        _mp4ErrorCode = READ_SL_CONFIG_DESCRIPTOR_FAILED;
    }

}

// Destructor
SLConfigDescriptor::~SLConfigDescriptor()
{
    // Empty
}
