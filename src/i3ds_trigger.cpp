#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include <boost/program_options.hpp>

#define BOOST_LOG_DYN_LINK

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <i3ds/trigger.hpp>
#include <i3ds/communication.hpp>
#include <i3ds/server.hpp>

#include "trigger_driver.h"

namespace po = boost::program_options;
namespace logging = boost::log;


namespace i3ds
{

class PetalinuxTrigger : public Trigger
{
public:

  typedef std::shared_ptr<PetalinuxTrigger> Ptr;
  static Ptr Create(NodeID id, bool emulated)
  {
    return std::make_shared<PetalinuxTrigger>(id, emulated);
  }

  PetalinuxTrigger(NodeID sensor, bool emulated) : Trigger(sensor) {
    emulated_ = emulated;
  }

private:
  bool emulated_;

protected:

  // Handler for trigger generator command, must be overloaded.
  void handle_generator(GeneratorService::Data& command) override {
    GENERATOR_ID gen_id = static_cast<GENERATOR_ID>(command.request.generator);
    std::cout << "Attempting to set generator " << gen_id << " period to " << command.request.period << std::endl;
    trigger_set_generator_period(gen_id, command.request.period);
  }

  // Handler for trigger internal channel command, must be overloaded.
  void handle_internal_channel(InternalChannelService::Data& command) {
    TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(command.request.channel);
    GENERATOR_ID gen_id = static_cast<GENERATOR_ID>(command.request.source);
    bool autostart = false;
    BOOST_LOG_TRIVIAL(info) << "Attempting to configure channel " << chan_id << ": gen " << gen_id << ", offset " << command.request.offset << ", duration " << command.request.duration << ", invert " << command.request.invert;
    if (emulated_) {
      BOOST_LOG_TRIVIAL(warning) << "  but not really since system is emulated.";
      return;
    }
    trigger_configure(chan_id, gen_id, command.request.offset, command.request.duration, command.request.invert, autostart);
  }

  // Handler for trigger external channel command, must be overloaded.
  void handle_external_channel(ExternalChannelService::Data& command) {
    BOOST_LOG_TRIVIAL(error) << "Handle external channel not implemented";
    //TODO (sigurdm): implement
    if (emulated_) {
      BOOST_LOG_TRIVIAL(warning) << "  System is emulated.";
      return;
    }
  }

  // Handler for channel enable command, must be overloaded.
  void handle_enable_channel(ChannelEnableService::Data& command) {
    BOOST_LOG_TRIVIAL(info) <<  "Enabling channels:";
    //TODO (sigurdm): implement array/mask version?
    for (int i=0; i<8; i++) {
      if (command.request.arr[i]) {
	TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(i+1);
	BOOST_LOG_TRIVIAL(info) <<  "  Channel " << chan_id;
	if (!emulated_) {
	  trigger_enable(chan_id);
	}
      }
    }
    if (emulated_) {
      BOOST_LOG_TRIVIAL(warning) << "  but not really, since system is emulated.";
    }
  }

  // Handler for channel disable command, must be overloaded.
  void handle_disable_channel(ChannelDisableService::Data& command) {
    BOOST_LOG_TRIVIAL(info) << "Disabling channels:";
    //TODO (sigurdm): implement array/mask version?
    for (int i=0; i<8; i++) {
      if (command.request.arr[i]) {
	TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(i + 1);
	BOOST_LOG_TRIVIAL(info) << "  Channel " << chan_id;
	if (!emulated_) {
	  trigger_disable(chan_id);
	}
      }
    }
    if (emulated_) {
      BOOST_LOG_TRIVIAL(warning) << "  but not really, since system is emulated.";
    }
  }

};
}

int main(int argc, char* argv[])
{
  NodeID node_id;
  std::string addr_server;

  po::options_description desc("Driver for the trigger system");
  bool emulated = false;

  desc.add_options()
  ("help", "Show help")
  ("node,n", po::value(&node_id)->default_value(300), "NodeID")
  ("addr-server,as", po::value(&addr_server)->default_value(""), "Address to the address-server")
  ("emulated", po::bool_switch(&emulated)->default_value(false), "Emulate the trigger connection.")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    BOOST_LOG_TRIVIAL(info) << desc;
    return -1;
  }

  BOOST_LOG_TRIVIAL(info) << "Initializing trigger system.";
  if (!emulated) {
    trigger_initialize();
  } else {
    BOOST_LOG_TRIVIAL(warning) << "WARNING - Using emulated system - WARNING";
  }

  i3ds::Context::Ptr context;
  if (addr_server.empty()) {
    context = i3ds::Context::Create();
  } else {
    context = std::make_shared<i3ds::Context>(addr_server);
  }

  BOOST_LOG_TRIVIAL(info) << "Creating trigger node with id: " << node_id;
  i3ds::PetalinuxTrigger::Ptr trigger = i3ds::PetalinuxTrigger::Create(node_id, emulated);
  i3ds::Server server(context);
  BOOST_LOG_TRIVIAL(info) << "Attaching server.";
  trigger->Attach(server);
  BOOST_LOG_TRIVIAL(info) << "Starting server.";
  server.Start();
  BOOST_LOG_TRIVIAL(info) << "Server started.";

  while(true) {
    sleep(1000);
  }

  server.Stop();
  if (!emulated) {
    trigger_deinitialize();
  }
  return 0;
}
