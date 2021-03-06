// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOURNEY_JOURNEY_DB_H_
#define JOURNEY_JOURNEY_DB_H_

#include <string>

#include <leveldb/db.h>

namespace journey {

class JourneyDB {
 public:
  JourneyDB();
  ~JourneyDB();

  bool Open(const std::string& path);

  int Put(const std::string& key, const std::string& value);
  int Get(const std::string& key, std::string* value);
  int Delete(const std::string& key);

 private:
  leveldb::DB* db_;

  // No copying allowed
  JourneyDB(const JourneyDB&);
  void operator=(const JourneyDB&);
};

}  // namespace journey

#endif  // JOURNEY_JOURNEY_DB_H_
