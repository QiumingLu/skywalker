// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/state_machine.h"
#include "util/sequence_number.h"

namespace skywalker {

static SequenceNumber seq_;

StateMachine::StateMachine()
    : id_(seq_.Next()) {
}

}  // namespace skywalker
