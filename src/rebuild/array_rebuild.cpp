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

#include "array_rebuild.h"

#include <functional>

#include "src/rebuild/partition_rebuild.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ArrayRebuild::ArrayRebuild(string array, ArrayDevice* dev, RebuildComplete cb,
    list<RebuildTarget*> tgt)
: arrayName(array),
  targetDev(dev),
  rebuildComplete(cb)
{
    progress = new RebuildProgress(arrayName);
    uint64_t totalStripes = 0;
    for (RebuildTarget* tar : tgt)
    {
        PartitionRebuild* ptnRbd = new PartitionRebuild(tar, dev, progress);
        uint64_t stripes = ptnRbd->TotalStripes();
        if (stripes > 0)
        {
            tasks.push_back(ptnRbd);
            totalStripes += stripes;
            POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
                "partition added to array {} for rebuild", arrayName);
        }
        else
        {
            delete ptnRbd;
        }
    }
    progress->SetTotal(totalStripes);
    rebuildDoneCb = bind(&ArrayRebuild::_RebuildDone, this, placeholders::_1);
}

void
ArrayRebuild::Start(void)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "ArrayRebuild::Start() array {} with total {} tasks", arrayName, tasks.size());
    if (tasks.empty())
    {
        RebuildResult res;
        res.array = arrayName;
        res.target = targetDev;
        res.result = RebuildState::READY;
        _RebuildCompleted(res);
    }
    else
    {
        _RebuildNext();
    }
}

void
ArrayRebuild::Discard(void)
{
    POS_TRACE_ERROR((int)POS_EVENT_ID::REBUILD_FAILED,
        "Array {} Rebuild discarded. Cannot start rebuild due to failure in preparation process",
        arrayName);
    tasks.clear();
    RebuildResult res;
    res.array = arrayName;
    res.target = targetDev;
    res.result = RebuildState::FAIL;
    _RebuildCompleted(res);
}

void
ArrayRebuild::Stop(void)
{
    for (PartitionRebuild* task : tasks)
    {
        task->Stop();
    }

    _WaitRebuildStop();
}

RebuildState
ArrayRebuild::GetState(void)
{
    return state;
}

uint64_t
ArrayRebuild::GetProgress(void)
{
    if (progress != nullptr)
    {
        return progress->Current();
    }
    return 0;
}

void
ArrayRebuild::_RebuildNext(void)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "ArrayRebuild::_RebuildNext() array {} has remaining {} tasks", arrayName, tasks.size());
    if (tasks.empty() == false)
    {
        PartitionRebuild* task = tasks.front();
        state = RebuildState::REBUILDING;
        task->Start(rebuildDoneCb);
    }
}

void
ArrayRebuild::_RebuildDone(RebuildResult res)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "ArrayRebuild::_RebuildDone array {} rebuild done with result {} ", arrayName, res.result);
    RebuildState taskResult = res.result;
    state = taskResult;

    mtx.lock();
    PartitionRebuild* task = tasks.front();
    delete task;
    tasks.pop_front();
    if (taskResult == RebuildState::CANCELLED ||
        taskResult == RebuildState::FAIL)
    {
        while (tasks.empty() == false)
        {
            PartitionRebuild* t = tasks.front();
            delete t;
            tasks.pop_front();
        }
    }
    mtx.unlock();

    if (tasks.empty() == true)
    {
        _RebuildCompleted(res);
    }
    else
    {
        _RebuildNext();
    }
}

void
ArrayRebuild::_RebuildCompleted(RebuildResult res)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::REBUILD_DEBUG_MSG,
        "ArrayRebuild::_RebuildCompleted array {} rebuild completed with result {} ", arrayName, state);
    switch (state)
    {
        case RebuildState::PASS:
            POS_TRACE_INFO((int)POS_EVENT_ID::REBUILD_RESULT_PASS,
                "array {} rebuild completed sucessfully", arrayName);
            break;
        case RebuildState::FAIL:
            POS_TRACE_ERROR((int)POS_EVENT_ID::REBUILD_FAILED,
                "array {} rebuild failure", arrayName);
            break;
        case RebuildState::CANCELLED:
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_RESULT_CANCELLED,
                "array {} rebuild cancelled", arrayName);
            break;
        default:
            POS_TRACE_ERROR((int)POS_EVENT_ID::REBUILD_FAILED,
                "array {} unhandled rebuild result", arrayName);
            assert(false);
            break;
    }

    rebuildComplete(res);
    cv.notify_one();
}

void
ArrayRebuild::_WaitRebuildStop(void)
{
    std::unique_lock<std::mutex> lock(mtx);
    while (tasks.empty() != true)
    {
        cv.wait(lock);
    }
}
} // namespace pos