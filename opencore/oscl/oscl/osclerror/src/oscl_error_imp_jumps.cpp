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

//Implementation file for OsclJump class,
//for use with the setjmp/longjmp error handler.

#include "oscl_error_imp.h"

#ifdef PVERROR_IMP_JUMPS

#include "oscl_assert.h"


////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF void OsclJump::StaticJump(int jmpcode)
//jump to latest jumpmark
{
    int32 error;
    OsclErrorTrapImp *trap = OsclErrorTrapImp::GetErrorTrap(error);
    if (!trap)
    {
        //Note: you can't leave here, since leave would
        //invoke this routine again.  It is not safe to return
        //either, because calling code is expecting an execution
        //end.
        OSCL_ASSERT(false);
        _OSCL_Abort();
    }
    else
    {
        trap->iJumpData->Jump(jmpcode);
    }
}


#endif




