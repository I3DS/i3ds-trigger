#ifndef _TRIGGER_DRIVER_H
#define _TRIGGER_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
// External triggers
typedef enum TRIGGER_ID
{
 TRIGGER_CAM_1 = 1,
 TRIGGER_CAM_2,
 TRIGGER_CAM_3,
 TRIGGER_CAM_4,
 TRIGGER_CAM_5,
 TRIGGER_6,
 TRIGGER_7,
 TRIGGER_8,

 TRIGGER_NUMBER_OF_TRIGGERS
} TRIGGER_ID;


// Internal generators
typedef enum GENERATOR_ID {
  LOCAL_GENERATOR_1 = 1,
  LOCAL_GENERATOR_2,
  LOCAL_GENERATOR_3,
  LOCAL_GENERATOR_4,

  GENERATOR_NUMBER_OF_GENERATORS
} GENERATOR_ID;


// Initialize the trigger driver
int trigger_initialize();

// Deinitialize the trigger driver
void trigger_deinitialize();


// Select generator for trigger
void trigger_select_generator(TRIGGER_ID trig_id, GENERATOR_ID gen_id);

// Internal generator period in us (0 to 16777215)
void trigger_set_generator_period(GENERATOR_ID trig_id, uint32_t pulsePeriod);


// Enable the trigger
void trigger_enable(TRIGGER_ID trig_id);
void trigger_mask_enable(uint32_t trig_mask);

// Disable the trigger
void trigger_disable(TRIGGER_ID trig_id);
void trigger_mask_disable(uint32_t trig_mask);

// Set the pulse length and width in us - (0 to 16777215) and (0 to 1023) respectively.
void trigger_set_pulse_data(TRIGGER_ID trig_id, uint32_t pulseDelay, uint32_t pulseWidth);

// Configure a trigger, by associating a trigger with a generator, a pulse delay,
//  a pulse width, if it is inverted or not and if it should be enabled
void trigger_configure(TRIGGER_ID trig_id, GENERATOR_ID gen_id,
		       uint32_t pulseDelay, uint32_t pulseWidth,
		       bool invert, bool enable);

// Set if the trigger output should be high when not triggered
void trigger_set_inverted(TRIGGER_ID trig_id, bool inverted);

#ifdef __cplusplus
}
#endif

#endif // _TRIGGER_DRIVER_H
