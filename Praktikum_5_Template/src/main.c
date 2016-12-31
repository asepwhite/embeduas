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
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <FreeRTOS.h>
#include <stdio.h>
#include <task.h>
#include <timers.h>

#define MY_ADC    ADCA
#define MY_ADC_CH ADC_CH0
#define MY_ADC2    ADCA
#define MY_ADC2_CH ADC_CH1
#define USART_SERIAL_EXAMPLE             &USARTE0
#define USART_SERIAL_EXAMPLE_BAUDRATE    9600
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false
static char strbuf[201];
static char sendbuff[201];
static char reads[100];
static char buffarray[200];
char in = 'x';
uint16_t result = 0;
uint16_t result2 = 0;
//int usarttest = 0;
int potensiotest = 0;
int servotest = 0;
int qtouchtest = 0;
int pingtest = 0;
int score = 0;
int phase = 0;
int incremental = 0;
int distance = 0;
int door = 0;
int heap = 0;
long increment = 0;
static char strbuf[201];

static portTASK_FUNCTION_PROTO(testPotentio, p_);
static portTASK_FUNCTION_PROTO(testLight, p_);
static portTASK_FUNCTION_PROTO(testServo, p_);
static portTASK_FUNCTION_PROTO(testQtouch, p_);
static portTASK_FUNCTION_PROTO(testUsart, p_);
static portTASK_FUNCTION_PROTO(testLCD, p_);
static portTASK_FUNCTION_PROTO(resetAll,p_);
static portTASK_FUNCTION_PROTO(testUsart,p_);
static portTASK_FUNCTION_PROTO(testHeap,p_);

void vTimerCallback(){
	increment++;
}

void PWM_Init(void)
{
	/* Set output */
	PORTC.DIR |= PIN0_bm;

	/* Set Register */
	TCC0.CTRLA = (PIN2_bm) | (PIN0_bm);
	TCC0.CTRLB = (PIN4_bm) | (PIN2_bm) | (PIN1_bm);
	
	/* Set Period */
	TCC0.PER = 1000;

	/* Set Compare Register value*/
	TCC0.CCA = 375;
}

static void adc_init(void)
{
	struct adc_config adc_conf;
	struct adc_channel_config adcch_conf;

	adc_read_configuration(&MY_ADC, &adc_conf);
	adcch_read_configuration(&MY_ADC, MY_ADC_CH, &adcch_conf);

	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_OFF, ADC_RES_12,ADC_REF_VCC);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_set_clock_rate(&adc_conf, 200000UL);

	adcch_set_input(&adcch_conf, J2_PIN0, ADCCH_NEG_NONE, 1);

	adc_write_configuration(&MY_ADC, &adc_conf);
	adcch_write_configuration(&MY_ADC, MY_ADC_CH, &adcch_conf);
}

static void adc_init2(void)
{
	struct adc_config adc_conf;
	struct adc_channel_config adcch_conf;

	adc_read_configuration(&MY_ADC2, &adc_conf);
	adcch_read_configuration(&MY_ADC2, MY_ADC2_CH, &adcch_conf);

	adc_set_conversion_parameters(&adc_conf, ADC_SIGN_OFF, ADC_RES_12,ADC_REF_VCC);
	adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
	adc_set_clock_rate(&adc_conf, 200000UL);

	adcch_set_input(&adcch_conf, J3_PIN0, ADCCH_NEG_NONE, 1);

	adc_write_configuration(&MY_ADC2, &adc_conf);
	adcch_write_configuration(&MY_ADC2, MY_ADC2_CH, &adcch_conf);
}

static uint16_t adc_read(){
	uint16_t result;
	adc_enable(&MY_ADC);
	adc_start_conversion(&MY_ADC, MY_ADC_CH);
	adc_wait_for_interrupt_flag(&MY_ADC, MY_ADC_CH);
	result = adc_get_result(&MY_ADC, MY_ADC_CH);
	return result;
}

static uint16_t adc_read2(){
	uint16_t result;
	adc_enable(&MY_ADC2);
	adc_start_conversion(&MY_ADC2, MY_ADC2_CH);
	adc_wait_for_interrupt_flag(&MY_ADC2, MY_ADC2_CH);
	result = adc_get_result(&MY_ADC2, MY_ADC2_CH);
	return result;
}

void setUpSerial()
{
	USARTE0_BAUDCTRLB = 0; //memastikan BSCALE = 0
	USARTE0_BAUDCTRLA = 0x0C; // 12
	
	//Disable interrupts, just for safety
	USARTE0_CTRLA = 0;
	//8 data bits, no parity and 1 stop bit
	USARTE0_CTRLC = USART_CHSIZE_8BIT_gc;
	
	//Enable receive and transmit
	USARTE0_CTRLB = USART_TXEN_bm | USART_RXEN_bm;
}

void sendChar(char c)
{
	while( !(USARTE0_STATUS & USART_DREIF_bm) ); //Wait until DATA buffer is empty
	USARTE0_DATA = c;
}

void sendString(char *text)
{
	while(*text)
	{
		usart_putchar(USART_SERIAL_EXAMPLE, *text++);
	}
}

char receiveChar()
{
	while( !(USARTE0_STATUS & USART_RXCIF_bm) ); //Wait until receive finish
	return USARTE0_DATA;
}

void receiveString()
{
	int i = 0;
	while(1){
		char inp = usart_getchar(USART_SERIAL_EXAMPLE);
		if(inp=='\n') break;
		else reads[i++] = inp;
	}
}

int main (void)
{
	// Insert system clock initialization code here (sysclk_init()).

	
	board_init();
	
	pmic_init();
	adc_init();
	adc_init2();
	gfx_mono_init();
	ioport_set_pin_level(LCD_BACKLIGHT_ENABLE_PIN, 1);
	PORTE_OUTSET = PIN3_bm; // PC3 as TX
	PORTE_DIRSET = PIN3_bm; //TX pin as output
	//PORTE_OUTCLR = PIN2_bm; //PC2 as RX
	//PORTE_DIRCLR = PIN2_bm; //RX pin as input
	setUpSerial();
	static usart_rs232_options_t USART_SERIAL_OPTIONS = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT
	};
	usart_init_rs232(USART_SERIAL_EXAMPLE, &USART_SERIAL_OPTIONS);
	
	TimerHandle_t timerPing = xTimerCreate("tPing", 2/portTICK_PERIOD_MS, pdTRUE, (void *) 0, vTimerCallback);
	
	xTaskCreate(testPotentio,"",500,NULL,1,NULL);
	xTaskCreate(testServo,"",500,NULL,1,NULL);
	xTaskCreate(testQtouch,"",500,NULL,1,NULL);
	xTaskCreate(testLight,"",500,NULL,1,NULL);
	xTaskCreate(testUsart,"",500,NULL,1,NULL);
	xTaskCreate(testHeap,"",500,NULL,1,NULL);
	xTaskCreate(testLCD,"",500,NULL,1,NULL);
	xTaskCreate(resetAll,"",500,NULL,1,NULL);
		
	xTimerStart(timerPing, 0);
	
	vTaskStartScheduler();

	// Insert application code here, after the board has been initialized.
}
static portTASK_FUNCTION(testLCD, p_){
	ioport_set_pin_level(LCD_BACKLIGHT_ENABLE_PIN, 1);
	while(1){
		
		//print potentio
		snprintf(strbuf, sizeof(strbuf), "Read Pot : %3d",result);
		gfx_mono_draw_string(strbuf,0, 0, &sysfont);
		
		//print qtouch
		snprintf(strbuf, sizeof(strbuf), "Qtouch : %3d",qtouchtest);
		gfx_mono_draw_string(strbuf,0, 8, &sysfont);
		
		//print potentio
		snprintf(strbuf, sizeof(strbuf), "Read Lgt : %3d",result2);
		gfx_mono_draw_string(strbuf,0, 16, &sysfont);
		
		//print heap
		snprintf(strbuf, sizeof(strbuf), "Heap : %3d",heap);
		gfx_mono_draw_string(strbuf,0, 24, &sysfont);
		
		//print servo
		//snprintf(strbuf, sizeof(strbuf), "Servo : %3d",servotest);
		//gfx_mono_draw_string(strbuf,0, 24, &sysfont);
		
		//print usart
		//snprintf(strbuf, sizeof(strbuf),"%3d :%3d :%3d :%3d \n", result,servotest,qtouchtest,heap);
		//gfx_mono_draw_string(strbuf,0, 24, &sysfont);
		//sendString(strbuf);
		
		
		vTaskDelay(5/portTICK_PERIOD_MS);
	}
}
static portTASK_FUNCTION(testPotentio, p_){

	while(1){
		result = adc_read();
		if(result <= 2000){
			gpio_set_pin_low(LED2_GPIO);
			potensiotest = 1;
		}
		else {
			gpio_set_pin_high(LED2_GPIO);
			potensiotest = 0;
		}
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}
static portTASK_FUNCTION(testHeap, p_){

	while(1){
		heap = xPortGetFreeHeapSize();
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}
static portTASK_FUNCTION(testServo, p_){
	PWM_Init();
	while(1){
		if(gpio_pin_is_low(GPIO_PUSH_BUTTON_1)){
			TCC0.CCA = 150;
			gpio_set_pin_low(LED0_GPIO);
			servotest = 1;
		}
		else if(gpio_pin_is_low(GPIO_PUSH_BUTTON_2)){
			TCC0.CCA = 1;
			gpio_set_pin_high(LED0_GPIO);
			servotest = 0;
		}
		else {
			TCC0.CCA = 375;
		}
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(testQtouch, p_){
	while(1){
		if(gpio_pin_is_high(QTOUCH_BUTTON_SNSK)){
			gpio_set_pin_low(LED1_GPIO);
			qtouchtest = 1;
			//delay_ms(5000);
		}else{
			gpio_set_pin_high(LED1_GPIO);
			qtouchtest=0;
		}
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(testLight, p_){
	while(1){
		result2 = adc_read2();
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}

static portTASK_FUNCTION(testUsart, p_){
	while(1){
		
		snprintf(sendbuff, sizeof(sendbuff),"%3d :%3d :%3d :%3d \n", result,servotest,qtouchtest,heap);
		//gfx_mono_draw_string(strbuf,0, 24, &sysfont);
		sendString(sendbuff);
		
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}
static portTASK_FUNCTION(resetAll, p_){

	while(1){
		if(gpio_pin_is_low(GPIO_PUSH_BUTTON_0)){
			gpio_set_pin_high(LED0_GPIO);
			gpio_set_pin_high(LED1_GPIO);
			gpio_set_pin_high(LED2_GPIO);
			pingtest = 0;
			servotest = 0;
			potensiotest = 0;
			qtouchtest = 0;
		}
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}