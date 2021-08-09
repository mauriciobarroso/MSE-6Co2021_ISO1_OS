/*
 * os_Core.c
 *
 * Created on: Jul 25, 2021
 * Author: Mauricio Barroso Benavides
 */

/* inclusions ----------------------------------------------------------------*/

#include "os_Core.h"

/* macros --------------------------------------------------------------------*/

/* typedef -------------------------------------------------------------------*/

/* internal data declaration -------------------------------------------------*/

/* external data declaration -------------------------------------------------*/

static os_t os;

/* internal functions declaration --------------------------------------------*/

static void scheduler(void);
static void setPendSV(void);
static int  comparePriorities(const void * n1vp, const void * n2vp);
static void sortTaks(os_Task_t * array, size_t n);

/* external functions definition ---------------------------------------------*/

os_Error_t os_Init(void) {
	os_Error_t err = OS_OK;

	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

	os.state = FROM_RESET_STATE;
	os.taskCurrent = NULL;
	os.taskNext = NULL;
	os.tasksNum = 0;

	/* Idle task initialization */
	os.taskIdle.stack = os.taskIdleStack;

	os.taskIdle.stack[STACK_SIZE_WORDS - XPSR_REG_POS] = INIT_XPSR;
	os.taskIdle.stack[STACK_SIZE_WORDS - PC_REG_POS] = (uint32_t)idleTask;
	os.taskIdle.stack[STACK_SIZE_WORDS - LR_REG_POS] = (uint32_t)returnHook;
	os.taskIdle.stack[STACK_SIZE_WORDS - LR_PREV_REG_POS] = EXC_RETURN;

	os.taskIdle.sp = (uint32_t)(os.taskIdle.stack + STACK_SIZE_WORDS - FULL_STACKING_SIZE);
	os.taskIdle.entryPoint = idleTask;
	os.taskIdle.priority = IDLE_TASK_PRIORITY;
	os.taskIdle.id = 0xFF;
	os.taskIdle.state = READY_STATE;

	return err;
}

/* todo: implement argument in the task */
os_Error_t os_CreateTask(void * task, const char * name, uint32_t priority, void * arg) {
	os_Error_t err = OS_OK;

	if(os.tasksNum < TASKS_MAX) {
		os.tasksArray[os.tasksNum].stack = os.tasksStack[os.tasksNum];

		os.tasksArray[os.tasksNum].stack[STACK_SIZE_WORDS - XPSR_REG_POS] = INIT_XPSR;
		os.tasksArray[os.tasksNum].stack[STACK_SIZE_WORDS - PC_REG_POS] = (uint32_t)task;
		os.tasksArray[os.tasksNum].stack[STACK_SIZE_WORDS - LR_REG_POS] = (uint32_t)returnHook;
		os.tasksArray[os.tasksNum].stack[STACK_SIZE_WORDS - LR_PREV_REG_POS] = EXC_RETURN;

		os.tasksArray[os.tasksNum].sp = (uint32_t)(os.tasksArray[os.tasksNum].stack + STACK_SIZE_WORDS - FULL_STACKING_SIZE);

		os.tasksArray[os.tasksNum].entryPoint = task;
		os.tasksArray[os.tasksNum].priority = priority;
		strncpy(os.tasksArray[os.tasksNum].name, name, strlen(name));
		os.tasksArray[os.tasksNum].id = os.tasksNum;
		os.tasksArray[os.tasksNum].state = READY_STATE;

		os.tasksNum++;
	}
	else {
		err = OS_FAIL;
		errorHook(NULL);
	}

	return err;
}

os_Error_t os_DeleteTask(uint32_t id) {
	os_Error_t err = OS_OK;

	return err;
}

os_Error_t os_StartScheduler(void) {
	os_Error_t err = OS_OK;

	sortTaks(os.tasksArray, os.tasksNum);

	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / SYSTICK_TIME);

	return err;
}

os_Error_t os_Yield(void) {
	os_Error_t err = OS_OK;

	scheduler();

	if(os.doScheduling == true) {
		setPendSV();
	}

	return err;
}

os_Error_t os_TaskDelay(uint32_t ticks) {
	os_Error_t err = OS_OK;

	if(ticks > 0) {
		os.taskCurrent->state = BLOCKED_STATE;
		os.taskCurrent->ticksBlocked = ticks;

		os_Yield();
	}


	return err;
}

os_Error_t Semaphore_CreateBinary(Semaphore_t * const me) {
	os_Error_t err = OS_OK;

	me->task = NULL;
	me->isGiven = false;

	return err;
}

os_Error_t Semaphore_Take(Semaphore_t * const me) {
	os_Error_t err = OS_OK;

	me->task = os.taskCurrent;


	if(me->isGiven == false) {
		me->task->state = BLOCKED_STATE;
		me->task->ticksBlocked = MAX_TIME_DELAY;

		os_Yield();
	}

	if(me->isGiven == true) {
		me->isGiven = false;
	}

	return err;
}

os_Error_t Semaphore_Give(Semaphore_t * const me) {
	os_Error_t err = OS_OK;

	me->isGiven = true;

	if(me->task != NULL) {
		me->task->ticksBlocked = 0;
	}

//	os_Yield();

	return err;
}

void SysTick_Handler(void) {
	/* Decrement ticks blocked in task with BLOCKED_STATE state*/
	for(size_t i = 0; i < os.tasksNum; i++) {
		if(os.tasksArray[i].state == BLOCKED_STATE) {
			if(os.tasksArray[i].ticksBlocked > 0) {
				os.tasksArray[i].ticksBlocked--;
			}

			if(os.tasksArray[i].ticksBlocked == 0) {
				os.tasksArray[i].state = READY_STATE;
			}
		}
	}

	/*
	 * Dentro del SysTick handler se llama al scheduler. Separar el scheduler de
	 * getContextoSiguiente da libertad para cambiar la politica de scheduling en cualquier
	 * estadio de desarrollo del OS. Recordar que scheduler() debe ser lo mas corto posible
	 */
	scheduler();

	if(os.doScheduling == true) {
		setPendSV();
	}

	tickHook();
}

/* Hooks */
void __attribute__((weak)) returnHook(void) {
	for(;;);
}

void __attribute__((weak)) tickHook(void) {
	__asm volatile( "nop" );
}

void __attribute__((weak)) errorHook(void *caller) {
	/*
	 * Revisar el contenido de control_OS.error para obtener informacion. Utilizar os_getError()
	 */
	for(;;);
}

void __attribute__((weak)) idleTask(void)  {
	for(;;) {
		__WFI();
	}
}

uint32_t getNextContext(uint32_t spCurrent) {
	uint32_t spNext;

	/*
	 * En la primera llamada a getContextoSiguiente, se designa que la primer tarea a ejecutar sea
	 * la tarea actual, la cual es la primer tarea inicializada y cuyo puntero de estructura fuese
	 * cargado por la funcion scheduler (observar flujo de programa). Como todas las tareas se crean
	 * en estado READY, directamente se cambia a estado RUNNING y se actualiza la variable de estado
	 * de sistema
	 */
	if(os.state == FROM_RESET_STATE) {
		spNext = os.taskCurrent->sp;

		os.taskCurrent->state = RUNNING_STATE;
		os.state = NORMAL_RUN_STATE;
	}

	/*
	 * En el caso que no sea la primera vez que se ejecuta esta funcion, se hace un cambio de contexto
	 * de manera habitual. Se guarda el MSP (sp_actual) en la variable correspondiente de la estructura
	 * de la tarea corriendo actualmente. Como a este punto no hay mas estados implementados (solamente
	 * READY y RUNNING) se pasa la tarea actual a estado READY.
	 * Se carga en la variable sp_siguiente el stack pointer de la tarea siguiente, que fue definida
	 * por el scheduler. Se actualiza la misma a estado RUNNING y se retorna al handler de PendSV
	 */
	else {
		os.taskCurrent->sp = spCurrent;

		if(os.taskCurrent->state == RUNNING_STATE) {
			os.taskCurrent->state = READY_STATE;
		}

		spNext = os.taskNext->sp;

		os.taskCurrent = os.taskNext;
		os.taskCurrent->state = RUNNING_STATE;
	}

	os.doScheduling = false;

	return spNext;
}

/* internal functions definition ---------------------------------------------*/

static void scheduler(void) {
	/* When the OS state is FROM_RESET_STATE set the first task in
	 * taskArray as taskCurrent */
	if(os.state == FROM_RESET_STATE) {
		os.taskCurrent = &os.taskIdle;
		os.taskNext = os.taskCurrent;
		os.doScheduling = true;
	}
	/* If not, search for the task with state RUNNING_STATE in taskArray
	 * and set the next task with READ_STATE state as taskNext */
	else {
		uint8_t index = os.taskCurrent->id + 1;
		uint8_t count = os.tasksNum;

		/* Search in tasks array for the next task with READY_STATE state */
		while(count > 0) {
			/* If index is the last task in the array, then reset index */
			if(index >= os.tasksNum) {
				index = 0;
			}

			/* If the task in the array is in READY_STATE state and its priority is greater than the current
			 *  task, then this will be the next task. If the current task is in BLOCKED_STATE state, then
			 * the priority is ignored */
			if(os.tasksArray[index].state == READY_STATE) {
				if(os.tasksArray[index].priority >= os.taskCurrent->priority) {
					os.taskNext = &os.tasksArray[index];
					os.doScheduling = true;
					break;
				}
				else if(os.taskCurrent->state == BLOCKED_STATE) {
					os.taskNext = &os.tasksArray[index];
					os.doScheduling = true;
					break;
				}
			}

			index++;
			count--;
		}

		/* If any task in the tasks array is in READY_STATE state, then the next task is the idle task */
		if(count == 0) {
			os.taskNext = &os.taskIdle;
			os.doScheduling = true;
		}
	}
}

static void setPendSV(void) {
	/**
	 * Se setea el bit correspondiente a la excepcion PendSV
	 */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	/**
	 * Instruction Synchronization Barrier; flushes the pipeline and ensures that
	 * all previous instructions are completed before executing new instructions
	 */
	__ISB();

	/**
	 * Data Synchronization Barrier; ensures that all memory accesses are
	 * completed before next instruction is executed
	 */
	__DSB();
}

static int  comparePriorities(const void * n1vp, const void * n2vp) {
	const os_Task_t * n1ptr = (const os_Task_t *)n1vp;
	const os_Task_t * n2ptr = (const os_Task_t *)n2vp;

	return n2ptr->priority - n1ptr->priority;
}

static void sortTaks(os_Task_t * array, size_t n) {
	qsort(array, n, sizeof(os_Task_t), comparePriorities);

	for(size_t i = 0; i < n; i++) {
		array[i].id = i;
	}
}

/* end of file ---------------------------------------------------------------*/
