#include "trigger_driver.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/*******************************************************************************
* Register Offsets, from camera_trig_gen_ip.h file
*******************************************************************************/
#define TRIG_OUT_EN      0x00
#define EXT_IN_INV       0x04
#define EXT_IN_BYPASS    0x08
#define TRIG_OUT_INV     0x0C
#define TRIG_OUT_BYPASS  0x10
#define TRIG_PERIOD_BASE 0x20
#define TRIG_SEL_BASE    0x40
#define PULSE_DELAY_BASE 0x60
#define PULSE_WIDTH_BASE 0x80

#define XPAR_CAMERA_TRIG_GEN_IP_0_S00_AXI_BASEADDR 0x80032000
#define XPAR_CAMERA_TRIG_GEN_IP_0_S00_AXI_HIGHADDR 0x80032FFF

struct trigger_device
{
  uint32_t trigger_output_enable; // 0x00
  uint32_t external_input_invert; // 0x04
  uint32_t external_input_bypass; // 0x08
  uint32_t trigger_output_invert; // 0x0C
  uint32_t trigger_output_bypass; // 0x10
  uint32_t unused_1;              // 0x14
  uint32_t unused_2;              // 0x18
  uint32_t unused_3;              // 0x1C
  uint32_t trigger_period[8];     // 0x20
  uint32_t trigger_select[8];     // 0x40
  uint32_t pulse_delay[8];        // 0x60
  uint32_t pulse_width[8];        // 0x80
};

// Remap to more human readable format
#define PHYSICAL_ADDRESS XPAR_CAMERA_TRIG_GEN_IP_0_S00_AXI_BASEADDR
#define ADDRESS_RANGE (XPAR_CAMERA_TRIG_GEN_IP_0_S00_AXI_HIGHADDR - XPAR_CAMERA_TRIG_GEN_IP_0_S00_AXI_BASEADDR)

struct trigger_device *trigger = NULL;

int _fdmemHandle;
const char _memDevice[] = "/dev/mem";

// Initialize the trigger driver
int trigger_initialize()
{
  // Open /dev/mem
  _fdmemHandle = open(_memDevice, O_RDWR | O_SYNC);

  if (_fdmemHandle < 0)
    {
      printf("Failed to open the file /dev/mem.\r\n");
      return -1;
    }
  else
    {
      printf("Opened file /dev/mem successfully.\r\n");
    }

  // mmap() /dev/mem using the physical base address provide in the #defines
  trigger = (struct trigger_device *) mmap(0, ADDRESS_RANGE, PROT_READ|PROT_WRITE, MAP_SHARED, _fdmemHandle, PHYSICAL_ADDRESS);

  return 0;
}

// Deinitialize the trigger driver
void trigger_deinitialize()
{
  // De-allocate
  if (munmap(trigger, ADDRESS_RANGE) == -1)
  {
    perror("Error occurred when un-mmapping /dev/mem\r\n");
  }

  trigger = NULL;

  // Close the character device
  close(_fdmemHandle);
}

// Convert from the GENERATOR_ID to a memory offset
static inline uint32_t generator2offset(GENERATOR_ID gen_id)
{
  return (uint32_t)(gen_id - 1);
}

// Convert from the TRIGGER_ID to an array offset
static inline uint32_t trigger2offset(TRIGGER_ID trig_id)
{
  return (uint32_t)(trig_id - 1);
}

// Convert from the TRIGGER_ID to bitmask
static inline uint32_t trigger2mask(TRIGGER_ID trig_id)
{
  return 1 << trigger2offset(trig_id);
}

// Enable the trigger
void trigger_enable(TRIGGER_ID trig_id)
{
  trigger->trigger_output_enable |= trigger2mask(trig_id);
}

// Disable the trigger
void trigger_disable(TRIGGER_ID trig_id)
{
  trigger->trigger_output_enable &= ~trigger2mask(trig_id);
}

// Select generator
void trigger_select_generator(TRIGGER_ID trig_id, GENERATOR_ID gen_id)
{
  trigger->trigger_select[trigger2offset(trig_id)] = generator2offset(gen_id);
}

// Configure a trigger, by associating a trigger with a generator,
//  a pulse delay, a pulse width and if it is inverted or not
void trigger_configure(TRIGGER_ID trig_id, GENERATOR_ID gen_id,
		       uint32_t pulseDelay, uint32_t pulseWidth,
		       bool inverted, bool enable)
{
  trigger_select_generator(trig_id, gen_id);
  trigger_set_pulse_data(trig_id, pulseDelay, pulseWidth);
  trigger_set_inverted(trig_id, inverted);

  if (enable)
    {
      trigger_enable(trig_id);
    }
}

// Set if the trigger output should be high when not triggered
void trigger_set_inverted(TRIGGER_ID trig_id, bool inverted)
{
  if (inverted)
    {
      trigger->trigger_output_invert |= trigger2mask(trig_id);
    }
  else
    {
      trigger->trigger_output_invert &= ~trigger2mask(trig_id);
    }
}

// Set the pulse length and width in us - (0 to 16777215) and (0 to 1023) respectively.
void trigger_set_pulse_data(TRIGGER_ID trig_id, uint32_t pulseDelay, uint32_t pulseWidth)
{
  const uint32_t offset = trigger2offset(trig_id);

  trigger->pulse_delay[offset] = pulseDelay;
  trigger->pulse_width[offset] = pulseWidth;
}

// Internal trigger priod in us (0 to 16777215)
void trigger_set_generator_period(GENERATOR_ID gen_id, uint32_t pulsePeriod)
{
  trigger->trigger_period[generator2offset(gen_id)] = pulsePeriod;
}
