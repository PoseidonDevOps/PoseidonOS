#include "test/integration-tests/journal/fixture/replay_test_fixture.h"

using ::testing::InSequence;
using ::testing::Return;

namespace pos
{
ReplayTestFixture::ReplayTestFixture(MockMapper* mapper,
    AllocatorMock* allocator, TestInfo* testInfo)
: mapper(mapper),
  allocator(allocator),
  testInfo(testInfo)
{
}

ReplayTestFixture::~ReplayTestFixture(void)
{
}

void
ReplayTestFixture::ExpectReturningUnmapStripe(StripeId vsid)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()),
        GetLSA(vsid)).WillOnce(Return(unmapAddr));
}

void
ReplayTestFixture::ExpectReturningUnmapStripes(void)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()),
        GetLSA).WillRepeatedly(Return(unmapAddr));
}

void
ReplayTestFixture::ExpectReturningUnmapStripes(StripeId vsid)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()),
        GetLSA(vsid)).WillRepeatedly(Return(unmapAddr));
}

void
ReplayTestFixture::ExpectReplayStripeAllocation(StripeId vsid, StripeId wbLsid)
{
    if (vsid % testInfo->numStripesPerSegment == 0)
    {
        EXPECT_CALL(*(allocator->GetSegmentCtxMock()),
            ReplaySegmentAllocation(vsid))
            .Times(1);
    }
    EXPECT_CALL(*(mapper->GetStripeMapMock()),
        SetLSA(vsid, wbLsid, IN_WRITE_BUFFER_AREA))
        .Times(1);
    EXPECT_CALL(*(allocator->GetWBStripeCtxMock()),
        ReplayStripeAllocation(vsid, wbLsid))
        .Times(1);
}

void
ReplayTestFixture::ExpectReplayBlockLogsForStripe(int volId, BlockMapList blksToWrite)
{
    // FIXME (huijeong.kim) Due to the limitation we have (see _AddLogInternal)
    // Replaying block map updated log could be non-sequential
    // InSequence s;

    for (auto blk : blksToWrite)
    {
        BlkAddr rba = std::get<0>(blk);
        VirtualBlks blks = std::get<1>(blk);

        for (uint32_t offset = 0; offset < blks.numBlks; offset++)
        {
            VirtualBlks blk = _GetBlock(blks, offset);
            EXPECT_CALL(*(mapper->GetVSAMapMock()), SetVSAsInternal(volId, rba + offset, blk));
            EXPECT_CALL(*(allocator->GetBlockAllocatorMock()), ValidateBlks(blk));
        }
    }
}

VirtualBlks
ReplayTestFixture::_GetBlock(VirtualBlks blks, uint32_t offset)
{
    VirtualBlks blk = {
        .startVsa = {
            .stripeId = blks.startVsa.stripeId,
            .offset = blks.startVsa.offset + offset},
        .numBlks = 1};
    return blk;
}

void
ReplayTestFixture::ExpectReplayStripeFlush(StripeTestFixture stripe)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()), SetLSA(stripe.GetVsid(),
        stripe.GetUserAddr().stripeId, stripe.GetUserAddr().stripeLoc)).Times(1);
    EXPECT_CALL(*(allocator->GetWBStripeCtxMock()),
        ReplayStripeFlushed(stripe.GetWbAddr().stripeId))
        .Times(1);
    EXPECT_CALL(*(allocator->GetSegmentCtxMock()),
        UpdateOccupiedStripeCount(stripe.GetUserAddr().stripeId))
        .Times(1);
}

void
ReplayTestFixture::ExpectReplayFullStripe(StripeTestFixture stripe)
{
    BlockMapList blksToWrite = stripe.GetBlockMapList();
    // FIXME (huijeong.kim) Due to the limitation we have (see _AddLogInternal)
    // Replaying block map updated log could be non-sequential
    {
        InSequence s;

        ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        ExpectReplayStripeFlush(stripe);
    }

    ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), blksToWrite);
}

void
ReplayTestFixture::ExpectReplayOverwrittenBlockLog(StripeTestFixture stripe)
{
    BlockMapList writtenVsas = stripe.GetBlockMapList();
    ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
    ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), writtenVsas);

    for (auto vsa = writtenVsas.begin(); vsa != writtenVsas.end() - 1; vsa++)
    {
        for (uint32_t blockOffset = 0; blockOffset < (*vsa).second.numBlks;
            blockOffset++)
        {
            VirtualBlks blks = _GetBlock((*vsa).second, blockOffset);
            EXPECT_CALL(*(allocator->GetBlockAllocatorMock()), InvalidateBlks(blks));
        }
    }
}

void
ReplayTestFixture::ExpectReplayUnflushedActiveStripe(VirtualBlkAddr tail, StripeTestFixture stripe)
{
    EXPECT_CALL(*(allocator->GetWBStripeAllocatorMock()),
        RestoreActiveStripeTail(testInfo->defaultTestVol, tail, stripe.GetWbAddr().stripeId))
        .Times(1);
    EXPECT_CALL(*(allocator->GetSegmentCtxMock()), ReplaySsdLsid).Times(1);
}

void
ReplayTestFixture::ExpectReplayFlushedActiveStripe(void)
{
    EXPECT_CALL(*(allocator->GetWBStripeCtxMock()),
        ResetActiveStripeTail(testInfo->defaultTestVol))
        .Times(1);
    EXPECT_CALL(*(allocator->GetSegmentCtxMock()), ReplaySsdLsid).Times(1);
}

VirtualBlkAddr
ReplayTestFixture::GetNextBlock(VirtualBlks blks)
{
    VirtualBlkAddr nextVsa = blks.startVsa;
    nextVsa.offset += blks.numBlks;
    return nextVsa;
}
} // namespace pos