#include "Game_2048.h"
#include "lvgl.h"
#include "stdlib.h"
#include "main.h"

#define     obj_game_width       240
#define     obj_game_height      240
#define     MATRIX_SIZE             4

#define 	obj_gamebox_font	lv_font_montserrat_16


#define     color_defaut     lv_color_hex(0XBABABA)     // 默认的颜色
#define     color_2        lv_color_hex(0XE6FFFE)        // 数字2的颜色
#define     color_4        lv_color_hex(0XCCFFEB)        // 数字4的颜色
#define     color_8        lv_color_hex(0XD7FFC9)         // 数字8的颜色
#define     color_16       lv_color_hex(0XECFFC9)       // 数字16的颜色
#define     color_32       lv_color_hex(0XFFEDCA)       // 数字32的颜色
#define     color_64       lv_color_hex(0XFFD5CA)        // 数字64的颜色
#define     color_128      lv_color_hex(0XC0E0FF)        // 数字128的颜色
#define     color_256      lv_color_hex(0XC1C3FF)       // 数字256的颜色
#define     color_512      lv_color_hex(0XE6BDFF)       // 数字512的颜色
#define     color_1024     lv_color_hex(0XDBFF2F)        // 数字1024的颜色
#define     color_2048     lv_color_hex(0X69F4FF)        // 数字2048的颜色

lv_obj_t *obj_gamebox;
lv_obj_t *gamepage;
lv_obj_t *label_score;
uint16_t score;
uint16_t game_matrix[MATRIX_SIZE][MATRIX_SIZE];
char *btnm_map[MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE];
static char *gamebtn[] = { "0","2","4","8","16","32","64","128","256","512","1024","2048"};

void Game_Init();
lv_color_t btn_set_color(int num);
void update_gamebox();
void update_btnm();
void slide_to_top();
void slide_to_bottom();
void slide_to_left();
void slide_to_right();

static void gamepage_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_DIR_TOP)
        {
            printf("LV_DIR_TOP\n");
            slide_to_top();
        }
        else if(dir == LV_DIR_BOTTOM)
        {
            printf("LV_DIR_BOTTOM\n");
            slide_to_bottom();
        }
        else if(dir == LV_DIR_LEFT)
        {
            printf("LV_DIR_LEFT\n");
            slide_to_left();
        }
        else if(dir == LV_DIR_RIGHT)
        {
            printf("LV_DIR_RIGHT\n");
            slide_to_right();
        }
    }
}
static void btn_restart_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        Game_Init();
        printf("restart\n");
    }
}
static void obj_gamebox_event_cb(lv_event_t *e)
{
    lv_event_code_t code= lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if(code == LV_EVENT_DRAW_PART_BEGIN)
    {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        if(dsc->part == LV_PART_ITEMS)
        {
            uint16_t btn_id = dsc->id;
            uint8_t btnm_x = btn_id/MATRIX_SIZE;
            uint8_t btnm_y = btn_id%MATRIX_SIZE;
            uint16_t num =(uint16_t)(1 << game_matrix[btnm_x][btnm_y]);
            dsc->rect_dsc->bg_color = btn_set_color(num);
            if(dsc->label_dsc)
            {
                dsc->label_dsc->color = lv_color_black();
            }
        }
    }
}
void Game2048_Init(void)
{
    lv_obj_t *new_screen = lv_obj_create(NULL);
    // 加载新屏幕（这会自动卸载旧屏幕）
    lv_scr_load(new_screen);
    gamepage = new_screen;
    lv_obj_set_style_bg_color(gamepage,lv_color_hex(0x474747),LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(gamepage, LV_OBJ_FLAG_SCROLLABLE);
    obj_gamebox = lv_btnmatrix_create(gamepage);
    btnm_map[4] = "\n";
    btnm_map[9] = "\n";
    btnm_map[14] = "\n";
    btnm_map[19] = "";
    Game_Init();
    lv_obj_set_style_text_font(obj_gamebox,&obj_gamebox_font,LV_STATE_DEFAULT);
    lv_btnmatrix_set_map(obj_gamebox,btnm_map);
    lv_btnmatrix_set_one_checked(obj_gamebox,true);
    lv_obj_clear_flag(obj_gamebox,LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_width(obj_gamebox,0,0);
    lv_obj_set_style_bg_opa(obj_gamebox,0,0);
    lv_obj_set_size(obj_gamebox,obj_game_width,obj_game_height);
    lv_obj_set_align(obj_gamebox,LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(obj_gamebox,LV_OBJ_FLAG_GESTURE_BUBBLE);
    label_score = lv_label_create(gamepage);
    lv_obj_align(label_score,LV_ALIGN_TOP_RIGHT,-20,20);
    lv_obj_set_style_text_font(label_score,&obj_gamebox_font,LV_STATE_DEFAULT);
    lv_label_set_text_fmt(label_score,"score:%d",score);

    lv_obj_t *btn_restart = lv_btn_create(gamepage);
    lv_obj_set_size(btn_restart,70,35);
    lv_obj_set_style_bg_color(btn_restart,lv_color_hex(0x2e7dff),LV_STATE_DEFAULT);
    lv_obj_align(btn_restart,LV_ALIGN_TOP_LEFT,20,20);
    lv_obj_t *label_restart = lv_label_create(btn_restart);
    lv_obj_center(label_restart);
    lv_label_set_text(label_restart,"restart");

    lv_obj_add_event_cb(gamepage,gamepage_event_cb,LV_EVENT_ALL,NULL);
    lv_obj_add_event_cb(obj_gamebox,obj_gamebox_event_cb,LV_EVENT_ALL,NULL);
    lv_obj_add_event_cb(btn_restart,btn_restart_event_cb,LV_EVENT_CLICKED,NULL);

}
void Game_Init()
{
    score = 0;
    for(int i=0;i<MATRIX_SIZE;i++)
    {
        for(int j=0;j<MATRIX_SIZE;j++)
        {
            btnm_map[i*5+j] = " ";
        }
    }
    for(int i=0;i<MATRIX_SIZE;i++)
    {
        for(int j=0;j<MATRIX_SIZE;j++)
        {
            game_matrix[i][j] = 0;
        }
    }
    update_gamebox();
    update_gamebox();
}
lv_color_t btn_set_color(int num)
{
    lv_color_t btn_color;
    switch(num)
    {
        case 1: btn_color = color_defaut;break;
        case 2:btn_color = color_2;break;
        case 4:btn_color = color_4;break;
        case 8:btn_color = color_8;break;
        case 16:btn_color = color_16;break;
        case 32:btn_color = color_32;break;
        case 64:btn_color = color_64;break;
        case 128:btn_color = color_128;break;
        case 256:btn_color = color_256;break;
        case 512:btn_color = color_512;break;
        case 1024:btn_color = color_1024;break;
        case 2048:btn_color = color_2048;break;
    }
    return btn_color;
}
void update_gamebox()
{
    uint16_t empty_position[MATRIX_SIZE*MATRIX_SIZE][2];
    uint8_t len = 0,pos,x,y;
    for(int i=0;i<MATRIX_SIZE;i++)
    {
        for(int j=0;j<MATRIX_SIZE;j++)
        {
            if(game_matrix[i][j] == 0)
            {
                empty_position[len][0] = i;
                empty_position[len][1] = j;
                len ++;
            }
        }
    }
    if(len > 0)
    {
        pos = rand()%len;
        x = empty_position[pos][0];
        y = empty_position[pos][1];
        game_matrix[x][y] = (rand()%2+1);
        update_btnm();
		printf("update btnm end\n");
    }
	lv_label_set_text_fmt(label_score,"socre:%d",score);
    lv_obj_invalidate(obj_gamebox); // 强制重绘
}
void update_btnm()
{
    uint8_t index = 0;
    for(int i=0;i<MATRIX_SIZE;i++)
    {
        for(int j=0;j<MATRIX_SIZE;j++)
        {
            if((index+1)%5 == 0)
                index ++;
            if(game_matrix[i][j] != 0)
                btnm_map[index] = gamebtn[game_matrix[i][j]];
            else
                btnm_map[index] = " ";
            index ++;
        }
    }
}
void slide_to_top()
{
    uint8_t slide_flag = 0,temp;
    for(int i=1;i<MATRIX_SIZE;i++)
    {
        for(int j=0;j<MATRIX_SIZE;j++)
        {
            if(game_matrix[i][j] != 0)
            {
                temp = i;
                while(temp > 0)
                {
                    if(game_matrix[temp][j] != game_matrix[temp-1][j] && game_matrix[temp-1][j] != 0)
                        break;
                    if(game_matrix[temp-1][j] != 0 && game_matrix[temp-1][j] == game_matrix[temp][j])
                    {
                        slide_flag = 1;
                        game_matrix[temp-1][j] += 1;
						score += (1 << game_matrix[temp-1][j]);
                        game_matrix[temp][j] = 0;
                    }
                    if(game_matrix[temp-1][j] == 0)
                    {
                        slide_flag = 1;
                        game_matrix[temp-1][j] = game_matrix[temp][j];
                        game_matrix[temp][j] = 0;
                    }
                    temp --;
                }
            }
        }
    }
    if(slide_flag == 1)
        update_gamebox();
}
void slide_to_bottom()
{
    uint8_t slide_flag = 0,temp;
    for(int i=MATRIX_SIZE-2;i>=0;i--)
    {
        for(int j=0;j<MATRIX_SIZE;j++)
        {
            if(game_matrix[i][j] != 0)
            {
                temp = i;
                while(temp < MATRIX_SIZE-1)
                {
                    if(game_matrix[temp][j] != game_matrix[temp+1][j] && game_matrix[temp+1][j] != 0)
                        break;
                    if(game_matrix[temp+1][j] != 0 && game_matrix[temp+1][j] == game_matrix[temp][j])
                    {
                        slide_flag = 1;
                        game_matrix[temp+1][j] += 1;
						score += (1 << game_matrix[temp+1][j]);
                        game_matrix[temp][j] = 0;
                    }
                    if(game_matrix[temp+1][j] == 0)
                    {
                        slide_flag = 1;
                        game_matrix[temp+1][j] = game_matrix[temp][j];
                        game_matrix[temp][j] = 0;
                    }
                    temp ++;
                }
            }
        }
    }
    if(slide_flag == 1)
        update_gamebox();
}
void slide_to_left()
{
    uint8_t slide_flag = 0,temp;
    for(int j=1;j<MATRIX_SIZE;j++)
    {
        for(int i=0;i<MATRIX_SIZE;i++)
        {
            if(game_matrix[i][j] != 0)
            {
                temp = j;
                while(temp > 0)
                {
                    if(game_matrix[i][temp] != game_matrix[i][temp-1] && game_matrix[i][temp-1] != 0)
                        break;
                    if(game_matrix[i][temp-1] != 0 && game_matrix[i][temp-1] == game_matrix[i][temp])
                    {
                        slide_flag = 1;
                        game_matrix[i][temp-1] += 1;
						score += (1 << game_matrix[i][temp-1]);						
                        game_matrix[i][temp] = 0;
                    }
                    if(game_matrix[i][temp-1] == 0)
                    {
                        slide_flag = 1;
                        game_matrix[i][temp-1] = game_matrix[i][temp];
                        game_matrix[i][temp] = 0;
                    }
                    temp --;
                }
            }
        }
    }
    if(slide_flag == 1)
        update_gamebox();
}
void slide_to_right()
{
    uint8_t slide_flag = 0,temp;
    for(int j=MATRIX_SIZE-2;j>=0;j--)
    {
        for(int i=0;i<MATRIX_SIZE;i++)
        {
            if(game_matrix[i][j] != 0)
            {
                temp = j;
                while(temp < MATRIX_SIZE-1)
                {
                    if(game_matrix[i][temp] != game_matrix[i][temp+1] && game_matrix[i][temp+1] != 0)
                        break;
                    if(game_matrix[i][temp+1] != 0 && game_matrix[i][temp+1] == game_matrix[i][temp])
                    {
                        slide_flag = 1;
                        game_matrix[i][temp+1] += 1;
						score += (1 << game_matrix[i][temp+1]);						
                        game_matrix[i][temp] = 0;
                    }
                    if(game_matrix[i][temp+1] == 0)
                    {
                        slide_flag = 1;
                        game_matrix[i][temp+1] = game_matrix[i][temp];
                        game_matrix[i][temp] = 0;
                    }
                    temp ++;
                }
            }
        }
    }
    if(slide_flag == 1)
        update_gamebox();
}
