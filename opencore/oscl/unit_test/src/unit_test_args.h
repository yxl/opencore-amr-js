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
#ifndef __UNIT_TEST_TEST_ARGS__
#define __UNIT_TEST_TEST_ARGS__

#include "unit_test_common.h"

class cmd_line
{
    public:

        virtual ~cmd_line() {}

        virtual int get_count(void) = 0;

        virtual void setup(int count, oscl_wchar** cmd_argv) = 0;
        virtual void setup(int count, char** cmd_argv) = 0;


        virtual void get_arg(int n, char* &arg) = 0;


        virtual void get_arg(int n, oscl_wchar* &arg) = 0;

        virtual bool is_wchar(void) = 0;
};



class cmd_line_linux : public cmd_line
{
    private:
        char** argv;
        int argc;

    public:
        void setup(int count, oscl_wchar** cmd_argv);
        void setup(int count, char** cmd_argv);

        int get_count(void);

        void get_arg(int n, char* &arg);
        void get_arg(int n, oscl_wchar *&arg);

        bool is_wchar(void);
};



#endif //__UNIT_TEST_TEST_ARGS__
