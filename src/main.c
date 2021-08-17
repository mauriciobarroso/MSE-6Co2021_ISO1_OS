/*
 * main.c
 *
 * Created on: Jul 25, 2021
 * Author: Mauricio Barroso Benavides
 */

/* inclusions ----------------------------------------------------------------*/

//#include <string.h>

#include "main.h"
#include "board.h"
#include "sapi.h"
#include "os_Core.h"

/* macros --------------------------------------------------------------------*/

#define TEC1_PORT_NUM	0	/* Button 1 port number */
#define TEC1_BIT_VAL	4	/* Button 1 bit value */

#define TEC2_PORT_NUM	0	/* Button 2 port number */
#define TEC2_BIT_VAL	8	/* Button 2 bit value */

#define MILISEC			1	/* 1 ms time */

/* typedef -------------------------------------------------------------------*/

/* Structure to identify the button pressed */
typedef enum {
	B1 = 0,
	B2
} id_e;

typedef struct {
	id_e id;
	uint32_t rising;
	uint32_t falling;
} button_t;


/* Structure to control the LEDs */
typedef struct {
	gpioMap_t led;
	uint32_t time;
	uint32_t rising;
	uint32_t falling;
} led_t;

/* data declaration ----------------------------------------------------------*/

/**/
Queue_t processQueue;
Queue_t outputQueue;

/* Button instances */
button_t b1 = {0};
button_t b2 = {0};

/* function declaration ------------------------------------------------------*/

/* Initializations */
static void initBoard(void);
static os_Error_t initTasks(void);

/* Errors */
static void errorHandler(void);

/* Tasks */
static void process(void * arg);
static void output(void * arg);

/* ISR handlers */
static void gpioISR(void * arg);

/* Utils */
static char * itoa(int value, char* result, int base);

/* main ----------------------------------------------------------------------*/

int main() {
	/* Board initialization */
	initBoard();

	/* OS initialization */
    os_Init();

    /* Queues initialization */
    Queue_Init(&processQueue, sizeof(button_t));
    Queue_Init(&outputQueue, sizeof(led_t));

    /* Initializacion button instances */
    b1.id = B1;
    b2.id = B2;

    /* Install IRQ services */
    os_InstallIRQ(PIN_INT0_IRQn, gpioISR, &b1);
    os_InstallIRQ(PIN_INT1_IRQn, gpioISR, &b2);

    /* Tasks initialization */
    if(initTasks() != OS_OK) {
    	errorHandler();
    }

    /* Start scheduler */
	os_StartScheduler();

	/* Infinite loop */
	for(;;);
}
/* function definition -------------------------------------------------------*/

/* Initializations */
static void initBoard(void) {
	Board_Init();
	boardConfig();

	/* Set interrupt 0 for falling and rising edges in button 1 */
	Chip_SCU_GPIOIntPinSel(0, TEC1_PORT_NUM, TEC1_BIT_VAL);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(0));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(0));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(0));

	/* Set interrupt 0 for falling and rising edges in button 2 */
	Chip_SCU_GPIOIntPinSel(1, TEC2_PORT_NUM, TEC2_BIT_VAL);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH(1));
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH(1));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(1));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH(1));

	/* Inicializar UART_USB a 115200 baudios */
	uartConfig(UART_USB, 115200);
}

static os_Error_t initTasks(void) {
	os_Error_t err = OS_OK;

    err = os_CreateTask(process, "Task 1", IDLE_TASK_PRIORITY + 4, NULL);

	if(err != OS_OK) {
		return OS_FAIL;
	}

    err = os_CreateTask(output, "Task 2", IDLE_TASK_PRIORITY + 3, NULL);

	if(err != OS_OK) {
		return OS_FAIL;
	}

	return err;
}

/* Errors */
static void errorHandler(void) {
	for(;;);
}

/* Tasks */
static void process(void * arg) {
	button_t button;
	button_t buttons[2];
	led_t led;

	int fallingTime = 0;
	int risingTime = 0;

	for(;;) {
		/* Wait for data */
		Queue_Receive(&processQueue, &button, MAX_TIME_DELAY);

		/* Assign data to buttons array */
		switch(button.id) {
			case B1:
				buttons[0] = button;

				break;

			case B2:
				buttons[1] = button;

				break;
		}

		/* If both button were pressed, then calculate the difference
		 * between the falling edges and continue */
		if(buttons[0].falling != 0 && buttons[1].falling != 0) {
			fallingTime = buttons[0].falling - buttons[1].falling;

			/* If both button were released, then calculate the difference
			 * between the rising edges and continue */
			if(buttons[0].rising != 0 && buttons[1].rising != 0) {
				risingTime = buttons[0].rising - buttons[1].rising;

				/* Determine the case according the value of the falling
				 * and rising edges */
				if(fallingTime > 0 && risingTime > 0) {
					led.led = LEDB;
					led.time = fallingTime + risingTime;
					led.rising = risingTime;
					led.falling = fallingTime;
				}
				else if(fallingTime > 0 && risingTime < 0) {
					led.led = LED1;
					led.time = fallingTime - risingTime;
					led.rising = - risingTime;
					led.falling = fallingTime;
				}
				else if(fallingTime < 0 && risingTime > 0) {
					led.led = LED2;
					led.time = - fallingTime + risingTime;
					led.rising = risingTime;
					led.falling = - fallingTime;
				}
				else {
					led.led = LED3;
					led.time = - fallingTime - risingTime;
					led.rising = - risingTime;
					led.falling = - fallingTime;
				}

				/* Send to queue and reset buttons values */
				Queue_Send(&outputQueue, &led);

				buttons[0].falling = 0;
				buttons[0].rising = 0;
				buttons[1].falling = 0;
				buttons[1].rising = 0;
			}
		}
		else {
			/* If only a button was pressed, then reset buttons values */
			if(buttons[0].rising != 0 || buttons[1].rising != 0) {
				buttons[0].falling = 0;
				buttons[0].rising = 0;
				buttons[1].falling = 0;
				buttons[1].rising = 0;
			}
		}
	}
}

static void output(void * arg) {
	led_t led = {0};
	led_t leds[4] = {0};

	char timeValue[] = "XXXXX";
	char ledColor[] = "XXXXXXXX";
	char message[] = "\t Tiempo entre flancos descendentes: XXXXX ms \n\r";

	for(;;) {
		/* Try to get data and continue */
		Queue_Receive(&outputQueue, &led, 0);

		/* If the data was received, then store the led data and
		 * define the led color string */
		if(led.led != 0) {
			switch(led.led) {
				case LEDB:
					leds[0] = led;
					strcpy(ledColor, "Azul");

					break;
				case LED1:
					leds[1] = led;
					strcpy(ledColor, "Amarillo");

					break;
				case LED2:
					leds[2] = led;
					strcpy(ledColor, "Rojo");

					break;
				case LED3:
					leds[3] = led;
					strcpy(ledColor, "Verde");

					break;

				default:
					break;
			}

			/* Write message without sprintf() */
			strcpy(message, "Led ");
			strcat(message, ledColor);
			strcat(message, " encendido \n\r");
			uartWriteString(UART_USB, message);
			strcpy(message,"\t Tiempo encendido: ");
			itoa(led.time, timeValue, 10);
			strcat(message, timeValue);
			strcat(message, " ms \n\r");
			uartWriteString(UART_USB, message);
			strcpy(message,"\t Tiempo entre flancos descendentes: ");
			itoa(led.falling, timeValue, 10);
			strcat(message, timeValue);
			strcat(message, " ms \n\r");
			uartWriteString(UART_USB, message);
			strcpy(message,"\t Tiempo entre flancos ascendentes: ");
			itoa(led.rising, timeValue, 10);
			strcat(message, timeValue);
			strcat(message, " ms \n\r");
			uartWriteString(UART_USB, message);
		}

		/* Turn on and turn off the LEDs according the led's structures */
		for(uint8_t i = 0; i < 4; i++) {
			if(leds[i].time > 0) {
				gpioWrite(leds[i].led, true);
				leds[i].time--;

				if(leds[i].time == 0) {
					gpioWrite(leds[i].led, false);
				}
			}
		}

		/* Reset the led received value */
		led.led = 0;
		led.time = 0;

		/* Wait 1 ms */
		os_TaskDelay(MILISEC);
	}
}

static void gpioISR(void * arg) {
	button_t * button = (button_t *)arg;

	/* Get ticks count when falling and rising flanks occur */
	if(button->falling == 0) {
		os_GetTickCounter(&button->falling);
	}
	else {
		os_GetTickCounter(&button->rising);
	}

	/* Sed to queue and clear interrupt flag */
	Queue_Send(&processQueue, button);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(button->id));

	/* Reset the falling and rising counters after send to queue */
	if(button->falling != 0 && button->rising != 0) {
		button->falling = 0;
		button->rising = 0;
	}

	os_Yield();
}

/* Utils */
static char * itoa(int value, char* result, int base) {
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

/* end of file ---------------------------------------------------------------*/
