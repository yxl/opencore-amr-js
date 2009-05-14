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
#if !defined(CPVCONTROLS_H)
#define CPVCONTROLS_H
#include "oscl_types.h"
#include "pvt_params.h"

class CPVControlsObserver
{
    public:
        virtual void HandleControlMessage(PS_ControlMsgHeader msg) = 0;
};

class CPVControls
{
    public:
        CPVControls() {}
        virtual ~CPVControls() {}
        virtual void DispatchControlMessage(PS_ControlMsgHeader msg) = 0;
        virtual void SetControlsObserver(CPVControlsObserver* observer) = 0;
    protected:
        //CPVControlsObserver* iObserver;
};
#endif
