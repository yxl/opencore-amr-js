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
#include "osclconfig.h"
#include "test_case_point.h"
#include "text_test_interpreter.h"

#include <iostream>

int
int local_main(FILE* filehandle, cmd_line* command_line)
{
    point_test_suite pnt_test(argv, argc);
    pnt_test.run_test();
    text_test_interpreter interpreter;

    std::cout << interpreter.interpretation(pnt_test.last_result());
}
