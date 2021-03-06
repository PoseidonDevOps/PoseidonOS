#include "src/metafs/mvm/volume/nvram/nvram_meta_volume.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_manager_mock.h"
#include "test/unit-tests/metafs/mvm/volume/catalog_manager_mock.h"
#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class NvRamMetaVolumeTestFixture : public ::testing::Test
{
public:
    NvRamMetaVolumeTestFixture(void)
    : inodeMgr(nullptr),
      catalogMgr(nullptr)
    {
    }

    virtual ~NvRamMetaVolumeTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        inodeMgr = new NiceMock<InodeManager>(arrayId);
        catalogMgr = new NiceMock<CatalogManager>(arrayId);

        volume = new NvRamMetaVolume(arrayId, maxLpn, inodeMgr, catalogMgr);
        volume->Init(metaStorage);
        volume->InitVolumeBaseLpn();
    }

    virtual void
    TearDown(void)
    {
        delete volume;
    }

protected:
    NvRamMetaVolume* volume;

    NiceMock<InodeManager>* inodeMgr;
    NiceMock<CatalogManager>* catalogMgr;

    int arrayId = 0;
    MetaLpnType maxLpn = 8192;
    MockMetaStorageSubsystem* metaStorage;
    MetaFilePropertySet prop;
};

TEST(NvRamMetaVolume, CreateDefault)
{
    NvRamMetaVolume* metaVolume = new NvRamMetaVolume();
    delete metaVolume;
}

TEST_F(NvRamMetaVolumeTestFixture, NVRAM_Meta_Volume_Normal)
{
    uint64_t chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;

    // then
    // prop is not suitable
    // the maximum size
    EXPECT_FALSE(volume->IsOkayToStore(6135 * chunkSize, prop));
    // expected count of the free lpn: 6136
    EXPECT_FALSE(volume->IsOkayToStore(6136 * chunkSize, prop));
    // more than possible
    // size is smaller than available but the number is not a multiple 8
    EXPECT_FALSE(volume->IsOkayToStore(6137 * chunkSize, prop));

    // prop is suitable
    prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
    EXPECT_TRUE(volume->IsOkayToStore(6135 * chunkSize, prop));
    // expected count of the free lpn: 6136
    EXPECT_TRUE(volume->IsOkayToStore(6136 * chunkSize, prop));
    // more than possible
    // size is smaller than available but the number is not a multiple 8
    EXPECT_FALSE(volume->IsOkayToStore(6137 * chunkSize, prop));

    prop.ioAccPattern = MetaFileAccessPattern::SmallSizeBlockIO;
    EXPECT_TRUE(volume->IsOkayToStore(6135 * chunkSize, prop));
    // expected count of the free lpn: 6136
    EXPECT_TRUE(volume->IsOkayToStore(6136 * chunkSize, prop));
    // more than possible
    // size is smaller than available but the number is not a multiple 8
    EXPECT_FALSE(volume->IsOkayToStore(6137 * chunkSize, prop));

    prop.ioOpType = MetaFileDominant::WriteDominant;
    EXPECT_TRUE(volume->IsOkayToStore(6135 * chunkSize, prop));
    // expected count of the free lpn: 6136
    EXPECT_TRUE(volume->IsOkayToStore(6136 * chunkSize, prop));
    // more than possible
    // size is smaller than available but the number is not a multiple 8
    EXPECT_FALSE(volume->IsOkayToStore(6137 * chunkSize, prop));
}

TEST_F(NvRamMetaVolumeTestFixture, IsNVRAMStore_WriteDominant)
{
    uint64_t chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;

    // expected count of the free lpn: 6136
    prop.ioAccPattern = MetaFileAccessPattern::NoSpecific;
    prop.ioOpType = MetaFileDominant::WriteDominant;

    EXPECT_TRUE(volume->IsOkayToStore(6136 * chunkSize, prop));
}

TEST_F(NvRamMetaVolumeTestFixture, IsOkayToStore_FileSizeZero)
{
    // not enough space
    EXPECT_TRUE(volume->IsFreeSpaceEnough(0));
}

TEST(NvRamMetaVolume, IsOkayToStore_Negative)
{
    NiceMock<MockInodeManager>* inodeMgr = new NiceMock<MockInodeManager>(0);
    NiceMock<MockCatalogManager>* catalogMgr = new NiceMock<MockCatalogManager>(0);
    NvRamMetaVolume* volume = new NvRamMetaVolume(0, 8192, inodeMgr, catalogMgr);
    uint64_t chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    MetaFilePropertySet prop;

    EXPECT_CALL(*inodeMgr, GetUtilizationInPercent).WillOnce(Return(99));

    // not enough space
    EXPECT_FALSE(volume->IsOkayToStore(6136 * chunkSize, prop));

    delete volume;
}
} // namespace pos
