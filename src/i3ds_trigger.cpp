#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include <boost/program_options.hpp>

#include <i3ds/trigger.hpp>
#include <i3ds/communication.hpp>
#include <i3ds/server.hpp>

#include "trigger_driver.h"

namespace po = boost::program_options;

namespace i3ds
{

class PetalinuxTrigger : public Trigger
{
public:

  typedef std::shared_ptr<PetalinuxTrigger> Ptr;
  static Ptr Create(Context::Ptr context, NodeID id)
  {
    return std::make_shared<PetalinuxTrigger>(context, id);
  }

  PetalinuxTrigger(Context::Ptr context, NodeID sensor);

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
    std::cout << "Attempting to configure channel " << chan_id << ": gen " << gen_id << ", offset " << command.request.offset << ", duration " << command.request.duration << ", invert " << command.request.invert  << std::endl;
    trigger_configure(chan_id, gen_id, command.request.offset, command.request.duration, command.request.invert, autostart);
  }

  // Handler for trigger external channel command, must be overloaded.
  void handle_external_channel(ExternalChannelService::Data& command) {
    std::cout << "Handle external channel (not implemented)" << std::endl;
    //TODO (sigurdm): implement
  }

  // Handler for channel enable command, must be overloaded.
  void handle_enable_channel(ChannelEnableService::Data& command) {
    std::cout << "Enabling channels:" << std::endl;
    //TODO (sigurdm): implement array/mask version?
    for (int i=0; i<8; i++) {
      if (command.request.arr[i]) {
	TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(i+1);
	std::cout << "  Channel " << chan_id << std::endl;
	trigger_enable(chan_id);
      }
    }
  }

  // Handler for channel disable command, must be overloaded.
  void handle_disable_channel(ChannelDisableService::Data& command) {
    std::cout << "Disabling channels:" << std::endl;
    //TODO (sigurdm): implement array/mask version?
    for (int i=0; i<8; i++) {
      if (command.request.arr[i]) {
	TRIGGER_ID chan_id = static_cast<TRIGGER_ID>(i + 1);
	std::cout << "  Channel " << chan_id  << std::endl;
	trigger_disable(chan_id);
      }
    }
  }

};
}

i3ds::PetalinuxTrigger::PetalinuxTrigger(Context::Ptr context, NodeID sensor)
  : Trigger(sensor)
{
}

int main(int argc, char* argv[])
{
  NodeID node_id = atoi(argv[1]);
  std::string addr_server;

  po::options_description desc("Driver for the trigger system");

  desc.add_options()
  ("help", "Show help")
  ("node,n", po::value(&node_id)->default_value(300), "NodeID")
  ("addr-server,as", po::value(&addr_server)->default_value(""), "Address to the address-server")
  ;

  printf("Initializing trigger system.\r\n");
  trigger_initialize();

  i3ds::Context::Ptr context;
  if (addr_server.empty()) {
    context = i3ds::Context::Create();
  } else {
    context = std::make_shared<i3ds::Context>(addr_server);
  }

  i3ds::PetalinuxTrigger::Ptr trigger = i3ds::PetalinuxTrigger::Create(context, node_id);
  i3ds::Server server(context);
  printf("Attaching server.\r\n");
  trigger->Attach(server);
  printf("Starting server.\r\n");
  server.Start();
  printf("Server started.\r\n");

  while(true) {
    sleep(1000);
  }

  server.Stop();
  trigger_deinitialize();
  return 0;
}
