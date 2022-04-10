//------------------------------------------------------------------------------
//             __             __   ___  __
//     | |\ | /  ` |    |  | |  \ |__  /__`
//     | | \| \__, |___ \__/ |__/ |___ .__/
//
//------------------------------------------------------------------------------

#include "adc.h"
#include <sam.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
//      __   ___  ___         ___  __
//     |  \ |__  |__  | |\ | |__  /__`
//     |__/ |___ |    | | \| |___ .__/
//
//------------------------------------------------------------------------------

#define ADCX_GROUP (0)
#define ADCY_GROUP (1)
#define ADCX_PIN   (PIN_PA02%32)
#define ADCY_PIN (PIN_PB08%32)
#define ADCJSW_PIN (PIN_PA20%32)
#define ADCJSW_GROUP (0)
#define ADCJSW_VAL (PORT->Group[ADCJSW_GROUP].IN.reg)
#define ADCJSW_PORT (PORT_PA20)
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

uint32_t debounce_time = 0;
uint32_t prev_debounce_time = 0;
static volatile uint8_t debounce_flag = 0;
static volatile uint8_t adc_state = 0;
static volatile uint16_t x_val = 0;
static volatile uint16_t y_val = 0;


//------------------------------------------------------------------------------
//      __   __   __  ___  __  ___      __   ___  __
//     |__) |__) /  \  |  /  \  |  \ / |__) |__  /__`
//     |    |  \ \__/  |  \__/  |   |  |    |___ .__/
//
//------------------------------------------------------------------------------

void adc_sync();
void adc_debounce();

//------------------------------------------------------------------------------
//      __        __          __
//     |__) |  | |__) |    | /  `
//     |    \__/ |__) |___ | \__,
//
//------------------------------------------------------------------------------

//==============================================================================
void adc_init()
{
    // set the ADC clock to be the 48 MHz source
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_ADC) | // Generic Clock ADC
    GCLK_CLKCTRL_GEN_GCLK0     | // Generic Clock Generator 0 is source
    GCLK_CLKCTRL_CLKEN;  // enable

    // select peripheral function B
    PORT->Group[ADCY_GROUP].PMUX[ADCY_PIN/2].bit.PMUXE = PORT_PMUX_PMUXE_B_Val;
    PORT->Group[ADCX_GROUP].PMUX[ADCX_PIN/2].bit.PMUXE = PORT_PMUX_PMUXE_B_Val;
   
    // Initialize the Joystick button:
     PORT->Group[ADCJSW_GROUP].PINCFG[ADCJSW_PIN].bit.INEN = 1;
     PORT->Group[ADCJSW_GROUP].DIRCLR.reg = ADCJSW_PIN;

  
    // enable the peripheral functions
    PORT->Group[ADCY_GROUP].PINCFG[ADCY_PIN].bit.PMUXEN = 1; 
    PORT->Group[ADCX_GROUP].PINCFG[ADCX_GROUP].bit.PMUXEN = 1;
    
	// set the ADC input pin (PA01 / AIN0) as an input 
	PORT->Group[ADCY_GROUP].DIRCLR.reg = PORT_PB08;
	PORT->Group[ADCX_GROUP].DIRCLR.reg = PORT_PA02;  

	// enable the ADC 
    ADC->CTRLA.bit.ENABLE = 0;
    adc_sync();

    // set the ADC to constantly update
    ADC->CTRLB.bit.FREERUN = 0;
    adc_sync();

    // set prescaler to /16
    ADC->CTRLB.bit.PRESCALER = 0x2;
    adc_sync();
    
    ADC->INPUTCTRL.bit.MUXPOS = ADC_INPUTCTRL_MUXPOS_PIN0_Val;
    adc_sync();
    
    // Default sampling time - (1/2 CLK_ADC cycles) ---F
    ADC->SAMPCTRL.bit.SAMPLEN = 0x3F;
    ADC->SAMPCTRL.reg = 0x00;

    // set the reference voltage (set reference to 1/2) 
    // External B 0x4
    ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val;
    
    //Make ADC run during Debug
    ADC->DBGCTRL.bit.DBGRUN = 1;
    
    // Gain 1/2 
    ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
    adc_sync();
  
    // ground
    ADC->INPUTCTRL.bit.MUXNEG = 0x18;
    adc_sync();
    
    //Resolution
    ADC->CTRLB.bit.RESSEL = 0x3; // 8 bits of resolution
    adc_sync();
    
    // input scan
    //input control
    // set to 2, pin 0, 1, 2----
    ADC->INPUTCTRL.bit.INPUTSCAN = 2;
    ADC->INPUTCTRL.bit.INPUTOFFSET = 0;
    
    // Turns on interrupt for when conversion is finished
    ADC->INTENSET.bit.RESRDY = 1;
    
    NVIC_DisableIRQ(ADC_IRQn);
    NVIC_ClearPendingIRQ(ADC_IRQn);
    NVIC_SetPriority(ADC_IRQn,1);
    NVIC_EnableIRQ(ADC_IRQn);
    
    
    
    // enable the ADC
    ADC->CTRLA.bit.ENABLE = 1;
    adc_sync();

    uint16_t dummy_result = (ADC->RESULT.reg);
    adc_sync();
}


//==============================================================================
uint8_t adc_button_val()
{  
  uint8_t ret_val = 0;
  if((ADCJSW_VAL & ADCJSW_PORT) == 0)
  {
    ret_val = 1;
  }
  return ret_val;
}


//------------------------------------------------------------------------------
//      __   __              ___  ___
//     |__) |__) | \  /  /\   |  |__
//     |    |  \ |  \/  /~~\  |  |___
//
//------------------------------------------------------------------------------

//==============================================================================
void adc_sync()
{
    // wait for the register to sync
    while(ADC->STATUS.bit.SYNCBUSY)
    {
        ;
    }
}

bool buttons_get_debounce(uint64_t millis)
{
  // The state of each button
  static bool button_state;
  // The time of the last state reading for each button
  static uint64_t old_millis = 0;

  // See if it has been longer than the DEBOUNCE_TIME since we changed state
  if ((millis - old_millis) >= 20)
  {
    // Get the new button
    button_state = adc_button_val();
    // set the timer to the new value
    old_millis = millis;
  }
  else
  {
    // We only want to read once every debounce cycle,
    // just return the existing state
    ;
  }
  return button_state;
}

// Automatically resets to 0
void update_adc()
{
  ADC->SWTRIG.bit.START = 1;
  adc_sync();
  while(ADC->INTFLAG.reg & ADC_INTFLAG_RESRDY);
}

uint16_t adc_get_x()
{
  return x_val;
}

uint16_t adc_get_y()
{
  return y_val;
}


// call in main 3 times, with delay in bewtween at start only

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

void ADC_Handler()
{
  // get value
  uint16_t cur = ADC->RESULT.reg;
  adc_sync();
  
  //AIN0 is read store for x_val
  if (0 == adc_state)
  {
    x_val = cur;
    adc_state++;
  }
  
  // Throw this away!
  else if(1 == adc_state)
  {
    adc_state++;
  }
  
  else if(2 == adc_state)
  {
    y_val = cur;
    adc_state = 0;
  }
}