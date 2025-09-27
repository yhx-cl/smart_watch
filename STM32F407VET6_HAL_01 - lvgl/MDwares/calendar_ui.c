#include "home.h"
#include "lvgl.h"
#include "stdlib.h"
#include "calendar_ui.h"
#include "main.h"

lv_obj_t *cal_page;
lv_obj_t *cal;

#define		 CALENDAR_WH		240	

void calendar_init()
{
    lv_obj_t *new_screen = lv_obj_create(NULL);
    // 加载新屏幕（这会自动卸载旧屏幕）
    lv_scr_load(new_screen);
    cal_page = new_screen;
    cal = lv_calendar_create(cal_page);
    lv_obj_set_size(cal,CALENDAR_WH,CALENDAR_WH);
    lv_obj_set_align(cal,LV_ALIGN_BOTTOM_MID);
}
