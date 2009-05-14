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
#ifndef TEST_CASE_H
#define TEST_CASE_H

#ifndef TEST_RESULT_H
#include "test_result.h"
#endif
#ifndef STRINGABLE_H
#include "stringable.h"
#endif

#include "unit_test_args.h"

// $Id
// A composite test case; its atomic test can be redefined by overriding
// test(); it can also serve as a suite of tests by adding test_cases
// to it via the add_test_case function.  It functions as a composite
// object (see the composite pattern in Design Patterns, p 163) without
// methods for getting individual children.

class test_case
{
    protected:
        //private methods and data
        //the result of the last test
        test_result m_last_result;

        cmd_line* _cmdLinePtr;

        //the collection of subtests
        _VECTOR(test_case*, unit_test_allocator) m_subtests;
        //runs the subtests
        void run_subtests(void);

    public:
        //construction/destruction
        test_case(void);

        virtual ~test_case(void);

        void SetCommandLine(cmd_line* cmd_line)
        {
            _cmdLinePtr = cmd_line;
        };

        cmd_line* getCommandLine()
        {
            return (_cmdLinePtr);
        };

        //composition methods
        //adds a child to the test case
        void adopt_test_case(test_case* new_case);
        //number of child test cases
        int subtest_count(void) const;

        //testing methods
        //runs the test.  This is a template method (Design Patterns 325)
        void run_test(void);
        //tests to see if the supplied condition is true; called by the test_is_true macro
        void test_is_true_stub(bool condition,
                               const _STRING& condition_name,
                               const _STRING& filename,
                               long line_number);
        //tests to see if two longs are equal
        void test_int_is_equal_stub(long actual,
                                    long expected,
                                    const _STRING& filename,
                                    long line_number);
        //tests to see if two doubles are within limits
        void test_double_is_equal_stub(double actual,
                                       double expected,
                                       const _STRING& filename,
                                       long line_number,
                                       double tolerance = 0.005);
        //tests to see if two strings are equal
        void test_string_is_equal_stub(const _STRING& actual,
                                       const _STRING& expected,
                                       const _STRING& filename,
                                       long line_number);
        //creates an "equality test failed" problem report
        test_problem create_equality_problem_report(const _STRING& actual_message,
                const _STRING& expected_message,
                const _STRING& filename,
                long line_number);


        template< class T >void test_is_equal_stub(const T& actual,
                const T& expected,
                const _STRING& filename,
                long line_number)
        {
            if (actual == expected)
            {
                m_last_result.add_success();
            }
            else
            {
                _STRING actual_message = valueToString(actual);
                _STRING expected_message = valueToString(expected);

                m_last_result.add_failure(create_equality_problem_report(actual_message,
                                          expected_message,
                                          filename,
                                          line_number));
            }
        }
        //the last result
        const test_result& last_result(void) const;

        //overrideables
        //sets up the test
        virtual void set_up(void);
        //tears down after the test, and cleans up
        virtual void tear_down(void);
        //the test itself.  It should either use the test_is_true/test_is_equal methods or
        //append to the result itself
        virtual void test(void);
};


// A version of test case that does not do any native new/delete calls.
// This can be used to avoid invoking Oscl memory manager
// when the global new/delete overload is used.
// It's necessary when testing low-level Oscl features, but
// can be used anytime it's undesirable to have the test case
// allocated through Oscl memory manager.
class test_case_LL: public test_case, public UnitTest_HeapBase
{
};


//these macros are used instead of the code stubs in test_case
//in order to use preprocessor features to get filename/line number
#define test_is_true( condition ) (this->test_is_true_stub( (condition), (#condition), __FILE__, __LINE__ ))
#define test_int_is_equal( actual_value, expected_value ) (this->test_int_is_equal_stub( actual_value, expected_value, __FILE__, __LINE__ ))
#define test_double_is_equal( actual_value, expected_value ) (this->test_double_is_equal_stub( actual_value, expected_value, __FILE__, __LINE__ ))
#define double_is_within_tolerance( actual_value, expected_value, tolerance ) ( this->test_double_is_equal_stub( actual_value, expected_value, __FILE__, __LINE__, tolerance ))
#define test_string_is_equal( actual_value, expected_value ) (this->test_string_is_equal_stub( actual_value, expected_value, __FILE__, __LINE__ ))
#define test_is_equal( actual_value, expected_value ) (this->test_is_equal_stub( actual_value, expected_value, __FILE__, __LINE__ ))

#endif


