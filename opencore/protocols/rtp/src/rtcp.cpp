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
/*                                                                               */
/*********************************************************************************/

/*
**   File:   rtcp.cpp
**
**   Description:
**      This module implements the RTCP class. This class is used to encode and
**      decode RTCP packets. Please refer to the RTCP design document for
**      details.
*/

/*
** Includes
*/

//#include <stdlib.h>
//#include <math.h>
#include "rtcp.h"

#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()


/*
** Constants
*/


/*
** Methods
*/
RTCP_Base::RTCP_Base(const uint8 version)
        : rtcpVersion(version)
{}

OSCL_EXPORT_REF RTCP_Base::~RTCP_Base()
{}


OSCL_EXPORT_REF RTCP_ReportBlock* RTCP_RR::get_report_block(uint index)
{


    if (index >= max_report_blocks)
    {
        return NULL;
    }

    if (index >= num_report_blocks)
    {
        num_report_blocks = index + 1;
    }

    if (index < NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS)
    {
        return preallocated_reports + index;
    }

    if (!additional_reports)
    {

        additional_reports = OSCL_ARRAY_NEW(RTCP_ReportBlock, max_report_blocks - NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS);
    }

    return additional_reports + (index - NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS);

}


const RTCP_ReportBlock* RTCP_RR::read_report_block(uint index) const
{


    if (index >= num_report_blocks)
    {
        return NULL;
    }

    if (index < NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS)
    {
        return preallocated_reports + index;
    }

    if (!additional_reports)
    {
        return NULL;
    }

    return additional_reports + (index - NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS);

}


bool RTCP_RR::set_max_report_blocks(uint new_max_report_blocks)
{

    if (additional_reports)
    {
        // reports array already exists
        if (new_max_report_blocks > max_report_blocks)
        {
            // only update max if greater than old max
            max_report_blocks = new_max_report_blocks;
            if (new_max_report_blocks > NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS)
            {
                // free current additional reports and allocate new array
                delete[] additional_reports;
                additional_reports = OSCL_ARRAY_NEW(RTCP_ReportBlock, new_max_report_blocks - NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS);
            }
        }
    }
    else if (new_max_report_blocks > NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS)
    {
        // allocate additional_reports array
        additional_reports = OSCL_ARRAY_NEW(RTCP_ReportBlock, new_max_report_blocks - NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS);
        max_report_blocks = new_max_report_blocks;
    }

    return true;

}


bool RTCP_RR::set_report_block(uint index, const RTCP_ReportBlock& report_block)
{
    if (index >= max_report_blocks)
    {
        return false;
    }

    if (index < NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS)
    {

        preallocated_reports[index] = report_block;

    }
    else if (!additional_reports)
    {
        // allocate the additional reports array
        additional_reports = OSCL_ARRAY_NEW(RTCP_ReportBlock, max_report_blocks - NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS);
        additional_reports[index - NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS] =
            report_block;
    }
    else
    {

        additional_reports[index - NUM_PREALLOCATED_RTCP_RR_REPORT_BLOCKS] =
            report_block;

    }

    num_report_blocks = index + 1;
    return true;
}


RTCP_ReportBlock* RTCP_SR::get_report_block(uint index)
{


    if (index >= max_report_blocks)
    {
        return NULL;
    }

    if (index >= num_report_blocks)
    {
        num_report_blocks = index + 1;
    }

    if (index < NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS)
    {
        return preallocated_reports + index;
    }

    if (!additional_reports)
    {
        additional_reports = OSCL_ARRAY_NEW(RTCP_ReportBlock, max_report_blocks - NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS);

    }

    return additional_reports + (index - NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS);

}


const RTCP_ReportBlock* RTCP_SR::read_report_block(uint index) const
{

    if (index >= num_report_blocks)
    {
        return NULL;
    }

    if (index < NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS)
    {
        return preallocated_reports + index;
    }

    if (!additional_reports)
    {
        return NULL;
    }

    return additional_reports + (index - NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS);

}


bool RTCP_SR::set_max_report_blocks(uint new_max_report_blocks)
{

    if (additional_reports)
    {
        // reports array already exists
        if (new_max_report_blocks > max_report_blocks)
        {
            // only update max if greater than old max
            max_report_blocks = new_max_report_blocks;
            if (new_max_report_blocks > NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS)
            {
                // free current additional reports and allocate new array
                delete[] additional_reports;
                additional_reports = OSCL_ARRAY_NEW(RTCP_ReportBlock, new_max_report_blocks - NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS);
            }
        }
    }
    else if (new_max_report_blocks > NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS)
    {
        // allocate additional_reports array
        additional_reports = OSCL_ARRAY_NEW(RTCP_ReportBlock, new_max_report_blocks - NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS);
        max_report_blocks = new_max_report_blocks;
    }

    return true;

}


bool RTCP_SR::set_report_block(uint index, const RTCP_ReportBlock& report_block)
{
    if (index >= max_report_blocks)
    {
        return false;
    }

    if (index < NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS)
    {

        preallocated_reports[index] = report_block;

    }
    else if (!additional_reports)
    {
        // allocate the additional reports array
        additional_reports = OSCL_ARRAY_NEW(RTCP_ReportBlock, max_report_blocks - NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS);
        additional_reports[index - NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS] =
            report_block;
    }
    else
    {

        additional_reports[index - NUM_PREALLOCATED_RTCP_SR_REPORT_BLOCKS] =
            report_block;

    }

    num_report_blocks = index + 1;
    return true;
}


void SDES_chunk::set_max_items(uint new_max_items)
{

    if (new_max_items > max_sdes_items)
    {
        // update sdes items
        max_sdes_items = new_max_items;

        if (new_max_items > NUM_PREALLOCATED_RTCP_CHUNK_ITEMS)
        {
            int32 extra_items = new_max_items - NUM_PREALLOCATED_RTCP_CHUNK_ITEMS;

            // now allocate the extra array
            SDES_item* new_additional = OSCL_ARRAY_NEW(SDES_item, extra_items);

            if (additional_items)
            {
                if (num_sdes_items >  NUM_PREALLOCATED_RTCP_CHUNK_ITEMS)
                {
                    // copy old additional to the new array
                    for (uint ii = 0; ii < num_sdes_items -
                            NUM_PREALLOCATED_RTCP_CHUNK_ITEMS; ++ii)
                    {
                        new_additional[ii] = additional_items[ii];
                    }
                }
                delete[] additional_items;
            }

            additional_items = new_additional;

        }

    }

}


SDES_item* SDES_chunk::get_item(uint item_index)
{
    if (item_index >= max_sdes_items)
    {
        return NULL;
    }

    if (item_index >= num_sdes_items)
    {
        num_sdes_items = item_index + 1;
    }

    if (item_index >= NUM_PREALLOCATED_RTCP_CHUNK_ITEMS)
    {
        if (!additional_items)
        {
            additional_items = OSCL_ARRAY_NEW(SDES_item, max_sdes_items - NUM_PREALLOCATED_RTCP_CHUNK_ITEMS);
        }

        return additional_items + (item_index - NUM_PREALLOCATED_RTCP_CHUNK_ITEMS);
    }

    return chunk_items + item_index;
}


const SDES_item* SDES_chunk::read_item(uint item_index) const
{
    if (item_index >= num_sdes_items)
    {
        return NULL;
    }

    if (item_index >= NUM_PREALLOCATED_RTCP_CHUNK_ITEMS)
    {
        if (!additional_items)
        {
            return NULL;
        }

        return additional_items + (item_index - NUM_PREALLOCATED_RTCP_CHUNK_ITEMS);
    }

    return chunk_items + item_index;
}


bool SDES_chunk::add_item(const SDES_item& item)
{

    const uint SDES_ARRAY_INCREMENT = 10;

    if (num_sdes_items > max_sdes_items)
    {
        // allocate more
        set_max_items(max_sdes_items + SDES_ARRAY_INCREMENT);
    }

    if (num_sdes_items >= NUM_PREALLOCATED_RTCP_CHUNK_ITEMS)
    {
        if (!additional_items)
        {
            additional_items = OSCL_ARRAY_NEW(SDES_item, max_sdes_items - NUM_PREALLOCATED_RTCP_CHUNK_ITEMS);
        }
        additional_items[num_sdes_items - NUM_PREALLOCATED_RTCP_CHUNK_ITEMS] = item;
        chunk_size += item.content.len + 2;
    }
    else
    {

        chunk_items[num_sdes_items] = item;
        chunk_size += item.content.len + 2;
    }

    ++num_sdes_items;
    return true;

}



void RTCP_SDES::set_max_chunks(uint new_max_chunks)
{

    if (new_max_chunks > max_chunks)
    {
        // update sdes items
        max_chunks = new_max_chunks;

        if (new_max_chunks > NUM_PREALLOCATED_RTCP_CHUNKS)
        {
            int32 extra_chunks = new_max_chunks - NUM_PREALLOCATED_RTCP_CHUNKS;

            // now allocate the extra array
            SDES_chunk* new_additional = OSCL_ARRAY_NEW(SDES_chunk, extra_chunks);

            if (additional_chunks)
            {
                if (chunk_count >  NUM_PREALLOCATED_RTCP_CHUNKS)
                {
                    // copy old additional to the new array
                    for (uint ii = 0; ii < chunk_count -
                            NUM_PREALLOCATED_RTCP_CHUNKS; ++ii)
                    {
                        new_additional[ii] = additional_chunks[ii];
                    }
                }
                delete[] additional_chunks;
            }

            additional_chunks = new_additional;

        }

    }

}


SDES_chunk* RTCP_SDES::get_chunk(uint chunk_index)
{
    if (chunk_index >= max_chunks)
    {
        return NULL;
    }

    if (chunk_index >= chunk_count)
    {
        chunk_count = chunk_index + 1;
    }

    if (chunk_index >= NUM_PREALLOCATED_RTCP_CHUNKS)
    {
        if (!additional_chunks)
        {
            additional_chunks = OSCL_ARRAY_NEW(SDES_chunk, max_chunks - NUM_PREALLOCATED_RTCP_CHUNKS);
        }

        return additional_chunks + (chunk_index - NUM_PREALLOCATED_RTCP_CHUNKS);
    }

    return chunk_array + chunk_index;
}

const SDES_chunk* RTCP_SDES::read_chunk(uint chunk_index) const
{
    if (chunk_index >= chunk_count)
    {
        return NULL;
    }

    if (chunk_index >= NUM_PREALLOCATED_RTCP_CHUNKS)
    {
        if (!additional_chunks)
        {
            return NULL;
        }

        return additional_chunks + (chunk_index - NUM_PREALLOCATED_RTCP_CHUNKS);
    }

    return chunk_array + chunk_index;
}


bool RTCP_SDES::add_chunk(const SDES_chunk& chunk)
{

    const uint SDES_ARRAY_INCREMENT = 10;

    if (chunk_count > max_chunks)
    {
        // allocate more
        set_max_chunks(max_chunks + SDES_ARRAY_INCREMENT);
    }

    if (chunk_count >= NUM_PREALLOCATED_RTCP_CHUNKS)
    {
        if (!additional_chunks)
        {
            additional_chunks = OSCL_ARRAY_NEW(SDES_chunk, max_chunks - NUM_PREALLOCATED_RTCP_CHUNKS);
        }
        additional_chunks[chunk_count - NUM_PREALLOCATED_RTCP_CHUNKS] = chunk;
    }
    else
    {
        chunk_array[chunk_count] = chunk;
    }

    ++chunk_count;
    return true;

}




