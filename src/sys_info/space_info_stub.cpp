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

#include "space_info.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"
#include "src/volume/volume_manager.h"

namespace ibofos
{
bool
SpaceInfo::IsEnough(uint64_t size)
{
    return Remaining() > size;
}

uint64_t
SpaceInfo::OPSize()
{
    uint64_t capa = TotalCapacity();
    return (uint64_t)(capa * 10 / 100);
}

uint64_t
SpaceInfo::TotalCapacity()
{
    return (uint64_t)1024 * (uint64_t)SZ_1GB;
}

uint64_t
SpaceInfo::SystemCapacity()
{
    uint64_t total = TotalCapacity();
    uint64_t op = OPSize();
    IBOF_TRACE_INFO(9000, "SystemCapacity: total:{} - op:{} = sys:{} ",
        total, op, total - op);
    return total - op;
}

uint64_t
SpaceInfo::Used()
{
    return VolumeManagerSingleton::Instance()->EntireVolumeSize();
}

uint64_t
SpaceInfo::Remaining()
{
    return SystemCapacity() - Used();
}

} // namespace ibofos
