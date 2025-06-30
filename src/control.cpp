
#include "control.h"
#include "encoders.h"

const float conversion_factor = 3.3f / (1 << 12);

button_t encoder_button   = {ENCODER_BUTTON_PIN, true, false, 0, false, false, LONG_PRESS_MS, false};
button_t reset_button     = {RESET_BUTTON_PIN,   true, false, 0, false, false, LONG_PRESS_MS, false};
buzzer_t buzz             = {0, false, false, false};
state_t activity          = {0, false, false, false, false};

static bool is_sleep_mode = false;

void control_init()
{
    // ADC init
    adc_init();
    adc_gpio_init(BATTERY);
    adc_select_input(3);

    // GPIO init
    _gpio_init(ENCODER_BUTTON_PIN);
    _gpio_init(RESET_BUTTON_PIN);
    _gpio_init(BATTERY);
    _gpio_init(I_BUS_5V);
    _gpio_init(O_BUZZER);
    _gpio_init(POW_EN);

    // Set directions
    gpio_set_dir(ENCODER_BUTTON_PIN,    GPIO_IN);
    gpio_set_dir(RESET_BUTTON_PIN,      GPIO_IN);
    gpio_set_dir(BATTERY,               GPIO_IN);
    gpio_set_dir(I_BUS_5V,              GPIO_IN);
    gpio_set_dir(O_BUZZER,              GPIO_OUT);
    gpio_set_dir(POW_EN,                GPIO_OUT);

    // Enable pull-ups where needed
    gpio_set_pulls(ENCODER_BUTTON_PIN,  true, false);   // pull-up
    gpio_set_pulls(RESET_BUTTON_PIN,    true, false);   // pull-up

    // Set initial output states
    gpio_put(POW_EN,    1); // EN DC-DC converter ON
    gpio_put(O_BUZZER,  0); // Buzzer off

    // Set initial states for buttons
    encoder_button.prev_state = gpio_get(ENCODER_BUTTON_PIN);
    reset_button.prev_state   = gpio_get(  RESET_BUTTON_PIN);
    
}

void pico_analogWrite(uint pin, uint8_t value) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap      (slice_num, 255); // 8bit resolution
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(pin), value);
    pwm_set_enabled   (slice_num, true);
}

float battery_measurement(void)
{
    return 2 * adc_read() * conversion_factor;
}

void button_update(button_t* btn) {
    bool state = gpio_get(btn->pin); // true - released, false - pushed (pull-up)
    uint32_t current_time_btn = to_ms_since_boot(get_absolute_time());

    if (!state && btn->prev_state) { // press
        btn->press_time = current_time_btn;
        btn->flag = true;
        btn->short_press = false;
        btn->long_press = false;
        btn->Fupdate = false;
        // activity.F_sleep_mode = false; // reset sleep mode flag on button press
    }
    if (state && !btn->prev_state && btn->flag) { // release
        uint32_t dt = current_time_btn - btn->press_time;
        if (dt < btn->long_press_ms) btn->short_press = true;
        btn->flag = false;
        btn->Fupdate = true;
    }
    if (!state && btn->flag && !btn->long_press &&  \
        current_time_btn - btn->press_time >= btn->long_press_ms) {
        btn->long_press = true;
        btn->flag = false;
        btn->Fupdate = true;
    }
    btn->prev_state = state;
}

void control_update(){
// ***********************************************************************************
//    NEED TO BE CALLED BEFORE display_update() because it drop encoder update flag
//        --------------------------------------------------------------------

    // button state update
    button_update(&encoder_button);
    if(encoder_button.Fupdate){
        if(encoder_button.long_press){
            if      (hand_encoder.mode == ENCODER_MODE_OFF)   {hand_encoder.mode = ENCODER_MODE_HAND;}
            else if (hand_encoder.mode == ENCODER_MODE_HAND)  {hand_encoder.mode = ENCODER_MODE_OFF;}
            else if (hand_encoder.mode == ENCODER_MODE_TARGET){hand_encoder.mode = ENCODER_MODE_HAND;}
            encoder_button.long_press = false;
        }

        if(encoder_button.short_press){
            if      (hand_encoder.mode == ENCODER_MODE_OFF)   {hand_encoder.mode = ENCODER_MODE_TARGET;}
            else if (hand_encoder.mode == ENCODER_MODE_HAND)  {hand_encoder.mode = ENCODER_MODE_OFF;}
            else if (hand_encoder.mode == ENCODER_MODE_TARGET){hand_encoder.mode = ENCODER_MODE_OFF;}
            encoder_button.short_press = false;
        }
    }

    // button state update
    button_update(&reset_button);

    if(reset_button.Fupdate)
    {
        if(reset_button.long_press){        // long click
            counter_encoder.counter = 0;    // reset counter + display update
            counter_encoder.Fupdate = 1;
            reset_button.long_press = false;
        }   
        if(reset_button.short_press){       // short click    
            reset_button.short_press = false;
        }
    }
}

void enter_sleep_mode() {
    pico_analogWrite(TFT_BL, 0);        // Turn off backlight pwm
    gpio_put(POW_EN, LOW);              // Turn off DC-DC buck

    // disable encoder irq
    gpio_set_irq_enabled(COUNT_ENCODER_CLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
    gpio_set_irq_enabled(ENCODER_CLK,       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
}

void exit_sleep_mode() {
    pico_analogWrite(TFT_BL, 200);      // Turn on backlight pwm
    gpio_put(POW_EN, HIGH);             // Turn on DC-DC buck

    // enable encoder irq
    gpio_set_irq_enabled(COUNT_ENCODER_CLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(ENCODER_CLK,       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

void control_check_activity(void){
    activity.current_vbus_state = gpio_get(I_BUS_5V);
    uint32_t curent_time_activity = to_ms_since_boot(get_absolute_time());
        
        if(activity.current_vbus_state && !activity.F_vin_5v){
            activity.F_vin_5v    = true;
            activity.F_sleep_mode = false;
        }
        // exit sleep plug in usb
        else if(!activity.current_vbus_state && activity.F_vin_5v){
            activity.F_vin_5v        = false;
            activity.last_activity_ms = curent_time_activity;
        }
        // exit sleep with buttons
        if(is_sleep_mode) {
            if(!gpio_get(ENCODER_BUTTON_PIN) || !gpio_get(RESET_BUTTON_PIN)) {
                activity.F_sleep_mode = false;
                activity.last_activity_ms = curent_time_activity;
            }
        }

        if(!activity.F_sleep_mode && !activity.F_vin_5v &&  \
            (curent_time_activity - activity.last_activity_ms > SLEEP_TIMEOUT)){
            activity.F_sleep_mode = true;
        }
    
}

void control_sleep_mode_update(void){

    if(activity.F_sleep_mode && !is_sleep_mode){
      enter_sleep_mode();
      is_sleep_mode = 1;
    }else if(!activity.F_sleep_mode && is_sleep_mode){
      exit_sleep_mode();
      is_sleep_mode = 0;
    }
}

void control_buzzer_beep(void){ 
    uint32_t beep_current_time = to_ms_since_boot(get_absolute_time());

    if((buzz.F_long || buzz.F_short) && !buzz.flag){
        buzz.beep_start = beep_current_time;
        gpio_put(O_BUZZER, 1);
        buzz.flag   = true;
    }

    if(buzz.flag){
        if(((beep_current_time - buzz.beep_start  >= SHORT_BEEP_TIME_MS) && buzz.F_short) \
        || ((beep_current_time - buzz.beep_start  >= LONG_BEEP_TIME_MS ) && buzz.F_long )){
            buzz.flag    = false;
            buzz.F_short = false;
            buzz.F_long  = false;
            gpio_put(O_BUZZER, 0);
        }
    }
}

