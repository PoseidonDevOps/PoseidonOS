#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/dump/dump_buffer.h"

namespace pos
{
class MockDumpBufferDeleter : public DumpBufferDeleter
{
public:
    using DumpBufferDeleter::DumpBufferDeleter;
};

class MockDumpBuffer : public DumpBuffer
{
public:
    using DumpBuffer::DumpBuffer;
};

} // namespace pos
