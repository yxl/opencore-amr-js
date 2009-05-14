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
#ifndef POINT_H
#define POINT_H

#ifndef STRINGABLE_H
#include "stringable.h"
#endif
#include <stdio.h>

//@doc


//@class a simple 2-d point class.  This is used for graphics placement, etc.
template < class T > class point : public stringable
{
        //@access public data members
    public:
        typedef point < T > t_point;
        //@cmember the x coordinate of the point
        T x;
        //@cmember the y coordinate of the point
        T y;
        //@access public methods
    public:
        //@cmember default constructor
        point(void): x(0), y(0)
        {
        };
        //@cmember copy constructor
        point(const t_point & rhs): x(rhs.x), y(rhs.y)
        {
        };
        //@cmember specific constructor
        point(const T & new_x, const T & new_y): x(new_x), y(new_y)
        {
        };
        //@cmember equality comparison operator
        bool operator == (const t_point & rhs) const
        {
            return ((x == rhs.x) && (y == rhs.y));
        };
        //@cmember inequality comparison operator
        bool operator != (const t_point & rhs) const
        {
            return !((*this) == rhs);
        };
        //@cmember adds a point to this point
        t_point & operator += (const t_point & rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return (*this);
        };
        //@cmember subtracts a point from this point
        t_point & operator -= (const t_point & rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return (*this);
        };
        //@cmember adds two points
        t_point operator + (const t_point & rhs) const
        {
            return t_point(x + rhs.x, y + rhs.y);
        };
        //@cmember subtracts two points
        t_point operator - (const t_point & rhs) const
        {
            return t_point(x - rhs.x, y - rhs.y);
        };
        //@cmember equals operator
        t_point & operator = (const t_point & rhs)
        {
            x = rhs.x; //should be x;
            y = rhs.y;
            return (*this);
        };
        //@cmember returns a string representing the point -- note: uses sprintf(); only works correctly for int points
        //just here to demonstrate to_string
        virtual std::string to_string(void) const
        {
            std::string Result = "(";
            char temp[100];
            sprintf(temp, "%d", x);
            Result.append(temp);
            Result.append(", ");
            sprintf(temp, "%d", y);
            Result.append(temp);
            Result.append(")");
            return Result;
        };
};

//@type int_point | commonly used element: a <c point> made up of ints
typedef point < int >int_point;


#endif

