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
/*
 * File:	h223.h
 * Comment:	H223 internal structure, global variable, and function declarations
 */

#ifndef _h223_H
#define _h223_H

#include "oscl_types.h"
#include "pvt_common.h"

#ifdef __cplusplus
extern "C"
{
#endif
    uint H223GetMuxStuffingSz(TPVH223Level level);
    uint H223GetMaxStuffingSz(TPVH223Level level);
    uint H223MuxStuffing(TPVH223Level level, uint8* pPdu, uint bufSz);
    bool H223CheckSync(TPVH223Level level, uint8* buf, uint bufSz, uint tolerance);
#ifdef __cplusplus
}
#endif
#endif	/* _h223_H */
