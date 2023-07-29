#include "kernel/global_channel.h"

namespace kernel {

Channel GlobalChannel::sleep_channel_("sleep_channel");
Channel GlobalChannel::console_channel_("console_channel");

}  // namespace kernel
