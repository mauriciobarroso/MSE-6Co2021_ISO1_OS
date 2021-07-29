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

/* external functions definition ---------------------------------------------*/

os_Error_t os_Init(void) {
	os_Error_t err = OS_OK;

	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

	os.state = FROM_RESET_STATE;
	os.taskCurrent = NULL;
	os.taskNext = NULL;
	os.tasksNum = 0;

	return err;
}

/* todo: implement argument in the task */
os_Error_t os_InitTask(void * task, const char * name, uint32_t priority, void * arg) {
	os_Error_t err = OS_OK;

	if(os.tasksNum < TASKS_MAX) {
		os.tasksArray[os.tasksNum].stack[STACK_SIZE_WORDS - XPSR_REG_POS] = INIT_XPSR;
		os.tasksArray[os.tasksNum].stack[STACK_SIZE_WORDS - PC_REG_POS] = (uint32_t)task;
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
	}

	return err;
}

/* todo: implement error handler */
void os_ErrorHandler(os_Error_t error) {
	for(;;) {

	}
}


void SysTick_Handler(void)  {

	/*
	 * Dentro del SysTick handler se llama al scheduler. Separar el scheduler de
	 * getContextoSiguiente da libertad para cambiar la politica de scheduling en cualquier
	 * estadio de desarrollo del OS. Recordar que scheduler() debe ser lo mas corto posible
	 */

	scheduler();

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

	return spNext;
}

/* internal functions definition ---------------------------------------------*/

static void scheduler(void)  {
	/* When the OS state is FROM_RESET_STATE set the first task in
	 * taskArray as taskCurrent */
	if(os.state == FROM_RESET_STATE) {
		os.taskCurrent = &os.tasksArray[0];
	}
	/* If not, search for the task with state RUNNING_STATE in taskArray
	 * and set the next task with READ_STATE state as taskNext */
	else {
		uint8_t index = os.taskCurrent->id + 1;
		uint8_t count = os.tasksNum;

		/* Search for the next task with READY_STATE state */
		while(count > 0) {
			if(index == os.tasksNum) {
				index = 0;
			}

			if(os.tasksArray[index].state == READY_STATE) {
				os.taskNext = &os.tasksArray[index];
				break;
			}

			index++;
			count--;
		}
	}
}

/* end of file ---------------------------------------------------------------*/
