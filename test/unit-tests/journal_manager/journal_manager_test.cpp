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

#include "src/journal_manager/journal_manager.h"

#include <gtest/gtest.h>

#include "journal_manager_spy.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/dirty_map_manager_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/log_group_releaser_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/journal_writer_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffered_segment_context_manager_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/buffer_offset_allocator_mock.h"
#include "test/unit-tests/journal_manager/log_write/journal_event_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/journal_volume_event_handler_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_handler_mock.h"
#include "test/unit-tests/journal_manager/status/journal_status_provider_mock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgReferee;

namespace pos
{
class JournalManagerTestFixture : public ::testing::Test
{
public:
    JournalManagerTestFixture(void)
    : config(nullptr),
      statusProvider(nullptr),
      logWriteHandler(nullptr),
      logWriteContextFactory(nullptr),
      journalEventFactory(nullptr),
      volumeEventHandler(nullptr),
      logBuffer(nullptr),
      bufferAllocator(nullptr),
      logGroupReleaser(nullptr),
      bufferedSegmentContext(nullptr),
      dirtyMapManager(nullptr),
      logFilledNotifier(nullptr),
      replayHandler(nullptr),
      arrayInfo(nullptr)
    {
    }
    virtual ~JournalManagerTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        config = new NiceMock<MockJournalConfiguration>;
        statusProvider = new NiceMock<MockJournalStatusProvider>;
        logWriteHandler = new NiceMock<MockLogWriteHandler>;
        journalWriter = new NiceMock<MockJournalWriter>;
        logWriteContextFactory = new NiceMock<MockLogWriteContextFactory>;
        journalEventFactory = new NiceMock<MockJournalEventFactory>;
        volumeEventHandler = new NiceMock<MockJournalVolumeEventHandler>;
        logBuffer = new NiceMock<MockJournalLogBuffer>;
        bufferAllocator = new NiceMock<MockBufferOffsetAllocator>;
        logGroupReleaser = new NiceMock<MockLogGroupReleaser>;
        checkpointManager = new NiceMock<MockCheckpointManager>;
        bufferedSegmentContext = new NiceMock<MockBufferedSegmentContextManager>;
        dirtyMapManager = new NiceMock<MockDirtyMapManager>;
        logFilledNotifier = new NiceMock<MockLogBufferWriteDoneNotifier>;
        callbackSequenceController = new NiceMock<MockCallbackSequenceController>;
        replayHandler = new NiceMock<MockReplayHandler>;
        arrayInfo = new NiceMock<MockIArrayInfo>;

        journal = new JournalManager(config, statusProvider,
            logWriteContextFactory, journalEventFactory, logWriteHandler,
            volumeEventHandler, journalWriter,
            logBuffer, bufferAllocator, logGroupReleaser, checkpointManager,
            bufferedSegmentContext, dirtyMapManager, logFilledNotifier,
            callbackSequenceController, replayHandler, arrayInfo);
    }

    virtual void
    TearDown(void)
    {
        delete journal;
        delete arrayInfo;
    }

protected:
    JournalManager* journal;

    // Journal Depend-On-Components
    NiceMock<MockJournalConfiguration>* config;
    NiceMock<MockJournalStatusProvider>* statusProvider;
    NiceMock<MockLogWriteHandler>* logWriteHandler;
    NiceMock<MockJournalWriter>* journalWriter;
    NiceMock<MockLogWriteContextFactory>* logWriteContextFactory;
    NiceMock<MockJournalEventFactory>* journalEventFactory;
    NiceMock<MockJournalVolumeEventHandler>* volumeEventHandler;
    NiceMock<MockJournalLogBuffer>* logBuffer;
    NiceMock<MockBufferOffsetAllocator>* bufferAllocator;
    NiceMock<MockLogGroupReleaser>* logGroupReleaser;
    NiceMock<MockCheckpointManager>* checkpointManager;
    NiceMock<MockBufferedSegmentContextManager>* bufferedSegmentContext;
    NiceMock<MockDirtyMapManager>* dirtyMapManager;
    NiceMock<MockLogBufferWriteDoneNotifier>* logFilledNotifier;
    NiceMock<MockCallbackSequenceController>* callbackSequenceController;
    NiceMock<MockReplayHandler>* replayHandler;
    NiceMock<MockIArrayInfo>* arrayInfo;
};

TEST_F(JournalManagerTestFixture, Init_testWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    EXPECT_CALL(*config, IsEnabled()).WillRepeatedly(Return(false));

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal should be initialized with INVALID state
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNAL_INVALID);
}

TEST_F(JournalManagerTestFixture, Init_testWithJournalEnabledAndLogBufferNotExist)
{
    // Given: Journal config manager is configured to be enabled
    ON_CALL(*config, IsEnabled()).WillByDefault(Return(true));
    ON_CALL(*logBuffer, DoesLogFileExist).WillByDefault(Return(false));
    ON_CALL(*logBuffer, Create).WillByDefault(Return(0));

    // Then: All sub-modules should be initiailized
    EXPECT_CALL(*config, Init);
    EXPECT_CALL(*bufferAllocator, Init);
    EXPECT_CALL(*dirtyMapManager, Init);
    EXPECT_CALL(*logWriteContextFactory, Init);
    EXPECT_CALL(*logGroupReleaser, Init);
    EXPECT_CALL(*logWriteHandler, Init);
    EXPECT_CALL(*volumeEventHandler, Init);
    EXPECT_CALL(*replayHandler, Init);
    EXPECT_CALL(*statusProvider, Init);
    EXPECT_CALL(*logBuffer, Init);

    // Then: Log buffer should be created
    EXPECT_CALL(*logBuffer, Create);

    // Then: Log filled subscriber to be added in this sequence
    {
        InSequence s;
        EXPECT_CALL(*logFilledNotifier, Register(dirtyMapManager));
        EXPECT_CALL(*logFilledNotifier, Register(bufferAllocator));
        EXPECT_CALL(*logFilledNotifier, Register(logWriteHandler));
    }

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal manager should be ready
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNALING);
}

TEST_F(JournalManagerTestFixture, Init_testWithJournalEnabledAndLogBufferExist)
{
    // Given: Journal config manager is configured to be enabled
    ON_CALL(*config, IsEnabled()).WillByDefault(Return(true));
    ON_CALL(*logBuffer, DoesLogFileExist).WillByDefault(Return(true));

    // Then: JournalConfiguration should be initialized with the size of loaded log buffer
    uint64_t expectedLogBufferSize = 16 * 1024;
    uint64_t loadedLogBufferSize = 0;
    EXPECT_CALL(*logBuffer, Open(_)).WillOnce(DoAll(SetArgReferee<0>(expectedLogBufferSize), Return(0)));
    EXPECT_CALL(*config, Init(expectedLogBufferSize, _));

    // Then: All sub-modules should be initiailized
    EXPECT_CALL(*bufferAllocator, Init);
    EXPECT_CALL(*dirtyMapManager, Init);
    EXPECT_CALL(*logWriteContextFactory, Init);
    EXPECT_CALL(*logGroupReleaser, Init);
    EXPECT_CALL(*logWriteHandler, Init);
    EXPECT_CALL(*volumeEventHandler, Init);
    EXPECT_CALL(*replayHandler, Init);
    EXPECT_CALL(*statusProvider, Init);
    EXPECT_CALL(*logBuffer, Init);

    // Then: Log filled subscriber to be added in this sequence
    {
        InSequence s;
        EXPECT_CALL(*logFilledNotifier, Register(dirtyMapManager));
        EXPECT_CALL(*logFilledNotifier, Register(bufferAllocator));
        EXPECT_CALL(*logFilledNotifier, Register(logWriteHandler));
    }

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal manager should be ready
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNALING);
}

TEST_F(JournalManagerTestFixture, Init_testIfFailedToOpenLogBuffer)
{
    // Given: Journal is enabled and Log file is existed
    ON_CALL(*config, IsEnabled()).WillByDefault(Return(true));
    ON_CALL(*logBuffer, DoesLogFileExist).WillByDefault(Return(true));

    // When: Fail to open the Log Buffer
    int expectReturnCode = -1;
    EXPECT_CALL(*logBuffer, Open).WillOnce(Return(expectReturnCode));

    // Then: JournalManager return the error code
    int actualReturnCode = journal->Init(nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(expectReturnCode, actualReturnCode);
}

TEST_F(JournalManagerTestFixture, Init_testIfJournalEnabledOptionIsChanged)
{
    // Given: Log file is existed
    ON_CALL(*logBuffer, DoesLogFileExist).WillByDefault(Return(true));
    EXPECT_CALL(*logBuffer, Open).WillOnce(Return(0));

    // When: Journal configuration will be changed from enabled to disabled
    EXPECT_CALL(*config, IsEnabled)
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    // Then: Journal manager bypass the process of replay
    int actualReturnCode = journal->Init(nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    int expectReturnCode = 0;

    EXPECT_EQ(expectReturnCode, actualReturnCode);
}

TEST(JournalManager, _DoRecovery_testIfExecutedWithoutInialization)
{
    // Given
    NiceMock<MockJournalConfiguration>* config = new NiceMock<MockJournalConfiguration>;
    JournalManagerSpy journal(config, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ON_CALL(*config, IsEnabled()).WillByDefault(Return(true));

    // When: Recovery is executed without journal initiailization
    int actualReturnCode = journal.DoRecovery();

    // Then: Journal replay will be failed
    int expectedReturnCode = -EID(JOURNAL_REPLAY_FAILED);
    EXPECT_EQ(expectedReturnCode, actualReturnCode);
}

TEST_F(JournalManagerTestFixture, Init_testIfReplayFailed)
{
    // Given: Journal configuraiton is enabled and success to open the log buffer
    ON_CALL(*config, IsEnabled()).WillByDefault(Return(true));
    ON_CALL(*logBuffer, DoesLogFileExist).WillByDefault(Return(true));
    EXPECT_CALL(*logBuffer, Open).WillOnce(Return(0));

    // When: Replay handler return the error code
    EXPECT_CALL(*replayHandler, Start).WillOnce(Return(-1));

    int actualReturnCode = journal->Init(nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // Then: Journal manager should be return the error code to indicate that replay is failed
    int expectedReturnCode = -EID(JOURNAL_REPLAY_FAILED);
    EXPECT_EQ(expectedReturnCode, actualReturnCode);
}

TEST_F(JournalManagerTestFixture, Dispose_testWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(false));

    // When: Journal is disposed
    journal->Dispose();
}

TEST_F(JournalManagerTestFixture, Dispose_testWithJournalEnabled)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // Then: Log buffer should be disposed
    EXPECT_CALL(*logBuffer, Dispose);
    EXPECT_CALL(*bufferAllocator, Dispose);
    EXPECT_CALL(*dirtyMapManager, Dispose);
    EXPECT_CALL(*logFilledNotifier, Dispose);
    EXPECT_CALL(*logWriteHandler, Dispose);
    EXPECT_CALL(*replayHandler, Dispose);
    EXPECT_CALL(*bufferedSegmentContext, Dispose);

    // When: Journal is disposed
    journal->Dispose();
}

TEST_F(JournalManagerTestFixture, Shutdown_testWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(false));

    // Then: Log buffer should not be disposed, nor reset to log buffer
    EXPECT_CALL(*logBuffer, SyncResetAll).Times(0);
    EXPECT_CALL(*logBuffer, Dispose).Times(0);

    // When: Journal shutdowns
    journal->Shutdown();
}

TEST_F(JournalManagerTestFixture, Shutdown_testWithJournalEnabled)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // Then: Log buffer should be disposed after reset
    {
        InSequence s;

        EXPECT_CALL(*logBuffer, SyncResetAll).Times(0);
        EXPECT_CALL(*logBuffer, Dispose);
    }

    // When: Journal shutdowns
    journal->Shutdown();
}

TEST_F(JournalManagerTestFixture, Init_testInitWhenLogBufferNotExist)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // When: Log buffer is not loaded
    EXPECT_CALL(*logBuffer, DoesLogFileExist).WillOnce(Return(false));

    // Then: Expect log buffer to be reset
    EXPECT_CALL(*logBuffer, Create).WillOnce(Return(0));
    EXPECT_CALL(*logBuffer, SyncResetAll).WillOnce(Return(0));

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal manager should be ready
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNALING);
}

TEST_F(JournalManagerTestFixture, Init_testInitWhenLogBufferLoaded)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // When: Log buffer is loaded
    EXPECT_CALL(*logBuffer, DoesLogFileExist).WillOnce(Return(true));

    // Then: Expect to start replay
    EXPECT_CALL(*logBuffer, Open).WillOnce(Return(0));
    EXPECT_CALL(*replayHandler, Start).WillOnce(Return(0));

    // When: Journal is initialized
    ASSERT_TRUE(journal->Init(nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == 0);

    // Then: Journal manager should be ready
    EXPECT_TRUE(journal->GetJournalManagerStatus() == JOURNALING);
}

TEST_F(JournalManagerTestFixture, IsEnabled_testWithJournalDisabled)
{
    // Given: Journal config manager is configured to be disabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(false));

    // When: Journal is asked if it's enabled
    // Then: Journal should be disabled
    EXPECT_TRUE(journal->IsEnabled() == false);
}

TEST_F(JournalManagerTestFixture, IsEnabled_testWithJournalEnabled)
{
    // Given: Journal config manager is configured to be enabled
    EXPECT_CALL(*config, IsEnabled).WillRepeatedly(Return(true));

    // When: Journal is asked if it's enabled
    // Then: Journal should be enabled
    EXPECT_TRUE(journal->IsEnabled() == true);
}

TEST_F(JournalManagerTestFixture, Flush_testIfExecutedSuccesfully)
{
    // When: Journal request flush
    journal->Flush();
}
} // namespace pos
