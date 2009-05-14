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
#include "osclconfig.h"
#include "oscl_int64_utils.h"

OSCL_EXPORT_REF void Oscl_Int64_Utils::set_int64(int64& input_value, const int32 upper, const int32 lower)
{
    input_value = (int64)((int64(upper) << 32) | lower);
};


OSCL_EXPORT_REF int32 Oscl_Int64_Utils::get_int64_upper32(const int64& input_value)
{
    return (int32)((input_value >> 32)  & 0xFFFFFFFF);
};


OSCL_EXPORT_REF int32 Oscl_Int64_Utils::get_int64_lower32(const int64& input_value)
{
    return (int32)((input_value)  & 0xFFFFFFFF);
};


OSCL_EXPORT_REF int32 Oscl_Int64_Utils::get_int64_middle32(const int64& input_value)
{
    return (int32)((input_value >> 16) & 0xFFFFFFFF);
};


OSCL_EXPORT_REF void Oscl_Int64_Utils::set_uint64(uint64& input_value, const uint32 upper, const uint32 lower)
{
    input_value = (uint64)((uint64(upper) << 32) | lower);
};


OSCL_EXPORT_REF uint32 Oscl_Int64_Utils::get_uint64_upper32(const uint64& input_value)
{
    return (uint32)((input_value >> 32)  & 0xFFFFFFFF);
};


OSCL_EXPORT_REF uint32 Oscl_Int64_Utils::get_uint64_lower32(const uint64& input_value)
{
    return (uint32)((input_value)  & 0xFFFFFFFF);
};


OSCL_EXPORT_REF uint32 Oscl_Int64_Utils::get_uint64_middle32(const uint64& input_value)
{
    return (uint32)((input_value >> 16) & 0xFFFFFFFF);
};
