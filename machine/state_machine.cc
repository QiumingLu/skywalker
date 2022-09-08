// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/state_machine.h"

#include <voyager/util/crc32c.h>

#include "skywalker/file.h"
#include "skywalker/logging.h"
#include "util/coding.h"

namespace skywalker {

static const int kHeaderSize = 16;

static bool Checksum(const std::string& data) {
  if (data.size() >= kHeaderSize) {
    uint32_t crc = DecodeFixed32(data.c_str() + data.size() - 4);
    if (crc == voyager::crc32c::Value(data.c_str(), data.size() - 4)) {
      return true;
    }
  }
  return false;
}

bool StateMachine::ReadCheckpoint(uint32_t group_id, uint64_t instance_id,
                                  const std::string& fname,
                                  google::protobuf::Message* message) {
  std::string data;
  Status status = ReadFileToString(FileManager::Instance(), fname, &data);
  if (!status.ok()) {
    LOG_ERROR("Group %u - instance %llu read failed %s.",
              group_id, (unsigned long long)instance_id,
              status.ToString().c_str());
    return false;
  }
  if (!Checksum(data)) {
    LOG_ERROR("Group %u - instance %llu read fname %s, checksum failed.",
              group_id, (unsigned long long)instance_id, fname.c_str());
    return false;
  }
  uint32_t gid = DecodeFixed32(data.c_str());
  uint64_t iid = DecodeFixed64(data.c_str() + 4);
  if (gid != group_id || iid != instance_id) {
    LOG_ERROR("Group %u - instance %llu read fname %s, which is gid=%u iid=%llu.",
              group_id, (unsigned long long)instance_id, fname.c_str(),
              gid, (unsigned long long)iid);
    return false;
  }
  if (!message->ParseFromArray(data.c_str() + 12,
                               static_cast<int>(data.size()) - kHeaderSize)) {
    LOG_ERROR("Group %u - instance %llu read fname %s, parse failed.",
              group_id, (unsigned long long)instance_id, fname.c_str());
    return false;
  }
  return true;
}

bool StateMachine::WriteCheckpoint(uint32_t group_id, uint64_t instance_id,
                                   const std::string& fname,
                                   const google::protobuf::Message& message) {
  size_t size = kHeaderSize + message.ByteSizeLong();
  std::string data;
  data.reserve(size);

  PutFixed32(&data, group_id);
  PutFixed64(&data, instance_id);
  message.AppendToString(&data);  
  uint32_t crc = voyager::crc32c::Value(data.c_str(), data.size());
  PutFixed32(&data, crc);

  Status status = WriteStringToFileSync(FileManager::Instance(), data, fname);
  if (!status.ok()) {
    LOG_ERROR("Group %u - instance %llu write failed %s.",
              group_id, (unsigned long long)instance_id,
              status.ToString().c_str());
    return false;
  }
  return true;
}

}  // namespace skywalker