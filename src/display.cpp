#include "display.h"
#include "hardware/pwm.h"

enum { SCREENBUFFER_SIZE_PIXELS = ((TFT_WIDTH * TFT_HEIGHT / 3)) };
static lv_color_t buf0 [SCREENBUFFER_SIZE_PIXELS];
static lv_color_t buf1 [SCREENBUFFER_SIZE_PIXELS];

// TFT instance
TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT); 

static bool R_flg = 0;
static bool Y_flg = 0;
static bool S_flg = 0;

// Set tick routine needed for LVGL internal timings
uint32_t my_tick_get_cb(void) {
    return to_ms_since_boot(get_absolute_time());
}

// Display flushing
void my_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {

    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    if (LV_COLOR_16_SWAP) {
        size_t len = lv_area_get_size( area );
        lv_draw_sw_rgb565_swap( px_map, len );
    }
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);

    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();

    lv_display_flush_ready(disp);
}

void display_init(void) {
    
    // Clear buffers
    //memset(buf0, 0, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t));
    //memset(buf1, 0, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t));

    // Initialize TFT display
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.begin();
    lv_init();

    lv_display_t *disp = lv_display_create(TFT_WIDTH, TFT_HEIGHT);

    // Set up display buffers
    lv_display_set_buffers(disp,    \
                            buf0,   \
                            buf1,   \
                            SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t),  \
                            LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Set up callbacks
    lv_display_set_flush_cb(disp, my_flush_cb);
    // Set vlgl tick
    lv_tick_set_cb(my_tick_get_cb);
    // Initialize UI
    ui_init();
}

void display_bat_stat(void){

    static float old_bat_vol;
    static bool old_vbat_state;
    static float bat_vol = battery_measurement();

    if(bat_vol != old_bat_vol || activity.current_vbus_state != old_vbat_state){
        //half more battery
        if(bat_vol > 4.0){
            lv_obj_update_flag(ui_Image7, LV_OBJ_FLAG_HIDDEN,0);
            lv_obj_update_flag(ui_Image5, LV_OBJ_FLAG_HIDDEN,1);
            lv_obj_update_flag(ui_Image6, LV_OBJ_FLAG_HIDDEN,1);
            pico_analogWrite(TFT_BL, 200); 
        }
        // half less battery
        else if((bat_vol < 4.0) && (bat_vol > 3.8)){
            lv_obj_update_flag(ui_Image7, LV_OBJ_FLAG_HIDDEN,1);
            lv_obj_update_flag(ui_Image5, LV_OBJ_FLAG_HIDDEN,0);
            lv_obj_update_flag(ui_Image6, LV_OBJ_FLAG_HIDDEN,1);
            pico_analogWrite(TFT_BL, 100); 
        }
        // low battery
        else if(bat_vol < 3.8){
            lv_obj_update_flag(ui_Image7, LV_OBJ_FLAG_HIDDEN,1);
            lv_obj_update_flag(ui_Image6, LV_OBJ_FLAG_HIDDEN,0);
            lv_obj_update_flag(ui_Image5, LV_OBJ_FLAG_HIDDEN,1);
            pico_analogWrite(TFT_BL, 50); 
        }
        // full batttery
        if(bat_vol >= 4.2 || activity.current_vbus_state){
            pico_analogWrite(TFT_BL, 255);
            lv_obj_update_flag(ui_Image2, LV_OBJ_FLAG_HIDDEN,0);
            lv_obj_update_flag(ui_Image3, LV_OBJ_FLAG_HIDDEN,1);
        }
        else if(!activity.current_vbus_state){
            lv_obj_update_flag(ui_Image2, LV_OBJ_FLAG_HIDDEN,1);
            lv_obj_update_flag(ui_Image3, LV_OBJ_FLAG_HIDDEN,0);
        }
    old_bat_vol = bat_vol;
    old_vbat_state = activity.current_vbus_state;
    } 
}

void display_update(void){
    // update arc and target count in normal mode
    if(hand_encoder.Fupdate){ 
        lv_arc_set_range(ui_Arc1, 0, hand_encoder.counter * 10);
        lv_label_set_text_fmt(ui_Label2, "%d", hand_encoder.counter);
        hand_encoder.Fupdate = 0;
    }
    
    if(counter_encoder.Fupdate){

        float turns = encoder_to_turns(counter_encoder.counter);
        float hand_turns = (float)hand_encoder.counter;

        // buzzer state
        if(turns == hand_turns - 1.0){
            buzz.F_long = false;
            buzz.F_short = true;
        }

        else if(turns == hand_turns){
            buzz.F_long = true;
            buzz.F_short = false;
        }

        // update arc value and counter value in hand mode

        lv_arc_set_value(ui_Arc1, turns * 10);
        //lv_label_set_text_fmt(ui_Label1, "%d.%d", (buf_counter_count / 10), abs(buf_counter_count  % 10));
        lv_label_set_text_fmt(ui_Label1, "%2.1f", turns);

        // red circle
        if(turns >= hand_encoder.counter && !R_flg){
            lv_obj_set_style_text_color  (ui_Label1, lv_color_make(0xff, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT); 
            lv_obj_set_style_border_color(ui_Panel5, lv_color_make(0xff, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
            R_flg = 1;
            Y_flg = 0;
            S_flg = 0;
        }
        // yello
        else if((int)turns == hand_encoder.counter - 1 && !Y_flg){
            lv_obj_set_style_text_color  (ui_Label1, lv_color_make(0xee, 0xff, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(ui_Panel5, lv_color_make(0xee, 0xff, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
            R_flg = 0;
            Y_flg = 1;
            S_flg = 0;
        }
        // standart blue 4A90E2
        else if(turns < hand_encoder.counter - 1 && !S_flg){
            lv_obj_set_style_text_color  (ui_Label1, lv_color_make(0x3d, 0xd7, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(ui_Panel5, lv_color_make(0x4A, 0x90, 0xE2), LV_PART_MAIN|LV_STATE_DEFAULT);
            R_flg = 0;
            Y_flg = 0;
            S_flg = 1;
        }
        counter_encoder.Fupdate = 0; //reset flag
    }

}
