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
/*
    The PVA_FF_IMpeg4File Class is INTERFACE that exsposes only those necessary
    methods of the underlying PVA_FF_Mpeg4File class.
*/

#define IMPLEMENT_IMpeg4File_H__


#include "oscl_base.h"
#include "a_impeg4file.h"
#include "mpeg4file.h"
#include "atomutils.h"
#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif

// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()

OSCL_EXPORT_REF
// Static method to create the MP4 file and return the PVA_FF_IMpeg4File interface
PVA_FF_IMpeg4File*
PVA_FF_IMpeg4File::createMP4File(int32 mediaType,
                                 PVA_FF_UNICODE_STRING_PARAM outputPath,
                                 PVA_FF_UNICODE_STRING_PARAM postFix,
                                 void* osclFileServerSession,
                                 uint32 fileAuthoringFlags,
                                 PVA_FF_UNICODE_STRING_PARAM outputFileName, uint32 aCacheSize)
{
    PVA_FF_Mpeg4File *mp4 = NULL;
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_Mpeg4File, (mediaType), mp4);

    mp4->SetTempOutputPath(outputPath);
    mp4->SetTempFilePostFix(postFix);
    mp4->SetCacheSize(aCacheSize);

    if (!mp4->setOutputFileName(outputFileName))
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_Mpeg4File, mp4);
        return NULL;
    }

    if (!(mp4->init(mediaType,
                    osclFileServerSession,
                    fileAuthoringFlags)))
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_Mpeg4File, mp4);
        return NULL;
    }

    return mp4;
}

OSCL_EXPORT_REF
PVA_FF_IMpeg4File*
PVA_FF_IMpeg4File::createMP4File(int32 mediaType,
                                 uint32 fileAuthoringFlags,
                                 MP4_AUTHOR_FF_FILE_HANDLE outputFileHandle,
                                 uint32 aCacheSize)
{
    PVA_FF_Mpeg4File *mp4 = NULL;//OSCL_NEW(PVA_FF_Mpeg4File, (mediaType));
    PV_MP4_FF_NEW(fp->auditCB, PVA_FF_Mpeg4File, (mediaType), mp4);

    if (!mp4->setOutputFileHandle(outputFileHandle))
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_Mpeg4File, mp4);
        return NULL;
    }

    mp4->SetCacheSize(aCacheSize);
    if (!(mp4->init(mediaType,
                    NULL,
                    fileAuthoringFlags)))
    {
        PV_MP4_FF_DELETE(NULL, PVA_FF_Mpeg4File, mp4);
        return NULL;
    }

    return mp4;
}
OSCL_EXPORT_REF void PVA_FF_IMpeg4File::DestroyMP4FileObject(PVA_FF_IMpeg4File* aMP4FileObject)
{
    PVA_FF_Mpeg4File* ptr = OSCL_STATIC_CAST(PVA_FF_Mpeg4File*, aMP4FileObject);
    PV_MP4_FF_DELETE(NULL, PVA_FF_Mpeg4File, ptr);
}
