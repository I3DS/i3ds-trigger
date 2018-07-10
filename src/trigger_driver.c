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

// Remap to more human readable format
#define PHYSICAL_ADDRESS XPAR_CAMERA_TRIG_GEN_IP_0_S00_AXI_BASEADDR
#define ADDRESS_RANGE (XPAR_CAMERA_TRIG_GEN_IP_0_S00_AXI_HIGHADDR - XPAR_CAMERA_TRIG_GEN_IP_0_S00_AXI_BASEADDR)

void *triggerBaseAddr = NULL;
int _fdmemHandle;
const char _memDevice[] = "/dev/mem";

#define TRIGGER_GET_UINT32P(X) (uint32_t *)(triggerBaseAddr + X)

// Initialize the trigger driver
int trigger_initialize() {

  // Open /dev/mem 
  _fdmemHandle = open( _memDevice, O_RDWR | O_SYNC );
	
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
  triggerBaseAddr = mmap(0, ADDRESS_RANGE, PROT_READ|PROT_WRITE, MAP_SHARED, _fdmemHandle, PHYSICAL_ADDRESS);
  return 0;
}

// Convert from the TRIGGER_ID to a memory offset
int trigger2offset(TRIGGER_ID trig_id) {
  // For now, the info is stored in the enum
  return (int)(trig_id - 1);
}

// Convert from the GENERATOR_ID to a memory offset
int generator2offset(GENERATOR_ID gen_id) {
  // For now, the info is stored in the enum
  return (int)(gen_id - 1);
}

// Deinitialize the trigger driver
void trigger_deinitialize() {
  // De-allocate
  if (munmap(triggerBaseAddr, ADDRESS_RANGE) == -1)
  {
    perror("Error occurred when un-mmapping /dev/mem\r\n");
  }

  triggerBaseAddr = NULL;

  /* close the character device */
  close(_fdmemHandle);
}


// Enable the trigger
void trigger_enable(TRIGGER_ID trig_id) {
  trigger_mask_enable((uint32_t)(1 << trigger2offset(trig_id)));
}
void trigger_mask_enable(uint32_t trig_mask) {
  *TRIGGER_GET_UINT32P(TRIG_OUT_EN) |= trig_mask;
}


// Disable the trigger
void trigger_disable(TRIGGER_ID trig_id) {
  trigger_mask_disable((uint32_t)(1 << trigger2offset(trig_id)));
}
void trigger_mask_disable(uint32_t trig_mask) {
  *TRIGGER_GET_UINT32P(TRIG_OUT_EN) &= ~trig_mask;
}


void trigger_select_generator(TRIGGER_ID trig_id, GENERATOR_ID gen_id) {
  *TRIGGER_GET_UINT32P(TRIG_SEL_BASE + trigger2offset(trig_id)*4) = generator2offset(gen_id);
}

// Configure a trigger, by associating a trigger with a generator,
//  a pulse delay, a pulse width and if it is inverted or not
void trigger_configure(TRIGGER_ID trig_id, GENERATOR_ID gen_id,
		       uint32_t pulseDelay, uint32_t pulseWidth,
		       bool inverted, bool enable) {

  trigger_select_generator(trig_id, gen_id);
  
  trigger_set_pulse_data(trig_id, pulseDelay, pulseWidth);
  trigger_set_inverted(trig_id, inverted);

  if (enable) {
    trigger_enable(trig_id);
  }
}



// Set if the trigger output should be high when not triggered
void trigger_set_inverted(TRIGGER_ID trig_id, bool inverted) {
  if (inverted) {
    *TRIGGER_GET_UINT32P(TRIG_OUT_INV) |= (uint32_t)(1 << trigger2offset(trig_id));
  } else {
    *TRIGGER_GET_UINT32P(TRIG_OUT_INV) &= ~(uint32_t)(1 << trigger2offset(trig_id));
  }
}

// Set the pulse length and width in us - (0 to 16777215) and (0 to 1023) respectively.
void trigger_set_pulse_data(TRIGGER_ID trig_id, uint32_t pulseDelay, uint32_t pulseWidth) {
  *TRIGGER_GET_UINT32P(PULSE_DELAY_BASE + trigger2offset(trig_id)*4) = pulseDelay;
  *TRIGGER_GET_UINT32P(PULSE_WIDTH_BASE + trigger2offset(trig_id)*4) = pulseWidth;
}

// Internal trigger priod in us (0 to 16777215)
void trigger_set_generator_period(GENERATOR_ID trig_id, uint32_t pulsePeriod) {
    *TRIGGER_GET_UINT32P(TRIG_PERIOD_BASE + trigger2offset(trig_id)*4) = pulsePeriod;
}
