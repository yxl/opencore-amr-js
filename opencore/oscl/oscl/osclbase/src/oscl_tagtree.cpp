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
#include "oscl_tagtree.h"

OSCL_EXPORT_REF Oscl_Tag_Base::tag_base_type Oscl_Tag_Base::tag_ancestor(tag_base_type& dest, const tag_base_type& src) const
{
    // dest and src can be the same for this call.
    // This is optimal in some cases when the src string can be modified to be the ancestor string.
    if (dest != src)
    {
        tag_copy(dest, src);
    }

    tag_base_type endptr = dest + oscl_strlen(dest);
    while (*endptr != '.' && endptr != dest) endptr--;
    *endptr = '\0';
    return dest;
}

OSCL_EXPORT_REF Oscl_Tag_Base::size_type Oscl_Tag_Base::tag_depth(const tag_base_type& t) const
{
    if (*t == '\0') return 0;

    tag_base_type ptr = t;
    size_type depth = 1;

    while (*ptr != '\0')
    {
        if (*ptr == '.') depth++;
        ptr++;
    }
    return depth;
}

