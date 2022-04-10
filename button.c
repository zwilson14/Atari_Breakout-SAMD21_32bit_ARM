//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "sam.h"
#include "pwm.h"
#include "button.h"
#include "adc.h"

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

// Note Frequencies:
#define C  (261.16)
#define Cs (277.18)
#define D  (293.66)
#define Ds (311.13)
#define E  (329.63)
#define F  (349.23)
#define Fs (369.99)
#define G  (392.00)
#define Gs (415.30)
#define A  (440.00)
#define As (466.16)
#define B  (493.88)
#define B3 (252)
#define Gs3 (209)

// Button Definitions:
//  BTN          ARM
// ==================
//  BTN_ACTIVE   PA08
//  BTN_0	      PB09
//  BTN_1        PA04

#define BTN_ACTIVE (PORT_PA08)
#define BTN_ACTIVE_GROUP (0)
#define BTN_ACTIVE_PIN (PIN_PA08%32)

#define BTN_0 (PORT_PB09)
#define BTN_0_GROUP (1)
#define BTN_0_PIN (PIN_PB09%32)

#define BTN_1 (PORT_PA04)
#define BTN_1_GROUP (0)
#define BTN_1_PIN (PIN_PA04%32)

// IN REGs
#define BTN_ACTIVE_VAL ((PORT->Group[BTN_ACTIVE_GROUP].IN.reg) & BTN_ACTIVE)
#define BTN_0_VAL ((PORT->Group[BTN_0_GROUP].IN.reg) & BTN_0)
#define BTN_1_VAL ((PORT->Group[BTN_1_GROUP].IN.reg) & BTN_1)

// Actual Button representation values:
static uint8_t button_val = 0;


//------------------------------------------------------------------------------
//     ___      __   ___  __   ___  ___  __
//      |  \ / |__) |__  |  \ |__  |__  /__`
//      |   |  |    |___ |__/ |___ |    .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//                __          __        ___  __
//     \  /  /\  |__) |  /\  |__) |    |__  /__`
//      \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void buttons_init()
{
  // Enables for signals
  PORT->Group[BTN_ACTIVE_GROUP].PINCFG[BTN_ACTIVE_PIN].bit.INEN = 1;
  PORT->Group[BTN_0_GROUP].PINCFG[BTN_0_PIN].bit.INEN = 1;
  PORT->Group[BTN_1_GROUP].PINCFG[BTN_1_PIN].bit.INEN = 1;
  
  // Config as inputs by clearing the register
  PORT->Group[BTN_ACTIVE_GROUP].DIRCLR.reg = BTN_ACTIVE;
  PORT->Group[BTN_0_GROUP].DIRCLR.reg = BTN_0;
  PORT->Group[BTN_1_GROUP].DIRCLR.reg = BTN_1;
  
}

uint32_t button_freq()
{
  // dummy freq val:
  uint32_t freq = 0;
  
  if (0 == button_val)
  {
    freq = Gs; 
  }
  
  if (1 == button_val)
  {
    freq = Ds;    
  }
  
  if (2 == button_val)
  {
    freq = B3;    
  }
  
  if (3 == button_val)
  {
    freq = Gs3;    
  }
     
  return freq;  
}

uint32_t button_adjust_freq(uint32_t note)
{
  uint64_t temp = 0;
  temp = (48000000/ note);
  temp = (temp / 2);
  
  return temp; // This returns the value to get the proper note
}

//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

// This goes off when GS (BTN_ACTIVE) goes off
uint8_t button_read()
{
  //Clear the value first:
  button_val = 0;
  
  // temp flags
  uint8_t s0 = 0;
  uint8_t s1 = 0;
  
  if (BTN_0_VAL)
  {
    button_val = 1;
    s1 = 1;
  }
  
  if (BTN_1_VAL)
  {
    button_val = 2;
    s0 = 1;
  }
  
  if ((s1) & (s0))
  {
    button_val = 3;
  }

    return button_val;
}

//------------------------------------------------------------------------------
//      __                  __        __        __
//     /  `  /\  |    |    |__)  /\  /  ` |__/ /__`
//     \__, /~~\ |___ |___ |__) /~~\ \__, |  \ .__/
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//        __   __  , __
//     | /__` |__)  /__`   
//     | .__/ |  \  .__/
//
//------------------------------------------------------------------------------
