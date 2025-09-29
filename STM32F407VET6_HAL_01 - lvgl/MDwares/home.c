#include "home.h"
#include "lvgl.h"
#include "stdlib.h"
#include "Game_2048.h"
#include "calendar_ui.h"
#include "main.h"

LV_IMG_DECLARE(game2048);
LV_IMG_DECLARE(calendar_icon);
lv_obj_t *tv_home;
lv_obj_t *imgbtn_game2048,*imgbtn_calendar;

void home();

static void homebtn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *tg = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED && tg == imgbtn_game2048)
    {
        Game2048_Init();
		printf("Game2048 start\n");
    }
	else if(code == LV_EVENT_CLICKED && tg == imgbtn_calendar)
    {
        calendar_init();
		printf("calendar init\n");
    }
}
static void tv_home_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *tg = lv_event_get_target(e);
	printf("tv_home_event\n");
	if(code == LV_EVENT_CLICKED)
	{
		printf("clicked\n");
	}
    if(code == LV_EVENT_GESTURE)
    {
		printf("slide\n");
    }
}
void ui_init()
{
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    home();
    lv_disp_load_scr(tv_home);
}
void home()
{
    tv_home = lv_tileview_create(NULL);
	lv_obj_add_flag(tv_home, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_t *tv1 = lv_tileview_add_tile(tv_home,0,0,LV_DIR_RIGHT);
    lv_obj_t *tv2 = lv_tileview_add_tile(tv_home,1,0,LV_DIR_LEFT);
	
    imgbtn_game2048 = lv_imgbtn_create(tv1);
    lv_imgbtn_set_src(imgbtn_game2048,LV_IMGBTN_STATE_RELEASED,NULL,&game2048,NULL);
    lv_obj_set_size(imgbtn_game2048,64,64);
    lv_obj_align(imgbtn_game2048,LV_ALIGN_TOP_LEFT,10,20);

	imgbtn_calendar = lv_imgbtn_create(tv1);
    lv_imgbtn_set_src(imgbtn_calendar,LV_IMGBTN_STATE_RELEASED,NULL,&calendar_icon,NULL);
    lv_obj_set_size(imgbtn_calendar,64,64);
    lv_obj_align(imgbtn_calendar,LV_ALIGN_TOP_MID,0,20);

    static lv_style_t style_pressed;
    lv_style_init(&style_pressed);
    lv_style_set_img_recolor_opa(&style_pressed, LV_OPA_40);
    lv_style_set_img_recolor(&style_pressed, lv_color_black());
    lv_obj_add_style(imgbtn_game2048, &style_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(imgbtn_calendar, &style_pressed, LV_STATE_PRESSED);

    lv_obj_add_event_cb(tv_home,tv_home_event_cb,LV_EVENT_ALL,NULL);
    lv_obj_add_event_cb(imgbtn_game2048,homebtn_event_cb,LV_EVENT_ALL,NULL);
    lv_obj_add_event_cb(imgbtn_calendar,homebtn_event_cb,LV_EVENT_ALL,NULL);


}
