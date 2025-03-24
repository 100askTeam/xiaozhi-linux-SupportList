/**
 ******************************************************************************
 * @file    lv_100ask_xz_ai_main.c
 * @author  百问科技
 * @version V1.0
 * @date    2025-3-17
 * @brief	100ask XiaoZhi AI base on LVGL
 ******************************************************************************
 * Change Logs:
 * Date           Author          Notes
 * 2025-3-17     zhouyuebiao     First version
 ******************************************************************************
 * @attention
 *
 * Copyright (C) 2008-2025 深圳百问网科技有限公司<https://www.100ask.net/>
 * All rights reserved
 *
 * 代码配套的视频教程：
 *      B站：   https://www.bilibili.com/video/BV1WE421K75k
 *      百问网：https://fnwcn.xetslk.com/s/39njGj
 *      淘宝：  https://detail.tmall.com/item.htm?id=779667445604
 *
 * 本程序遵循MIT协议, 请遵循协议！
 * 免责声明: 百问网编写的文档, 仅供学员学习使用, 可以转发或引用(请保留作者信息),禁止用于商业用途！
 * 免责声明: 百问网编写的程序, 仅供学习参考，假如被用于商业用途, 但百问网不承担任何后果！
 *
 * 百问网学习平台   : https://www.100ask.net
 * 百问网交流社区   : https://forums.100ask.net
 * 百问网LVGL文档   : https://lvgl.100ask.net
 * 百问网官方B站    : https://space.bilibili.com/275908810
 * 百问网官方淘宝   : https://100ask.taobao.com
 * 百问网微信公众号 ：百问科技 或 baiwenkeji
 * 联系我们(E-mail):  support@100ask.net 或 fae_100ask@163.com
 *
 *                             版权所有，盗版必究。
 ******************************************************************************
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "lv_100ask_xz_ai_main.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/
typedef struct _lv_100ask_xz_ai {
	lv_obj_t  * state_bar_img_wifi;
	lv_obj_t  * state_bar_label_state;
	lv_obj_t  * state_bar_img_battery;
	lv_obj_t  * img_emoji;
	lv_obj_t  * label_chat;
} T_lv_100ask_xz_ai, *PT_lv_100ask_xz_ai;


/**********************
 *  STATIC PROTOTYPES
 **********************/
static void init_freetype(void);
static void deinit_freetype(void);

static void init_style(void);

static void screen_onclicked_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
static PT_lv_100ask_xz_ai g_pt_lv_100ask_xz_ai;

static lv_style_t g_style_chat_font;
static lv_style_t g_style_state_font;

static lv_font_t * gp_chat_font_freetype;
static lv_font_t * gp_state_font_freetype;

extern void lvgl_lock(void);

extern void lvgl_unlock(void);

/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_100ask_xz_ai_main(void)
{
    /* init */
    g_pt_lv_100ask_xz_ai = (T_lv_100ask_xz_ai *)lv_malloc(sizeof(T_lv_100ask_xz_ai));
    
    lv_fs_stdio_init();	

    init_freetype();
    init_style();

    /* state bar */
    lv_obj_t * cont_state_bar = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(cont_state_bar);
    lv_obj_set_size(cont_state_bar, LV_PCT(100), 40);
    lv_obj_set_align(cont_state_bar, LV_ALIGN_TOP_MID);
    lv_obj_set_style_radius(cont_state_bar, 0, 0);
    lv_obj_set_style_bg_opa(cont_state_bar, LV_OPA_60, 0);
    lv_obj_set_style_pad_hor(cont_state_bar, 10, 0);
    lv_obj_set_layout(cont_state_bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont_state_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont_state_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // wifi
    g_pt_lv_100ask_xz_ai->state_bar_img_wifi = lv_image_create(cont_state_bar);
    lv_image_set_src(g_pt_lv_100ask_xz_ai->state_bar_img_wifi, LV_SYMBOL_WIFI);
    
    // state
    g_pt_lv_100ask_xz_ai->state_bar_label_state = lv_label_create(cont_state_bar);
    lv_obj_add_style(g_pt_lv_100ask_xz_ai->state_bar_label_state, &g_style_state_font, 0);
    lv_obj_set_width(g_pt_lv_100ask_xz_ai->state_bar_label_state, LV_PCT(70));
    lv_label_set_text(g_pt_lv_100ask_xz_ai->state_bar_label_state, "待命");

    // battery
    g_pt_lv_100ask_xz_ai->state_bar_img_battery = lv_image_create(cont_state_bar);
    lv_image_set_src(g_pt_lv_100ask_xz_ai->state_bar_img_battery, LV_SYMBOL_BATTERY_FULL);


    /* emoji */ 
    // https://www.iconfont.cn/search/index?searchType=icon&q=%E5%9C%86%E8%84%B8%E8%A1%A8%E6%83%85
    g_pt_lv_100ask_xz_ai->img_emoji = lv_image_create(lv_screen_active());
    lv_image_set_src(g_pt_lv_100ask_xz_ai->img_emoji, ASSETS_PATH"img_naughty.png");
    lv_obj_align(g_pt_lv_100ask_xz_ai->img_emoji, LV_ALIGN_CENTER, 0, -40);

    
    /* chat */
    g_pt_lv_100ask_xz_ai->label_chat = lv_label_create(lv_screen_active());
    lv_obj_set_width(g_pt_lv_100ask_xz_ai->label_chat, LV_PCT(90));
    lv_obj_add_style(g_pt_lv_100ask_xz_ai->label_chat, &g_style_chat_font, 0);
    lv_label_set_text(g_pt_lv_100ask_xz_ai->label_chat, "Hi！有什么可以帮到你呢？");
    lv_obj_align_to(g_pt_lv_100ask_xz_ai->label_chat, g_pt_lv_100ask_xz_ai->img_emoji, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);


    // screen touch
    lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(lv_layer_top(), screen_onclicked_event_cb, LV_EVENT_CLICKED, NULL);
}


void SetStateString(const char *str)
{
    lvgl_lock();
    lv_label_set_text(g_pt_lv_100ask_xz_ai->state_bar_label_state, str);
    lvgl_unlock();
}

void SetText(const char *str)
{
    lvgl_lock();
    lv_label_set_text(g_pt_lv_100ask_xz_ai->label_chat, str);
    lvgl_unlock();
}

void SetEmotion(const char *jpgFile)
{
    lvgl_lock();
    lv_image_set_src(g_pt_lv_100ask_xz_ai->img_emoji, jpgFile);
    lvgl_unlock();
}


void OnClicked(void)
{
    static uint16_t index = 0;
    static char *str[][3] = {
        {"待命", "聆听", "回答"},
        {"现在是待命状态哦。", "现在是聆听状态哦。", "现在是回答状态哦。"},
        {ASSETS_PATH "img_joke.png", ASSETS_PATH "img_naughty.png", ASSETS_PATH "img_think.png"},
    };
#if 0
    lvgl_lock();
    lv_label_set_text(g_pt_lv_100ask_xz_ai->state_bar_label_state, str[0][index]);
    lv_label_set_text(g_pt_lv_100ask_xz_ai->label_chat, str[1][index]);

    lv_image_set_src(g_pt_lv_100ask_xz_ai->img_emoji, str[2][index]);
    lvgl_unlock();
#endif
    if(index >= 2) index = 0;
    else index++;

    LV_LOG_USER("Clicked, index: %d", index);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
static void lv_100ask_xz_ai_main_deinit(void)
{
    deinit_freetype();
    lv_free(g_pt_lv_100ask_xz_ai);

    lv_deinit();
}



static void init_freetype(void)
{
    /*Create a font*/
    gp_chat_font_freetype = lv_freetype_font_create(PATH_PREFIX "HarmonyOS_Sans_SC_Regular.ttf",
                                                    LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                                    26,
                                                    LV_FREETYPE_FONT_STYLE_NORMAL);
                                                    
    gp_state_font_freetype = lv_freetype_font_create(PATH_PREFIX "HarmonyOS_Sans_SC_Regular.ttf",
                                                    LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                                    20,
                                                    LV_FREETYPE_FONT_STYLE_NORMAL);

    if((!gp_chat_font_freetype) || (!gp_state_font_freetype)) {
        LV_LOG_ERROR("freetype font create failed.");
        exit(-1);
    }
}

static void deinit_freetype(void)
{
    lv_freetype_font_delete(gp_chat_font_freetype);
    lv_freetype_font_delete(gp_state_font_freetype);
}


static void init_style(void)
{
    /*Create style with the new font*/;
    lv_style_init(&g_style_chat_font);
    lv_style_set_text_font(&g_style_chat_font, gp_chat_font_freetype);
    lv_style_set_text_align(&g_style_chat_font, LV_TEXT_ALIGN_CENTER);

    lv_style_init(&g_style_state_font);
    lv_style_set_text_font(&g_style_state_font, gp_state_font_freetype);
    lv_style_set_text_align(&g_style_state_font, LV_TEXT_ALIGN_CENTER);
}


static void screen_onclicked_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        OnClicked();
    }
}


