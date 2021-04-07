/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "rebuild_handler.h"

#include "src/array/array.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
RebuildHandler::RebuildHandler(bool isStop, ArrayDevice* target)
: isStop(isStop),
  target(target)
{
}

bool
RebuildHandler::Execute(void)
{
    Array* sysArray = ArraySingleton::Instance();
    if (isStop == true)
    {
        IBOF_TRACE_DEBUG(IBOF_EVENT_ID::REBUILD_DEBUG_MSG, "RebuildHandler::TriggerStop");
        thread t([sysArray]() {
            sysArray->StopRebuilding();
            bool stop = false;
            EventSmartPtr event(new RebuildHandler(stop));
            EventScheduler* scheduler = EventArgument::GetEventScheduler();
            scheduler->EnqueueEvent(event);
        });
        t.detach();
    }
    else
    {
        IBOF_TRACE_DEBUG(IBOF_EVENT_ID::REBUILD_DEBUG_MSG, "RebuildHandler::TriggerStart");
        bool retry = sysArray->TriggerRebuild(target);
        if (retry)
        {
            return false;
        }
    }

    return true;
}

} // namespace ibofos
