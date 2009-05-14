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
#include "test_case.h"


//adopts a test case; we use "adopt" because we intend to take over
//the destruction of the passed object
void
test_case::adopt_test_case(test_case* new_case)
{
    m_subtests.push_back(new_case);
}

//the number of subtests
int
test_case::subtest_count(void) const
{
    return m_subtests.size();
}

//runs all the subtests
void
test_case::run_subtests(void)
{
    _VECTOR(test_case*, unit_test_allocator)::iterator iter = m_subtests.begin();
    for (;iter != m_subtests.end();
            ++iter)
    {
        (*iter)->run_test();
        m_last_result.add_result((*iter)->last_result());
    }
}

//actually runs the test
void
test_case::run_test(void)
{
    m_last_result.delete_contents();
    set_up();
    run_subtests();

    test();
    tear_down();
}

//tests to see if the supplied condition is true; called by the
//test_is_true macro
void
test_case::test_is_true_stub(bool condition,
                             const _STRING& condition_name,
                             const _STRING& filename,
                             long line_number)
{
    if (condition)
    {
        m_last_result.add_success();
    }
    else
    {

        _STRING message = "\"";
        _APPEND(message, condition_name);
        _APPEND(message, _STRING("\" evaluated to false."));

        m_last_result.add_failure(test_problem(filename,
                                               line_number,
                                               message));
    }
}


//creates a test problem representing an inequality
test_problem
test_case::create_equality_problem_report(const _STRING& actual_message,
        const _STRING& expected_message,
        const _STRING& filename,
        long line_number)
{
    _STRING message = "Equality failed.  Got ";
    _APPEND(message, actual_message);
    _APPEND(message, ", expected ");
    _APPEND(message, expected_message);
    return test_problem(filename, line_number, message);
}


//checks to see if two longs are equal
//tests to see if two longs are equal
void
test_case::test_int_is_equal_stub(long actual,
                                  long expected,
                                  const _STRING& filename,
                                  long line_number)
{
    if (actual == expected)
    {
        m_last_result.add_success();
    }
    else
    {
        char temp[100];
        sprintf(temp, "%ld", actual);
        _STRING actual_message(temp);
        sprintf(temp, "%ld", expected);
        _STRING expected_message(temp);
        m_last_result.add_failure(create_equality_problem_report(actual_message,
                                  expected_message,
                                  filename,
                                  line_number));
    }
}

//replacement for stdio fabs routine
static double _fabs(double in)
{
    if (in < 0)
        return (-in);
    else
        return in;
}

void
test_case::test_double_is_equal_stub(double actual,
                                     double expected,
                                     const _STRING& filename,
                                     long line_number,
                                     double tolerance)
{
    if (_fabs(actual - expected) < _fabs(tolerance))
    {
        m_last_result.add_success();
    }
    else
    {
        char temp[100];
        sprintf(temp, "%f", actual);
        _STRING actual_message(temp);
        sprintf(temp, "%f", expected);
        _STRING expected_message(temp);

        m_last_result.add_failure(create_equality_problem_report(actual_message,
                                  expected_message,
                                  filename,
                                  line_number));
    }
}


//tests to see if two strings are equal
void
test_case::test_string_is_equal_stub(const _STRING& actual,
                                     const _STRING& expected,
                                     const _STRING& filename,
                                     long line_number)
{
    if (actual == expected)
    {
        m_last_result.add_success();
    }
    else
    {
        _STRING actual_message = _STRING("\"");
        _APPEND(actual_message, actual);
        _APPEND(actual_message, "\"");
        _STRING expected_message = _STRING("\"");
        _APPEND(expected_message, expected);
        _APPEND(expected_message, "\"");

        m_last_result.add_failure(create_equality_problem_report(actual_message,
                                  expected_message,
                                  filename,
                                  line_number));
    }
}

//sets up the test (here, does nothing)
void
test_case::set_up(void)
{
}

//tears down the test (here, does nothing)
void
test_case::tear_down(void)
{
}

//the test itself; here, does nothing
void
test_case::test(void)
{
}

//construction
test_case::test_case(void)
{
    _cmdLinePtr = NULL;
    m_last_result.delete_contents();
    m_subtests.clear();
}

//destruction
test_case::~test_case(void)
{
    m_last_result.delete_contents();

    _VECTOR(test_case*, unit_test_allocator)::iterator iter = m_subtests.begin();

    for (;iter != m_subtests.end();
            ++iter)
    {
        delete((*iter));
        (*iter) = NULL;
    }
    m_subtests.clear();
}

//the last result after testing
const test_result&
test_case::last_result(void) const
{
    return m_last_result;
}

