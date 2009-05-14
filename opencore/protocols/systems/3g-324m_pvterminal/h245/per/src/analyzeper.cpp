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
// ----------------------------------------------------------------------
//
// This Software is an original work of authorship of PacketVideo Corporation.
// Portions of the Software were developed in collaboration with NTT  DoCoMo,
// Inc. or were derived from the public domain or materials licensed from
// third parties.  Title and ownership, including all intellectual property
// rights in and to the Software shall remain with PacketVideo Corporation
// and NTT DoCoMo, Inc.
//
// -----------------------------------------------------------------------
// ============================================================
// FILE: AnalyzePER.c
//
// DESCRIPTION: PER analysis support routines.
//   These routines provide support for the automatically
//   generated functions in h245_analysis.[ch].
//
// Written by Ralph Neff, PacketVideo, 3/6/2000
// (c) 2000 PacketVideo Corp.
// ============================================================

#include "oscl_base.h"
#include "per_headers.h"
#include "genericper.h"
#include "analyzeper.h"

#ifdef PVANALYZER	// Analyzer interface function
#include <tchar.h>
void PVAnalyzer(unsigned int tag, Tint8 *message_fmt, ...);
#endif

/* --------------------------------------------- */
/* ----------- BASE LEVEL int32ERFACE ------------ */
/* --------------------------------------------- */

/*
void MyPVAnalyzer(uint16 tag, uint8 *outString)
{
	FILE *fp = fopen("analyzer.txt","a");
	fprintf(fp, outString);
	fprintf(fp, "\n");
	fclose(fp);
}
*/

// =========================================================
// Show245()
//
// This function takes an output analysis line, adds the
// proper indent, and sends the result out to the display
// routine.  The 'tag' argument is simply passed on.
// =========================================================
void Show245(uint16 tag, uint16 indent, const char* inString)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(inString);
#ifdef PVANALYZER  //07/12/01wjj
    uint32 i;
    uint8 outString[200];

    /* Construct outString with proper indent */
    for (i = 0;i < indent;++i)
    {
        outString[i] = ' ';
    }
    sprintf(outString + i, "%s", inString);

//#ifdef PVANALYZER  //07/12/01wjj
    /* Send outString to display */
    PVAnalyzer(tag, outString);        /* The real deal */
#endif
}

// =========================================================
// ShowHexData()
//
// This function shows a hex dump of a string of uint8s.
// The resulting output line(s) gets displayed by call to
// Show245().  The first two input arguments (tag, indent)
// are simply passed through.
// =========================================================
void ShowHexData(uint16 tag, uint16 indent, uint16 size, uint8* data)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(data);
#ifdef PVANALYZER  //07/12/01wjj
    uint32 i;
    uint8 outString[100];

    for (i = 0;i < size;++i)
    {
        sprintf(outString + 3*(i % 16), "%02x ", data[i]);
        if ((i % 16) == 15 || (i + 1) == size)
        {
            outString[3*(i%16+1)] = '\0';
            Show245(tag, indent, outString);
        }
    }
#endif
}

/* --------------------------------------------- */
/* ----------- LOW LEVEL OBJECTS --------------- */
/* --------------------------------------------- */

// =========================================================
// ShowPERNull()
//
// This function displays a NULL object via call to Show245().
// =========================================================
void ShowPERNull(uint16 tag, uint16 indent, const char* label)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s = NULL", label);
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPERBoolean()
//
// This function displays a BOOLEAN object via call to
// Show245().
// =========================================================
void ShowPERBoolean(uint16 tag, uint16 indent, const char* label, uint32 value)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(value);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    if (value)
    {
        sprintf(outString, "%s = TRUE", label);
    }
    else
    {
        sprintf(outString, "%s = FALSE", label);
    }
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPERInteger()
//
// This function displays an int32EGER object via call to
// Show245().
// =========================================================
void ShowPERInteger(uint16 tag, uint16 indent, const char* label, uint32 value)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(value);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s = %u", label, value);
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPERSignedInteger()
//
// This function displays the signed variant of an int32EGER
// object via call to Show245().
// =========================================================
void ShowPERSignedInteger(uint16 tag, uint16 indent, const char* label, int32 value)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(value);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s = %d", label, value);
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPERUnboundedInteger()
//
// This function displays the unbounded variant of an int32EGER
// object via call to Show245().  We currently take the
// input type to be uint32.
// =========================================================
void ShowPERUnboundedInteger(uint16 tag, uint16 indent, const char* label, uint32 value)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(value);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s = %u", label, value);
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPEROctetString()
//
// This function displays an OCTETSTRING object via call
// to Show245().
// =========================================================
void ShowPEROctetString(uint16 tag, uint16 indent, const char* label, PS_OCTETSTRING x)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(x);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s is an OCTET STRING (SIZE = %hu)",
            label, x->size);
    Show245(tag, indent, outString);
    ShowHexData(tag, (uint16)(indent + 2), x->size, x->data);
#endif
}

// =========================================================
// ShowPERBitString()
//
// This function displays a BITSTRING object via calls
// to Show245().
// =========================================================
void ShowPERBitString(uint16 tag, uint16 indent, const char *label, PS_BITSTRING x)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(x);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s is a BIT STRING (SIZE = %hu)",
            label, x->size);
    Show245(tag, indent, outString);
    ShowHexData(tag, (uint16)(indent + 2), (uint16)(x->size / 8), x->data);
#endif
}

// =========================================================
// ShowPERCharString()
//
// This function displays a int8STRING object via calls
// to Show245().
// =========================================================
void ShowPERCharString(uint16 tag, uint16 indent, const char* label, PS_int8STRING x)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(x);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];
    uint8 displayType = 1;  /* Select output type -- See Below */

    sprintf(outString, "%s is a int8 STRING (SIZE = %hu)",
            label, x->size);
    Show245(tag, indent, outString);

    if (displayType == 0)
    {
        /* Show Hex Version of string data */
        ShowHexData(tag, (uint16)(indent + 2), x->size, x->data);
    }
    else
    {
        /* Show Printable character version */
        sprintf(outString, "stringdata = \"%s\"", x->data);
        Show245(tag, (uint16)(indent + 2), outString);
    }
#endif
}

// =========================================================
// ShowPERObjectID()
//
// This function displays an OBJECT IDENTIFIER via calls
// to Show245().
// =========================================================
void ShowPERObjectID(uint16 tag, uint16 indent, const char* label, PS_OBJECTIDENT x)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(x);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s is an OBJECT IDENTIFIER (SIZE = %hu)",
            label, x->size);
    Show245(tag, indent, outString);
    ShowHexData(tag, (uint16)(indent + 2), x->size, x->data);
#endif
}

/* --------------------------------------------- */
/* ---------- HIGHER LEVEL OBJECTS ------------- */
/* --------------------------------------------- */

// =========================================================
// ShowPERChoice()
//
// This function displays the identity of a toplevel
// CHOICE definition.
// =========================================================
void ShowPERChoice(uint16 tag, uint16 indent, const char *label, const char *typestring)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(typestring);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s = %s (CHOICE)", label, typestring);
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPERSequence()
//
// This function displays the identity of a toplevel
// SEQUENCE definition.
// =========================================================
void ShowPERSequence(uint16 tag, uint16 indent, const char* label, const char* typestring)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(typestring);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s = %s (SEQUENCE)", label, typestring);
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPERSequenceof()
//
// This function displays the identity of a toplevel
// SEQUENCE OF definition.
// =========================================================
void ShowPERSequenceof(uint16 tag, uint16 indent, const char* label, const char* typestring)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(typestring);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s = %s (SEQUENCE-OF)", label, typestring);
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPERSetof()
//
// This function displays the identity of a toplevel
// SET OF definition.
// =========================================================
void ShowPERSetof(uint16 tag, uint16 indent, const char* label, const char* typestring)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(typestring);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s = %s (SET-OF)", label, typestring);
    Show245(tag, indent, outString);
#endif
}

// =========================================================
// ShowPERClosure()
//
// This function displays the closure of a top level object.
// It should be called once with the existing (already
// incremented) indent level.
// =========================================================
void ShowPERClosure(uint16 tag, uint16 indent, const char* label)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    if (indent >= 2)  /* Reduce indent, guarantee minimum 0 */
    {
        indent -= 2;
    }
    sprintf(outString, "(end of %s)", label);
    Show245(tag, indent, outString);
#endif
}

/* --------------------------------------------- */
/* ----------- ARRAYS OF OBJECTS --------------- */
/* --------------------------------------------- */

// =========================================================
// ShowPERIntegers()
//
// This function displays an int32EGER object which is the
// ofitem of a SeqOf or SetOf.  It does so via call to
// ShowPERInteger(), after constructing a special label.
// =========================================================
void ShowPERIntegers(uint16 tag, uint16 indent, const char* label, uint32 value, uint32 number)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(value);
    OSCL_UNUSED_ARG(number);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s[%u]", label, number);
    ShowPERInteger(tag, indent, outString, value);
#endif
}

// =========================================================
// ShowPEROctetStrings()
//
// This function displays an OCTET STRING which is the
// ofitem of a SeqOf or SetOf.  It does so via call to
// ShowPEROctetString(), after constructing a special label.
// =========================================================
void ShowPEROctetStrings(uint16 tag, uint16 indent, const char* label, PS_OCTETSTRING x, uint32 number)
{
    OSCL_UNUSED_ARG(tag);
    OSCL_UNUSED_ARG(indent);
    OSCL_UNUSED_ARG(label);
    OSCL_UNUSED_ARG(x);
    OSCL_UNUSED_ARG(number);
#ifdef PVANALYZER  //07/12/01wjj
    uint8 outString[100];

    sprintf(outString, "%s[%u]", label, number);
    ShowPEROctetString(tag, indent, outString, x);
#endif
}

// -----------------------------------------------------------
// Dummy PVAnalyzer() Function
// --> This may be added to provide the analyzer function if
//       PVAnalyzer() is not provided at the application layer.
//       However, the app layer is really where the analyzer
//       should reside.
// --> Note: This requires <stdio.h> and <time.h> to be included.
// -----------------------------------------------------------
//#ifdef PVANALYZER
//void PVAnalyzer( uint32 tag, Tint8 *pPerDataIn,...)
//{
//  FILE    *pFile;
//  uint8 *pFileName ="h245per.txt";
//  uint8 pTimeBuf[32];
//  uint8 *tagString;
//
//  pFile = fopen( pFileName, "a+");
//  if( pFile != NULL )
//  {
//    _tzset();
//    _strtime( pTimeBuf );
//
//    if(tag==2)
//        tagString="PerEnc";
//    else if(tag==4)
//        tagString="PerEnx";
//    else if(tag==8)
//        tagString="PerDec";
//    else if(tag==16)
//        tagString="PerDe";
//    else if(tag==32)
//        tagString="  Se  ";
//    else if(tag==64)
//        tagString="  Tsc ";
//
//    fprintf( pFile, "<%s> <%s> %s\n", pTimeBuf, tagString, pPerDataIn );
//    fclose(pFile);
//  } /*end if(pFile != NULL) */
//}
//#endif
