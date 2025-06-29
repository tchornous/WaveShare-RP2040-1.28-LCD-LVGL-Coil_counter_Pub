
#include "main.h"
#include "encoders.h"
#include "control.h"
#include "display.h"

// tasks list
task_t tasks[] = {
    { control_update,               5,    0 },
    { display_update,               5,    0 },
    { control_check_activity,       50,   0 },
    { control_buzzer_beep,          50,   0 },
    { control_sleep_mode_update,    50,   0 },
    { display_bat_stat,             1000, 0 }
};

#define TASK_COUNT (sizeof(tasks)/sizeof(tasks[0]))

// run all func from task list
void sheduler_run(void){
    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (uint8_t i = 0; i < TASK_COUNT; ++i){
        if (now - tasks[i].last_run >= tasks[i].period_ms){
            tasks[i].last_run = now;
            tasks[i].func();
        }
    }
}

int main(){
    display_init();
    control_init();
    encoders_init();

    while (1){
        sheduler_run();

        lv_timer_handler();
        sleep_ms(1);
    }
}
