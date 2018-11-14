///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __I3DS_PETA_TRIGGER_HPP
#define __I3DS_PETA_TRIGGER_HPP

#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include <i3ds/trigger.hpp>

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace logging = boost::log;

namespace i3ds
{

class PetaTrigger : public Trigger
{
public:

  typedef std::shared_ptr<PetaTrigger> Ptr;

  static Ptr Create(NodeID id)
  {
    return std::make_shared<PetaTrigger>(id);
  }

  PetaTrigger(NodeID sensor);
  virtual ~PetaTrigger();

  void Stop();

protected:

  void handle_generator(GeneratorService::Data& command);
  void handle_internal_channel(InternalChannelService::Data& command);
  void handle_external_channel(ExternalChannelService::Data& command);
  void handle_enable_channel(ChannelEnableService::Data& command);
  void handle_disable_channel(ChannelDisableService::Data& command);

};

} // namespace i3ds

#endif
