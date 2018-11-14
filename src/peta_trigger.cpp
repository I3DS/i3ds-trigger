///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////

#include "peta_trigger.hpp"
#include "trigger_driver.h"

i3ds::PetaTrigger::PetaTrigger(NodeID sensor)
  : Trigger(sensor)
{
  BOOST_LOG_TRIVIAL(info) << "Create trigger with NodeID: " << node();
}

i3ds::PetaTrigger::~PetaTrigger()
{
  BOOST_LOG_TRIVIAL(info) << "Destroy trigger with NodeID: " << node();
}

void
i3ds::PetaTrigger::handle_generator(GeneratorService::Data& command)
{
  GENERATOR_ID gen_id = static_cast<GENERATOR_ID>(command.request.generator);

  BOOST_LOG_TRIVIAL(info) << "Set generator " << gen_id << " period to " << command.request.period;

  trigger_set_generator_period(gen_id, command.request.period);
}

void
i3ds::PetaTrigger::handle_internal_channel(InternalChannelService::Data& command)
{
  TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(command.request.channel);
  GENERATOR_ID gen_id = static_cast<GENERATOR_ID>(command.request.source);

  BOOST_LOG_TRIVIAL(info) << "Configure channel " << chan_id << " with: "
			  << "gen = " << gen_id
			  << ", offset = " << command.request.offset
			  << ", duration = " << command.request.duration
			  << ", invert = " << command.request.invert;

  trigger_configure(chan_id, gen_id, command.request.offset, command.request.duration, command.request.invert, false);
}

void
i3ds::PetaTrigger::handle_external_channel(ExternalChannelService::Data& command)
{
  BOOST_LOG_TRIVIAL(error) << "Handle external channel not implemented";
  throw CommandError(error_unsupported, "External channel not implemented");
}

void
i3ds::PetaTrigger::handle_enable_channel(ChannelEnableService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "Enabling channels:";

  for (int i = 0; i < 8; i++) {
    if (command.request.arr[i]) {
      TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(i + 1);
      BOOST_LOG_TRIVIAL(info) <<  " " << chan_id;
      trigger_enable(chan_id);
    }
  }
}

void
i3ds::PetaTrigger::handle_disable_channel(ChannelDisableService::Data& command)
{
  BOOST_LOG_TRIVIAL(info) << "Disabling channels:";

  for (int i = 0; i < 8; i++) {
    if (command.request.arr[i]) {
      TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(i + 1);
      BOOST_LOG_TRIVIAL(info) << " " << chan_id;
      trigger_disable(chan_id);
    }
  }
}

void
i3ds::PetaTrigger::Stop()
{
  BOOST_LOG_TRIVIAL(info) << "Disabling all channels:";

  for (int i = 0; i < 8; i++) {
    TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(i + 1);
    BOOST_LOG_TRIVIAL(info) << " " << chan_id;
    trigger_disable(chan_id);
  }
}
