//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "pwm.h"
#include <sam.h>
#include <component/pm.h>
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

// LED  AVR   ARM
// ==============
// R0 = PD5 - PA15
// G0 = PD6 - PA20
// B0 = PB1 - PA07
// R1 = PD4 - PA09
// G1 = PB2 - PA18
// B1 = PB0 - PA06

// PMUX register, 22.7
// only 16 not 32 like Port io groups

#define LED_R0 (PORT_PA15)
#define LED_R0_GROUP (0)
// click on pin_pa15 goto imp. and the pins keep counting up to 40s
// don't know what those are for so mod off
#define LED_R0_PIN (PIN_PA15%32)

#define LED_G0 (PORT_PA20)
#define LED_G0_GROUP (0)
#define LED_G0_PIN (PIN_PA20%32)

#define LED_B0 (PORT_PA07)
#define LED_B0_GROUP (0)
#define LED_B0_PIN (PIN_PA07%32)

// This is the definitions for the Piezo pin
#define PIEZO_PIN (PIN_PA09%32)
#define PIEZO_GROUP (0)

// Constants for Clock multiplexers
#define GENERIC_CLOCK_MULTIPLEXER_TCC0_TCC1 (0x1A)

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
void pwm_init()
{
  
  // Enable Multiplexing for PIN - A09
  PORT -> Group[PIEZO_GROUP].PINCFG[PIEZO_PIN].bit.PMUXEN = 1;
  	
  // Configure Multiplexing, the piezo uses Peripheral Function E (TCC0)
  PORT -> Group[PIEZO_GROUP].PMUX[PIEZO_PIN/2].bit.PMUXO = 0x4;
  	
  // Page 656:
  // Before the TCC is enabled, it must be configured as outlined by the following steps:
  //   Enable the TCC bus clock (CLK_TCCx_APB) first
  // This is enabling the power manager on the specific clock sources
  PM->APBCMASK.bit.TCC0_ = 1;
  // PM->APBCMASK.bit.TCC1_ = 1;
  
// ----------------------------------------------------------------------------------------------
   //Put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0x1A (TCC0 and TCC1)
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GENERIC_CLOCK_MULTIPLEXER_TCC0_TCC1 ) | // Generic Clock Multiplexer 0
                      GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is source
                      GCLK_CLKCTRL_CLKEN ;
                      
   
  
  // In data sheet, Specific wave form/ tcc combos needed for specific leds, this inits the wave gen.
  //   Select the Waveform Generation operation in WAVE register (WAVE.WAVEGEN)
  TCC0->WAVE.bit.WAVEGEN      = 0x1;      // Normal PWM mode = 0x2 //Match Freq = 0x1
  while ( TCC0->SYNCBUSY.bit.WAVE )
  {
    ;
  }
  
  // While loops are essential, they allow for synchronization process to occur
  
  // This inverts the waveform output for individual channels of choice using the DRVCTRL.INVEN
  //   The waveform output can be inverted for the individual channels using the Waveform Output Invert Enable bit group in the Driver register (DRVCTRL.INVEN)
  TCC0->DRVCTRL.vec.INVEN     = 0;        // No inversion
//   TCC0->PER.reg               = LED_FULL_BRIGHTNESS; // LED_FULL_BRIGHTNESS is 255
//   while ( TCC0->SYNCBUSY.bit.PER )
//   {
//     ;
//   }
  
  // G0 CC
  TCC0->CC[0].bit.CC          = (LED_HALF_BRIGHTNESS); 
  while ( TCC0->SYNCBUSY.bit.CC0 )
  {
    ;
  }
  
  //Set Wave extension configuration
//  TCC0->WEXCTRL.bit.OTMX = 0x2; // makes no toggling of channels, channel 0 the whole way through
  
  // Enable double buffering
  TCC0->CTRLBCLR.bit.LUPD = 1;
  while ( TCC0->SYNCBUSY.bit.CTRLB )
  {
    ;
  }
  
  // Enable the TCC0
  TCC0->CTRLA.bit.ENABLE = 1;
  while ( TCC0->SYNCBUSY.bit.ENABLE )  
  {
    ;
  }  
  
 }



// updates the waveform output to be sent to PA09 (The Piezo Speaker)
void pwm_update_dc(uint64_t value)
{
  
  TCC0->CCB[0].bit.CCB = value;
  while ( TCC0->SYNCBUSY.bit.CCB0 )
  {
    ;
  }
}

void pwm_enable()
{
  // Enable the TCC0
  TCC0->CTRLA.bit.ENABLE = 1;
  while ( TCC0->SYNCBUSY.bit.ENABLE )
  {
    ;
  }
}

void pwm_disable()
{
  // Enable the TCC0
  TCC0->CTRLA.bit.ENABLE = 0;
  while ( TCC0->SYNCBUSY.bit.ENABLE )
  {
    ;
  }  
}

uint32_t pwm_adjust_freq(uint32_t note)
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
