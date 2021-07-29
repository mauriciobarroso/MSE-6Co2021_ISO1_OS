/*
 * os_Core.h
 *
 * Created on: Jul 25, 2021
 * Author: Mauricio Barroso Benavides
 */

#ifndef _OS_CORE_H_
#define _OS_CORE_H_

/* inclusions ----------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "board.h"

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* macros --------------------------------------------------------------------*/

/**/
#define STACK_SIZE_BYTES	256					/**< Stack frame size in bytes */
#define STACK_SIZE_WORDS	(STACK_SIZE_BYTES \
							/ sizeof(uint32_t))	/**< Stack frame size in words */

/**/
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
#define STACK_FRAME_SIZE	8	/**< */
#define FULL_STACKING_SIZE	17	/**< */
#define TASKS_MAX			8	/**< */
#define TASK_NAME_LEN		16	/**< */

/* typedef -------------------------------------------------------------------*/
/**
 * @brief Task states.
 */
typedef enum {
	READY_STATE = 0,	/**<  */
	RUNNING_STATE,		/**<  */
	BLOCKED_STATE,		/**<  */
	SUSPENDED_STATE		/**<  */
} os_TaskState_e;	/* todo: write descriptions */

/**
 * @brief OS states.
 */
typedef enum {
	FROM_RESET_STATE = 0,		/**<  */
	NORMAL_RUN_STATE			/**<  */
} os_State_e;	/* todo: write descriptions */

/**
 * @brief OS states.
 */
typedef enum {
	OS_OK = 0,
	OS_FAIL = -1,
} os_Error_t;

/**
 * @brief OS task parameters.
 */
typedef struct {
	uint32_t stack[STACK_SIZE_WORDS];	/**<  */
	uint32_t sp;						/**<  */
	void * entryPoint;					/**<  */
	uint32_t priority;					/**<  */
	char name[TASK_NAME_LEN + 1];		/**<  */
	uint32_t id;						/**<  */
	os_TaskState_e state;				/**<  */
	uint32_t ticksBlocked;				/**<  */
} os_Task_t;	/* todo: define correctly data types and write descriptions */

/**
 * @brief OS control parameters.
 */
typedef struct {
	os_Task_t tasksArray[TASKS_MAX];	/**<  */
	uint8_t tasksNum;					/**<  */
	uint32_t error;						/**<  */
	uint32_t state;						/**<  */
	uint32_t doScheduling;				/**<  */
	os_Task_t * taskCurrent;			/**<  */
	os_Task_t * taskNext;				/**<  */
} os_t;	/* todo: define correctly data types and write descriptions */

/* external data declaration -------------------------------------------------*/

/**
 * @brief OS initialization function.
 * @return none
 */
os_Error_t os_Init(void);	/* todo: use OOP declaration */

/**
 * @brief OS task initialization function.
 * @param task
 * @param parameters
 * @return none
 */
os_Error_t os_InitTask(void * task, const char * name, uint32_t priority, void * arg);	/* todo: use OOP declaration and write descriptions*/

/**
 * @brief OS initialization function.
 * @return none
 */
void os_ErrorHandler(os_Error_t error);	/* todo: use OOP declaration */

/* external functions declaration --------------------------------------------*/

/* cplusplus -----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */

/* end of file ---------------------------------------------------------------*/

#endif /* #ifndef _OS_CORE_H_ */
