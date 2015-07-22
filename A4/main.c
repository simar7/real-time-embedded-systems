/*
    FreeRTOS V7.2.0 - Copyright (C) 2012 Real Time Engineers Ltd.


    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!
    
    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?                                      *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    
    http://www.FreeRTOS.org - Documentation, training, latest information, 
    license and contact details.
    
    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell 
    the code with commercial support, indemnification, and middleware, under 
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under 
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/


/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Standard include. */
#include <stdio.h>

/* Global Constants */
#define MSEC_IN_SEC 1000
#define ONE_SEC_IN_SIM 1680000

/* Deadline arrays */
int dar[4] = {0, 4, 6, 8};

/*-----------------------------------------------------------*/

/* The ITM port is used to direct the printf() output to the serial window in 
the Keil simulator IDE. */
#define mainITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define mainITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define mainDEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define mainTRCENA          0x01000000

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the accompanying PDF application note.
 */
static void prvTaskA( void *pvParameters );
static void prvTaskB( void *pvParameters );
static void prvTaskC( void *pvParameters );

/* Task Handles */
xTaskHandle task1;
xTaskHandle task2;
xTaskHandle task3;

/*
 * Redirects the printf() output to the serial window in the Keil simulator
 * IDE.
 */
int fputc( int iChar, FILE *pxNotUsed );

/*-----------------------------------------------------------*/
/* One array position is used for each task created by this demo.  The 
variables in this array are set and cleared by the trace macros within
FreeRTOS, and displayed on the logic analyzer window within the Keil IDE -
the result of which being that the logic analyzer shows which task is
running when. */
unsigned long ulTaskNumber[ configEXPECTED_NO_RUNNING_TASKS ];

/*-----------------------------------------------------------*/
int main(void)
{
	
	xTaskCreate( prvTaskA, ( signed char * ) "Task_A", configMINIMAL_STACK_SIZE, NULL, 3, &task1 );
	xTaskCreate( prvTaskB, ( signed char * ) "Task_B", configMINIMAL_STACK_SIZE, NULL, 2, &task2 );
	xTaskCreate( prvTaskC, ( signed char * ) "Task_C", configMINIMAL_STACK_SIZE, NULL, 1, &task3 );
	
			 
	/* Start the tasks running. */
	vTaskStartScheduler();

	
	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/*
	Calculate the priorities depending upon the current state of the
	deadlines. This function is called from within each task before
	and after running the task for the given period of time.
*/
void setPri()
{
	/*
		Type 1 Conditions:
		1 > 2 > 3
		2 > 1 > 3
		3 > 2 < 1
	*/
	if (dar[1] > dar[2]) {
		if (dar[2] > dar[3]) {
			vTaskPrioritySet(task1, 1);
			vTaskPrioritySet(task2, 2);
			vTaskPrioritySet(task3, 3);
			return;
		}
	}
	
	if (dar[2] > dar[1]) {
		if (dar[1] > dar[3]) {
			vTaskPrioritySet(task2, 1);
			vTaskPrioritySet(task1, 2);
			vTaskPrioritySet(task3, 3);
			return;
		}
	}

	if (dar[3] > dar[2]) {
		if (dar[2] < dar[1]) {
			vTaskPrioritySet(task3, 1);
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task2, 2);
			return;
		}
	}

	/* 
		Type 2 conditions:
		2 > 3 > 1
		1 > 3 > 2
		3 > 1 > 2
	*/
	if (dar[2] > dar[3]) {
		if (dar[3] > dar[1]) {
			vTaskPrioritySet(task3, 2);
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task2, 1);
			return;
		}
	}
	if (dar[1] > dar[3]) {
		if (dar[3] > dar[2]) {
			vTaskPrioritySet(task3, 2);
			vTaskPrioritySet(task1, 1);
			vTaskPrioritySet(task2, 3);
			return;
		}
	}
	if (dar[3] > dar[1]) {
		if (dar[1] > dar[2]) {
			vTaskPrioritySet(task3, 1);
			vTaskPrioritySet(task1, 2);
			vTaskPrioritySet(task2, 3);
			return;
		}
	}
	/*
		Type 3 conditions (equality conditions):
		1 = 2 , 1 > 3
		1 = 2 , 1 < 3
		1 = 3 , 1 > 2
		1 = 3 , 1 < 2
		2 = 3 , 2 > 1
		2 = 3 , 2 > 1
		1 = 2 = 3
	*/
	if (dar[1] == dar[2]) {
		if (dar[1] > dar[3]) {
			vTaskPrioritySet(task1, 2);
			vTaskPrioritySet(task2, 1);
			vTaskPrioritySet(task3, 3);
			return;
		}
		if (dar[1] < dar[3]) {
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task3, 1);
			vTaskPrioritySet(task2, 2);
			return;
		}
	}
	
	if (dar[1] == dar[3]) {
		if (dar[1] > dar[2]) {
			vTaskPrioritySet(task2, 3);
			vTaskPrioritySet(task1, 2);
			vTaskPrioritySet(task3, 1);
			return;
		}
		if (dar[1] < dar[2]) {
			vTaskPrioritySet(task2, 1);
			vTaskPrioritySet(task3, 2);
			vTaskPrioritySet(task1, 3);
			return;
		}
	}

	if (dar[2] == dar[3]) {
		if (dar[2] > dar[1]) {
			vTaskPrioritySet(task3, 1);
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task2, 2);
			return;
		}
		if (dar[2] < dar[1]) {
			vTaskPrioritySet(task3, 2);
			vTaskPrioritySet(task1, 1);
			vTaskPrioritySet(task2, 3);
			return;
		}
	}

	if (dar[1] == dar[2]) {
		if (dar[2] == dar[3]) {
			vTaskPrioritySet(task3, 1);
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task2, 2);
			return;
		}
	}
	
	// if we ever reach here set them as outlined
	vTaskPrioritySet(task3, 1);
	vTaskPrioritySet(task1, 3);
	vTaskPrioritySet(task2, 2);
	
}


/*-----------------------------------------------------------*/

static void prvTaskC( void *pvParameters )
{
	portTickType xNextWakeTime;
	const portTickType xFrequency = 8 * MSEC_IN_SEC;
	long wait_counter;

	xNextWakeTime = 0;

	for( ;; )
	{
			setPri();
			for(wait_counter = 0; wait_counter < 3 * ONE_SEC_IN_SIM; wait_counter++) {}
			vTaskDelayUntil(&xNextWakeTime, xFrequency);
			dar[3] += 8;
			setPri();
	}
}
/*-----------------------------------------------------------*/


static void prvTaskB( void *pvParameters )
{
	portTickType xNextWakeTime;
	const portTickType xFrequency = 6 * MSEC_IN_SEC;
	long wait_counter;
	
	xNextWakeTime = 0;

	for( ;; )
	{			
			setPri();
			for(wait_counter = 0; wait_counter < 2 * ONE_SEC_IN_SIM; wait_counter++) {}
			vTaskDelayUntil(&xNextWakeTime, xFrequency);
			dar[2] += 6;
			setPri();
	}
}
/*-----------------------------------------------------------*/

static void prvTaskA( void *pvParameters )
{
	portTickType xNextWakeTime;
	const portTickType xFrequency = 4 * MSEC_IN_SEC;
	long wait_counter;
	
	xNextWakeTime = 0;

	for( ;; )
	{
			setPri();
			for(wait_counter = 0; wait_counter < ONE_SEC_IN_SIM; wait_counter++) {}
			vTaskDelayUntil(&xNextWakeTime, xFrequency);
			dar[1] += 4;
			setPri();
	}
}
/*-----------------------------------------------------------*/

int fputc( int iChar, FILE *pxNotUsed ) 
{
	/* Just to avoid compiler warnings. */
	( void ) pxNotUsed;

	if( mainDEMCR & mainTRCENA ) 
	{
		while( mainITM_Port32( 0 ) == 0 );
		mainITM_Port8( 0 ) = iChar;
  }

  return( iChar );
}
