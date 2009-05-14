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
#include "pv_mime_string_utils.h"
#include "oscl_mem.h"

char *oscl_strcpy(char *str1, char *str2)
{
    return oscl_strncpy(str1, str2, oscl_strlen(str2) + 1);
}

void pv_mime_toggle(int *quote_flag)
{
    if (*quote_flag == 0)
        *quote_flag = 1;
    else
        *quote_flag = 0;
}

char *pv_mime_string_append_param(char *mimestring, char *string, char* newstring)
{
    if (mimestring == NULL)
    {
        return NULL;

    }

    /* @todo fix the length guards here.  The uses of strncat
     * are false ... need to check against length of TARGET
     * string, not source!  But that's not available here...
     */
    oscl_strcpy(newstring, mimestring);
    oscl_strncat(newstring, (char *)";", oscl_strlen(";"));
    oscl_strncat(newstring, string, oscl_strlen(string));
    return newstring;
}

char *pv_mime_string_append_type(char *mimestring, char *string, char* newstring)
{
    int i = 0;
    if (mimestring == NULL)
    {
        return NULL;
    }

    while (mimestring[i] != '\0' && mimestring[i] != ';')
    {
        i++;
    }

    if (mimestring[i] == '\0') // i.e mime strig has no parametres
    {
        oscl_strcpy(newstring, mimestring);
        oscl_strncat(newstring, (char *)"/", oscl_strlen("/"));
        oscl_strncat(newstring, string, oscl_strlen(string));
    }
    else // mimestring[i] == ';'
    {
        if (i > 0)	// skip if i==0 (mimestring==";...")
            // (is that an error case we shd flag?)
        {
            oscl_strncpy(newstring, mimestring, i - 1);
        }
        oscl_strncat((newstring), (char *)"/", oscl_strlen("/"));
        oscl_strncat(newstring, string, oscl_strlen(string));
        oscl_strncat(newstring, (mimestring + i), oscl_strlen(mimestring + i));
    }

    return newstring;
}

int pv_mime_string_parse_type(char *mimestring, char *string, char * &stringaddr)
{
    if (mimestring == NULL)
    {
        stringaddr = NULL;
        return 0;
    }

    int i = 0, j = 0;
    char *tempcomp = OSCL_ARRAY_NEW(char, oscl_strlen(mimestring));
    // if NEW fails, this will OSCL_LEAVE .. so tempcomp is non-null here

    while ((mimestring[i] != '\0') && (mimestring[i] != ';'))
    {
        while (mimestring[i] != '/')
        {
            if (mimestring[i] == '\0')
                break;

            tempcomp[j++] = mimestring[i++];
        }

        tempcomp[j] = '\0';

        if (!oscl_CIstrcmp(string, tempcomp))
        {
            int ptr = i - j;
            stringaddr = &mimestring[ptr];
            OSCL_ARRAY_DELETE(tempcomp);
            return (oscl_strlen(stringaddr));
        }

        j = 0;
        i++;
    }

    OSCL_ARRAY_DELETE(tempcomp);
    return 0;
}

OSCL_EXPORT_REF int pv_mime_string_parse_param(char *mimestring, char *string, char * &stringaddr)
{
    int i = 0, j = 0, quote_flag = 0;

    if (mimestring == NULL)
    {
        stringaddr = NULL;
        return 0;
    }

    char *tempcomp = OSCL_ARRAY_NEW(char, oscl_strlen(mimestring));
    // Clear all types
    while (mimestring[i] != '\0' && mimestring[i] != ';')
    {
        i++;
    }

    if (mimestring[i] == '\0')  // If there are no parametres
    {
        OSCL_ARRAY_DELETE(tempcomp);
        return 0;
    }
    else // There are parametres we have to locate the one we want
    {
        i++; // get over ";"
        while (mimestring[i] != '\0')
        {
            if ((mimestring[i] == '\"') && (mimestring[i-1] != '\\'))
            {
                pv_mime_toggle(&quote_flag);
            }
            while (!(mimestring[i] == ';' && quote_flag == 0))
            {
                if (mimestring[i] == '\0')
                    break;

                tempcomp[j++] = mimestring[i++];
                if ((mimestring[i] == '\"') && (mimestring[i-1] != '\\'))
                {
                    pv_mime_toggle(&quote_flag);
                }
            }

            tempcomp[j] = '\0';
            if (!oscl_CIstrcmp(string, tempcomp))
            {
                int ptr = i - j;
                stringaddr = &mimestring[ptr];
                OSCL_ARRAY_DELETE(tempcomp);
                return (oscl_strlen(stringaddr));
            }

            if (mimestring[i] == '\0') break;

            j = 0;
            i++;
        }
    }

    OSCL_ARRAY_DELETE(tempcomp);
    return 0;
}


OSCL_EXPORT_REF int pv_mime_strcmp(const char *mimestring1, const char *mimestring2)
{
    int i = 0;

    // Some constants used in this method
    const char MIME_PARAM_DELIMITER_CHAR = ';';
    const char MIME_SUBTYPE_DELIMITER_CHAR = '/';

    // constants for the return values
    const int RETVAL_NO_MATCH = -1;
    const int RETVAL_MATCH = 0;
    const int RETVAL_FIRST_PREFIX = 1;
    const int RETVAL_SECOND_PREFIX = 2;

    if ((mimestring1 == NULL) || (mimestring2 == NULL))
    {
        // if either string pointer is NULL then return NO MATCH
        return RETVAL_NO_MATCH;
    }

    // Iterate through the strings doing case-insensitive matching until the
    // the first non-matching character or a terminating character.
    for (i = 0;mimestring1[i] != '\0' && oscl_tolower(mimestring1[i]) == oscl_tolower(mimestring2[i]);i++)
    {
        if ((mimestring1[i] == MIME_PARAM_DELIMITER_CHAR) ||
                (mimestring2[i] == MIME_PARAM_DELIMITER_CHAR))
        {
            // one of the strings has reached the parameters section.
            // The type string has been extracted
            break;
        }
    }

    // initialize the length values of both strings to 'i+1' then the checks
    // below will correct it if we actually reached a string terminator
    // for that string.
    int len1 = i + 1;
    int len2 = len1;


    // set the length for the two strings
    if (mimestring1[i] == MIME_PARAM_DELIMITER_CHAR ||
            mimestring1[i] == '\0')
    {
        len1 = i;
    }

    if (mimestring2[i] == MIME_PARAM_DELIMITER_CHAR ||
            mimestring2[i] == '\0')
    {
        len2 = i;
    }

    if (len1 == len2)
    {
        // if the lengths are equal && one string has got '\0' or MIME_PARAM_DELIMITER_CHAR
        if (len1 == i)
        {
            return RETVAL_MATCH;
        }
        return RETVAL_NO_MATCH;
    }
    else if (len1 < len2)
    {
        // check if string1 is a prefix.
        /* Must have subtype delimiter character in string2 for the prefix
         * condition to be true.  For example something like str1 = "aa/bb" and
         * str2 = "aa/bb/cc".  Instead if str2 = "aa/bbcc" then str1 would
         * not be considered a prefix
         */
        if (mimestring2[i] == MIME_SUBTYPE_DELIMITER_CHAR)
        {
            return RETVAL_FIRST_PREFIX;
        }
        return RETVAL_NO_MATCH;
    }
    else
    {	// must be len2 < len1
        // see comments on previous case for how to check for prefix condition
        if (mimestring1[i] == MIME_SUBTYPE_DELIMITER_CHAR)
        {
            return RETVAL_SECOND_PREFIX;
        }
        return RETVAL_NO_MATCH;
    }

}


OSCL_EXPORT_REF int pv_mime_strstr(char *mimestring1, char *mimestring2)
{
    //skip any .../ in either string.
    char* str1 = mimestring1;
    char* str2 = mimestring2;
    if (oscl_strncmp(str1, ".../", 4) == 0)
        str1 += 4;
    if (oscl_strncmp(str2, ".../", 4) == 0)
        str2 += 4;

    //if either string is empty, they match.
    if (str1[0] == '\0'
            || str2[0] == '\0')
        return 0;

    //count components.
    int n1 = pv_mime_string_compcnt(str1);
    int n2 = pv_mime_string_compcnt(str2);

    //swap if needed so that str1 is the larger.
    if (n2 > n1)
    {
        {
            char* temp = str1;
            str1 = str2;
            str2 = temp;
        }
        {
            int temp = n1;
            n1 = n2;
            n2 = temp;
        }
    }

    //match str2 against any sub-set of str1.
    for (int n = 0;n < n1;n++)
    {
        char* str = NULL;
        pv_mime_string_extract_type(n, str1, str);
        int cmp = pv_mime_strcmp(str, str2);
        if (cmp != (-1))
            return cmp;
    }
    return (-1);//no match.
}

OSCL_EXPORT_REF int pv_mime_string_extract_param(int compnumber, char *mimestring, char * &string)
{
    int count = 0, j = 0, start = 0, quote_flag = 0;

    if (mimestring == NULL)
    {
        string = NULL;
        return 0;
    }

    // Seek to the beginning of the parametre list
    while (mimestring[j] != '\0' && mimestring[j] != ';')
    {
        j++;
    }

    start = j + 1;
    j++; // get j past ";"

    if (mimestring[j] == '\0')
    {
        return 0; // mimestring has no parametres
    }
    else // mimestring has ";"
    {
        while (mimestring[j] != '\0')
        {
            if ((mimestring[j] == '\"') && (mimestring[j-1] != '\\'))
            {
                pv_mime_toggle(&quote_flag);
            }

            if (((mimestring[j] == ';') || (mimestring[j] == '\0')) && (quote_flag == 0))
            {
                if (compnumber == count)
                {
                    string = &mimestring[start];
                    return (j -start);

                }
                else
                    count++;

                start = j + 1;
            }

            if (j++ > (int)oscl_strlen(mimestring))
                return 0;
        }

        if (compnumber == count && mimestring[j] == '\0')
        {
            string = &mimestring[start];
            return (j -start);
        }
        return 0;
    }
}

OSCL_EXPORT_REF int pv_mime_string_extract_type(int compnumber, char *mimestring, char * &string)
{
    int count = 0 , j = 0, start = 0;
    if (mimestring == NULL)
    {
        string = NULL;
        return 0;
    }

    while (mimestring[j] != '\0')
    {
        if ((mimestring[j] == '/') ||
                (mimestring[j] == '\0') ||
                (mimestring[j] == ';'))
        {
            if (compnumber == count)
            {
                string = &mimestring[start];
                return (j -start);
            }
            else
            {
                count++;
            }

            start = j + 1;

            if (mimestring[j] == ';')
            {
                break;
            }
        }

        if (j++ > (int)oscl_strlen(mimestring))
        {
            return 0;
        }
    }

    if (compnumber == count && mimestring[j] == '\0')
    {
        string = &mimestring[start];
        return (j -start);
    }

    return 0;
}

OSCL_EXPORT_REF int pv_mime_string_compcnt(char* mimestring)
{
    int count = 1;  /* mime string will have at least one component */
    int quote_flag = 0;

    if (mimestring == NULL)
    {
        return 0;
    }

    for (int i = 0; mimestring[i] != '\0'; i++)
    {
        if ((mimestring[i] == '\"') && (mimestring[i-1] != '\\'))
        {
            pv_mime_toggle(&quote_flag);
        }

        if (mimestring[i] == '/' && !quote_flag)
            count++;
    }
    return count;
}

OSCL_EXPORT_REF int pv_mime_string_parcnt(char* mimestring)
{
    int count = 0;  /* mime string will have zero or more parametres*/
    int quote_flag = 0;

    if (mimestring == NULL)
    {
        return 0;
    }

    for (int i = 0; mimestring[i] != '\0'; i++)
    {
        if ((mimestring[i] == '\"') && (mimestring[i-1] != '\\'))
        {
            pv_mime_toggle(&quote_flag);
        }

        if (mimestring[i] == ';' && !quote_flag)
            count++;

    }
    return count;
}

bool pv_mime_string_is_relative(char* mimestring)
{
    if (mimestring == NULL)
    {
        return false;
    }
    else
    {
        if ((oscl_strncmp(mimestring, ".../", 4) == 0) && (oscl_strlen(mimestring) >= 5))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

char * pv_mime_string_strappend(char *mimestring, char *mimestring1, char* newstring)
{
    int i = 0, j = 0, k = 0;
    if ((mimestring == NULL) && (mimestring1 != NULL))
    {
        oscl_strcpy(newstring, mimestring1);
        return newstring;
    }
    else if ((mimestring1 == NULL) && (mimestring != NULL))
    {
        oscl_strcpy(newstring, mimestring);
        return newstring;

    }
    else if ((mimestring1 == NULL) && (mimestring == NULL))
    {
        newstring = NULL;
        return newstring;
    }

    if (pv_mime_string_is_relative(mimestring1))
    {
        j = 4;
    }

    while (mimestring[k] != '\0' && mimestring[k] != ';')
    {
        if ((mimestring[k] == '.') && (mimestring[k-1] == '/') && (mimestring[k+1] == '.') && (mimestring[k+2] == '/'))
        {
            k = k + 3; // get past "../"
            i = i - 2; // get past "/."

            while ((i >= 0) && (newstring[i] != '/'))
            {
                i--;
            }
            i++; // get past '/'
        }
        else
        {
            newstring[i] = mimestring[k];
            i++;
            k++;
        }

    }

    newstring[i++] = '/';

    // merge the TYPES first
    while (mimestring1[j] != '\0' && mimestring1[j] != ';')
    {
        if ((mimestring1[j] == '.') && (mimestring1[j-1] == '/') && (mimestring1[j+1] == '.') && (mimestring1[j+2] == '/'))
        {
            j = j + 3; // get past "../"
            i = i - 2; // get past "/."
            while ((i >= 0) && (newstring[i] != '/'))
            {
                i--;
            }

            i++; // get past '/'

        }
        else
        {
            newstring[i] = mimestring1[j];
            i++;
            j++;
        }
    }

    // Merge parametres of mimestring
    while (mimestring[k] != '\0')
    {
        newstring[i] = mimestring[k];
        k++;
        i++;
    }

    // Merge in parametres of mimestring1
    while (mimestring1[j] != '\0')
    {
        newstring[i] = mimestring1[j];
        i++;
        j++;
    }

    newstring[i] = '\0';
    return newstring;
}

