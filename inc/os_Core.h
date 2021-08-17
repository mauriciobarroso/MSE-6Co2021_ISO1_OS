/*
 * os_Core.h
 *
 * Created on: Jul 25, 2021
 * Author: Mauricio Barroso Benavides
 */

#ifndef _OS_CORE_H_
#define _OS_CORE_H_

/* inclusions ----------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "board.h"

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* macros --------------------------------------------------------------------*/

/* Idle task*/
#define IDLE_TASK_PRIORITY	0UL			/**< Idle task default priority */
#define IDLE_TASK_ID		0xFFFFFFFF	/**< Idle task default ID */

/* SysTick */
#define SYSTICK_TIME		1000	/**< SysTick time in us */

/**/
#define STACK_SIZE_BYTES	512					/**< Stack frame size in bytes */
#define STACK_SIZE_WORDS	(STACK_SIZE_BYTES \
							/ sizeof(uint32_t))	/**< Stack frame size in words */

/* Stack frame registers positions */
#define XPSR_REG_POS		1	/**< xPSR register position in stack frame */
#define PC_REG_POS			2	/**< PC register position in stack frame */
#define LR_REG_POS			3	/**< LR register position in stack frame */
#define R12_REG_POS			4	/**< R12 register position in stack frame */
#define R3_REG_POS			5	/**< R3 register position in stack frame */
#define R2_REG_POS			6	/**< R2 register position in stack frame */
#define R1_REG_POS			7	/**< R1 register position in stack frame */
#define R0_REG_POS			8	/**< R0 register position in stack frame */
#define LR_PREV_REG_POS		9	/**< LR_PREV register position in stack frame */
#define R4_REG_POS			10	/**< R4 register position in stack frame */
#define R5_REG_POS			11	/**< R5 register position in stack frame */
#define R6_REG_POS			12	/**< R6 register position in stack frame */
#define R7_REG_POS			13	/**< R7 register position in stack frame */
#define R8_REG_POS			14	/**< R8 register position in stack frame */
#define R9_REG_POS			15	/**< R9 register position in stack frame */
#define R10_REG_POS			16	/**< R10 register position in stack frame */
#define R11_REG_POS			17	/**< R11 register position in stack frame */

/**/
#define INIT_XPSR 			1 << 24		/**< Set xPSR Thumb bit */
#define EXC_RETURN			0xFFFFFFF9	/**< EXC_RETURN value to return to thread mode, no FPU */

/**/
#define STACK_FRAME_SIZE	8	/**< Stack frame size */
#define FULL_STACKING_SIZE	17	/**< Full stack frame size */
#define TASKS_MAX			8	/**< Max number of tasks */
#define TASK_NAME_LEN		16	/**< Length of tasks names*/

/**/
#define MAX_TIME_DELAY		0xFFFFFFFF	/**< Max delay time */

/**/
#define QUEUE_SIZE_BYTES	64			/**< Queue size in bytes */

/**/
#define IRQ_NUM				53			/**< IRQ available number */

/* typedef -------------------------------------------------------------------*/
/**
 * @brief Task states.
 */
typedef enum {
	READY_STATE = 0,	/**< Task ready to run */
	RUNNING_STATE,		/**< Task currently running */
	BLOCKED_STATE,		/**< Task blocked by an OS API */
	SUSPENDED_STATE		/**< Task suspended by an OS API */
} os_TaskState_e;

/**
 * @brief OS states.
 */
typedef enum {
	FROM_RESET_STATE = 0,	/**< OS is coming from a HW reset */
	NORMAL_RUN_STATE,		/**< OS is running normally */
	IRQ_RUN_STATE,		/**< OS is running normally */
} os_State_e;

/**
 * @brief OS states.
 */
typedef enum {
	OS_FAIL = -1,	/**< OS API function fail */
	OS_OK = 0		/**< OS API function successful */
} os_Error_t;

/**
 * @brief OS task parameters.
 */
typedef struct {
	uint32_t * stack;				/**< Pointer to task stack */
	uint32_t sp;					/**< Task stack pointer */
	void * entryPoint;				/**< Pointer to code to execute */
	uint32_t priority;				/**< Task priority */
	char name[TASK_NAME_LEN + 1];	/**< Task name */
	uint32_t id;					/**< Task ID */
	os_TaskState_e state;			/**< Task state */
	uint32_t ticksBlocked;			/**< Number of ticks that the task is blocked */
} os_Task_t;

/**
 * @brief OS control parameters.
 */
typedef struct {
	os_Task_t tasksArray[TASKS_MAX];					/**< Array of tasks to execute */
	uint32_t tasksStack[TASKS_MAX][STACK_SIZE_WORDS];	/**< Array of stacks for the tasks array */
	uint8_t tasksNum;									/**< Number of tasks initialized in the tasks array */
	os_Task_t taskIdle;									/**< Idle task */
	uint32_t taskIdleStack[STACK_SIZE_WORDS];			/**< Idle task stack */
	uint32_t error;										/**< Last error occurred in the OS */
	os_State_e state;									/**< OS state */
	bool doScheduling;									/**< Flag to do the schduling proccess */
	os_Task_t * taskCurrent;							/**< Pointer to the current task running */
	os_Task_t * taskNext;								/**< Pointer to the next task to run */
	uint16_t criticalCounter;							/**< Critical section counter */
	uint32_t tickCounter;								/**< OS tick counter */
} os_t;

/**
 * @brief Semaphore control structure.
 */
typedef struct {
	os_Task_t * task;	/**< Task associated to semaphore */
	bool isGiven;		/**< Variable to detemrine if task is given */
} Semaphore_t;

/**
 * @brief queue states.
 */
typedef enum {
	QUEUE_EMPTY_STATE = 0,	/**< Queue is empty */
	QUEUE_AVAILABLE_STATE,	/**< Queue has spaces available */
	QUEUE_FULL_STATE		/**< Queue is full */
} Queue_State_e;

/**
 * @brief Queue control structure.
 */
typedef struct {
	uint8_t data[QUEUE_SIZE_BYTES];	/**< Queue data array */
	size_t size;					/**< Queue data size */
	size_t len;						/**< Queue length (number of elements) */
	size_t head;					/**< Queue index head */
	size_t tail;					/**< Queue index tail */
	Queue_State_e state;			/**< Queue state */
	os_Task_t * task;				/**< Task associated to queue */
} Queue_t;

/**
 * @brief Queue control structure.
 */
typedef struct {
	void (* handler)(void *);	/**< ISR handler */
	void * arg;					/**< ISR handler argument */
} ISR_t;

/* external data declaration -------------------------------------------------*/

/* OS API */

/**
 * @brief OS initialization function.
 * @return none
 */
os_Error_t os_Init(void);

/**
 * @brief OS task creation function.
 * @param task
 * @param name
 * @param priority
 * @param arg
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_CreateTask(void * task, const char * name, uint32_t priority, void * arg);

/**
 * @brief OS task deletion function.
 * @param id
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_DeleteTask(uint32_t id);	/* todo: implement */

/**
 * @brief OS scheduler start.
 * @return none
 */
os_Error_t os_StartScheduler(void);

/**
 * @brief OS API to force scheduling.
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_Yield(void);

/**
 * @brief OS API to enter critical sections.
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_EnterCritical(void);

/**
 * @brief OS API to exit critical sections.
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_ExitCritical(void);

/**
 * @brief OS API to install an IRQ services.
 * @param irq
 * @param isr
 * @param arg
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_InstallIRQ(LPC43XX_IRQn_Type irq, void * isr, void * arg);

/**
 * @brief OS API to exit critical sections.
 * @param irq
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_UninstallIRQ(LPC43XX_IRQn_Type irq);

/**
 * @brief OS API to delay and block task.
 * @param ticks
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_TaskDelay(uint32_t ticks);

/**
 * @brief OS API to delay and block task.
 * @param ticks
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t os_GetTickCounter(uint32_t * ticks);

/* Synchronization API */

/**
 * @brief OS API to create a semaphore (binary).
 * @param me
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t Semaphore_Init(Semaphore_t * const me);

/**
 * @brief OS API to take a binary semaphore.
 * @param me
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t Semaphore_Take(Semaphore_t * const me);

/**
 * @brief OS API to give a binary semaphore.
 * @param me
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t Semaphore_Give(Semaphore_t * const me);

/**
 * @brief OS API to create a queue.
 * @param me
 * @param size
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t Queue_Init(Queue_t * const me, size_t size);

/**
 * @brief OS API to send/write data into a queue.
 * @param me
 * @param data
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t Queue_Send(Queue_t * const me, void * data);

/**
 * @brief OS API to receive/read data from a queue.
 * @param me
 * @param data
 * @param ticks
 * @return - OS_OK: successful
 * 		   - OS_FAIL: fail
 */
os_Error_t Queue_Receive(Queue_t * const me, void * data, uint32_t ticks);

/**
 * @brief Hook de retorno de tareas
 * @details Esta funcion no deberia accederse bajo ningun concepto, porque
 * 			ninguna tarea del OS debe retornar. Si lo hace, es un comportamiento anormal
 * 			y debe ser tratado.
 * @param none
 * @return none
 */
void __attribute__((weak)) returnHook(void);

/**
 * @brief Hook de tick de sistema
 * @details Se ejecuta cada vez que se produce un tick de sistema. Es llamada desde
 * 			el handler de SysTick.
 * @param none
 * @return none
 * @warning Esta funcion debe ser lo mas corta posible porque se ejecuta dentro del
 * 			handler mencionado, por lo que tiene prioridad sobre el cambio de contexto
 * 			y otras IRQ.
 * @warning Esta funcion no debe bajo ninguna circunstancia utilizar APIs del OS dado
 * 			que podria dar lugar a un nuevo scheduling.
 */
void __attribute__((weak)) tickHook(void);

/**
 * @brief Hook de error de sistema
 * @details Esta funcion es llamada en caso de error del sistema, y puede ser utilizada
 * 			a fin de hacer debug. El puntero de la funcion que llama a errorHook es
 * 			pasado como parametro para tener informacion de quien la esta llamando, y
 * 			dentro de ella puede verse el codigo de error en la estructura de control de
 * 			sistema. Si ha de implementarse por el usuario para manejo de errores, es
 * 			importante tener en cuenta que la estructura de control solo esta disponible
 * 			dentro de este archivo.
 * @param caller Puntero a la funcion donde fue llamado errorHook. Implementado solo a
 *				 fines de trazabilidad de errores.
 * @return none.
 */
void __attribute__((weak)) errorHook(void *caller);

/**
 * @brief Tarea Idle (segundo plano)
 * @details Esta tarea se ejecuta solamente cuando todas las demas tareas estan en estado
 * 			bloqueado. Puede ser redefinida por el usuario.
 * @param none
 * @return none.
 * @warning No debe utilizarse ninguna funcion API del OS dentro de esta funcion. No debe
 * 			ser causa de un re-scheduling.
 */
void __attribute__((weak)) idleTask(void);

/* external functions declaration --------------------------------------------*/

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */

/* end of file ---------------------------------------------------------------*/

#endif /* #ifndef _OS_CORE_H_ */
