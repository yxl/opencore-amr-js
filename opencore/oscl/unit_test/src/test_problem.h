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
#ifndef TEST_PROBLEM_H
#define TEST_PROBLEM_H

#include "unit_test_common.h"



//$Id
//this is a test "problem" -- a note when a test failed or erred
class test_problem
{
    protected:
        //protected data members
        //line number of the error, if appropriate
        long m_line_number;
        //filename of the error, if appropriate
        _STRING m_filename;
        //additional message as appropriate
        _STRING m_message;
    public:
        //construction
        test_problem(const _STRING& filename, long line_number, const _STRING& message);
        test_problem(const test_problem& rhs);
        test_problem& operator= (const test_problem& rhs);
        //data access
        long line_number(void) const;
        const _STRING& filename(void) const;
        const _STRING& message(void) const;

};

#endif
