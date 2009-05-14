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
#include "test_case_point.h"



class default_constructor_test : public point_test
{
        virtual void test(void)
        {
            int_point test_point;
            test_is_equal(test_point, int_point(0, 0));
        }
};

class specific_constructor_test : public point_test
{
        virtual void test(void)
        {
            test_is_equal(top_left_point, int_point(1, 2));
        }
};

class copy_constructor_test : public point_test
{
        virtual void test(void)
        {
            int_point test_point(top_left_point);
            test_is_equal(test_point, int_point(1, 2));
        }
};

class equality_operator_test : public point_test
{
        virtual void test(void)
        {
            int_point test_equal_point(1, 2);
            test_is_true(top_left_point == top_left_point);
            test_is_true(top_left_point == test_equal_point);
        }
};

class inequality_operator_test : public point_test
{
        virtual void test(void)
        {
            int_point first_equal_point(1, 9);
            int_point second_equal_point(9, 2);
            test_is_true(top_left_point != first_equal_point);
            test_is_true(top_left_point != second_equal_point);
            test_is_true(top_left_point != bottom_right_point);
        }
};

class self_addition_test : public point_test
{
        virtual void test(void)
        {
            int_point test_point(10, 20);
            test_point += top_left_point;
            test_is_equal(test_point, int_point(11, 22));
        }
};

class self_subtraction_test : public point_test
{
        virtual void test(void)
        {
            int_point test_point(10, 20);
            test_point -= top_left_point;
            test_is_equal(test_point, int_point(9, 18));
        }
};

class addition_test : public point_test
{
        virtual void test(void)
        {
            int_point test_point = top_left_point + bottom_right_point;
            test_is_equal(test_point, int_point(4, 6));
        }
};

class subtraction_test : public point_test
{
        virtual void test(void)
        {
            int_point test_point = top_left_point - bottom_right_point;
            test_is_equal(test_point, int_point(-2, -2));
        }
};

class copy_operator_test : public point_test
{
        virtual void test(void)
        {
            int_point test_point;
            test_point = top_left_point;
            test_is_equal(test_point, top_left_point);
        }
};

class cmd_args_test : public point_test
{
    public:
        cmd_args_test(char** args_list, int args_count)
        {
            SetArgs(args_list);
            SetArgsCount(args_count);
        }
        virtual void test(void)
        {
            int i;
            for (i = 0; i < iArgsCount; i++)
            {
                printf("Input Argument %d is %s \r\n", i, iInputArgs[i]);
            }
        }
};


point_test_suite::point_test_suite(char** args_list, int args_count) :
        test_case()
{
    SetArgs(args_list);
    SetArgsCount(args_count);

    adopt_test_case(new default_constructor_test);
    adopt_test_case(new specific_constructor_test);
    adopt_test_case(new copy_constructor_test);
    adopt_test_case(new equality_operator_test);
    adopt_test_case(new inequality_operator_test);
    adopt_test_case(new self_addition_test);
    adopt_test_case(new self_subtraction_test);
    adopt_test_case(new addition_test);
    adopt_test_case(new subtraction_test);
    adopt_test_case(new copy_operator_test);
    adopt_test_case(new cmd_args_test(GetArgs(), GetArgsCount()));
}

void
point_test::set_up(void)
{
    top_left_point = int_point(1, 2);
    bottom_right_point = int_point(3, 4);
}
