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
#ifndef OMA2BOXES_H_INCLUDED
#define OMA2BOXES_H_INCLUDED

#ifndef FULLATOM_H_INCLUDED
#include "fullatom.h"
#endif

#ifndef ATOMUTILS_H_INCLUDED
#include "atomutils.h"
#endif

#ifndef ATOMDEFS_H_INCLUDED
#include "atomdefs.h"
#endif

#ifndef SAMPLEENTRY_H_INCLUDED
#include "sampleentry.h"
#endif

#ifndef ESDATOM_H_INCLUDED
#include "esdatom.h"
#endif

#ifndef GPP_AMRDECODERSPECIFICINFO_H_INCLUDED
#include "3gpp_amrdecoderspecificinfo.h"
#endif

#ifndef GPP_H263DECODERSPECIFICINFO_H_INCLUDED
#include "3gpp_h263decoderspecificinfo.h"
#endif

#ifndef H263DECODERSPECIFICINFO_H_INCLUDED
#include "h263decoderspecificinfo.h"
#endif

#ifndef AVCCONFIGURATIONBOX_H_INCLUDED
#include "avcconfigurationbox.h"
#endif

#ifndef MPEG4SAMPLEENTRYEXTENSIONS_H_INCLUDED
#include "mpeg4sampleentryextensions.h"
#endif

#ifndef DECODERSPECIFICINFO_H_INCLUDED
#include "decoderspecificinfo.h"
#endif

class OMADRMKMSBox : public FullAtom
{
    public:
        OMADRMKMSBox(MP4_FF_FILE *fp,
                     uint32 size,
                     uint32 type);  // Default constructor
        virtual ~OMADRMKMSBox() ;

        uint8* getODKMData()
        {
            return _pODKMData;
        }
        uint32 getODKMDataSize()
        {
            return _size;
        }

    private:
        uint8* _pODKMData;
};


class MutableDRMInformationBox : public Atom
{
    public:
        MutableDRMInformationBox(MP4_FF_FILE *fp,
                                 uint32 size,
                                 uint32 type);  // Default constructor
        virtual ~MutableDRMInformationBox() {};

    private:
};

class SchemeInformationBox : public Atom
{
    public:
        SchemeInformationBox(MP4_FF_FILE *fp,
                             uint32 size,
                             uint32 type);  // Default constructor
        virtual ~SchemeInformationBox();  // Destructor

        uint32 getTrackLevelOMA2DRMInfoSize()
        {
            if (_pOMADRMKMSBox != NULL)
            {
                return (_pOMADRMKMSBox->getODKMDataSize());
            }
            return 0;
        }

        uint8* getTrackLevelOMA2DRMInfo()
        {
            if (_pOMADRMKMSBox != NULL)
            {
                return (_pOMADRMKMSBox->getODKMData());
            }
            return NULL;
        }

    private:
        OMADRMKMSBox* _pOMADRMKMSBox;

};

class OriginalFormatbox : public Atom
{
    public:
        OriginalFormatbox(MP4_FF_FILE *fp,
                          uint32 size,
                          uint32 type);  // Default constructor
        virtual ~OriginalFormatbox(); // Destructor

    private:
        uint32 _data_format;

};

class ProtectionSchemeInformationBox : public Atom
{
    public:
        ProtectionSchemeInformationBox(MP4_FF_FILE *fp,
                                       uint32 size,
                                       uint32 type);  // Default constructor
        virtual ~ProtectionSchemeInformationBox();  // Destructor

        uint32 getTrackLevelOMA2DRMInfoSize()
        {
            if (_pSchemeInformationBox != NULL)
            {
                return (_pSchemeInformationBox->getTrackLevelOMA2DRMInfoSize());
            }
            return 0;
        }

        uint8* getTrackLevelOMA2DRMInfo()
        {
            if (_pSchemeInformationBox != NULL)
            {
                return (_pSchemeInformationBox->getTrackLevelOMA2DRMInfo());
            }
            return NULL;
        }

    private:
        SchemeInformationBox* _pSchemeInformationBox;
        OriginalFormatbox* _pOriginalFormatBox;
};

class EcnaBox : public SampleEntry
{

    public:
        EcnaBox(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~EcnaBox();

        // Member gets and sets
        const ESDAtom &getESDAtom() const
        {
            return *_pes;
        }

        uint16 getTimeScale()
        {
            return _timeScale;
        }

        virtual uint32 getESID() const;
        virtual const ESDescriptor *getESDescriptor() const;
        virtual    uint8 getObjectTypeIndication() const;
        virtual DecoderSpecificInfo *getDecoderSpecificInfo() const;
        uint32 getAverageBitrate() const;
        uint32 getMaxBitrate() const;
        // Get the max size buffer needed to retrieve the media samples
        uint32 getMaxBufferSizeDB() const;
        ProtectionSchemeInformationBox *_pProtectionSchemeInformationBox;

    private:
        // Reserved constants
        uint32 _reserved1[2]; // = { 0, 0 };
        uint16 _reserved2; // = 2;
        uint16 _reserved3; // = 16;
        uint32 _reserved4; // = 0;
        uint16 _reserved5; // = 0;
        uint16 _timeScale;

        ESDAtom *_pes;
        AMRSpecificAtom *_pAMRSpecificAtom;
        AMRSpecificAtom *_pAMRWBSpecificAtom;
        Oscl_Vector<DecoderSpecificInfo *, OsclMemAllocator> *_pAMRDecSpecInfoArray;
        Oscl_Vector<DecoderSpecificInfo *, OsclMemAllocator> *_pAMRWBDecSpecInfoArray;
};

class EcnvBox : public SampleEntry
{

    public:
        EcnvBox(MP4_FF_FILE *fp, uint32 size, uint32 type);
        virtual ~EcnvBox();

        // Member gets and sets
        const ESDAtom &getESDAtom() const
        {
            return *_pes;
        }

        virtual uint32 getESID() const;
        virtual const ESDescriptor *getESDescriptor() const;

        // Getting and setting the Mpeg4 VOL header
        DecoderSpecificInfo *getDecoderSpecificInfo() const;
        uint8 getObjectTypeIndication() const;

        // Get the max size buffer needed to retrieve the media samples
        uint32 getMaxBufferSizeDB() const;
        uint32 getAverageBitrate() const;
        uint32 getMaxBitrate() const;
        uint16 getWidth() const;
        uint16 getHeight() const;
        ProtectionSchemeInformationBox *_pProtectionSchemeInformationBox;

    private:
        // Reserved constants
        uint32 _reserved1[4]; // = { 0, 0, 0, 0 };
        uint32 _reserved2; // = 0x014000f0;
        uint32 _reserved3; // = 0x00480000;
        uint32 _reserved4; // = 0x00480000;
        uint32 _reserved5; // = 0;
        uint16 _reserved6; // = 1;
        uint8  _reserved7[32]; // = 0;
        uint16 _reserved8; // = 24;
        uint16 _reserved9; // = -1; // (16) SIGNED!

        bool createAVCDecoderSpecificInfo(MP4_FF_FILE *fp);
        ESDAtom *_pes;
        H263SpecficAtom          *_pH263SpecificAtom;
        H263DecoderSpecificInfo  *_pH263decSpecificInfo;
        DecoderSpecificInfo* _decoderSpecificInfo;
        AVCConfigurationBox          *_pAVCConfigurationBox;
        MPEG4BitRateBox              *_pMPEG4BitRateBox;
};

class BoxRecord;
class FontRecord;
class StyleRecord;
class FontTableAtom;

class EnctBox : public SampleEntry
{

    public:
        OSCL_IMPORT_REF EnctBox(MP4_FF_FILE *fp, uint32 size, uint32 type);
        OSCL_IMPORT_REF virtual ~EnctBox();  // Destructor

        uint32 getDisplayFlags()
        {
            return _displayFlags;
        }

        int8 getHorzJustification()
        {
            return _horzJustification;
        }

        int8 getVertJustification()
        {
            return _vertJustification;
        }

        uint8 *getBackgroundColourRGBA()
        {
            return _pBackgroundRGBA;
        }

        OSCL_IMPORT_REF int16 getBoxTop();
        OSCL_IMPORT_REF int16 getBoxLeft();
        OSCL_IMPORT_REF int16 getBoxBottom();
        OSCL_IMPORT_REF int16 getBoxRight();

        OSCL_IMPORT_REF uint16 getStartChar() ;
        OSCL_IMPORT_REF uint16 getEndChar() ;
        OSCL_IMPORT_REF uint16 getFontID();
        OSCL_IMPORT_REF uint8 getFontStyleFlags() ;
        OSCL_IMPORT_REF uint8 getfontSize();
        OSCL_IMPORT_REF uint8 *getTextColourRGBA();
        OSCL_IMPORT_REF uint16  getFontListSize();
        OSCL_IMPORT_REF FontRecord   *getFontRecordAt(uint16 index) ;
        ProtectionSchemeInformationBox *_pProtectionSchemeInformationBox;

    private:
        uint32 	        _displayFlags;
        int8  	        _horzJustification;
        int8  	        _vertJustification;
        uint8 		    *_pBackgroundRGBA;
        BoxRecord 	    *_pBoxRecord;
        StyleRecord 	*_pStyleRecord;
        FontTableAtom	*_pFontTableAtom;
};


#endif // OMA2BOXES_H_INCLUDED

