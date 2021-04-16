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

#include "poverty_cpu_set_generator.h"

#include <numa.h>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
PovertyCpuSetGenerator::PovertyCpuSetGenerator(
    const CoreDescriptionArray& coreDescriptions)
{
    if (unlikely(TOTAL_CORE_COUNT < MIN_POVERTY_CORE_COUNT))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::AFTMGR_CPU_COUNT_NOT_ENOUGH;
        POS_TRACE_CRITICAL(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        exit(-1);
    }

    uint32_t startCore = 0;

    for (auto& iter : coreDescriptions)
    {
        CoreType type = iter.type;
        uint32_t nextCore = 0;

        _AddCoreSet(startCore, 1, &nextCore, type);

        if (type == CoreType::REACTOR)
        {
            startCore = nextCore;
        }
    }
}

PovertyCpuSetGenerator::~PovertyCpuSetGenerator(void)
{
}

} // namespace pos