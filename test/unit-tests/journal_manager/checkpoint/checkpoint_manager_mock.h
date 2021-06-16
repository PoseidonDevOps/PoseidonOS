#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/checkpoint/checkpoint_manager.h"

namespace pos
{
class MockCheckpointManager : public CheckpointManager
{
public:
    using CheckpointManager::CheckpointManager;
    MOCK_METHOD(void, Init, (IMapFlush* mapFlush, IContextManager* ctxManager, EventScheduler* scheduler, CallbackSequenceController* seqController, DirtyMapManager* dMapManager), (override));
    MOCK_METHOD(int, RequestCheckpoint, (int logGroupId, EventSmartPtr callback), (override));
    MOCK_METHOD(CheckpointStatus, GetStatus, (), (override));
};

} // namespace pos
