#include <stdio.h>
#include <conio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* Define event bits */
#define SENSOR_DATA_READY_BIT   (1 << 0)  // Event bit 0
#define COMMAND_RECEIVED_BIT    (1 << 1)  // Event bit 1

/* Priorities of the tasks */
#define TASK1_PRIORITY          (tskIDLE_PRIORITY + 2)
#define TASK2_PRIORITY          (tskIDLE_PRIORITY + 1)
#define EVENT_GENERATOR_PRIORITY (tskIDLE_PRIORITY + 3)

/* Event group handle */
EventGroupHandle_t xEventGroup;

/* Task function prototypes */
static void Task1(void* pvParameters);
static void Task2(void* pvParameters);
static void EventGeneratorTask(void* pvParameters);

/*-----------------------------------------------------------*/
void main_blinky(void)
{
    /* Create the event group */
    xEventGroup = xEventGroupCreate();

    if (xEventGroup != NULL)
    {
        /* Create the tasks */
        xTaskCreate(Task1, "Task 1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, NULL);
        xTaskCreate(Task2, "Task 2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, NULL);
        xTaskCreate(EventGeneratorTask, "Event Generator", configMINIMAL_STACK_SIZE, NULL, EVENT_GENERATOR_PRIORITY, NULL);

        /* Start the FreeRTOS scheduler */
        vTaskStartScheduler();
    }
    else
    {
        printf("Event group creation failed.\r\n");
    }

    /* Infinite loop if scheduler fails to start */
    for (;;);
}

/*-----------------------------------------------------------*/
static void Task1(void* pvParameters)
{
    /* Prevent compiler warnings */
    (void)pvParameters;

    for (;;)
    {
        /* Wait for the SENSOR_DATA_READY_BIT */
        printf("Task 1: Waiting for sensor data...\r\n");
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,               /* Event group handle */
            SENSOR_DATA_READY_BIT,     /* Bit to wait for */
            pdTRUE,                    /* Clear bit on exit */
            pdFALSE,                   /* Wait for any bit */
            portMAX_DELAY              /* Wait indefinitely */
        );

        if (uxBits & SENSOR_DATA_READY_BIT)
        {
            /* Event received */
            printf("Task 1: Sensor data ready! Processing data...\r\n");
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

/*-----------------------------------------------------------*/
static void Task2(void* pvParameters)
{
    /* Prevent compiler warnings */
    (void)pvParameters;

    for (;;)
    {
        /* Wait for the COMMAND_RECEIVED_BIT */
        printf("Task 2: Waiting for command...\r\n");
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,               /* Event group handle */
            COMMAND_RECEIVED_BIT,      /* Bit to wait for */
            pdTRUE,                    /* Clear bit on exit */
            pdFALSE,                   /* Wait for any bit */
            portMAX_DELAY              /* Wait indefinitely */
        );

        if (uxBits & COMMAND_RECEIVED_BIT)
        {
            /* Event received */
            printf("Task 2: Command received! Executing command...\r\n");
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}

/*-----------------------------------------------------------*/
static void EventGeneratorTask(void* pvParameters)
{
    /* Prevent compiler warnings */
    (void)pvParameters;

    for (;;)
    {
        /* Toggle between setting SENSOR_DATA_READY_BIT and COMMAND_RECEIVED_BIT */
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Simulate sensor data ready */
        printf("Event Generator: Sensor data ready event set.\r\n");
        xEventGroupSetBits(xEventGroup, SENSOR_DATA_READY_BIT);

        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Simulate command received */
        printf("Event Generator: Command received event set.\r\n");
        xEventGroupSetBits(xEventGroup, COMMAND_RECEIVED_BIT);
    }
}
