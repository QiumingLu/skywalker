#include "skywalker/state_machine.h"
#include "util/sequence_number.h"

namespace skywalker {

static SequenceNumber seq_;

StateMachine::StateMachine()
    : id_(seq_.Next()) {
}

}  // namespace skywalker
