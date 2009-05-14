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

#include "oscl_base.h"
#include "oscl_registry_client_impl.h"

#include "oscl_error_codes.h"

#include "osclconfig_proc.h"
#if (OSCL_HAS_SINGLETON_SUPPORT)
//1st choice implementation-- uses Oscl Singleton

//nothing needed.  client goes directly to server impl.

#else //OSCL_HAS_ ...
//3rd choice implementation

//non-functional stubs are implemented in the header-- nothing needed here.

#endif //oscl config





