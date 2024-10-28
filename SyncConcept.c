#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "timers.h"

/* Event bits */
#define SENSOR_DATA_READY_BIT (1 << 0)

/* Task priorities */
#define TASK1_PRIORITY        (tskIDLE_PRIORITY + 2)
#define TASK2_PRIORITY        (tskIDLE_PRIORITY + 1)

/* Handles */
EventGroupHandle_t xEventGroup;
SemaphoreHandle_t xBinarySemaphore;
SemaphoreHandle_t xMutex;
TimerHandle_t xTimer;

/* Shared resource */
int sharedCounter = 0;

/* Task functions */
static void Task1(void* pvParameters);
static void Task2(void* pvParameters);
static void TimerCallback(TimerHandle_t xTimer);

/*-----------------------------------------------------------*/
void main_blinky(void)
{
    /* Create the event group */
    xEventGroup = xEventGroupCreate();

    /* Create binary semaphore */
    xBinarySemaphore = xSemaphoreCreateBinary();

    /* Create mutex */
    xMutex = xSemaphoreCreateMutex();

    if (xEventGroup != NULL && xBinarySemaphore != NULL && xMutex != NULL)
    {
        /* Create tasks */
        xTaskCreate(Task1, "Task 1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, NULL);
        xTaskCreate(Task2, "Task 2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, NULL);

        /* Create a software timer that expires every 1000ms */
        xTimer = xTimerCreate("Timer", pdMS_TO_TICKS(1000), pdTRUE, (void*)0, TimerCallback);
        if (xTimer != NULL)
        {
            xTimerStart(xTimer, 0);
        }

        /* Start scheduler */
        vTaskStartScheduler();
    }
    else
    {
        printf("Initialization failed.\r\n");
    }

    /* Infinite loop if scheduler fails */
    for (;;);
}

/*-----------------------------------------------------------*/
static void Task1(void* pvParameters)
{
    for (;;)
    {
        /* Wait for the SENSOR_DATA_READY_BIT to be set by the timer */
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,
            SENSOR_DATA_READY_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        if (uxBits & SENSOR_DATA_READY_BIT)
        {
            printf("Task 1: Sensor data ready event received.\n");

            /* Access shared resource with mutex */
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdPASS)
            {
                sharedCounter++;
                printf("Task 1: Updated shared counter to %d.\n", sharedCounter);
                xSemaphoreGive(xMutex); // Release mutex
            }
        }
    }
}

/*-----------------------------------------------------------*/
static void Task2(void* pvParameters)
{
    for (;;)
    {
        /* Wait for the binary semaphore to be given */
        if (xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdPASS)
        {
            printf("Task 2: Command received and processing.\n");

            /* Access shared resource with mutex */
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdPASS)
            {
                sharedCounter += 2;
                printf("Task 2: Updated shared counter to %d.\n", sharedCounter);
                xSemaphoreGive(xMutex); // Release mutex
            }
        }
    }
}

/*-----------------------------------------------------------*/
static void TimerCallback(TimerHandle_t xTimer)
{
    /* Set event bit for Task 1 */
    printf("Timer: Triggering sensor data event.\n");
    xEventGroupSetBits(xEventGroup, SENSOR_DATA_READY_BIT);

    /* Give semaphore to Task 2 */
    xSemaphoreGive(xBinarySemaphore);
}
