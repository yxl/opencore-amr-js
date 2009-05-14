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
#include "unit_test_args.h"



void cmd_line_linux::setup(int , oscl_wchar**)
{
    argc = 0;
    argv = NULL;
}

void cmd_line_linux::setup(int count, char** cmd_argv)
{
    argc = count;
    argv = cmd_argv;
}


int cmd_line_linux::get_count(void)
{
    return argc;
}

void cmd_line_linux::get_arg(int n, char* &arg)
{
    if (n > argc)
        arg = NULL;
    else
        arg = argv[n];
}

void cmd_line_linux::get_arg(int , oscl_wchar *&arg)
{
    arg = NULL;
}

bool cmd_line_linux::is_wchar(void)
{
    return false;
}

