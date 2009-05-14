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
#ifndef TEST_PV_FRAME_METADATA_UTILITY_CONFIG_H_INCLUDED
#define TEST_PV_FRAME_METADATA_UTILITY_CONFIG_H_INCLUDED

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

// The default test file to use
#define DEFAULTSOURCEFILENAME "test.mp4"
#define DEFAULTSOURCEFORMATTYPE PVMF_MIME_MPEG4FF

// Set to 1 to enable test cases that output YUV 4:2:0, 0 to disable
#define RUN_YUV420_TESTCASES 1

// Set to 1 to enable test cases that output RGB 12bpp, 0 to disable
#define RUN_RGB12_TESTCASES 0

// Set to 1 to enable test cases that output RGB 16bpp, 0 to disable
#define RUN_RGB16_TESTCASES 1

// Set to 1 to enable test cases that output RGB 24bpp, 0 to disable
#define RUN_RGB24_TESTCASES 0

// Set to 1 to enable test cases that uses Janus CPM, 0 to disable
#define RUN_JANUSCPM_TESTCASES 1

// The source test file without video track
//#define NOVIDEOSOURCEFILENAME "test_amr.mp4"
#define NOVIDEOSOURCEFILENAME "test_aac.mp4"
#define NOVIDEOSOURCEFORMATTYPE PVMF_MIME_MPEG4FF

// Set to 1 to use the scheduler native to the system instead of PV scheduler
#define USE_NATIVE_SCHEDULER 0

// The string to prepend to source filenames
#define SOURCENAME_PREPEND_STRING ""
#define SOURCENAME_PREPEND_WSTRING _STRLIT_WCHAR("")

// The string to prepend to output filenames
#define OUTPUTNAME_PREPEND_STRING ""
#define OUTPUTNAME_PREPEND_WSTRING _STRLIT_WCHAR("")

#endif // TEST_PV_FRAME_METADATA_UTILITY_CONFIG_H_INCLUDED


