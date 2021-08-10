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

/* typedef -------------------------------------------------------------------*/

typedef enum {
	TASK1,
	TASK2,
	TASK3,
} id_e;

typedef struct {
	id_e id;
	int led;
} app_t;

/* data declaration ----------------------------------------------------------*/

Semaphore_t semaphore1;
Queue_t queue;

/* function declaration ------------------------------------------------------*/

/* Initializations */
static void initBoard(void);
static os_Error_t initTasks(void);

/* Errors */
static void errorHandler(void);

/* Tasks */
static void task1(void * arg);
static void task2(void * arg);
static void task3(void * arg);
static void task4(void * arg);
static void task5(void * arg);
static void task6(void * arg);
static void task7(void * arg);
static void task8(void * arg);

/* main ----------------------------------------------------------------------*/

int main() {
	/* Board initialization */
	initBoard();

	/* OS initialization */
    os_Init();

    /* Semaphores initialization */
    Semaphore_Init(&semaphore1);

    /* Queues initialization */
    Queue_Init(&queue, sizeof(app_t));

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
}

static os_Error_t initTasks(void) {
	os_Error_t err = OS_OK;

    err = os_CreateTask(task1, "Task 1", IDLE_TASK_PRIORITY + 4, NULL);

	if(err != OS_OK) {
		return OS_FAIL;
	}

    err = os_CreateTask(task2, "Task 2", IDLE_TASK_PRIORITY + 5, NULL);

	if(err != OS_OK) {
		return OS_FAIL;
	}

//    err = os_CreateTask(task3, "Task 3", IDLE_TASK_PRIORITY + 2, NULL);
//
//	if(err != OS_OK) {
//		return OS_FAIL;
//	}
//
//    err = os_CreateTask(task4, "Task 4", IDLE_TASK_PRIORITY + 1, NULL);
//
//	if(err != OS_OK) {
//		return OS_FAIL;
//	}
//
//    err = os_CreateTask(task5, "Task 5", IDLE_TASK_PRIORITY + 1, NULL);
//
//	if(err != OS_OK) {
//		return OS_FAIL;
//	}
//
//    err = os_CreateTask(task6, "Task 6", IDLE_TASK_PRIORITY + 1, NULL);
//
//	if(err != OS_OK) {
//		return OS_FAIL;
//	}
//
//    err = os_CreateTask(task7, "Task 7", IDLE_TASK_PRIORITY + 1, NULL);
//
//	if(err != OS_OK) {
//		return OS_FAIL;
//	}
//
//    err = os_CreateTask(task8, "Task 8", IDLE_TASK_PRIORITY + 1, NULL);
//
//	if(err != OS_OK) {
//		return OS_FAIL;
//	}

	return err;
}

/* Errors */
static void errorHandler(void) {
	for(;;);
}

/* Tasks */
static void task1(void * arg) {
	app_t data = {
			.id = TASK1,
			.led = LED1,
	};

	for(;;) {
		Queue_Send(&queue, &data);

		data.led++;

		if(data.led == LED3 + 1) {
			data.led = LED1;
		}

		os_TaskDelay(500);
	}
}

static void task2(void * arg) {
	app_t data;

	for(;;) {
		Queue_Receive(&queue, &data);

		gpioToggle(data.led);
	}
}

static void task3(void * arg) {
	for(;;) {
		gpioToggle(LED2);
		os_TaskDelay(2000);
	}
}

static void task4(void * arg) {
	for(;;) {
		gpioToggle(LED3);
		os_TaskDelay(4000);
	}
}

static void task5(void * arg) {
	for(;;) {
		os_TaskDelay(999);
	}
}

static void task6(void * arg) {
	for(;;) {
		os_TaskDelay(999);
	}
}

static void task7(void * arg) {
	for(;;) {
		os_TaskDelay(999);
	}
}

static void task8(void * arg) {
	for(;;) {
		os_TaskDelay(999);
	}
}

/* end of file ---------------------------------------------------------------*/
