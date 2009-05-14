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
#ifndef STRINGABLE_H
#define STRINGABLE_H

#include "unit_test_common.h"

class stringable
{
    public:
        virtual _STRING to_string(void) const = 0;
        virtual ~stringable() {}
};

template< class TT >
inline
_STRING valueToString(const TT &)
{
    return "<value not stringable>";
}

inline
_STRING valueToString(const bool & value)
{
    return (true == value) ? "true" : "false";
}

inline
_STRING valueToString(const int & value)
{
    char buffer[32];
    sprintf(buffer, "%d", value);
    return buffer;
}

#endif
