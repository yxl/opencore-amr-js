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
#ifndef TEST_POINT_H
#define TEST_POINT_H

#ifndef TEST_CASE_H
#include "test_case.h"
#endif
#ifndef POINT_H
#include "point.h"
#endif

class point_test : public test_case
{
    protected:
        int_point top_left_point;
        int_point bottom_right_point;
    public:
        virtual void set_up(void);
};

class point_test_suite : public test_case
{
    public:
        point_test_suite(void);
        point_test_suite(char**, int);

};

#endif
