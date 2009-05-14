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

#include "layer.h"

OSCL_EXPORT_REF Layer::Layer() : bound_layer(NULL)
{
}

OSCL_EXPORT_REF OsclAny Layer::Bind(Layer* layer)
{
    PacketOutput *pOutput;

    if (!layer) return;

    //Unbind if already bound.
    if (IsBound()) UnBind();
    if (layer->IsBound()) layer->UnBind();

    //Set this layer's output.
    pOutput = GetPacketOutput();
    if (pOutput) pOutput->SetPacketOutput(layer->GetPacketInput());

    //Set other incoming layer's output.
    pOutput = layer->GetPacketOutput();
    if (pOutput) pOutput->SetPacketOutput(GetPacketInput());

    layer->SetBoundLayer(this);
    SetBoundLayer(layer);
    return;
}

OSCL_EXPORT_REF OsclAny Layer::UnBind()
{
    PacketOutput *pOutput;
    if (bound_layer)
    {
        pOutput = GetPacketOutput();
        if (pOutput) pOutput->ResetPacketOutput();

        pOutput = bound_layer->GetPacketOutput();
        if (pOutput) pOutput->ResetPacketOutput();

        bound_layer->SetBoundLayer(NULL);
        SetBoundLayer(NULL);
    }
}

inline Layer* Layer::GetBoundLayer()
{
    return bound_layer;
}

OSCL_EXPORT_REF bool Layer::IsBound()
{
    return (bound_layer != NULL);
}
