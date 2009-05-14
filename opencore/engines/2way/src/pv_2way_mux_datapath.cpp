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
#include "pv_2way_mux_datapath.h"

CPV2WayMuxDatapath *CPV2WayMuxDatapath::NewL(PVLogger *aLogger,
        PVMFFormatType aFormat,
        CPV324m2Way *a2Way)
{
    CPV2WayMuxDatapath *self = OSCL_NEW(CPV2WayMuxDatapath, (aLogger, aFormat, a2Way));
    OsclError::LeaveIfNull(self);

    if (self)
    {
        OSCL_TRAPSTACK_PUSH(self);
        self->ConstructL();
    }

    OSCL_TRAPSTACK_POP();
    return self;
}

void CPV2WayMuxDatapath::OpenComplete()
{
    i2Way->CheckConnect();
}

void CPV2WayMuxDatapath::PauseComplete()
{
    //Mux cannot pause
}

void CPV2WayMuxDatapath::ResumeComplete()
{
    //Mux cannot resume
}

void CPV2WayMuxDatapath::CloseComplete()
{
    i2Way->iIsStackConnected = false;
    i2Way->CheckDisconnect();
}

void CPV2WayMuxDatapath::DatapathError()
{
    i2Way->SetState(EDisconnecting);
    Close();
}





