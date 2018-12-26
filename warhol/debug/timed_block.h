// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <stdlib.h>

#include <atomic>

#include "warhol/platform/platform.h"
#include "warhol/utils/log.h"

// TODO(Cristian): When providing a way to print and swap these values,
//                 remember to do a atomic swap, so we don't lose any values.

namespace warhol {

#define TIMED_BLOCK()                                         \
  static size_t timed_block_id_##__LINE__ =                   \
      ::warhol::TimedBlockRecord::GetNewTimedBlockRecordID(); \
  ::warhol::TimedBlock(                     \
      timed_block_id_##__LINE__, __FILE__, __LINE__, __FUNCTION__);

struct TimedBlockRecord {
  const char* filename;
  const char* function;

  uint32_t line;
  uint32_t reserved;

  // Low 32 bits:   Cycle count.
  // High 32 bits:  Hit count.
  std::atomic<uint64_t> hit_and_cycle_count;

  // Returns an unique ID for a new Time record entry for |g_time_record_array|.
  // Asserts that the value never gets past |kMaxTimedBlockRecords|.
  static constexpr size_t kMaxTimedBlockRecords = 1024;
  static TimedBlockRecord gTimeBlockRecordArray[];

  static size_t GetNewTimedBlockRecordID();
  static size_t GetTimedBlockRecordCount();
};

// This is a declaration of the array of time records that are defined in
// timed_block.cc

struct TimedBlock {
  TimedBlockRecord* record;
  uint64_t start_cycles;
  uint32_t hit_count;

  TimedBlock(size_t id, const char* filename, int line,
             const char* function, uint32_t hit_count = 1) {
    this->hit_count = hit_count;
    record = TimedBlockRecord::gTimeBlockRecordArray + id;
    record->filename = filename;
    record->line = line;
    record->function = function;

    start_cycles = Platform::GetHighPerformanceCounter();
  }

  ~TimedBlock() {
    uint64_t delta = Platform::GetHighPerformanceCounter() - start_cycles;
    delta |= ((uint64_t)hit_count << 32);

    // Atomic addition.
    record->hit_and_cycle_count += delta;
  }
};

}  // namespace warhol
