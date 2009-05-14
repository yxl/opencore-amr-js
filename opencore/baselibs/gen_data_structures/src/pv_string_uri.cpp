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

//               P V S T R I N G URI   C L A S S

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "pv_string_uri.h"
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


//////////////////////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVStringUri::PersentageToEscapedEncoding(char* aUrl, uint32 & aUrlMaxOutLength)
{
    int32 err;
    uint32 Url_Counter   = 0;
    uint32 Url_Tmp_Counter = 0;
    aUrlMaxOutLength = 0;
    mbchar* TempUrlBuffer = NULL;
    uint32 iPersentage_Flag = false;
    bool iRet = false;

    OSCL_TRY(err, TempUrlBuffer = OSCL_ARRAY_NEW(mbchar, MAX_FULL_REQUEST_SIZE));
    if ((err != OsclErrNone) || (TempUrlBuffer == NULL))
    {
        return false;
    }

    oscl_memset(TempUrlBuffer, 0, sizeof(TempUrlBuffer));
    while (aUrl[Url_Counter] != '\0')
    {

        switch (aUrl[Url_Counter])
        {
            case PERSENTAGE:
            {
                if (!(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '0') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '1') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == 'A') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '7') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '8') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '9') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == 'B') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '4') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '6') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == 'C') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == 'F') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '5') &&
                        !(aUrl[Url_Counter+1] == '2' && aUrl[Url_Counter+2] == '3') &&
                        !(aUrl[Url_Counter+1] == '3' && aUrl[Url_Counter+2] == 'A') &&
                        !(aUrl[Url_Counter+1] == '3' && aUrl[Url_Counter+2] == 'B') &&
                        !(aUrl[Url_Counter+1] == '3' && aUrl[Url_Counter+2] == 'D') &&
                        !(aUrl[Url_Counter+1] == '3' && aUrl[Url_Counter+2] == 'F') &&
                        !(aUrl[Url_Counter+1] == '5' && aUrl[Url_Counter+2] == 'B') &&
                        !(aUrl[Url_Counter+1] == '5' && aUrl[Url_Counter+2] == 'D') &&
                        !(aUrl[Url_Counter+1] == '4' && aUrl[Url_Counter+2] == '0'))
                {
                    TempUrlBuffer[Url_Tmp_Counter] = '%';
                    TempUrlBuffer[++Url_Tmp_Counter] = '2';
                    TempUrlBuffer[++Url_Tmp_Counter] = '5';
                    break;
                }
                iPersentage_Flag = true;
                break;
            }
            default :
                TempUrlBuffer[Url_Tmp_Counter] = aUrl[Url_Counter];
                break;
        }

        if (iPersentage_Flag)
        {
            TempUrlBuffer[Url_Tmp_Counter] = aUrl[Url_Counter];
            iPersentage_Flag = false;
        }
        Url_Counter++;
        Url_Tmp_Counter++;
    }

    TempUrlBuffer[Url_Tmp_Counter] = '\0';
    if (TempUrlBuffer && *TempUrlBuffer)
    {
        oscl_memset(aUrl, 0, sizeof(aUrl));
        oscl_strncpy(aUrl, TempUrlBuffer, (Url_Tmp_Counter));
        aUrl[oscl_strlen(TempUrlBuffer)] = '\0';
        aUrlMaxOutLength = Url_Tmp_Counter;
        iRet = true;
    }

    OSCL_ARRAY_DELETE(TempUrlBuffer);
    TempUrlBuffer = NULL;
    return iRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF bool PVStringUri::IllegalCharactersToEscapedEncoding(char* aUrl, uint32 &aUrlMaxOutLength)
{

    int32 err;
    uint32 Url_Counter   = 0;
    uint32 Url_Tmp_Counter = 0;
    aUrlMaxOutLength = 0;
    //mbchar IllegalTmpBuf[4];
    unsigned char* IllegalTmpBuf = NULL;
    uint32 Lenght = 0;
    mbchar* TempUrlBuffer = NULL;
    bool iRet = false;


    OSCL_TRY(err, TempUrlBuffer = OSCL_ARRAY_NEW(mbchar, MAX_FULL_REQUEST_SIZE));
    if ((err != OsclErrNone) || (TempUrlBuffer == NULL))
    {
        return false;
    }


    oscl_memset(TempUrlBuffer, 0, sizeof(TempUrlBuffer));

    while (aUrl[Url_Counter] != '\0')
    {
        if (!(aUrl[Url_Counter] >= 63 && aUrl[Url_Counter] <= 91) &&
                !(aUrl[Url_Counter] >= 97  && aUrl[Url_Counter] <= 122) &&
                !(aUrl[Url_Counter] >= 35  && aUrl[Url_Counter] <= 59) &&
                !(aUrl[Url_Counter] == 33) &&
                !(aUrl[Url_Counter] == 61) &&
                !(aUrl[Url_Counter] == 93) &&
                !(aUrl[Url_Counter] == 95))
        {
            //Dec To Hexa Conversion

            IllegalTmpBuf = DecimalToHexConverter(aUrl[Url_Counter] , Lenght);
            TempUrlBuffer[Url_Tmp_Counter] = '%';
            TempUrlBuffer[++Url_Tmp_Counter] = IllegalTmpBuf[1];
            TempUrlBuffer[++Url_Tmp_Counter] = IllegalTmpBuf[2];
            OSCL_ARRAY_DELETE(IllegalTmpBuf);
            IllegalTmpBuf = NULL;
        }

        else
        {
            TempUrlBuffer[Url_Tmp_Counter] = aUrl[Url_Counter];


        }

        Url_Counter++;
        Url_Tmp_Counter++;
    }

    TempUrlBuffer[Url_Tmp_Counter] = '\0';
    if (TempUrlBuffer && *TempUrlBuffer)
    {
        oscl_memset(aUrl, 0, sizeof(aUrl));
        oscl_strncpy(aUrl, TempUrlBuffer, Url_Tmp_Counter);
        aUrl[oscl_strlen(TempUrlBuffer)] = '\0';

        aUrlMaxOutLength = Url_Tmp_Counter;
        iRet = true;
    }

    OSCL_ARRAY_DELETE(TempUrlBuffer);
    TempUrlBuffer = NULL;
    return iRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF unsigned char *PVStringUri::DecimalToHexConverter(unsigned char aNumberIn, uint32 & aUrlMaxConvertLenght)
{
    unsigned char* FinalBuffer = NULL;
    //FinalBuffer = 0;

    unsigned char size = sizeof(unsigned char) * 2;
    FinalBuffer = OSCL_ARRAY_NEW(unsigned char, (size + 2));

    mbchar *LookUpTable = OSCL_CONST_CAST(mbchar*, "0123456789ABCDEF"); //corresponding index.
    unsigned char Temp = aNumberIn;

    FinalBuffer[0] = ' ';
    unsigned char mask = 0x000f;

    for (unsigned char Counter = 0; Counter < size; Counter++)
    {
        Temp = aNumberIn;
        Temp >>= (4 * Counter);
        Temp &= mask;
        FinalBuffer[size - Counter] = LookUpTable[Temp];
    }

    FinalBuffer[size + 1] = '\0';
    aUrlMaxConvertLenght = oscl_strlen((char*)FinalBuffer);
    return FinalBuffer;
}

