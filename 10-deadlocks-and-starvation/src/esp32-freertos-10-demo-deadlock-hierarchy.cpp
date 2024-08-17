/*
    Introduction to RTOS Part 10 - Deadlock and Starvation by Shawn Hymel
    URL: https://www.youtube.com/watch?v=hRsWi4HIENc&list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz&index=10

    Efraim Manurung, 12th August 2024
    Version 1.0
    
    Concepts from URL: https://www.digikey.nl/en/maker/projects/introduction-to-rtos-solution-to-part-10-deadlock-and-starvation/872c6a057901432e84594d79fcb2cc5d

    Demonstrate the hierarchy solution to preventing deadlock. Assign a priority
    rank to each mutex. Taks must take multiple mutexes (or semaphores) in order of priority
    every time, and release them in reverse order.
*/

#include<Arduino.h>

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0;
#else
    static const BaseType_t app_cpu = 1;
#endif

// Globals
static SemaphoreHandle_t mutex_1;
static SemaphoreHandle_t mutex_2;

//*****************************************************************************
// Tasks

// Task A (high priority)
void doTaskA(void *parameters) {

    // Loop forever
    while(1) {

        // Take mutex 1 (introduce wait to force deadlock)
        xSemaphoreTake(mutex_1, portMAX_DELAY);
        Serial.println("Task A took mutex 1");
        vTaskDelay(1 / portTICK_PERIOD_MS);

        // Take mutex 2
        xSemaphoreTake(mutex_2, portMAX_DELAY);
        Serial.println("Task A took mutex 2");

        // Critical section protected by 2 mutexes
        Serial.println("Task A doing some work");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Give back mutexes
        xSemaphoreGive(mutex_2);
        xSemaphoreGive(mutex_1);

        // Wait to let the other task execute
        Serial.println("Task A going to sleep");
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

// Task B (low priority)
void doTaskB(void *parameters) {

    // Loop forever
    while(1) {

        // Take mutex 2 (introduce wait to force deadlock)
        xSemaphoreTake(mutex_1, portMAX_DELAY);
        Serial.println("Task B took mutex 1");
        vTaskDelay(1 / portTICK_PERIOD_MS);

        // Take mutex 1
        xSemaphoreTake(mutex_2, portMAX_DELAY);
        Serial.println("Task B took mutex 2");

        // Critical section protected by 2 mutexes
        Serial.println("Task B doing some work");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Give back mutexes
        xSemaphoreGive(mutex_2);
        xSemaphoreGive(mutex_1);

        // Wait to let the other task execute
        Serial.println("Task B going to sleep");
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void setup() {

    // Configure Serial
    Serial.begin(115200);

    // Wait a moment to start (so we don't miss Serial output)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    Serial.println();
    Serial.println("---FreeRTOS Deadlock Demo Hierarchy---");

    // Create mutexes before starting tasks
    mutex_1 = xSemaphoreCreateMutex();
    mutex_2 = xSemaphoreCreateMutex();

    // Start task A (high priority)
    xTaskCreatePinnedToCore(doTaskA,
                            "Task A",
                            1024,
                            NULL,
                            2,
                            NULL,
                            app_cpu);
    
    // Start Task B (low priority)
    xTaskCreatePinnedToCore(doTaskB,
                            "Task B",
                            1024,
                            NULL,
                            1,
                            NULL,
                            app_cpu);

    // Delete "setup and loop" task
    vTaskDelete(NULL);
}

void loop() {
    // Execution should never get here
}