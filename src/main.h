#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h> //needs to TFT_eSPI library

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include "pico/time.h"


#define START_TARGET_VALUE 25
#define SLEEP_TIMEOUT 60000 // 60 seconds

typedef void (*task_func_t)(void);

typedef struct {
    task_func_t func;
    uint32_t period_ms;
    uint32_t last_run;
} task_t;

// Function declarations
void sheduler_run(void);

#endif // MAIN_H
