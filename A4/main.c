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
#include "timers.h"

/* Standard include. */
#include <stdio.h>

/* API Includes */
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 			1

/* Global Constants */
#define MSEC_IN_SEC 1000
#define NUM_TIMERS 3

/* An array to hold handles to the created timers. */
xTimerHandle xTimers[ NUM_TIMERS ];

/* An array to hold a count of the number of times each timer expires. */
long lExpireCounters[ NUM_TIMERS ] = { 0 };

/* Global counter to hold which timer expired */
long lArrayIndexofTimer;

/*-----------------------------------------------------------*/

/* Priorities at which the tasks are created. */
#define mainQUEUE_A_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define	mainQUEUE_B_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_C_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define mainQUEUE_SEND_FREQUENCY_MS			( 10 / portTICK_RATE_MS )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define mainQUEUE_LENGTH					( 1 )

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
void vTimerCallback( xTimerHandle pxTimer );


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
	
	long x;

	
	xTaskCreate( prvTaskA, ( signed char * ) "Task_A", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_A_TASK_PRIORITY, NULL );
	xTaskCreate( prvTaskB, ( signed char * ) "Task_B", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_B_TASK_PRIORITY, NULL );
	xTaskCreate( prvTaskC, ( signed char * ) "Task_C", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_C_TASK_PRIORITY, NULL );
	

	 /* Create then start some timers.  Starting the timers before the RTOS scheduler
	 has been started means the timers will start running immediately that
	 the RTOS scheduler starts. */
	 for( x = 0; x < NUM_TIMERS; x++ )
	 {
			 xTimers[ x ] = xTimerCreate
						(  /* Just a text name, not used by the RTOS kernel. */
									 "Timer",
									 /* The timer period in ticks. */
									 ( 100 * x ),
									 /* The timers will auto-reload themselves when they expire. */
									 pdTRUE,
									 /* Assign each timer a unique id equal to its array index. */
									 ( void * ) x,
									 /* Each timer calls the same callback when it expires. */
									 vTimerCallback
						);

			 if( xTimers[ x ] == NULL )
			 {
					 /* The timer was not created. */
			 }
			 else
			 {
					 /* Start the timer.  No block time is specified, and even if one was
					 it would be ignored because the RTOS scheduler has not yet been
					 started. */
					 if( xTimerStart( xTimers[ x ], 0 ) != pdPASS )
					 {
							 /* The timer could not be set into the Active state. */
					 }
			 }
		}
			 
	/* Start the tasks running. */
	vTaskStartScheduler();

	
	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/


 /* Define a callback function that will be used by multiple timer instances.
 The callback function does nothing but count the number of times the
 associated timer expires, and stop the timer once the timer has expired
 10 times. */
 void vTimerCallback( xTimerHandle pxTimer )
{

	 /* Optionally do something if the pxTimer parameter is NULL. */
	 configASSERT( pxTimer );

	 /* Which timer expired? */
	 lArrayIndexofTimer = ( long ) pvTimerGetTimerID( pxTimer );

	 /* Increment the number of times that pxTimer has expired. */
	 lExpireCounters[ lArrayIndexofTimer ] += 1;

}

/*-----------------------------------------------------------*/

static void prvTaskC( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequency = 8 * MSEC_IN_SEC;

	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		if ( lArrayIndexofTimer == 0) {
			vTaskDelayUntil(&xLastWakeTime, xFrequency);
		}
	}
}
/*-----------------------------------------------------------*/


static void prvTaskB( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequency = 6 * MSEC_IN_SEC;
	
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		if ( lArrayIndexofTimer == 1) {
			vTaskDelayUntil(&xLastWakeTime, xFrequency);
		}
	}
}
/*-----------------------------------------------------------*/

static void prvTaskA( void *pvParameters )
{
	portTickType xLastWakeTime;
	const portTickType xFrequency = 4 * MSEC_IN_SEC;
	
	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		if ( lArrayIndexofTimer == 2) {
			vTaskDelayUntil(&xLastWakeTime, xFrequency);
		}
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
