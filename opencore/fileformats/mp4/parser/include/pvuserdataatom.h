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
/*                          MPEG-4 UserDataAtom Class                            */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This UserDataAtom Class is a container atom for informative user-data.
*/


#ifndef PVUSERDATAATOM_H_INCLUDED
#define PVUSERDATAATOM_H_INCLUDED

#ifndef ATOM_H_INCLUDED
#include "atom.h"
#endif

#ifndef ISUCCEEDFAIL_H_INCLUDED
#include "isucceedfail.h"
#endif

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

class PVUserDataAtom : public Atom
{

    public:
        PVUserDataAtom(); // Constructor
        PVUserDataAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Constructor
        PVUserDataAtom(PVUserDataAtom &atom); // Copy constructor
        virtual ~PVUserDataAtom();

        // Member gets
        OSCL_wString& getPVVersion()
        {
            return _version;    // Special member get
        }
        OSCL_wString& getPVTitle(MP4FFParserOriginalCharEnc &charType)
        {
            charType = ORIGINAL_CHAR_TYPE_UTF16;
            return _title;
        }
        OSCL_wString& getPVAuthor(MP4FFParserOriginalCharEnc &charType)
        {
            charType = ORIGINAL_CHAR_TYPE_UTF16;
            return _author;
        }
        OSCL_wString& getPVCopyright(MP4FFParserOriginalCharEnc &charType)
        {
            charType = ORIGINAL_CHAR_TYPE_UTF16;
            return _copyright;
        }
        OSCL_wString& getPVDescription(MP4FFParserOriginalCharEnc &charType)
        {
            charType = ORIGINAL_CHAR_TYPE_UTF16;
            return _description;
        }
        OSCL_wString& getPVRating(MP4FFParserOriginalCharEnc &charType)
        {
            charType = ORIGINAL_CHAR_TYPE_UTF16;
            return _rating;
        }
        OSCL_wHeapString<OsclMemAllocator> getPVCreationDate(MP4FFParserOriginalCharEnc &charType)
        {
            charType = ORIGINAL_CHAR_TYPE_UTF16;
            return _creationDate;
        }

    private:
        OSCL_wHeapString<OsclMemAllocator> _version; // Static for the version of the file format code
        OSCL_wHeapString<OsclMemAllocator> _title;
        OSCL_wHeapString<OsclMemAllocator> _author;
        OSCL_wHeapString<OsclMemAllocator> _copyright;
        OSCL_wHeapString<OsclMemAllocator> _description;
        OSCL_wHeapString<OsclMemAllocator> _rating;
        OSCL_wHeapString<OsclMemAllocator> _creationDate;
};

class PVContentTypeAtom : public Atom
{

    public:
        PVContentTypeAtom(MP4_FF_FILE *fp, uint32 size, uint32 type); // Constructor
        virtual ~PVContentTypeAtom() {};

        uint32 getContentType() const
        {
            return _contentType;
        }

    private:
        uint32 _contentType;
};

#endif // PVUSERDATAATOM_H_INCLUDED


