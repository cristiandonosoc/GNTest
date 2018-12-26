// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include "src/debug/timed_block.h"

#include <assert.h>

namespace warhol {

namespace {

std::atomic<uint64_t> gNextTimedBlockRecordId = 0;

} // namespace

TimedBlockRecord TimedBlockRecord::gTimeBlockRecordArray
    [TimedBlockRecord::kMaxTimedBlockRecords] = {};

size_t TimedBlockRecord::GetNewTimedBlockRecordID() {
  uint64_t id = gNextTimedBlockRecordId++;
  assert(id < TimedBlockRecord::kMaxTimedBlockRecords);
  return id;
}

size_t TimedBlockRecord::GetTimedBlockRecordCount() {
  return gNextTimedBlockRecordId;
}

}  // namespace warhol
