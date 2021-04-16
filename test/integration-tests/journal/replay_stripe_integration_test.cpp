#include "gtest/gtest.h"

#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

namespace pos
{
class ReplayStripeIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    ReplayStripeIntegrationTest(void);
    virtual ~ReplayStripeIntegrationTest(void) = default;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

ReplayStripeIntegrationTest::ReplayStripeIntegrationTest(void)
: JournalManagerTestFixture(GetLogFileName())
{
}

void
ReplayStripeIntegrationTest::SetUp(void)
{
}

void
ReplayStripeIntegrationTest::TearDown(void)
{
}

TEST_F(ReplayStripeIntegrationTest, ReplayOverwrite)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayOverwrite");

    InitializeJournal();

    BlkAddr rba = std::rand() % testInfo->defaultTestVolSizeInBlock;
    StripeId vsid = std::rand() % testInfo->numUserStripes;
    uint32_t numBlksToOverwrite = std::rand() % testInfo->numBlksPerStripe + 1;

    StripeTestFixture stripe(vsid, testInfo->defaultTestVol);
    writeTester->WriteOverwrittenBlockLogs(stripe, rba, 0, numBlksToOverwrite);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReplayOverwrittenBlockLog(stripe);

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;
    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);
    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayStripeIntegrationTest, ReplayFullStripe)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayFullStripe");

    InitializeJournal();

    StripeId vsid = std::rand() % testInfo->numUserStripes;
    StripeTestFixture stripe(vsid, testInfo->defaultTestVol);
    writeTester->WriteLogsForStripe(stripe);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReplayFullStripe(stripe);
    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayStripeIntegrationTest, ReplayFullStripeSeveralTimes)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayFullStripeSeveralTimes");

    InitializeJournal();

    std::list<StripeTestFixture> writtenStripes;

    uint32_t writtenLogSize = 0;
    uint32_t currentVsid = 0;
    while (writtenLogSize < logGroupSize / 1024)
    {
        StripeTestFixture stripe(currentVsid++, testInfo->defaultTestVol);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripes.push_back(stripe);

        writtenLogSize += (stripe.GetBlockMapList().size() * sizeof(BlockWriteDoneLog));
        writtenLogSize += sizeof(StripeMapUpdatedLog);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    for (auto stripeLog : writtenStripes)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayStripeIntegrationTest, ReplayeSeveralUnflushedStripe)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayeSeveralUnflushedStripe");

    InitializeJournal();

    StripeTestFixture partialStripe(0, testInfo->defaultTestVol);
    int startOffset = std::rand() % testInfo->numBlksPerStripe;
    writeTester->WriteBlockLogsForStripe(partialStripe, startOffset, testInfo->numBlksPerStripe - startOffset);

    StripeTestFixture fullStripe(1, testInfo->defaultTestVol);
    writeTester->WriteBlockLogsForStripe(fullStripe, 0, testInfo->numBlksPerStripe);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    {
        InSequence s;

        replayTester->ExpectReplayStripeAllocation(partialStripe.GetVsid(), partialStripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(partialStripe.GetVolumeId(), partialStripe.GetBlockMapList());

        VirtualBlks writtenLastBlock = partialStripe.GetBlockMapList().back().second;
        VirtualBlkAddr tailVsa = ReplayTestFixture::GetNextBlock(writtenLastBlock);
        EXPECT_CALL(*(testAllocator->GetWBStripeAllocatorMock()),
            ReconstructActiveStripe(testInfo->defaultTestVol, partialStripe.GetWbAddr().stripeId, tailVsa, _));
        EXPECT_CALL(*(testAllocator->GetWBStripeAllocatorMock()),
            FinishReconstructedStripe(partialStripe.GetWbAddr().stripeId, tailVsa));
    }

    {
        InSequence s;

        replayTester->ExpectReplayStripeAllocation(fullStripe.GetVsid(), fullStripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(fullStripe.GetVolumeId(), fullStripe.GetBlockMapList());

        VirtualBlks writtenLastBlock = fullStripe.GetBlockMapList().back().second;

        VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);
        EXPECT_CALL(*(testAllocator->GetWBStripeAllocatorMock()),
            RestoreActiveStripeTail(testInfo->defaultTestVol, tail, fullStripe.GetWbAddr().stripeId))
            .Times(1);
    }

    EXPECT_CALL(*(testAllocator->GetSegmentCtxMock()), ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayStripeIntegrationTest, ReplayBlockWritesFromStart)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayBlockWritesFromStart");

    InitializeJournal();

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    writeTester->WriteBlockLogsForStripe(stripe, 0, numBlks);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    {
        InSequence s;

        replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());
    }

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;

    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);
    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayStripeIntegrationTest, ReplayBlockWrites)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayBlockWrites");

    InitializeJournal();

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    writeTester->WriteBlockLogsForStripe(stripe, testInfo->numBlksPerStripe - numBlks, numBlks);
    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
    replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;
    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);

    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayStripeIntegrationTest, ReplayBlockWritesFromStartToEnd)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayBlockWritesFromStartToEnd");

    InitializeJournal();

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    writeTester->WriteBlockLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    {
        InSequence s;

        replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());
    }

    VirtualBlkAddr tail = {
        .stripeId = stripe.GetVsid(),
        .offset = testInfo->numBlksPerStripe};

    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayStripeIntegrationTest, ReplayBlockWritesAndFlush)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayBlockWritesAndFlush");

    InitializeJournal();

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    uint32_t startOffset = testInfo->numBlksPerStripe - numBlks;
    writeTester->WriteBlockLogsForStripe(stripe, startOffset, numBlks);
    bool writeSuccessful = writeTester->WriteStripeLog(stripe.GetVsid(), stripe.GetWbAddr(), stripe.GetUserAddr());
    EXPECT_TRUE(writeSuccessful == true);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    {
        InSequence s;

        replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());
        replayTester->ExpectReplayStripeFlush(stripe);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayStripeIntegrationTest, ReplayFlush)
{
    POS_TRACE_DEBUG(9999, "ReplayStripeIntegrationTest::ReplayFlushButMapNotUpdated");

    InitializeJournal();

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    bool writeSuccessful = writeTester->WriteStripeLog(stripe.GetVsid(), stripe.GetWbAddr(), stripe.GetUserAddr());
    EXPECT_TRUE(writeSuccessful == true);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
    replayTester->ExpectReplayStripeFlush(stripe);

    EXPECT_CALL(*(testAllocator->GetSegmentCtxMock()), ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}
} // namespace pos