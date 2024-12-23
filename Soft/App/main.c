#include "lvgl/lvgl.h"
#include "DEV_Config.h"
#include "lv_drivers/display/fbdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define DISP_HOR 240
#define DISP_VER 320
#define DISP_BUF_SIZE (DISP_HOR * DISP_VER)

/*Image declare*/ 
//LV_IMG_DECLARE(cat);

void fbdev_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);

/**
 * A meter with multiple arcs
 */

//void lv_show_img(lv_obj_t * img,const lv_img_dsc_t img_dat){
//    lv_obj_clean(img);
//    lv_img_set_src(img, &img_dat);
//    lv_obj_center(img);
//}

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    static lv_disp_t * disp;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf       = &disp_buf;
    disp_drv.flush_cb       = fbdev_flush;
    disp_drv.hor_res        = DISP_HOR;
    disp_drv.ver_res        = DISP_VER;
    disp_drv.sw_rotate      = 1;
    disp_drv.antialiasing   = 1;
    disp = lv_disp_drv_register(&disp_drv);
    lv_disp_set_rotation(disp, LV_DISP_ROT_270); //Workaround for semi-compatible 320x170 st7789 display

    /*Initialize pin*/
    DEV_ModuleInit();

    /*Two rows*/
    const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    const lv_coord_t row_dsc[] = {35, 40, LV_GRID_FR(1), 35, LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    lv_obj_t * cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_center(cont);
    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);
    lv_obj_set_style_pad_row (cont, 0, 0);
    lv_obj_set_style_pad_column (cont, 0, 0);
    lv_obj_set_style_pad_top (cont, 0, 0);
    lv_obj_set_style_pad_left (cont, 0, 0);
    lv_obj_set_style_pad_right (cont, 0, 0);
    lv_obj_set_style_pad_bottom (cont, 0, 0);
    lv_obj_set_style_border_width (cont, 0, 0);

    lv_obj_t * stat_label;
    lv_obj_t * stat_obj;
    static lv_style_t stat_style;
    lv_style_init(&stat_style);
    lv_style_set_radius(&stat_style, 0);
    lv_style_set_bg_opa(&stat_style, LV_OPA_COVER);
    lv_style_set_bg_color(&stat_style, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
    lv_style_set_border_width(&stat_style, 0);

    stat_obj = lv_obj_create(cont);
    lv_obj_add_style(stat_obj, &stat_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(stat_obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 1, 1);
    stat_label = lv_label_create(stat_obj);
    lv_label_set_text(stat_label, "STATUS");
    lv_obj_center(stat_label);

    lv_obj_t * meas_obj;
    meas_obj = lv_obj_create(cont);
    lv_obj_set_grid_cell(meas_obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 2, 1);

        /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_timer_handler();
        usleep(5000);

    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}

