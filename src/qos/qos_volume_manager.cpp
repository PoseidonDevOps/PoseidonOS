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
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
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

#include "src/qos/qos_volume_manager.h"

#include "src/include/array_mgmt_policy.h"
#include "src/include/pos_event_id.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/aio_submission_adapter.h"
#include "src/logger/logger.h"
#include "src/qos/io_queue.h"
#include "src/qos/parameter_queue.h"
#include "src/qos/qos_manager.h"
#include "src/qos/rate_limit.h"
#include "src/qos/submission_adapter.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/event_scheduler/event_scheduler.h"

namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumeManager::QosVolumeManager(QosContext* qosCtx, bool feQos, uint32_t arrayIndex,
    QosArrayManager* qosArrayMgr, SpdkPosNvmfCaller* spdkPosNvmfCaller,
    SpdkPosVolumeCaller* spdkPosVolumeCaller)
: VolumeEvent("QosManager", "", arrayIndex),
  feQosEnabled(feQos),
  qosContext(qosCtx),
  spdkPosNvmfCaller(spdkPosNvmfCaller),
  spdkPosVolumeCaller(spdkPosVolumeCaller)
{
    arrayId = arrayIndex;
    qosArrayManager = qosArrayMgr;
    volumeOperationDone = false;
    eventFrameworkApi = EventFrameworkApiSingleton::Instance();
    for (uint32_t reactor = 0; reactor < M_MAX_REACTORS; reactor++)
    {
        for (uint32_t volId = 0; volId < MAX_VOLUME_COUNT; volId++)
        {
            SetVolumeLimit(reactor, volId, DEFAULT_MAX_BW_IOPS, false);
            SetVolumeLimit(reactor, volId, DEFAULT_MAX_BW_IOPS, true);
            pendingIO[reactor][volId] = 0;
        }
    }
    VolumeEventPublisherSingleton::Instance()->RegisterSubscriber(this, "", arrayId);
    try
    {
        bwIopsRateLimit = new BwIopsRateLimit;
        parameterQueue = new ParameterQueue;
        ioQueue = new IoQueue<pos_io*>;
    }
    catch (std::bad_alloc& ex)
    {
        assert(0);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
QosVolumeManager::~QosVolumeManager(void)
{
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, "", arrayId);
    delete bwIopsRateLimit;
    delete parameterQueue;
    delete ioQueue;
    if (spdkPosNvmfCaller != nullptr)
    {
        delete spdkPosNvmfCaller;
    }
    if (spdkPosVolumeCaller != nullptr)
    {
        delete spdkPosVolumeCaller;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::UpdateSubsystemToVolumeMap(uint32_t nqnId, uint32_t volId)
{
    if (std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volId) != nqnVolumeMap[nqnId].end())
    {
        return;
    }
    nqnVolumeMap[nqnId].push_back(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::DeleteVolumeFromSubsystemMap(uint32_t nqnId, uint32_t volId)
{
    std::vector<int>::iterator position = std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volId);
    if (position != nqnVolumeMap[nqnId].end())
    {
        nqnVolumeMap[nqnId].erase(position);
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/

std::vector<int>
QosVolumeManager::GetVolumeFromActiveSubsystem(uint32_t nqnId)
{
    return nqnVolumeMap[nqnId];
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::_RateLimit(uint32_t reactor, int volId)
{
    return bwIopsRateLimit->IsLimitExceeded(reactor, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_UpdateRateLimit(uint32_t reactor, int volId, uint64_t size)
{
    bwIopsRateLimit->UpdateRateLimit(reactor, volId, size);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::HandlePosIoSubmission(IbofIoSubmissionAdapter* aioSubmission, pos_io* volIo)
{
    if (false == feQosEnabled)
    {
        return;
    }
    uint32_t reactorId = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
    uint32_t volId = volIo->volume_id;
    uint64_t currentBw = 0;
    uint64_t currentIO = 0;
    uint32_t blockSize = 0;

    currentBw = volumeQosParam[reactorId][volId].currentBW;
    currentIO = volumeQosParam[reactorId][volId].currentIOs;
    blockSize  = volIo->length;

    if ((pendingIO[reactorId][volId] == 0) && (_RateLimit(reactorId, volId) == false))
    {
        currentBw = currentBw + volIo->length;
        currentIO++;
        aioSubmission->Do(volIo);
        _UpdateRateLimit(reactorId, volId, volIo->length);
    }
    else
    {
        pendingIO[reactorId][volId]++;
        _EnqueueVolumeUbio(reactorId, volId, volIo);
        while (!IsExitQosSet())
        {
            if (_RateLimit(reactorId, volId) == true)
            {
                break;
            }
            pos_io* queuedVolumeIo = nullptr;
            queuedVolumeIo = _DequeueVolumeUbio(reactorId, volId);
            if (queuedVolumeIo == nullptr)
            {
                break;
            }
            currentBw = currentBw + queuedVolumeIo->length;
            currentIO++;
            pendingIO[reactorId][volId]--;
            aioSubmission->Do(queuedVolumeIo);
            _UpdateRateLimit(reactorId, volId, queuedVolumeIo->length);
        }
    }
    volumeQosParam[reactorId][volId].currentBW = currentBw;
    volumeQosParam[reactorId][volId].currentIOs = currentIO;
    volumeQosParam[reactorId][volId].blockSize = blockSize;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_EnqueueParams(uint32_t reactor, uint32_t volId, bw_iops_parameter& volume_param)
{
    parameterQueue->EnqueueParameter(reactor, volId, volume_param);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bw_iops_parameter
QosVolumeManager::DequeueParams(uint32_t reactor, uint32_t volId)
{
    return parameterQueue->DequeueParameter(reactor, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_ClearVolumeParameters(uint32_t volId)
{
    parameterQueue->ClearParameters(volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_EnqueueVolumeUbio(uint32_t reactorId, uint32_t volId, pos_io* io)
{
    ioQueue->EnqueueIo(reactorId, volId, io);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
pos_io*
QosVolumeManager::_DequeueVolumeUbio(uint32_t reactorId, uint32_t volId)
{
    return ioQueue->DequeueIo(reactorId, volId);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_UpdateVolumeMaxQos(int volId, uint64_t maxiops, uint64_t maxbw, std::string arrayName)
{
    qos_vol_policy volumePolicy;
    volumePolicy.maxBw = maxbw;
    // update max iops here to display for qos list
    volumePolicy.maxIops = maxiops;
    volumePolicy.policyChange = true;
    volumePolicy.maxValueChanged = true;
    qosArrayManager->UpdateVolumePolicy(volId, volumePolicy);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    _UpdateVolumeMaxQos(volEventBase->volId, volEventPerf->maxiops, volEventPerf->maxbw, volArrayInfo->arrayName);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    qos_vol_policy volumePolicy;
    volumePolicy.policyChange = true;
    qosArrayManager->UpdateVolumePolicy(volEventBase->volId, volumePolicy);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    struct pos_volume_info *vInfo = new (struct pos_volume_info);
    _CopyVolumeInfo(vInfo->array_name, (volArrayInfo->arrayName).c_str(), (volArrayInfo->arrayName).size());
    vInfo->id = volEventBase->volId;
    vInfo->iops_limit = volEventPerf->maxiops;
    vInfo->bw_limit = volEventPerf->maxbw;
    _SetVolumeOperationDone(false);

    // enqueue in same reactor as NvmfVolumePos::VolumeMounted so that pos_disk structure is populated with correct values before qos tries to access it.
    eventFrameworkApi->SendSpdkEvent(eventFrameworkApi->GetFirstReactor(),
            _VolumeMountHandler, vInfo, this);

    while (_GetVolumeOperationDone() == false)
    {
        if (_GetVolumeOperationDone() == true)
        {
            break;
        }
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager:: _VolumeMountHandler(void *arg1, void *arg2)
{
    struct pos_volume_info* volInfo = static_cast<pos_volume_info*>(arg1);
    QosVolumeManager *qosVolumeManager = static_cast<QosVolumeManager *>(arg2);
    qosVolumeManager->_InternalVolMountHandlerQos(volInfo);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_InternalVolMountHandlerQos(struct pos_volume_info *volMountInfo)
{
    if (volMountInfo)
    {
        string bdevName = _GetBdevName(volMountInfo->id, volMountInfo->array_name);
        uint32_t nqnId = 0;
        nqnId = spdkPosVolumeCaller->GetAttachedSubsystemId(bdevName.c_str());
        UpdateSubsystemToVolumeMap(nqnId, volMountInfo->id);
        _UpdateVolumeMaxQos(volMountInfo->id, volMountInfo->iops_limit, volMountInfo->bw_limit, volMountInfo->array_name);
        _SetVolumeOperationDone(true);
        delete (volMountInfo);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    if (false == feQosEnabled)
    {
        return (int)POS_EVENT_ID::VOL_EVENT_OK;
    }
    struct pos_volume_info* vInfo = new (struct pos_volume_info);
    _CopyVolumeInfo(vInfo->array_name, (volArrayInfo->arrayName).c_str(), (volArrayInfo->arrayName).size());
    vInfo->id = volEventBase->volId;
    vInfo->iops_limit = 0;
    vInfo->bw_limit = 0;
    _SetVolumeOperationDone(false);

    eventFrameworkApi->SendSpdkEvent(eventFrameworkApi->GetFirstReactor(),
            _VolumeUnmountHandler, vInfo, this);

    while (_GetVolumeOperationDone() == false)
    {
        if (_GetVolumeOperationDone() == true)
        {
            break;
        }
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_VolumeUnmountHandler(void *arg1, void *arg2)
{
    struct pos_volume_info *volUnmountInfo = static_cast <struct pos_volume_info*>(arg1);
    QosVolumeManager *qosVolumeManager = static_cast<QosVolumeManager *>(arg2);
    qosVolumeManager->_InternalVolUnmountHandlerQos(volUnmountInfo);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_InternalVolUnmountHandlerQos(struct pos_volume_info* volUnmountInfo)
{
    if (volUnmountInfo)
    {
        string bdevName = _GetBdevName(volUnmountInfo->id, volUnmountInfo->array_name);
        uint32_t nqnId = 0;
        nqnId = spdkPosVolumeCaller->GetAttachedSubsystemId(bdevName.c_str());
        std::vector<int>::iterator position = std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volUnmountInfo->id);
        if (position != nqnVolumeMap[nqnId].end())
        {
            nqnVolumeMap[nqnId].erase(position);
        }
        _ClearVolumeParameters(volUnmountInfo->id);
        _SetVolumeOperationDone(true);
        delete(volUnmountInfo);
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    _UpdateVolumeMaxQos(volEventBase->volId, volEventPerf->maxiops, volEventPerf->maxbw, volArrayInfo->arrayName);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    qos_vol_policy volumePolicy = qosArrayManager->GetVolumePolicy(volEventBase->volId);
    if ((volumePolicy.maxBw == volEventPerf->maxbw) && (volumePolicy.maxIops == volEventPerf->maxiops))
    {
        return (int)POS_EVENT_ID::VOL_EVENT_OK;
    }
    std::string arrName = GetArrayName();
    if (0 == arrayName.compare(arrName))
    {
        _UpdateVolumeMaxQos(volEventBase->volId, volEventPerf->maxiops, volEventPerf->maxbw, arrayName);
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    for (auto volId : volList)
    {
        struct pos_volume_info* vInfo = new (struct pos_volume_info);
        _CopyVolumeInfo(vInfo->array_name, (volArrayInfo->arrayName).c_str(), (volArrayInfo->arrayName).size());
        vInfo->id = volId;
        vInfo->iops_limit = 0;
        vInfo->bw_limit = 0;
        _SetVolumeOperationDone(false);

        eventFrameworkApi->SendSpdkEvent(eventFrameworkApi->GetFirstReactor(),
            _VolumeDetachHandler, vInfo, this);

        while (_GetVolumeOperationDone() == false)
        {
            if (_GetVolumeOperationDone() == true)
            {
                break;
            }
        }
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_VolumeDetachHandler(void *arg1, void *arg2)
{
    struct pos_volume_info *volDetachInfo = static_cast <struct pos_volume_info*>(arg1);
    QosVolumeManager *qosVolumeManager = static_cast<QosVolumeManager*>(arg2);
    qosVolumeManager->_InternalVolDetachHandlerQos(volDetachInfo);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_InternalVolDetachHandlerQos(struct pos_volume_info* volDetachInfo)
{
    if (volDetachInfo)
    {
        string bdevName = _GetBdevName(volDetachInfo->id, volDetachInfo->array_name);
        uint32_t nqnId = 0;
        nqnId = spdkPosVolumeCaller->GetAttachedSubsystemId(bdevName.c_str());
        std::vector<int>::iterator position = std::find(nqnVolumeMap[nqnId].begin(), nqnVolumeMap[nqnId].end(), volDetachInfo->id);
        if (position != nqnVolumeMap[nqnId].end())
        {
            nqnVolumeMap[nqnId].erase(position);
        }
        _ClearVolumeParameters(volDetachInfo->id);
        _SetVolumeOperationDone(true);
        delete(volDetachInfo);
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::ResetRateLimit(uint32_t reactor, int volId, double offset)
{
    int64_t setBwLimit = GetVolumeLimit(reactor, volId, false);
    int64_t setIopsLimit = GetVolumeLimit(reactor, volId, true);
    bwIopsRateLimit->ResetRateLimit(reactor, volId, offset, setBwLimit, setIopsLimit);
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int
QosVolumeManager::VolumeQosPoller(uint32_t reactor, IbofIoSubmissionAdapter* aioSubmission, double offset)
{
    uint32_t retVal = 0;
    pos_io* queuedVolumeIo = nullptr;
    uint64_t currentBW = 0;
    uint64_t currentIO = 0;
    volList[reactor].clear();
    for (auto it = nqnVolumeMap.begin(); it != nqnVolumeMap.end(); it++)
    {
        uint32_t subsys = it->first;
        if (spdkPosNvmfCaller->SpdkNvmfGetReactorSubsystemMapping(reactor, subsys) != INVALID_SUBSYSTEM)
        {
            volList[reactor][subsys] = GetVolumeFromActiveSubsystem(subsys);
        }
    }
    for (auto subsystem = volList[reactor].begin(); subsystem != volList[reactor].end(); subsystem++)
    {
        std::vector<int> volumeList = volList[reactor][subsystem->first];

        for (uint32_t i = 0; i < volumeList.size(); i++)
        {
            int volId = volumeList[i];
            currentBW = 0;
            currentIO = 0;
            ResetRateLimit(reactor, volId, offset);
            _EnqueueVolumeParameter(reactor, volId, offset);
            while (!IsExitQosSet())
            {
                if (_RateLimit(reactor, volId) == true)
                {
                    break;
                }
                queuedVolumeIo = _DequeueVolumeUbio(reactor, volId);
                if (queuedVolumeIo == nullptr)
                {
                    break;
                }
                currentBW = currentBW + queuedVolumeIo->length;
                currentIO++;
                pendingIO[reactor][volId]--;
                aioSubmission->Do(queuedVolumeIo);
                _UpdateRateLimit(reactor, volId, queuedVolumeIo->length);
            }
            volumeQosParam[reactor][volId].currentBW = currentBW;
            volumeQosParam[reactor][volId].currentIOs = currentIO;
        }
    }
    return retVal;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_EnqueueVolumeParameter(uint32_t reactor, uint32_t volId, double offset)
{
    uint64_t currentBW = volumeQosParam[reactor][volId].currentBW / offset;
    uint64_t currentIops = volumeQosParam[reactor][volId].currentIOs / offset;
    bool minimumPolicyInEffect = QosManagerSingleton::Instance()->IsMinimumPolicyInEffectInSystem();
    qos_vol_policy volPolicy = qosArrayManager->GetVolumePolicy(volId);
    bool enqueueParameters = false;

    enqueueParameters = minimumPolicyInEffect || (0 != volPolicy.maxBw) || (0 != volPolicy.maxIops);
    enqueueParameters = enqueueParameters && currentBW;
    if (enqueueParameters)
    {
        volumeQosParam[reactor][volId].valid = M_VALID_ENTRY;
        volumeQosParam[reactor][volId].currentBW = currentBW;
        volumeQosParam[reactor][volId].currentIOs = currentIops;
        _EnqueueParams(reactor, volId, volumeQosParam[reactor][volId]);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::SetVolumeLimit(uint32_t reactor, uint32_t volId, int64_t value, bool iops)
{
    if (true == iops)
    {
        volReactorIopsWeight[reactor][volId] = value;
    }
    else
    {
        volReactorWeight[reactor][volId] = value;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
int64_t
QosVolumeManager::GetVolumeLimit(uint32_t reactor, uint32_t volId, bool iops)
{
    if (true == iops)
    {
        return volReactorIopsWeight[reactor][volId];
    }
    else
    {
        return volReactorWeight[reactor][volId];
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::GetSubsystemVolumeMap(std::unordered_map<int32_t, std::vector<int>>& subSysVolMap)
{
    std::unique_lock<std::mutex> uniqueLock(subsysVolMapLock);
    subSysVolMap = nqnVolumeMap;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::SetArrayName(std::string name)
{
    arrayName = name;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
std::string
QosVolumeManager::GetArrayName(void)
{
    return arrayName;
}

string
QosVolumeManager::_GetBdevName(uint32_t volId, string arrayName)
{
    return BDEV_NAME_PREFIX + to_string(volId) + "_" + arrayName;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_CopyVolumeInfo(char* destInfo, const char* srcInfo, int len)
{
    strncpy(destInfo, srcInfo, len);
    destInfo[len] = '\0';
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
void
QosVolumeManager::_SetVolumeOperationDone(bool value)
{
    volumeOperationDone = value;
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis
 *
 * @Returns
 */
/* --------------------------------------------------------------------------*/
bool
QosVolumeManager::_GetVolumeOperationDone(void)
{
    return volumeOperationDone;
}
} // namespace pos
