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
//                     PV_MIME_STRING_UTILS
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

/**
 *  @file pv_mime_string_utils.h
 *  @brief This file provide operations like compose,append, compare
 *  on mime string
 *
 */

#ifndef OSCL_MIME_STRING_UTILS_H
#define OSCL_MIME_STRING_UTILS_H

// - - Inclusion - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_STDSTRING_H_INCLUDED
#include "oscl_stdstring.h"
#endif

#ifndef OSCL_BASE_MACROS_H_INCLUDED
#include "oscl_base_macros.h"
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/**
 * Append new type to mime string
 *
 * @param mimestring	original mime string
 * @param string	string to append to mime string.
 * @param newstring	address where the newly appended string will get stored
 *
 * @return              new mime string
 */
char * pv_mime_string_append_type(char *mimestring, char *string, char* newstring);


/**
 * Append new parametre to mime string
 *
 * @param mimestring	original mime string
 * @param string	string to append to mime string.
 * @param newstring	address where the newly appended string will get stored
 *
 * @return              new mime string
 */
char * pv_mime_string_append_param(char *mimestring, char *string, char* newstring);


/**
 * Parse mime string to search for a type
 * a case in-sensitive search is performed to locate a string in mimestring.
 *
 * @param mimestring    original mime string
 * @param string		string to search in mime string.
 * @param stringaddr    address of mimestring starting from string
 *
 * @return              length of the rest of the string.
 */
int pv_mime_string_parse_type(char *mimestring, char *string, char * &stringaddr);

/**
 * Parse mime string to search for a parametre
 * a case in-sensitive search is performed to locate a string in mimestring.
 *
 * @param mimestring    original mime string
 * @param string        string to search in mime string.
 * @param stringaddr    address of mimestring starting from string
 *
 * @return              length of the rest of the string.
 */
OSCL_IMPORT_REF int pv_mime_string_parse_param(char *mimestring, char *string, char * &stringaddr);

/**
 * compare two strings (case in-sensitive comparision)
 *
 * @param mimestring1   string to compare.
 * @param mimestring2	string to compare.
 *
 * @return -1  no match
 *			0  if exact match
 *		    1  if first string is prefix of second string.
 *		    2  if second string is prefix of first string.

 */
OSCL_IMPORT_REF int pv_mime_strcmp(const char *mimestring1, const char *mimestring2);

/**
 * compare two strings (case in-sensitive comparision)
 *
 * @param mimestring1   string to compare.
 * @param mimestring2	string to compare.
 *
 * @return 0  if exact match
 *		   1  if one string is sub-string of another string.
 *        -1  no match
 */
OSCL_IMPORT_REF int pv_mime_strstr(char *mimestring1, char *mimestring2);

/**
 * Extract a type from mime string.
 *
 * @param compnumber     position of the component (0-based index).
 * @param mimestring	 mime string.
 * @param string         address of rest of mime string starting from compnumber.
 *
 * @return               length of the component.
 *
 */
OSCL_IMPORT_REF int pv_mime_string_extract_type(int compnumber, char *mimestring, char * &string);

/**
 * Extract a parametre from mime string.
 *
 * @param compnumber     position of the component (0-based index).
 * @param mimestring     mime string.
 * @param string         address of rest of mime string starting from compnumber.
 *
 * @return               length of the component.
 *
 */
OSCL_IMPORT_REF int pv_mime_string_extract_param(int compnumber, char *mimestring, char * &string);


/**
 * Count number of components in a mime string.
 *
 * @param mimestring    mime string.
 *
 * @return  number of components in mimestring.
 *
 */
OSCL_IMPORT_REF int pv_mime_string_compcnt(char* mimestring);


/**
 * Count number of parametres in a mime string.
 *
 * @param mimestring    mime string.
 *
 * @return  number of components in mimestring.
 *
 */
OSCL_IMPORT_REF int pv_mime_string_parcnt(char* mimestring);


/**
 * Check if the mime string is relative (i.e precedes with .../ ).
 *
 * @param mimestring    mime string.
 *
 * @return  true if the mime string is relative false otherwise.
 *
 */
bool pv_mime_string_is_relative(char* mimestring);


/**
 * Append new relative mime string to an existing one.
 *
 * @param mimestring    original mime string
 * @param rel_mimestring  relative mime string that needs to be appended.
 *
 * @return              new mime string if "rel-mimestring" is indeed a relative mime string NULL otherwise.
 */
char * pv_mime_string_strappend(char *mimestring, char *rel_mimestring, char* newstring);

#endif
