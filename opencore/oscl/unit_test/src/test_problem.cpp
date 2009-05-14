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
#include "test_problem.h"

//constructor (dumb initializer) for test_problem
test_problem::test_problem(const _STRING& filename,
                           long line_number,
                           const _STRING& message) :
        m_line_number(line_number),
        m_filename(filename),
        m_message(message)
{
}

//copy constructor
test_problem::test_problem(const test_problem& rhs) :
        m_line_number(rhs.m_line_number),
        m_filename(rhs.m_filename),
        m_message(rhs.m_message)
{
}

//copy operator
test_problem&
test_problem::operator=(const test_problem & rhs)
{
    m_line_number = rhs.m_line_number;
    m_filename = rhs.m_filename;
    m_message = rhs.m_message;
    return *this;
}

//data access
long
test_problem::line_number(void) const
{
    return m_line_number;
}


//data access
const _STRING&
test_problem::filename(void) const
{
    return m_filename;
}

//data access
const _STRING&
test_problem::message(void) const
{
    return m_message;
}
