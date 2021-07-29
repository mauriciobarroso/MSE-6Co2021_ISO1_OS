/*
 * main.c
 *
 * Created on: Jul 25, 2021
 * Author: Mauricio Barroso Benavides
 */

/* inclusions ----------------------------------------------------------------*/

#include "main.h"
#include "board.h"
#include "sapi.h"
#include "os_Core.h"

/* macros --------------------------------------------------------------------*/

#define SYSTICK_TIME	1000	/**< SysTick time in us */

/* typedef -------------------------------------------------------------------*/

/* data declaration ----------------------------------------------------------*/

/* function declaration ------------------------------------------------------*/

static void init(void);

static void task1(void * arg);
static void task2(void * arg);
static void task3(void * arg);
static void task4(void * arg);

/* main ----------------------------------------------------------------------*/

int main() {
	init();

	/* OS initialization */
    os_Init();

    /* Tasks initialization */
    os_Error_t err;

    err = os_InitTask(task1, "Task 1", 1, NULL);

	if( err != OS_OK) {
		os_ErrorHandler(err);
	}

    err = os_InitTask(task2, "Task 2", 1, NULL);

	if( err != OS_OK) {
		os_ErrorHandler(err);
	}

    err = os_InitTask(task3, "Task 3", 1, NULL);

	if( err != OS_OK) {
		os_ErrorHandler(err);
	}

    err = os_InitTask(task4, "Task 4", 1, NULL);

	if( err != OS_OK) {
		os_ErrorHandler(err);
	}

	/* Infinite loop */
	for(;;) {

	}
}

/* function definition -------------------------------------------------------*/

static void init(void)  {
	Board_Init();
	boardConfig();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / SYSTICK_TIME);
}

static void task1(void * arg)  {
	uint32_t h = 0;
	uint32_t i = 0;

	while (1) {
		h++;
		i++;

		if(i == 5000000)
		{
			gpioToggle(LEDB);
			i = 0;
		}
	}
}

static void task2(void * arg)  {
	uint32_t j = 0;
	uint32_t k = 0;

	while (1) {
		j++;
		k++;
		if(j == 5000000)
		{
			gpioToggle(LED1);
			j = 0;
		}
	}
}

static void task3(void * arg)  {
	uint32_t l = 0;
	uint32_t m = 0;

	while (1) {
		l++;
		m++;
		if(l == 5000000)
		{
			gpioToggle(LED2);
			l = 0;
		}
	}
}

static void task4(void * arg)  {
	uint32_t l = 0;
	uint32_t m = 0;

	while (1) {
		l++;
		m++;
		if(l == 5000000)
		{
			gpioToggle(LED3);
			l = 0;
		}
	}
}

/* end of file ---------------------------------------------------------------*/
