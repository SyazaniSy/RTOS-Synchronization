#include <stdio.h>
#include <conio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

/* Task Priorities */
#define TASK1_PRIORITY (tskIDLE_PRIORITY + 2)
#define TASK2_PRIORITY (tskIDLE_PRIORITY + 1)
#define TASK3_PRIORITY (tskIDLE_PRIORITY + 1)

/* Queue handle declaration */
static QueueHandle_t xQueue = NULL;

/* Define message types */
typedef enum {
    SENSOR_DATA,
    COMMAND_DATA
} MessageType;

/* Structure to hold a message with type and data */
typedef struct {
    MessageType type;
    int data;
} QueueMessage;

/* Task function prototypes */
static void Task1(void* pvParameters); // Sender Task
static void Task2(void* pvParameters); // SensorData Receiver Task
static void Task3(void* pvParameters); // CommandData Receiver Task

/*-----------------------------------------------------------*/
void main_blinky(void)
{
    /* Create a queue capable of containing 5 QueueMessage structures */
    xQueue = xQueueCreate(5, sizeof(QueueMessage));

    if (xQueue != NULL)
    {
        /* Create the tasks */
        xTaskCreate(Task1, "Task 1", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, NULL);
        xTaskCreate(Task2, "Task 2", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, NULL);
        xTaskCreate(Task3, "Task 3", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, NULL);

        /* Start the FreeRTOS scheduler */
        vTaskStartScheduler();
    }
    else
    {
        printf("Queue creation failed.\r\n");
    }

    /* Infinite loop if scheduler fails to start */
    for (;;);
}
/*-----------------------------------------------------------*/

static void Task1(void* pvParameters)
{
    /* Prevent compiler warnings */
    (void)pvParameters;

    QueueMessage message;
    int counter = 0;

    for (;;)
    {
        /* Alternate between sending SENSOR_DATA and COMMAND_DATA */
        if (counter % 2 == 0)
        {
            message.type = SENSOR_DATA;
            message.data = counter;  // Mock sensor data
            printf("Task 1: Sending Sensor Data %d to the queue.\r\n", message.data);
        }
        else
        {
            message.type = COMMAND_DATA;
            message.data = counter;  // Mock command data
            printf("Task 1: Sending Command Data %d to the queue.\r\n", message.data);
        }

        /* Send the message to the queue, waiting 100ms if the queue is full */
        if (xQueueSend(xQueue, &message, pdMS_TO_TICKS(100)) != pdPASS)
        {
            printf("Task 1: Failed to send to the queue.\r\n");
        }

        /* Increment counter and delay before the next send */
        counter++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
/*-----------------------------------------------------------*/

static void Task2(void* pvParameters)
{
    /* Prevent compiler warnings */
    (void)pvParameters;

    QueueMessage receivedMessage;

    for (;;)
    {
        /* Wait indefinitely for data from the queue */
        if (xQueueReceive(xQueue, &receivedMessage, portMAX_DELAY) == pdTRUE)
        {
            /* Process Sensor Data only */
            if (receivedMessage.type == SENSOR_DATA)
            {
                printf("Task 2: Processing Sensor Data %d from the queue.\r\n", receivedMessage.data);

                /* Simulate processing time */
                vTaskDelay(pdMS_TO_TICKS(200));
            }
        }
    }
}
/*-----------------------------------------------------------*/

static void Task3(void* pvParameters)
{
    /* Prevent compiler warnings */
    (void)pvParameters;

    QueueMessage receivedMessage;

    for (;;)
    {
        /* Wait indefinitely for data from the queue */
        if (xQueueReceive(xQueue, &receivedMessage, portMAX_DELAY) == pdTRUE)
        {
            /* Process Command Data only */
            if (receivedMessage.type == COMMAND_DATA)
            {
                printf("Task 3: Processing Command Data %d from the queue.\r\n", receivedMessage.data);

                /* Simulate processing time */
                vTaskDelay(pdMS_TO_TICKS(200));
            }
        }
    }
}
