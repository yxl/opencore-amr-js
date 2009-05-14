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
#ifndef FILE_CONVERTER_TEST_H
#define FILE_CONVERTER_TEST_H


#include "test_case.h"
#include "oscl_types.h"
#include "oscl_string_containers.h"
#include "text_test_interpreter.h"
#include "oscl_vector.h"
#include "oscl_linked_list.h"
#include "rtp_file_converter.h"





class rtp_file_converter_test : public test_case
{
    public:
        virtual void setup(void) {};

        int convert(cmd_line* cmd_line_ptr);

        virtual void test(void);

        rtp_file_converter_test();
        ~rtp_file_converter_test();


        octet* buffer;
        int index;

};


class rtp_file_converter_test_suite : public test_case
{
    public:
        rtp_file_converter_test_suite(cmd_line* cmd_line_ptr)
        {
            iPtrCommandLine = cmd_line_ptr;
        }

        void add_test_case(rtp_file_converter_test *testcase)
        {
            testcase->SetCommandLine(iPtrCommandLine);
            adopt_test_case(testcase);
        }

    private:
        cmd_line* iPtrCommandLine;

};


#endif // FILE_CONVERTER_TEST_H

