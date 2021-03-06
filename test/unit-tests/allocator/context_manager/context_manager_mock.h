#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/context_manager.h"

namespace pos
{
class MockContextManager : public ContextManager
{
public:
    using ContextManager::ContextManager;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, FlushContexts, (EventSmartPtr callback, bool sync), (override));
    MOCK_METHOD(int, FlushRebuildContext, (EventSmartPtr callback, bool sync), (override));
    MOCK_METHOD(void, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(SegmentId, AllocateFreeSegment, (), (override));
    MOCK_METHOD(SegmentId, AllocateGCVictimSegment, (), (override));
    MOCK_METHOD(SegmentId, AllocateRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, ReleaseRebuildSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
    MOCK_METHOD(int, MakeRebuildTarget, (), (override));
    MOCK_METHOD(int, StopRebuilding, (), (override));
    MOCK_METHOD(uint32_t, GetRebuildTargetSegmentCount, (), (override));
    MOCK_METHOD(int, GetNumOfFreeSegment, (bool needLock), (override));
    MOCK_METHOD(GcMode, GetCurrentGcMode, (), (override));
    MOCK_METHOD(int, GetGcThreshold, (GcMode mode), (override));
    MOCK_METHOD(uint64_t, GetStoredContextVersion, (int owner), (override));
    MOCK_METHOD(void, IncreaseValidBlockCount, (SegmentId segId, uint32_t count), (override));
    MOCK_METHOD(void, DecreaseValidBlockCount, (SegmentId segId, uint32_t count), (override));
    MOCK_METHOD(int, SetNextSsdLsid, (), (override));
    MOCK_METHOD(char*, GetContextSectionAddr, (int owner, int section), (override));
    MOCK_METHOD(int, GetContextSectionSize, (int owner, int section), (override));
    MOCK_METHOD(RebuildCtx*, GetRebuildCtx, (), (override));
    MOCK_METHOD(SegmentCtx*, GetSegmentCtx, (), (override));
    MOCK_METHOD(AllocatorCtx*, GetAllocatorCtx, (), (override));
    MOCK_METHOD(WbStripeCtx*, GetWbStripeCtx, (), (override));
    MOCK_METHOD(ContextReplayer*, GetContextReplayer, (), (override));
    MOCK_METHOD(GcCtx*, GetGcCtx, (), (override));
    MOCK_METHOD(std::mutex&, GetCtxLock, (), (override));
    MOCK_METHOD(BlockAllocationStatus*, GetAllocationStatus, (), (override));
};

} // namespace pos
