#ifndef H_DISPLAY
#define H_DISPLAY

#include "main.h"
#include "control.h"
#include "encoders.h"

//#include <lvgl.h>
//#include <TFT_eSPI.h>

/*
                                    TFT_eSPI Library settings "User_setup.h"
                                    ---------------------------------------

                                    "User_Setup_Select.h" > lcd driver
                                    ----------------------------------
// #include <User_Setups/Setup200_GC9A01.h> 
*/

// Function declarations
extern void display_init(void);
extern void display_update(void);
extern void display_bat_stat(void);

// Display flushing callback for LVGL
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *pixelmap);

// LVGL tick callback
extern uint32_t my_tick_get_cb(void);

// External declarations
extern TFT_eSPI tft;

#endif // H_DISPLAY
