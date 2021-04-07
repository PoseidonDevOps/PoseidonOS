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

#include "array_device_manager.h"

namespace ibofos
{
ArrayDeviceManager::ArrayDeviceManager(DeviceManager* sysDevMgr)
: sysDevMgr_(sysDevMgr)
{
}

ArrayDeviceManager::~ArrayDeviceManager()
{
}

int
ArrayDeviceManager::Import(DeviceSet<string> nameSet)
{
    return 0;
}

int
ArrayDeviceManager::Import(DeviceSet<DeviceMeta> metaSet,
    uint32_t& missingCnt,
    uint32_t& brokenCnt)
{
    return 0;
}

int
ArrayDeviceManager::AddSpare(string devName)
{
    return 0;
}

void
ArrayDeviceManager::Clear()
{
}

DeviceSet<DeviceMeta>
ArrayDeviceManager::ExportToMeta(void)
{
    DeviceSet<DeviceMeta> metaSet;
    return metaSet;
}

DeviceSet<string>
ArrayDeviceManager::ExportToName(void)
{
    return DeviceSet<string>();
}

int
ArrayDeviceManager::RemoveSpare(string devName)
{
    return 0;
}

int
ArrayDeviceManager::ReplaceWithSpare(ArrayDevice* target)
{
    return 0;
}

ArrayDevice*
ArrayDeviceManager::GetFaulty(void)
{
    return nullptr;
}

ArrayDevice*
ArrayDeviceManager::GetRebuilding(void)
{
    return nullptr;
}

int
ArrayDeviceManager::_CheckConstraints(ArrayDeviceList* devs)
{
    return 0;
}

int
ArrayDeviceManager::_CheckDevsCount(ArrayDeviceSet devSet)
{
    return 0;
}

int
ArrayDeviceManager::_CheckFaultTolerance(DeviceSet<ArrayDevice*> devSet)
{
    return 0;
}

int
ArrayDeviceManager::_CheckNvmCapacity(const ArrayDeviceSet& devSet)
{
    return 0;
}

uint64_t
ArrayDeviceManager::_ComputeMinNvmCapacity(const uint32_t logicalChunkCount)
{
    return 0;
}

int
ArrayDeviceManager::_CheckSsdsCapacity(const ArrayDeviceSet& devSet)
{
    return 0;
}

tuple<ArrayDevice*, ArrayDeviceType>
ArrayDeviceManager::GetDev(UBlockDevice* uBlock)
{
    return make_tuple(nullptr, ArrayDeviceType::NONE);
}

ArrayDevice*
ArrayDeviceManager::_GetBaseline(const vector<ArrayDevice*>& devs)
{
    return nullptr;
}

} // namespace ibofos
