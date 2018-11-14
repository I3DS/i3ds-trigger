///////////////////////////////////////////////////////////////////////////\file
///
///   Copyright 2018 SINTEF AS
///
///   This Source Code Form is subject to the terms of the Mozilla
///   Public License, v. 2.0. If a copy of the MPL was not distributed
///   with this file, You can obtain one at https://mozilla.org/MPL/2.0/
///
////////////////////////////////////////////////////////////////////////////////

#include <csignal>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <memory>

#include <boost/program_options.hpp>

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <i3ds/trigger.hpp>
#include <i3ds/communication.hpp>
#include <i3ds/server.hpp>

#include "peta_trigger.hpp"
#include "trigger_driver.h"

namespace po = boost::program_options;
namespace logging = boost::log;

volatile bool running;

void signal_handler(int signum)
{
  BOOST_LOG_TRIVIAL(info) << "STOP";
  running = false;
}

int main(int argc, char* argv[])
{
  NodeID node_id;

  po::options_description desc("Driver for the trigger system");

  desc.add_options()
  ("help", "Show help")
  ("node,n", po::value(&node_id)->default_value(300), "NodeID")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      BOOST_LOG_TRIVIAL(info) << desc;
      return -1;
    }

  BOOST_LOG_TRIVIAL(info) << "Initializing trigger system...";

  trigger_initialize();

  i3ds::Context::Ptr context = i3ds::Context::Create();

  i3ds::Server server(context);

  i3ds::PetaTrigger trigger(node_id);

  trigger.Attach(server);

  running = true;
  signal(SIGINT, signal_handler);

  server.Start();

  while (running)
    {
      sleep(1);
    }

  server.Stop();
  trigger.Stop();

  trigger_deinitialize();

  return 0;
}
