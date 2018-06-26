#include <stdio.h>
#include "trigger_driver.h"
#include <i3ds/trigger.hpp>

namespace i3ds
{
  
class PetalinuxTrigger : Trigger
{

protected:

  // Handler for trigger generator command, must be overloaded.
  void handle_generator(GeneratorService::Data& command) override {
  }

  // Handler for trigger internal channel command, must be overloaded.
  void handle_internal_channel(InternalChannelService::Data& command) {
  }

  // Handler for trigger external channel command, must be overloaded.
  void handle_external_channel(ExternalChannelService::Data& command) {
  }

  // Handler for channel enable command, must be overloaded.
  void handle_enable_channel(ChannelEnableService::Data& command) {
  }

  // Handler for channel disable command, must be overloaded.
  void handle_disable_channel(ChannelDisableService::Data& command) {
  }

};
}

int main(void)
{
	char userInput = '0';

	TRIGGER_ID trig_id = TRIGGER_CAM_3;
	uint32_t pulsePeriod = 100000;
	uint32_t pulseDelay = 0;
	uint32_t pulseWidth = 100;
	bool invert = false;
	// int bypass = 0;
	GENERATOR_ID gen_id = LOCAL_GENERATOR_1;

	printf("Initializing system.\r\n");
	trigger_initialize();

	printf("Setting all generators to %d us.\r\n", pulsePeriod);
	for(int intTrigChan=0; intTrigChan <= GENERATOR_NUMBER_OF_GENERATORS; intTrigChan++) {
	  trigger_set_generator_period((GENERATOR_ID)intTrigChan, pulsePeriod);
	}

	printf("Configuring CAM_%d: generator %d, pulse delay: %u us, pulse width: %u us, inverted: %s.\r\n",
	       trig_id+1, gen_id, pulseDelay, pulseWidth, invert ? "true" : "false");
	trigger_configure(trig_id, gen_id, pulseDelay, pulseWidth*2, invert, true);

	
	printf("Type 'q' to quit, '1' to switch LEDs on, '0' to switch them off\r\n");
	while(userInput != 'q') 
	{
		scanf("%c", &userInput);
		if (userInput == '1') 
		{
			printf("TRIG ON\r\n");
			trigger_enable(trig_id);
		} 
		else if (userInput == '0')
		{
			printf("LEDS OFF\r\n");
			trigger_disable(trig_id);
		} else if (userInput == 'I')
		{
		  printf("Inverting trigger\r\n");
		  invert ^= true;
		  trigger_set_inverted(trig_id, invert);
		}
	}

	printf("Quitting...\r\n");

	trigger_deinitialize();

	return 0;
}
