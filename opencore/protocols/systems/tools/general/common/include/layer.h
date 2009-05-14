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
/************************************************************************/
/*  file name       : layer.h                                           */
/*  file contents   :											        */
/*  draw            :                                                   */
/*----------------------------------------------------------------------*/
/*  amendment       :                                                   */
/*      Copyright (C) 2002 PacketVideo                                  */
/************************************************************************/

#if !defined(LAYER_H)
#define LAYER_H

#include "packet_io.h"

/* The Layer interface represents an upper or lower layer of an element in the stack.
It aggregates one PacketInput and one PacketOutput
*/
class Layer
{
    public:
        enum
        {
            PACKET_INPUT = 0,
            PACKET_OUTPUT
        }TLayerNode;
        virtual ~Layer() {}
        OSCL_IMPORT_REF virtual PacketInput* GetPacketInput() = 0;
        OSCL_IMPORT_REF virtual PacketOutput* GetPacketOutput() = 0;
        OSCL_IMPORT_REF OsclAny SetBoundLayer(Layer* layer)
        {
            bound_layer = layer;
        }
        OSCL_IMPORT_REF virtual OsclAny Bind(Layer* layer);
        OSCL_IMPORT_REF virtual OsclAny UnBind();
        OSCL_IMPORT_REF virtual bool IsBound();
    protected:
        OSCL_IMPORT_REF Layer();
        Layer* GetBoundLayer();
        Layer* bound_layer;
};

/*
A SimpleStackElement has one upper layer and one lower layer
*/
class SimpleStackElement
{
    public:
        virtual ~SimpleStackElement() {}
        OSCL_IMPORT_REF virtual Layer* GetUpperLayer() = 0;
        OSCL_IMPORT_REF virtual Layer* GetLowerLayer() = 0;
        /*OSCL_IMPORT_REF virtual int Start(){ return 0;}
        OSCL_IMPORT_REF virtual int Stop(){ return 0;}*/

};

#endif /* LAYER_H */
