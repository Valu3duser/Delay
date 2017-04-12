/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * This is a bare minimum user application template.
 *
 * For documentation of the board, go \ref group_common_boards "here" for a link
 * to the board-specific documentation.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# Basic usage of on-board LED and button
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

struct adc_module adc_instance;
struct tc_module tc_instance;
struct dac_module dac_instance;

#define ADC_SAMPLES 8192

const uint16_t divisor = ADC_SAMPLES - 1;

volatile uint16_t adc_result_buffer[ADC_SAMPLES];
volatile uint16_t sample_index =0;
volatile uint16_t read_index = 0;
uint8_t count = 0;
volatile bool adc_read_done = false;

uint16_t delay = 4000;


void adc_complete_callback(struct adc_module *const module);
void configure_adc(void);
void configure_adc_callbacks(void);

void tc_callback(struct tc_module *const module_inst);
void configure_tc_callbacks(void);
void configure_tc(void);

void configure_dac_channel(void);
void configure_dac(void);

int main (void)
{
	system_init();
	
	configure_adc();
	configure_adc_callbacks();

	configure_tc();
	configure_tc_callbacks();

	 configure_dac();
	 configure_dac_channel();


	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(PIN_PB16, &config_port_pin);
	
	dac_enable(&dac_instance);
	system_interrupt_enable_global();
	 
	tc_enable(&tc_instance);
	while (1) 
	{
	//dac_chan_write(&dac_instance, DAC_CHANNEL_0, 0x0fff);
		/* Infinite loop */
	}
}
void adc_complete_callback(struct adc_module *const module)
{
	
	adc_read_done = true;
//	dac_chan_write(&dac_instance, DAC_CHANNEL_0, adc_result_buffer[sample_index]);
// 	if(delay == ADC_SAMPLES -1)
// 	{
// 		delay = 0;
// 	}
// 	else
// 	{
// 		delay++;
// 	}
}

void configure_adc(void)
{
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);
	#if (!SAML21) && (!SAML22) && (!SAMC21) && (!SAMR30)
	config_adc.gain_factor     = ADC_GAIN_FACTOR_DIV2;
	#endif
	config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV8;
	config_adc.reference       = ADC_REFERENCE_INTVCC2;//ADC_REFERENCE_INTVCC1;
	#if (SAMC21)
	config_adc.positive_input  = ADC_POSITIVE_INPUT_PIN5;
	#else
	config_adc.positive_input  = ADC_POSITIVE_INPUT_PIN6;
	#endif
	config_adc.resolution      = ADC_RESOLUTION_12BIT;
	//config_adc.freerunning = true;
	#if (SAMC21)
	adc_init(&adc_instance, ADC1, &config_adc);
	#else
	adc_init(&adc_instance, ADC, &config_adc);
	#endif
	adc_enable(&adc_instance);
	//configure_adc_callbacks();
}
void configure_adc_callbacks(void)
{
	adc_register_callback(&adc_instance, adc_complete_callback, ADC_CALLBACK_READ_BUFFER);
	adc_enable_callback(&adc_instance, ADC_CALLBACK_READ_BUFFER);
}

//gclk_0 = 4MHz 
void configure_tc(void)
{
	struct tc_config config_tc;
	tc_get_config_defaults(&config_tc);
	config_tc.wave_generation = TC_WAVE_GENERATION_MATCH_FREQ;
	config_tc.clock_source = GCLK_GENERATOR_0;
	config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV16;
	config_tc.counter_16_bit.compare_capture_channel[0] =125;
	tc_init(&tc_instance, TC3, &config_tc);
	//tc_enable(&tc_instance);
}
void configure_tc_callbacks(void)
{
	tc_register_callback(&tc_instance, tc_callback,TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&tc_instance, TC_CALLBACK_CC_CHANNEL0);
}
void tc_callback(struct tc_module *const module_inst)
{
	port_pin_toggle_output_level(PIN_PB16);
	dac_chan_write(&dac_instance, DAC_CHANNEL_0, adc_result_buffer[read_index]);
	read_index = (read_index + 1) & divisor;

	adc_read_buffer_job(&adc_instance,&adc_result_buffer[sample_index],1);
	sample_index = (sample_index+1) & divisor;
	port_pin_toggle_output_level(PIN_PB16);
}
void configure_dac(void)
{
	struct dac_config config_dac;
	dac_get_config_defaults(&config_dac);
	config_dac.reference = DAC_REFERENCE_VDDANA;
	dac_init(&dac_instance, DAC, &config_dac);
}
void configure_dac_channel(void)
{
	struct dac_chan_config config_dac_chan;
	dac_chan_get_config_defaults(&config_dac_chan);
	dac_chan_set_config(&dac_instance, DAC_CHANNEL_0, &config_dac_chan);
	dac_chan_enable(&dac_instance, DAC_CHANNEL_0);
}