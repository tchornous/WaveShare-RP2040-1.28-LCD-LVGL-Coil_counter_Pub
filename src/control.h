#ifndef H_CONTROL
#define H_CONTROL

#include "main.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

// Pin definitions
#define ENCODER_BUTTON_PIN 20
#define RESET_BUTTON_PIN   27
#define POW_EN             26
#define I_BUS_5V           16
#define O_BUZZER           17
#define BATTERY            29

// constants
#define LONG_PRESS_MS      500
#define LONG_BEEP_TIME_MS  1000
#define SHORT_BEEP_TIME_MS 200

// check activity struct
typedef struct {
    uint32_t last_activity_ms;
    bool current_vbus_state;
    bool prev_vbus_state;
    bool F_vin_5v;
    bool F_sleep_mode;
} state_t;

// buzzer struct
typedef struct {
    uint32_t beep_start;
    bool F_long;
    bool F_short;
    bool flag;
}buzzer_t;

// button struct
typedef struct {
    uint pin;
    bool prev_state;
    bool flag;
    uint32_t press_time;
    bool short_press;
    bool long_press;
    uint32_t long_press_ms;
    bool Fupdate;
} button_t;

extern button_t encoder_button;
extern button_t   reset_button;
extern buzzer_t           buzz;
extern state_t        activity;

extern void control_init(void);
extern void control_update(void);
extern void control_check_activity(void);
extern void pico_analogWrite(uint pin, uint8_t value);
extern float battery_measurement(void);
extern void button_update(button_t* btn);
extern void control_sleep_mode_update(void);
extern void control_buzzer_beep(void);

void enter_sleep_mode(void);
void exit_sleep_mode(void);

#endif // H_CONTROL