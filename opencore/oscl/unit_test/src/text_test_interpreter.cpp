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
#include "text_test_interpreter.h"


_STRING _yak_itoa(int input)
{
    char buffer[32];
    sprintf(buffer, "%d", input);
    return buffer;
}

//the header for the report
_STRING
text_test_interpreter::header(const test_result& result) const
{
    _STRING Result("\nTest output begins\nNumber of tests: ");
    _APPEND(Result, _yak_itoa(result.total_test_count()));
    _APPEND(Result, "\n");
    return Result;

}

_STRING
text_test_interpreter::footer(const test_result& result) const
{
    result.total_test_count();
    _STRING Result("Test output ends\n");
    return Result;
}

_STRING
text_test_interpreter::successes(const test_result& result) const
{
    _STRING Result("Successes: ");
    _APPEND(Result, _yak_itoa(result.success_count()));
    _APPEND(Result, "\n");
    return Result;
}

_STRING
text_test_interpreter::failures(const test_result& result) const
{
    _STRING Result("Failures: ");
    _APPEND(Result, _yak_itoa(result.failures().size()));
    _APPEND(Result, "\n");
    _APPEND(Result, problem_vector_string(result.failures()));
    _APPEND(Result, "\n");
    return Result;
}

_STRING
text_test_interpreter::errors(const test_result& result) const
{
    _STRING Result("Errors: ");
    _APPEND(Result, _yak_itoa(result.errors().size()));
    _APPEND(Result, "\n");
    _APPEND(Result, problem_vector_string(result.errors()));
    _APPEND(Result, "\n");
    return Result;
}

_STRING
text_test_interpreter::problem_vector_string(const _VECTOR(test_problem, unit_test_allocator)&
        vect) const
{
    _STRING Result;

    for (_VECTOR(test_problem, unit_test_allocator)::const_iterator iter = vect.begin();
            iter != vect.end();
            ++iter)
    {
        _APPEND(Result, problem_string(*iter));
    }
    return Result;
}

_STRING
text_test_interpreter::problem_string(const test_problem& problem) const
{
    _STRING Result;
    _APPEND(Result, problem.filename());
    _APPEND(Result, ":");
    _APPEND(Result, _yak_itoa(problem.line_number()));
    _APPEND(Result, ":");
    _APPEND(Result, problem.message());
    _APPEND(Result, "\n");
    return Result;
}

_STRING
text_test_interpreter::interpretation(const test_result& result_to_interpret) const
{
    _STRING Result;
    _APPEND(Result, header(result_to_interpret));
    _APPEND(Result, successes(result_to_interpret));
    _APPEND(Result, failures(result_to_interpret));
    _APPEND(Result, errors(result_to_interpret));
    _APPEND(Result, footer(result_to_interpret));
    return Result;
}

