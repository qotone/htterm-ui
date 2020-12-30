#include "lvgl/lvgl.h"

#include "lv_drivers/display/fbdev.h"
//#include "lv_drivers/indev/mouse_hid.h"
//#include "lv_examples/lv_apps/demo/demo.h"
#include "lvgl.h"
//#include "lv_ex_conf.h"
#include "lv_font.h"
#include "ui_fb.h"
#include "lodepng.h"
#include "lv_png.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include "keypad.h"
#include "lv_settings.h"
#include "cJSON.h"
#include <malloc.h>
#include <stdlib.h>


#define WIDGET_PAD_HEIGHT    (15)
#define WIDGET_PAD_INNER     (5)
#define COLOR_FOCUS          (0x1abc9c) // 0x19d3da
#define COLOR_MENU_BG        (0x534e52) //686d76

static pthread_t tid;
static int lv_running = 0, lv_w = 1280, lv_h = 1024;
static uint8_t menu_selected = 0x0f;
static uint8_t list_selected = 0x0f;
/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t*  g;
static lv_obj_t *tv = NULL;

static lv_obj_t *list;
static lv_obj_t *main_page;
static lv_obj_t *sub_page = NULL;
static lv_font_t *font_smallest;
static lv_font_t *font_small;
static lv_font_t *font_medium;
static lv_font_t *font_title;
static lv_style_t style_btn;
static lv_style_t style_box;
static lv_style_t style_sub_label;
static lv_style_t style_img_btn;
static lv_style_t style_scrollbar;
static lv_obj_t *label_title;
static lv_obj_t *cont_img_menu;
static lv_obj_t *btn_setting;
static lv_obj_t *btn_video;
static lv_obj_t *btn_stat;
static lv_obj_t *btn_dialnose;
static lv_obj_t *btn_cur_checked = NULL;
static lv_indev_t * indev_keypad_v;
static lv_indev_t * indev_keypad_h;


static void* lvgl_main(void* p);




/* static void selectors_create(lv_obj_t * parent); */
/* static void text_input_create(lv_obj_t * parent); */
/* static void msgbox_create(void); */


/* static void msgbox_event_cb(lv_obj_t * msgbox, lv_event_t e); */
/* static void tv_event_cb(lv_obj_t * ta, lv_event_t e); */
/* static void ta_event_cb(lv_obj_t * ta, lv_event_t e); */
/* static void kb_event_cb(lv_obj_t * kb, lv_event_t e); */
/* static lv_obj_t * add_list_btn(lv_obj_t *page, uint8_t list_id); */
static void list_menu_refresh_group(void);
static void tab_menu_refresh_group(void);
static void menu_btn_event_cb(lv_obj_t * btn, lv_event_t event);
static void list_btn_event_cb(lv_obj_t *btn, lv_event_t event);
static void sub_page_event_cb(lv_obj_t *page,lv_event_t event);
static int get_child_indexof(lv_obj_t *child,lv_obj_t *parent);



// ui pages
static void cam_setting_panel(lv_coord_t _y);

LV_IMG_DECLARE(img_lv_demo_music_list_border);



void tab_menu_refresh_group(void)
{
    lv_group_remove_all_objs(g);
    lv_indev_enable(indev_keypad_v, false);
    lv_indev_enable(indev_keypad_h,true);
    lv_obj_t * btn = lv_obj_get_child_back(cont_img_menu/* lv_page_get_scrl(list) */, NULL);
    uint32_t i = 0;
    while(btn) {
        //if(i == track_id) break;
        lv_group_add_obj(g, btn);
        if(i == 0){
            lv_group_focus_obj(btn);
        }
        i++;
        btn = lv_obj_get_child_back(cont_img_menu/* lv_page_get_scrl(list) */, btn);
    }
}

void calling_ctrl_panel()
{
    if(sub_page == NULL){
        sub_page = lv_page_create(lv_scr_act(), main_page);
        lv_obj_set_style_local_bg_color(sub_page,LV_PAGE_PART_BG,LV_STATE_DEFAULT,lv_color_hex(0x413f57));//0x4c4965

        //lv_obj_set_size(main_page, 300, lv_obj_get_height_fit(lv_scr_act()) - 20);
        lv_obj_align(sub_page, main_page, LV_ALIGN_OUT_RIGHT_TOP, 0,0);
    }

    lv_obj_t *label_ip_addr = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_ip_addr, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_ip_addr,"IP 地址");
    lv_obj_align(label_ip_addr, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 20);

    lv_obj_t *ta_ip = lv_textarea_create(sub_page, NULL);
    lv_obj_set_style_local_text_font(ta_ip, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, font_small);
    //lv_obj_add_style(dd_cam_obj, LV_DROPDOWN_PART_MAIN, &style_box);
    lv_obj_set_event_cb(ta_ip, sub_page_event_cb);
    lv_obj_align(ta_ip, label_ip_addr, LV_ALIGN_OUT_BOTTOM_LEFT, 0, LV_DPI / 20);
    lv_textarea_set_one_line(ta_ip, true);
    //lv_textarea_set_cursor_hidden(ta_ip, true);
    lv_textarea_set_placeholder_text(ta_ip, "192.168.xx.xx");
    lv_textarea_set_text(ta_ip, "");
    lv_obj_set_width_fit(ta_ip, lv_obj_get_width_fit(sub_page) - lv_obj_get_x(ta_ip));

    lv_obj_t *label_speed = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_speed, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_speed,"连接发送速率");
    lv_obj_align(label_speed, ta_ip, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_speed = lv_dropdown_create(sub_page, NULL);
    lv_obj_set_style_local_text_font(dd_speed, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_text_font(dd_speed, LV_DROPDOWN_PART_SCROLLBAR, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_text_font(dd_speed, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_pattern_opa(dd_speed, LV_DROPDOWN_PART_SCROLLBAR, LV_STATE_DEFAULT, LV_OPA_90);
    lv_obj_align(dd_speed, label_speed, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_speed, "256K\n512K\n768K\n1024K\n1280K\n1536K\n1792K\n2048K\n2304K\n2560K\n2816K\n3072K\n3328K\n3500K\n4000K\n5000K\n6000K");
    lv_obj_set_width_fit(dd_speed, lv_obj_get_width_fit(sub_page) - lv_obj_get_x(dd_speed));
    lv_obj_set_event_cb(dd_speed, sub_page_event_cb);


    lv_obj_t *label_video_mode = lv_label_create(sub_page, label_ip_addr);
    lv_label_set_static_text(label_video_mode,"视频模式");
    lv_obj_align(label_video_mode, dd_speed, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);
    lv_obj_t *dd_video_mode = lv_dropdown_create(sub_page, dd_speed);
    lv_dropdown_set_options(dd_video_mode, "缺省\nH.264\nH.265");
    lv_obj_align(dd_video_mode, label_video_mode, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_obj_set_event_cb(dd_video_mode, sub_page_event_cb);

    lv_obj_t *label_audio_mode = lv_label_create(sub_page, label_ip_addr);
    lv_label_set_static_text(label_audio_mode,"音频模式");
    lv_obj_align(label_audio_mode, dd_video_mode, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);
    lv_obj_t *dd_audio_mode = lv_dropdown_create(sub_page, dd_speed);
    lv_dropdown_set_options(dd_audio_mode, "G.711A\nG.726\nG.711U");
    lv_obj_align(dd_audio_mode, label_audio_mode, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_obj_set_event_cb(dd_audio_mode, sub_page_event_cb);


    lv_obj_t *btn_meeting = lv_btn_create(sub_page, NULL);
    lv_obj_t *label_meeting = lv_label_create(btn_meeting, NULL);
    lv_obj_set_style_local_text_font(btn_meeting, LV_BTN_PART_MAIN, LV_STATE_DEFAULT ,font_small);
    lv_label_set_text(label_meeting,"召开会议");
    lv_obj_align(btn_meeting,dd_audio_mode, LV_ALIGN_OUT_BOTTOM_LEFT, 20, 2*WIDGET_PAD_HEIGHT);
    lv_obj_set_width_fit(btn_meeting, lv_obj_get_width_fit(sub_page) - lv_obj_get_x(btn_meeting));
    //lv_obj_set_height_fit(btn_meeting,  50);

    lv_obj_t *btn_meeting_second = lv_btn_create(sub_page, NULL);
    lv_obj_t *label_second = lv_label_create(btn_meeting_second, NULL);
    lv_obj_set_style_local_text_font(btn_meeting_second, LV_BTN_PART_MAIN, LV_STATE_DEFAULT ,font_small);
    lv_label_set_text(label_second,"呼叫第二流");
    lv_obj_align(btn_meeting_second,btn_meeting, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);
    lv_obj_set_width_fit(btn_meeting_second, lv_obj_get_width_fit(sub_page) - lv_obj_get_x(btn_meeting_second));



    lv_group_remove_all_objs(g);
    lv_group_add_obj(g, ta_ip);
    lv_group_add_obj(g, dd_speed);
    lv_group_add_obj(g, dd_video_mode);
    lv_group_add_obj(g, dd_audio_mode);
    lv_group_add_obj(g, btn_meeting);
    lv_group_add_obj(g, btn_meeting_second);

}

void cam_setting_panel(lv_coord_t _y)
{

    if(sub_page == NULL){
        sub_page = lv_page_create(lv_scr_act(), main_page);
        lv_obj_set_style_local_bg_color(sub_page,LV_PAGE_PART_BG,LV_STATE_DEFAULT,lv_color_hex(0x413f57));//0x4c4965

        //lv_obj_set_size(main_page, 300, lv_obj_get_height_fit(lv_scr_act()) - 20);
        lv_obj_align(sub_page, main_page, LV_ALIGN_OUT_RIGHT_TOP, 0,0);
    }

    //printf("_y = %d\n", _y);
    //lv_obj_set_height(sub_page,lv_obj_get_height(lv_scr_act()) - _y);
    lv_obj_t *label_cam_select = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_cam_select, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_cam_select,"控制口所接摄像头");
    lv_obj_align(label_cam_select, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 20);

    lv_obj_t *dd_cam_obj = lv_dropdown_create(sub_page, NULL);
    //lv_obj_add_style(dd_cam_obj, LV_DROPDOWN_PART_MAIN, &style_box);
    lv_obj_set_event_cb(dd_cam_obj, sub_page_event_cb);

    lv_obj_set_style_local_text_font(dd_cam_obj, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_text_font(dd_cam_obj, LV_DROPDOWN_PART_SCROLLBAR, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_text_font(dd_cam_obj, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_pattern_opa(dd_cam_obj, LV_DROPDOWN_PART_SCROLLBAR, LV_STATE_DEFAULT, LV_OPA_90);
    lv_dropdown_set_options(dd_cam_obj, "视频1\n视频2\n视频3\n视频4\n视频5");
    lv_obj_align(dd_cam_obj, label_cam_select, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_obj_set_width_fit(dd_cam_obj, lv_obj_get_width_fit(sub_page) - 20);

    lv_obj_t *label_cam_speed = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_cam_speed, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_cam_speed,"发送速率");
    lv_obj_align(label_cam_speed, dd_cam_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_cam_speed = lv_dropdown_create(sub_page, dd_cam_obj);
    lv_obj_align(dd_cam_speed, label_cam_speed, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_cam_speed, "256K\n512K\n768K\n1024K\n1280K\n1536K\n1792K\n2048K\n2304K\n2560K\n2816K\n3072K\n3328K\n3500K\n4000K\n5000K\n6000K");
    lv_obj_set_event_cb(dd_cam_speed, sub_page_event_cb);

    lv_obj_t *label_cam_proto = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_cam_proto, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_cam_proto,"摄像机控制协议");
    lv_obj_align(label_cam_proto, dd_cam_speed, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_cam_proto = lv_dropdown_create(sub_page, dd_cam_obj);
    lv_obj_align(dd_cam_proto, label_cam_proto, LV_ALIGN_OUT_BOTTOM_LEFT, 0,WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_cam_proto, "VISCA\nPELCO_P\nPELCO_D\nMinrray_VHD\nUnknown");

    lv_obj_set_event_cb(dd_cam_proto, sub_page_event_cb);

    lv_obj_t *label_cam_addr = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_cam_addr, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_cam_addr,"地址码");
    lv_obj_align(label_cam_addr, dd_cam_proto, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_cam_addr = lv_dropdown_create(sub_page, dd_cam_obj);
    lv_obj_align(dd_cam_addr, label_cam_addr, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_cam_addr, "缺省\n1\n2\n3");
    lv_obj_set_event_cb(dd_cam_addr, sub_page_event_cb);

    lv_obj_t *label_cam_com = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_cam_com, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_cam_com,"摄像机控制端口");
    lv_obj_align(label_cam_com, dd_cam_addr, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_cam_com = lv_dropdown_create(sub_page, dd_cam_obj);
    lv_obj_align(dd_cam_com, label_cam_com, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_cam_com, "None\nCOM1\nCOM2\nCOM3");
    lv_obj_set_event_cb(dd_cam_com, sub_page_event_cb);

    lv_obj_t *label_cam_baud = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_cam_baud, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_cam_baud,"波特率");
    lv_obj_align(label_cam_baud, dd_cam_com, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_cam_baud = lv_dropdown_create(sub_page, dd_cam_obj);
    lv_obj_align(dd_cam_baud, label_cam_baud, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_cam_baud, "缺省\n1200\n2400\n4800\n9600\n19200\n38400\n57600\n115200");

    lv_obj_set_event_cb(dd_cam_baud, sub_page_event_cb);
    lv_obj_t *cb_add_invert = lv_checkbox_create(sub_page, NULL);
    lv_obj_set_style_local_text_font(cb_add_invert, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, font_smallest);
    lv_obj_align(cb_add_invert, dd_cam_baud, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 2 *WIDGET_PAD_HEIGHT);
    lv_checkbox_set_text_static(cb_add_invert, "启动逆光补偿");
    lv_obj_set_event_cb(cb_add_invert, sub_page_event_cb);
    lv_obj_t *cb_remote_ctrl = lv_checkbox_create(sub_page, cb_add_invert);
    lv_obj_align(cb_remote_ctrl, cb_add_invert, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);
    lv_checkbox_set_text_static(cb_remote_ctrl, "允许远端控制本地摄像机");

    lv_obj_set_event_cb(cb_remote_ctrl, sub_page_event_cb);
    lv_obj_t *cb_remote_annexq = lv_checkbox_create(sub_page, cb_add_invert);
    lv_obj_align(cb_remote_annexq, cb_remote_ctrl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);
    lv_checkbox_set_text_static(cb_remote_annexq, "远遥使用AnnexQ");

    lv_obj_set_event_cb(cb_remote_annexq, sub_page_event_cb);

    lv_group_remove_all_objs(g);
    lv_group_add_obj(g, dd_cam_obj);
    lv_group_add_obj(g, dd_cam_speed);
    lv_group_add_obj(g, dd_cam_proto);
    lv_group_add_obj(g, dd_cam_addr);
    lv_group_add_obj(g, dd_cam_com);
    lv_group_add_obj(g, dd_cam_baud);

    lv_group_add_obj(g, cb_add_invert);
    lv_group_add_obj(g, cb_remote_ctrl);
    lv_group_add_obj(g, cb_remote_annexq);
    lv_group_focus_obj(dd_cam_obj);
}


static void video_setting_panel(void)
{

    if(sub_page == NULL){
        sub_page = lv_page_create(lv_scr_act(), main_page);
        //lv_obj_set_size(main_page, 300, lv_obj_get_height_fit(lv_scr_act()) - 20);
        lv_obj_align(sub_page, main_page, LV_ALIGN_OUT_RIGHT_TOP, 0,0);
    }

    //printf("_y = %d\n", _y);
    //lv_obj_set_height(sub_page,lv_obj_get_height(lv_scr_act()) - _y);
    lv_obj_t *label_video_select = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_video_select, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_video_select,"视频输入端子");
    lv_obj_align(label_video_select, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 20);

    lv_obj_t *dd_video_select = lv_dropdown_create(sub_page, NULL);
    //lv_obj_add_style(dd_cam_obj, LV_DROPDOWN_PART_MAIN, &style_box);
    lv_obj_set_event_cb(dd_video_select, sub_page_event_cb);

    lv_obj_set_style_local_text_font(dd_video_select, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_text_font(dd_video_select, LV_DROPDOWN_PART_SCROLLBAR, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_text_font(dd_video_select, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, font_small);
    lv_obj_set_style_local_pattern_opa(dd_video_select, LV_DROPDOWN_PART_SCROLLBAR, LV_STATE_DEFAULT, LV_OPA_90);
    lv_dropdown_set_options(dd_video_select, "缺省\nUSB摄像头\nIP摄像头");
    lv_obj_align(dd_video_select, label_video_select, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_obj_set_width_fit(dd_video_select, lv_obj_get_width_fit(sub_page) - 20);

    lv_obj_t *label_input_format = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_input_format, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_input_format,"视频输入格式");
    lv_obj_align(label_input_format, dd_video_select, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_input_format = lv_dropdown_create(sub_page, dd_video_select);
    lv_obj_align(dd_input_format, label_input_format, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_input_format, "缺省\n480I(720*480)\n576I(720*576)\n480P(720*480)\n576P(720*576)\n720P50(1280*720)\n720P60(1280*720)\n1080P50(1920*1080)\n1080P60(1920*1080)\n1080P25(1920*1080)\n1080P30(1920*1080)");
    lv_obj_set_event_cb(dd_input_format, sub_page_event_cb);

    lv_obj_t *label_input_framerate = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_input_framerate, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_input_framerate,"帧率(f/s)");
    lv_obj_align(label_input_framerate, dd_input_format, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);


    lv_obj_t *sb_input_frame_rate = lv_spinbox_create(sub_page, NULL);
    lv_obj_align(sb_input_frame_rate, label_input_framerate, LV_ALIGN_OUT_BOTTOM_LEFT, 0,WIDGET_PAD_INNER);
    lv_spinbox_set_range(sb_input_frame_rate, 1, 60);

    lv_obj_set_event_cb(sb_input_frame_rate, sub_page_event_cb);

    lv_obj_t *label_encode_format = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_encode_format, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_encode_format,"视频编码格式");
    lv_obj_align(label_encode_format, sb_input_frame_rate, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_encode_format = lv_dropdown_create(sub_page, dd_video_select);
    lv_obj_align(dd_encode_format, label_encode_format, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_encode_format, "缺省\nH.264\nH.265");
    lv_obj_set_event_cb(dd_encode_format, sub_page_event_cb);

    lv_obj_t *label_output_select = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_output_select, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_output_select,"视频输出端子");
    lv_obj_align(label_output_select, dd_encode_format, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_output_select = lv_dropdown_create(sub_page, dd_video_select);
    lv_obj_align(dd_output_select, label_output_select, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_output_select, "缺省\nHDMI1(高清)\nHDMI2(高清)");
    lv_obj_set_event_cb(dd_output_select, sub_page_event_cb);

    lv_obj_t *label_output_format = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_output_format, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_output_format,"视频输出格式");
    lv_obj_align(label_output_format, dd_output_select, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *dd_output_format = lv_dropdown_create(sub_page, dd_video_select);
    lv_obj_align(dd_output_format, label_output_format, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_dropdown_set_options(dd_output_format, "缺省\n480I(720*480)\n576I(720*576)\n480P(720*480)\n576P(720*576)\n720P50(1280*720)\n720P60(1280*720)\n1080P50(1920*1080)\n1080P60(1920*1080)\n1080P25(1920*1080)\n1080P30(1920*1080)");
    lv_obj_set_event_cb(dd_output_format, sub_page_event_cb);


    lv_obj_t *label_output_brightness = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_output_brightness, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_output_brightness,"亮度");
    lv_obj_align(label_output_brightness, dd_output_format, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *slider_brightness = lv_slider_create(sub_page, NULL);
    lv_obj_align(slider_brightness, label_output_brightness, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_obj_set_event_cb(slider_brightness, sub_page_event_cb);

    lv_obj_t *label_output_chroma = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_output_chroma, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_output_chroma,"色度");
    lv_obj_align(label_output_chroma, slider_brightness, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *slider_chroma = lv_slider_create(sub_page, NULL);
    lv_obj_align(slider_chroma, label_output_chroma, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_obj_set_event_cb(slider_chroma, sub_page_event_cb);

    lv_obj_t *label_output_saturability = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_output_saturability, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_output_saturability,"饱和度");
    lv_obj_align(label_output_saturability, slider_chroma, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *slider_saturablity = lv_slider_create(sub_page, NULL);
    lv_obj_align(slider_saturablity, label_output_saturability, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_obj_set_event_cb(slider_saturablity, sub_page_event_cb);

    lv_obj_t *label_output_contrast = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_output_contrast, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_output_contrast,"对比度");
    lv_obj_align(label_output_contrast, slider_saturablity, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);

    lv_obj_t *slider_contrast = lv_slider_create(sub_page, NULL);
    lv_obj_align(slider_contrast, label_output_contrast, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER);
    lv_obj_set_event_cb(slider_contrast, sub_page_event_cb);

    lv_obj_t *label_bitrate = lv_label_create(sub_page, NULL);
    lv_obj_add_style(label_bitrate, LV_LABEL_PART_MAIN, &style_sub_label);
    lv_label_set_static_text(label_bitrate,"码率");
    lv_obj_align(label_bitrate, slider_contrast, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT);


    lv_obj_t *spinbox_bitrate = lv_spinbox_create(sub_page, NULL);
    lv_obj_align(spinbox_bitrate, label_bitrate, LV_ALIGN_OUT_BOTTOM_LEFT, 0,WIDGET_PAD_INNER);
    lv_spinbox_set_range(spinbox_bitrate, 1, 15000);

    lv_obj_set_event_cb(spinbox_bitrate, sub_page_event_cb);

// for  test input
    /* lv_obj_t *label_test = lv_label_create(sub_page, label_bitrate); */
    /* lv_label_set_static_text(label_test,"Test"); */
    /* lv_obj_align(label_test, spinbox_bitrate, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_HEIGHT); */
    /* lv_obj_t *text_test = lv_textarea_create(sub_page, NULL); */
    /* lv_obj_align(text_test, label_test, LV_ALIGN_OUT_BOTTOM_LEFT, 0, WIDGET_PAD_INNER); */
    /* lv_obj_set_event_cb(text_test, sub_page_event_cb); */



    lv_group_remove_all_objs(g);
    lv_group_add_obj(g, dd_video_select);
    lv_group_add_obj(g, dd_input_format);
    lv_group_add_obj(g, sb_input_frame_rate);
    lv_group_add_obj(g, dd_encode_format);
    lv_group_add_obj(g, dd_output_select);
    lv_group_add_obj(g, dd_output_format);

    lv_group_add_obj(g, slider_brightness);
    lv_group_add_obj(g, slider_chroma);
    lv_group_add_obj(g, slider_saturablity);
    lv_group_add_obj(g, slider_contrast);
    lv_group_add_obj(g, spinbox_bitrate);
    //lv_group_add_obj(g, text_test);
    lv_group_focus_obj(dd_video_select);

#if 0
    lv_obj_t *cb_add_invert = lv_checkbox_create(sub_page, NULL);
    lv_obj_set_style_local_text_font(cb_add_invert, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, font_smallest);
    lv_obj_align(cb_add_invert, dd_cam_baud, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 20);
    lv_checkbox_set_text_static(cb_add_invert, "启动逆光补偿");
    lv_obj_set_event_cb(cb_add_invert, sub_page_event_cb);
    lv_obj_t *cb_remote_ctrl = lv_checkbox_create(sub_page, cb_add_invert);
    lv_obj_align(cb_remote_ctrl, cb_add_invert, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_checkbox_set_text_static(cb_remote_ctrl, "允许远端控制本地摄像机");

    lv_obj_set_event_cb(cb_remote_ctrl, sub_page_event_cb);
    lv_obj_t *cb_remote_annexq = lv_checkbox_create(sub_page, cb_add_invert);
    lv_obj_align(cb_remote_annexq, cb_remote_ctrl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_checkbox_set_text_static(cb_remote_annexq, "远遥使用AnnexQ");

    lv_obj_set_event_cb(cb_remote_annexq, sub_page_event_cb);
#endif
}

static void sub_page_event_cb(lv_obj_t *page,lv_event_t event)
{
    if(event != LV_EVENT_KEY) return;

    uint32_t *key = (uint32_t *) lv_event_get_data();


    if(*key == LV_KEY_ESC){
        if(sub_page != NULL){
            lv_group_remove_all_objs(g);
            lv_obj_del(sub_page);
            sub_page = NULL;

            lv_obj_t * btn = lv_obj_get_child_back(lv_page_get_scrl(list), NULL);
            uint32_t i = 0;
            lv_obj_t *checked_btn = NULL;
            while(btn) {
                //if(i == track_id) break;


                printf("state = 0x%x\n",lv_obj_get_state(btn, LV_OBJ_PART_MAIN));
                if((lv_obj_get_state(btn, LV_OBJ_PART_MAIN) & (LV_STATE_CHECKED)) == LV_STATE_CHECKED){
                    printf("index %d is checked\n",i );
                    lv_obj_clear_state(btn, LV_STATE_CHECKED);
                    //lv_obj_clear_state(btn,LV_STATE_PRESSED | LV_STATE_FOCUSED);
                    /* lv_obj_set_state(btn, LV_STATE_FOCUSED); */
                    /* lv_group_focus_obj(btn); */
                    checked_btn = btn;
                }
                /* if(i == 0){ */
                /*     lv_group_focus_obj(btn); */
                /* } */

                lv_group_add_obj(g, btn);
                if(checked_btn != NULL) lv_group_focus_obj(checked_btn);
//                else
                i++;
                btn = lv_obj_get_child_back(lv_page_get_scrl(list), btn);
            }
        }

    }
}


static int get_child_indexof(lv_obj_t *child,lv_obj_t *parent)
{
    int i = -1;
    lv_obj_t * _btn = lv_obj_get_child_back(parent, NULL);
    while(_btn){

        if(i == -1) i = 0;

        if(child == _btn){
            break;
        }
        i++;
        _btn = lv_obj_get_child_back(parent, _btn);
    }
    return i;
}


static void list_btn_event_cb(lv_obj_t *btn, lv_event_t event)
{
    //if(event != LV_EVENT_KEY) return;

    printf("event = %d\n",event);

    if(event == LV_EVENT_CLICKED){
        list_selected = get_child_indexof(btn, lv_page_get_scrl(list));
        lv_obj_set_state(btn, LV_STATE_CHECKED);
#if 0
        if(menu_selected == 0){
            ;
        }else if(menu_selected == 1){
            ;
        }else if (menu_selected == 2) {
            ;
        }else if (menu_selected == 3) {
            ;
        }else {
            printf("FUNC%s@L%d:%s\n",__func__,__LINE__,"There must be something wrong!!!\n" );
        }
#endif

        uint8_t selected_index = (uint8_t)((menu_selected & 0x0f) << 4) | (list_selected & 0x0F);
        switch (selected_index) {
        case 0x00: {

            cam_setting_panel(0);
            break;
        }
        case 0x01: {

            //printf("0x%02x\n",selected_index);
            video_setting_panel();
            break;
        }
        case 0x02: {
            printf("0x%02x\n",selected_index);
            break;
        }
        case 0x03: {
            printf("0x%02x\n",selected_index);
            break;
        }
        case 0x04: {
            printf("0x%02x\n",selected_index);
            break;
        }
        case 0x05: {
            printf("0x%02x\n",selected_index);
            break;
        }
        case 0x06: {
            printf("0x%02x\n",selected_index);
            break;
        }
        case 0x10: {
            //printf("0x%02x\n",selected_index);
            calling_ctrl_panel();
            break;
        }
        case 0x11: {
            printf("0x%02x\n",selected_index);
            break;
        }
        case 0x12: {
            printf("0x%02x\n",selected_index);
            break;
        }
        case 0x13: {
            printf("0x%02x\n",selected_index);
            break;
        }
        default:
            printf("FUNC%s@L%d:%s\n",__func__,__LINE__,"There must be something wrong!!!\n" );
            break;
        }
        //cam_setting_panel(0);
        return;
    }else if(event == LV_EVENT_DEFOCUSED ){
        uint32_t last_key = lv_indev_get_key(lv_indev_get_act());
        if(btn == lv_obj_get_child_back(lv_page_get_scrl(list), NULL)){
            printf("defocused with %d\n",last_key);
            if(last_key == LV_KEY_PREV){
                lv_group_remove_all_objs(g);
                lv_obj_t * _btn = lv_obj_get_child_back(cont_img_menu/* lv_page_get_scrl(list) */, NULL);
                lv_obj_t *focus_btn;
                uint32_t i = 0;
                while(_btn) {
                    //if(i == track_id) break;

                    printf("img btn %d state = 0x%02x\n",i,lv_imgbtn_get_state(_btn));
                    if(lv_imgbtn_get_state(_btn) ==  LV_BTN_STATE_CHECKED_RELEASED){
                        focus_btn = _btn;
                    }
                    lv_group_add_obj(g, _btn);
                    i++;
                    _btn = lv_obj_get_child_back(cont_img_menu/* lv_page_get_scrl(list) */, _btn);
                }
                lv_indev_enable(indev_keypad_v, false);
                lv_indev_enable(indev_keypad_h,true);
                lv_indev_wait_release(indev_keypad_h);
                lv_group_focus_obj(focus_btn);
            }

        }
    }

/* if(event == LV_EVENT_KEY){ */

        /*     uint32_t * key = (uint32_t *)lv_event_get_data(); */
        /*     printf("%s%d\n","key pressed :",*key); */
/*     if(*key == LV_KEY_ESC) */
        /*         tab_menu_refresh_group(); */
        /* } */
#if 0
    uint32_t * key = (uint32_t *)lv_event_get_data();
    printf("%s%d\n","key pressed :",*key);
    if(*key == LV_KEY_ESC)
        tab_menu_refresh_group();
    else if(*key == LV_KEY_ENTER){
        lv_area_t coord;
        printf("lv_obj_get_y=%d\n",lv_obj_get_y(btn));
        lv_obj_get_coords(btn, &coord);
        printf("lv_obj_get_coords,x = %d,y = %d\n",coord.x2,coord.y2);
        lv_obj_get_inner_coords(btn, &coord);
        printf("lv_obj_get_inner_coords,x = %d,y = %d\n",coord.x2,coord.y2);
        cam_setting_panel(coord.y1);
    }
#endif
}

static void menu_btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    printf("event = %d\n",event);
    /* if(event != LV_EVENT_CLICKED) return; */

    if(event == LV_EVENT_FOCUSED){

        menu_selected = get_child_indexof(btn, cont_img_menu); // update menu selected index;


        lv_imgbtn_set_state(btn, LV_BTN_STATE_CHECKED_RELEASED);

        if(btn == btn_setting){

            // create setting list menu
            lv_label_set_text(label_title, "系统设置");

            lv_page_clean(list);

            lv_obj_t * btn = lv_obj_create(list, NULL);



            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            lv_obj_t *label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "摄像头设置");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            lv_obj_t *img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/monitor_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            lv_obj_t *img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);


            lv_obj_t * border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);



            btn = lv_obj_create(list, NULL);
            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "视频设置");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);


            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/video_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);


            btn = lv_obj_create(list, NULL);

            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "双流设置");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/misaligned-semicircle_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            btn = lv_obj_create(list, NULL);

            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "音频设置");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/microphone_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);


            btn = lv_obj_create(list, NULL);


            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "网络设置");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/connection-point_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            btn = lv_obj_create(list, NULL);

            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "会议设置");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/phone-video-call_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            btn = lv_obj_create(list, NULL);

            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "字幕设置");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/text-message_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            list_menu_refresh_group();
            //lv_group_focus_obj(btn);
        }else if (btn == btn_video) {

            //lv_group_remove_all_objs(g);
            //lv_obj_clean(list);
            lv_page_clean(list);
            lv_label_set_text(label_title, "视频会议");


            lv_obj_t * btn = lv_obj_create(list, NULL);
            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            lv_obj_t *label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "呼叫控制");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            lv_obj_t *img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/phone-call_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            lv_obj_t *img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);


            lv_obj_t * border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            btn = lv_obj_create(list, NULL);
            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "常用控制");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/tool_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);

            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            btn = lv_obj_create(list, NULL);
            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "会议控制");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/every-user_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);


            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            btn = lv_obj_create(list, NULL);
            lv_obj_set_size(btn, lv_page_get_width_fit(list), 60);
            lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
            lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
            lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
            lv_obj_set_event_cb(btn, list_btn_event_cb);
            lv_page_glue_obj(btn, true);

            label_list = lv_label_create(btn, NULL);
            lv_label_set_text(label_list, "流媒体控制");
            lv_obj_set_style_local_text_font(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_medium);
            lv_obj_set_style_local_text_color(label_list, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
            lv_obj_align(label_list, NULL, LV_ALIGN_CENTER, 0, 0);

            img_left =  lv_img_create(btn, NULL);
            lv_img_set_src(img_left, "/nfsroot/hi3531d/v600/app/assert/video-file_28.png");
            lv_obj_align(img_left, NULL, LV_ALIGN_IN_LEFT_MID, 50, 0);

            img_right =  lv_img_create(btn, NULL);
            lv_img_set_src(img_right, "/nfsroot/hi3531d/v600/app/assert/right_28.png");
            lv_obj_align(img_right, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);


            border = lv_img_create(btn, NULL);
            lv_img_set_src(border, &img_lv_demo_music_list_border);
            lv_obj_set_width(border, lv_obj_get_width(btn));
            lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
            list_menu_refresh_group();
            //lv_obj_set_size(main_page, 500, lv_obj_get_height_fit(lv_scr_act()) - 20);
        }else if (btn == btn_stat) {
            lv_label_set_text(label_title, "系统信息");
            lv_page_clean(list);
            //lv_obj_set_size(main_page, 300, lv_obj_get_height_fit(lv_scr_act()) - 20);
        }else if (btn == btn_dialnose) {
            lv_label_set_text(label_title, "系统诊断");
            lv_page_clean(list);
            //lv_obj_set_size(main_page, 600, lv_obj_get_height_fit(lv_scr_act()) - 20);
        }


    }else if (event == LV_EVENT_DEFOCUSED) {
        lv_imgbtn_set_state(btn, LV_BTN_STATE_RELEASED);
    }else if( event == LV_EVENT_KEY){
        uint32_t *key  = (uint32_t *)lv_event_get_data();
        printf("%s@%d menu_btn key event key= %d\n", __func__,__LINE__,*key);
        if(*key  == LV_KEY_DOWN){
            lv_group_remove_all_objs(g);


            lv_obj_t * _btn = lv_obj_get_child_back(lv_page_get_scrl(list), NULL);
            //if(_btn != NULL){
            while(_btn) {
                //if(i == track_id) break;
                lv_group_add_obj(g, _btn);
                /* if(i == 0){ */
                /*     lv_group_focus_obj(btn); */
                /* } */
                /* i++; */
                _btn = lv_obj_get_child_back(lv_page_get_scrl(list), _btn);
            }
            lv_event_send(lv_obj_get_child_back(lv_page_get_scrl(list), NULL), LV_EVENT_FOCUSED, NULL);
            //}


            lv_indev_enable(indev_keypad_h, false);
            lv_indev_enable(indev_keypad_v,true);
            lv_indev_wait_release(indev_keypad_v);
        }
    }


}


int lvgl_stop(void)
{
    lv_running = 0;
    keypad_uninit();
    return pthread_join(tid, NULL);
}

int lvgl_start(int w, int h)
{
    lv_w = w;
    lv_h = h;
    lv_running = 1;
    return pthread_create(&tid, NULL, lvgl_main, NULL);
}

void kp_callback(irkey_info_s irkey)
{

    if(irkey.irkey_state_code == 0x01 && irkey.irkey_datal == REMOTE_MENU){


    }else{
        state = irkey.irkey_state_code ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
        if(state == LV_INDEV_STATE_PR){
            last_key = irkey.irkey_datal;//keycode_to_ascii(rcv_irkey_info[i].irkey_datal);
        }
    }


}

static lv_obj_t * add_list_btn(lv_obj_t *page, uint8_t list_id)
{
    char time[32];
    lv_snprintf(time, sizeof(time), "list_%02d",list_id);

    lv_obj_t * btn = lv_obj_create(page, NULL);
    lv_obj_set_size(btn, lv_page_get_width_fit(page), 60);
    lv_obj_clean_style_list(btn, LV_BTN_PART_MAIN);
    lv_obj_add_style(btn, LV_BTN_PART_MAIN, &style_btn);
    lv_obj_add_protect(btn, LV_PROTECT_PRESS_LOST);
    /* lv_obj_set_event_cb(btn, btn_event_cb); */
    lv_page_glue_obj(btn, true);
#if 0
    if(list_id >= 3) {
        lv_obj_add_state(btn, LV_STATE_DISABLED);
    }
#endif

    LV_IMG_DECLARE(img_lv_demo_music_list_border);
    lv_obj_t * border = lv_img_create(btn, NULL);
    lv_img_set_src(border, &img_lv_demo_music_list_border);
    lv_obj_set_width(border, lv_obj_get_width(btn));
    lv_obj_align(border, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    return btn;
}



static void list_menu_refresh_group(void)
{
#if 0
    lv_group_remove_all_objs(g);
    lv_indev_enable(indev_keypad_h, false);
    lv_indev_enable(indev_keypad_v,true);
    lv_obj_t * btn = lv_obj_get_child_back(lv_page_get_scrl(list), NULL);
    uint32_t i = 0;
    while(btn) {
        //if(i == track_id) break;
        lv_group_add_obj(g, btn);
        if(i == 0){
            lv_group_focus_obj(btn);
        }
        i++;
        btn = lv_obj_get_child_back(lv_page_get_scrl(list), btn);
    }
#endif
}


static void* lvgl_main(void* p)
{
    /*LittlevGL init*/
    lv_init();


    /*Linux frame buffer device init*/
    fbdev_init();
    lv_png_init();

    /*Initialize and register a display driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    /*Set the resolution of the display*/
    disp_drv.hor_res = lv_w; //1280
    disp_drv.ver_res = lv_h; //1024
#define DISP_BUF_SIZE (80*LV_HOR_RES_MAX/*1280*/)

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];
    /*Initialize a descriptor for the buffer*/
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    disp_drv.buffer = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    lv_disp_drv_register(&disp_drv);


    g = lv_group_create();
    //lv_group_set_wrap(g, false);


    /*------------------
     * Keypad
     * -----------------*/

    /*Initialize your keypad or keyboard if you have*/
    keypad_init();
#if 0
    lv_indev_drv_t kp_drv;
    /*Register a keypad input device*/
    lv_indev_drv_init(&kp_drv);
    kp_drv.type = /* LV_INDEV_TYPE_KEYPAD */LV_INDEV_TYPE_ENCODER;
    kp_drv.read_cb = keypad_read;
    lv_indev_t * indev_keypad = lv_indev_drv_register(&kp_drv);
    lv_indev_set_group(indev_keypad, g);
#endif
    lv_indev_drv_t kp_drv_h;
    /*Register a keypad input device*/
    lv_indev_drv_init(&kp_drv_h);
    kp_drv_h.type = LV_INDEV_TYPE_KEYPAD/* LV_INDEV_TYPE_ENCODER */;
    kp_drv_h.read_cb = keypad_read_navi_h;
    indev_keypad_h = lv_indev_drv_register(&kp_drv_h);
    lv_indev_set_group(indev_keypad_h, g);

    lv_indev_drv_t kp_drv_v;
    /*Register a keypad input device*/
    lv_indev_drv_init(&kp_drv_v);
    kp_drv_v.type = LV_INDEV_TYPE_KEYPAD/* LV_INDEV_TYPE_ENCODER */;
    kp_drv_v.read_cb = keypad_read_navi_v;
    indev_keypad_v = lv_indev_drv_register(&kp_drv_v);
    lv_indev_set_group(indev_keypad_v, g);

    lv_indev_enable(indev_keypad_h, false);
    lv_indev_enable(indev_keypad_v, false);

    /* Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     * add objects to the group with `lv_group_add_obj(group, obj)`
     * and assign this input device to group to navigate in it:
     * `lv_indev_set_group(indev_keypad, group);` */





    /*Create a Demo*/

#if 1
#define H_PAD (20)
    // transparent bg
    // https://docs.lvgl.io/v7/en/html/overview/display.html#transparent-screens
    lv_obj_set_style_local_bg_opa(lv_scr_act(),LV_OBJMASK_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_disp_set_bg_opa(NULL, LV_OPA_TRANSP);

    // font init
    font_smallest = &puhui_regular_18;
    font_small = &puhui_regular_20;
    font_medium = &puhui_regular_24;
    font_title = &puhui_regular_28;

    // style init
    lv_style_init(&style_scrollbar);
    lv_style_set_size(&style_scrollbar, LV_STATE_DEFAULT, 4);
    lv_style_set_bg_opa(&style_scrollbar, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style_scrollbar, LV_STATE_DEFAULT, lv_color_hex3(0xeee));
    lv_style_set_radius(&style_scrollbar, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_style_set_pad_right(&style_scrollbar, LV_STATE_DEFAULT, 4);

    //lv_style_set_border_width(&style_scrollbar, LV_STATE_DEFAULT, 0);

    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_style_set_bg_opa(&style_btn, LV_STATE_PRESSED, LV_OPA_COVER);

    lv_style_set_bg_opa(&style_btn, LV_STATE_CHECKED, LV_OPA_80);
    lv_style_set_bg_opa(&style_btn, LV_STATE_FOCUSED, LV_OPA_80);
    lv_style_set_bg_color(&style_btn, LV_STATE_PRESSED, lv_color_hex(0x413f57/* 0x4c4965 */));
    lv_style_set_bg_color(&style_btn, LV_STATE_CHECKED, lv_color_hex(COLOR_FOCUS/* 0x413f57 *//* 0x4c4965 */));
    lv_style_set_bg_color(&style_btn, LV_STATE_FOCUSED, lv_color_hex(0x19d3da));

    lv_style_set_text_opa(&style_btn, LV_STATE_DISABLED, LV_OPA_40);
    lv_style_set_image_opa(&style_btn, LV_STATE_DISABLED, LV_OPA_40);


    //style for cont_menu btns,
    lv_style_init(&style_img_btn);
#if 0
    lv_style_set_image_recolor_opa(&style_img_btn, LV_STATE_CHECKED, LV_OPA_30); //checked without focus dark
    lv_style_set_image_recolor(&style_img_btn, LV_STATE_CHECKED, LV_COLOR_BLACK/* lv_color_hex(COLOR_FOCUS) */);
    lv_style_set_image_recolor_opa(&style_img_btn, LV_STATE_CHECKED | LV_STATE_FOCUSED, LV_OPA_TRANSP);//checked with focused light ....
#else
    lv_style_set_image_recolor_opa(&style_img_btn, LV_STATE_FOCUSED, LV_OPA_COVER);
    lv_style_set_image_recolor(&style_img_btn, LV_STATE_FOCUSED, lv_color_hex(0x19d3da)/* lv_color_hex(COLOR_FOCUS) */);
#endif

    // outline for focused
    /* lv_style_set_outline_width(&style_img_btn, LV_STATE_FOCUSED, 2); */
    /* lv_style_set_outline_color(&style_img_btn, LV_STATE_FOCUSED, lv_color_hex(0x1abc9c)); */

    // style for subpage container,eg.ddlist,textarea,
    lv_style_init(&style_box);
    lv_style_set_text_font(&style_box, LV_STATE_DEFAULT, font_small);
    lv_style_set_text_font(&style_box, LV_STATE_DEFAULT, font_small);
    lv_style_set_text_color(&style_box, LV_STATE_FOCUSED, lv_color_hex(COLOR_FOCUS));

    // style for subpage label
    lv_style_init(&style_sub_label);
    lv_style_set_text_opa(&style_sub_label, LV_STATE_DEFAULT, LV_OPA_90);
    lv_style_set_text_color(&style_sub_label, LV_STATE_DEFAULT, lv_color_hex(/* COLOR_FOCUS */0xFFFFFF));
    lv_style_set_text_font(&style_sub_label, LV_STATE_DEFAULT, font_smallest);

    // main container init
    main_page = lv_page_create(lv_scr_act(), NULL);
    lv_obj_set_size(main_page, 300, lv_obj_get_height_fit(lv_scr_act()) - 20);
    lv_obj_align(main_page, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);



    // style the main page
    lv_obj_clean_style_list(main_page, LV_PAGE_PART_SCROLLABLE);
    lv_obj_clean_style_list(main_page,LV_PAGE_PART_SCROLLBAR);
    lv_obj_add_style(main_page, LV_PAGE_PART_SCROLLBAR, &style_scrollbar);
    lv_obj_set_style_local_bg_color(main_page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x343247));
    lv_obj_set_style_local_pad_left(main_page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_pad_right(main_page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

    // main title label
    label_title = lv_label_create(main_page, NULL);
    lv_obj_set_style_local_text_font(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_title);
    lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(COLOR_FOCUS));
    //lv_obj_set_style_local_margin_top(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_label_set_text(label_title, "系统设置");
    lv_obj_align(label_title, NULL, LV_ALIGN_IN_TOP_MID,0,0);

    // container for img menu
    cont_img_menu = lv_cont_create(main_page,NULL);
    lv_obj_clean_style_list(cont_img_menu, LV_PAGE_PART_SCROLLABLE);
    lv_obj_clean_style_list(cont_img_menu,LV_PAGE_PART_SCROLLBAR);
    lv_obj_add_style(cont_img_menu, LV_PAGE_PART_SCROLLBAR, &style_scrollbar);
    lv_obj_set_style_local_bg_color(cont_img_menu, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, /* lv_color_hex(0x65675c) */ lv_color_hex(COLOR_MENU_BG));
    lv_obj_set_style_local_border_width(cont_img_menu, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_click(cont_img_menu, false);
    lv_obj_set_size(cont_img_menu, lv_obj_get_width_fit(main_page) - 2 * 10, 50);
    lv_obj_align(cont_img_menu, label_title, LV_ALIGN_OUT_BOTTOM_LEFT, - lv_obj_get_x(label_title) + 10, 10);
    lv_cont_set_layout(cont_img_menu, LV_LAYOUT_ROW_MID);

    btn_setting = lv_imgbtn_create(cont_img_menu, NULL);

    lv_obj_add_style(btn_setting, LV_IMGBTN_PART_MAIN, &style_img_btn);


    lv_imgbtn_set_src(btn_setting, LV_BTN_STATE_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_setting_normal.png");
    lv_imgbtn_set_src(btn_setting, LV_BTN_STATE_CHECKED_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_setting_focus.png");
    lv_imgbtn_set_src(btn_setting, LV_BTN_STATE_CHECKED_PRESSED,"/nfsroot/hi3531d/v600/app/assert/btn_setting_focus.png");
    lv_imgbtn_set_src(btn_setting, LV_BTN_STATE_PRESSED,"/nfsroot/hi3531d/v600/app/assert/btn_setting_normal.png");

    lv_imgbtn_set_src(btn_setting, LV_BTN_STATE_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_setting_normal.png");
    lv_imgbtn_set_src(btn_setting, LV_BTN_STATE_CHECKED_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_setting_focus.png");
    lv_imgbtn_set_src(btn_setting, LV_BTN_STATE_CHECKED_PRESSED,"/nfsroot/hi3531d/v600/app/assert/btn_setting_focus.png");
    lv_imgbtn_set_src(btn_setting, LV_BTN_STATE_PRESSED,"/nfsroot/hi3531d/v600/app/assert/btn_setting_normal.png");



    btn_video = lv_imgbtn_create(cont_img_menu, NULL);

    lv_obj_add_style(btn_video, LV_IMGBTN_PART_MAIN, &style_img_btn);
    lv_imgbtn_set_src(btn_video, LV_BTN_STATE_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_video_normal.png");
    lv_imgbtn_set_src(btn_video, LV_BTN_STATE_CHECKED_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_video_focus.png");
    lv_imgbtn_set_src(btn_video, LV_BTN_STATE_CHECKED_PRESSED, "/nfsroot/hi3531d/v600/app/assert/btn_video_focus.png");
    lv_imgbtn_set_src(btn_video, LV_BTN_STATE_PRESSED, "/nfsroot/hi3531d/v600/app/assert/btn_video_normal.png");


    btn_stat = lv_imgbtn_create(cont_img_menu, NULL);

    lv_obj_add_style(btn_stat, LV_IMGBTN_PART_MAIN, &style_img_btn);
    lv_imgbtn_set_src(btn_stat, LV_BTN_STATE_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_stat_normal.png");
    lv_imgbtn_set_src(btn_stat, LV_BTN_STATE_CHECKED_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_stat_focus.png");
    lv_imgbtn_set_src(btn_stat, LV_BTN_STATE_CHECKED_PRESSED, "/nfsroot/hi3531d/v600/app/assert/btn_stat_focus.png");
    lv_imgbtn_set_src(btn_stat, LV_BTN_STATE_PRESSED, "/nfsroot/hi3531d/v600/app/assert/btn_stat_normal.png");


    btn_dialnose = lv_imgbtn_create(cont_img_menu, NULL);

    lv_obj_add_style(btn_dialnose, LV_IMGBTN_PART_MAIN, &style_img_btn);
    lv_imgbtn_set_src(btn_dialnose, LV_BTN_STATE_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_dialnose_normal.png");
    lv_imgbtn_set_src(btn_dialnose, LV_BTN_STATE_CHECKED_RELEASED, "/nfsroot/hi3531d/v600/app/assert/btn_dialnose_focus.png");
    lv_imgbtn_set_src(btn_dialnose, LV_BTN_STATE_CHECKED_PRESSED, "/nfsroot/hi3531d/v600/app/assert/btn_dialnose_focus.png");
    lv_imgbtn_set_src(btn_dialnose, LV_BTN_STATE_PRESSED, "/nfsroot/hi3531d/v600/app/assert/btn_dialnose_normal.png");

    /*Create an empty white main container*/
    list = lv_page_create(main_page, NULL);
    lv_obj_set_size(list, lv_obj_get_width_fit(main_page), lv_obj_get_height_fit(main_page) - lv_obj_get_y(cont_img_menu)- lv_obj_get_height(cont_img_menu) - 20);
    lv_obj_set_y(list, lv_obj_get_y(cont_img_menu) + lv_obj_get_height(cont_img_menu) + 10);
    lv_obj_set_x(list, 0); //-lv_obj_get_style_pad_left(main_page, LV_PAGE_PART_BG)
    lv_obj_clean_style_list(list, LV_PAGE_PART_BG);
    lv_obj_clean_style_list(list, LV_PAGE_PART_SCROLLABLE);
    lv_obj_clean_style_list(list, LV_PAGE_PART_SCROLLBAR);
    lv_obj_add_style(list, LV_PAGE_PART_SCROLLBAR, &style_scrollbar);

    lv_page_set_scrl_layout(list, LV_LAYOUT_COLUMN_MID);

    /* printf("margin of main_page = %d \n",lv_obj_get_style_pad_left(main_page, LV_OBJ_PART_MAIN)); */

#endif // end


    lv_indev_enable(indev_keypad_h, true);

    lv_group_add_obj(g, btn_setting);
    lv_group_add_obj(g, btn_video);
    lv_group_add_obj(g, btn_stat);
    lv_group_add_obj(g, btn_dialnose);

    lv_obj_set_event_cb(btn_setting, menu_btn_event_cb);
    lv_obj_set_event_cb(btn_video, menu_btn_event_cb);
    lv_obj_set_event_cb(btn_stat, menu_btn_event_cb);
    lv_obj_set_event_cb(btn_dialnose, menu_btn_event_cb);
    /* lv_group_focus_obj(btn_setting); */
    lv_event_send(btn_setting, LV_EVENT_FOCUSED, NULL);

    /*Handle LitlevGL tasks (tickless mode)*/
    while(lv_running) {

        lv_task_handler();
        usleep(5*1000);


    }


    return NULL;
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



#if 0
/**********************
 *   STATIC FUNCTIONS
 **********************/

static void selectors_create(lv_obj_t * parent)
{
    lv_page_set_scrl_layout(parent, LV_LAYOUT_COLUMN_MID);

    selector_objs.btn = lv_btn_create(parent, NULL);

    lv_obj_t * label = lv_label_create(selector_objs.btn, NULL);
    lv_label_set_text(label, "Button");

    selector_objs.cb = lv_checkbox_create(parent, NULL);

    selector_objs.slider = lv_slider_create(parent, NULL);
    lv_slider_set_range(selector_objs.slider, 0, 10);

    selector_objs.sw = lv_switch_create(parent, NULL);

    selector_objs.spinbox = lv_spinbox_create(parent, NULL);

    selector_objs.dropdown = lv_dropdown_create(parent, NULL);
    lv_obj_set_event_cb(selector_objs.dropdown, dd_enc);

    selector_objs.roller = lv_roller_create(parent, NULL);

    selector_objs.list = lv_list_create(parent, NULL);
    if(lv_obj_get_height(selector_objs.list) > lv_page_get_height_fit(parent)) {
        lv_obj_set_height(selector_objs.list, lv_page_get_height_fit(parent));
    }
    lv_list_add_btn(selector_objs.list, LV_SYMBOL_OK, "Apply");
    lv_list_add_btn(selector_objs.list, LV_SYMBOL_CLOSE, "Close");
    lv_list_add_btn(selector_objs.list, LV_SYMBOL_EYE_OPEN, "Show");
    lv_list_add_btn(selector_objs.list, LV_SYMBOL_EYE_CLOSE, "Hide");
    lv_list_add_btn(selector_objs.list, LV_SYMBOL_TRASH, "Delete");
    lv_list_add_btn(selector_objs.list, LV_SYMBOL_COPY, "Copy");
    lv_list_add_btn(selector_objs.list, LV_SYMBOL_PASTE, "Paste");
}

static void text_input_create(lv_obj_t * parent)
{
    textinput_objs.ta1 = lv_textarea_create(parent, NULL);
    lv_obj_set_event_cb(textinput_objs.ta1, ta_event_cb);
    lv_obj_align(textinput_objs.ta1, NULL, LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 20);
    lv_textarea_set_one_line(textinput_objs.ta1, true);
    lv_textarea_set_cursor_hidden(textinput_objs.ta1, true);
    lv_textarea_set_placeholder_text(textinput_objs.ta1, "Type something");
    lv_textarea_set_text(textinput_objs.ta1, "");

    textinput_objs.ta2 = lv_textarea_create(parent, textinput_objs.ta1);
    lv_obj_align(textinput_objs.ta2, textinput_objs.ta1, LV_ALIGN_OUT_BOTTOM_MID, 0, LV_DPI / 20);

    textinput_objs.kb = NULL;
}

static void msgbox_create(void)
{
    lv_obj_t * mbox = lv_msgbox_create(lv_layer_top(), NULL);
    lv_msgbox_set_text(mbox, "Welcome to the keyboard and encoder demo");
    lv_obj_set_event_cb(mbox, msgbox_event_cb);
    lv_group_add_obj(g, mbox);
    lv_group_focus_obj(mbox);

    lv_group_focus_freeze(g, true);

    static const char * btns[] = {"Ok", "Cancel", ""};
    lv_msgbox_add_btns(mbox, btns);
    lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_local_bg_opa(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
    lv_obj_set_style_local_bg_color(lv_layer_top(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_click(lv_layer_top(), true);
}


static void msgbox_event_cb(lv_obj_t * msgbox, lv_event_t e)
{
    if(e == LV_EVENT_CLICKED) {
        uint16_t b = lv_msgbox_get_active_btn(msgbox);
        if(b == 0 || b == 1) {
            lv_obj_del(msgbox);
            lv_obj_reset_style_list(lv_layer_top(), LV_OBJ_PART_MAIN);
            lv_obj_set_click(lv_layer_top(), false);
            lv_event_send(tv, LV_EVENT_REFRESH, NULL);
        }
    }
}
#endif
