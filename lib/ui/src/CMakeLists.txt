SET(SOURCES screens/ui_Screen2.c
    screens/ui_Screen1.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_aege_240x240_png.c
    images/ui_img_1366475832.c
    images/ui_img_150891397.c
    images/ui_img_446092942.c
    images/ui_img_1120861132.c
    images/ui_img_1124193684.c
    fonts/ui_font_FontHighL1.c
    fonts/ui_font_FontHighL2.c)

add_library(ui ${SOURCES})
