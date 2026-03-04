#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_main_view() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main_view = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1280, 720);
    {
        lv_obj_t *parent_obj = obj;
        {
            // main_panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_panel = obj;
            lv_obj_set_pos(obj, 320, 0);
            lv_obj_set_size(obj, LV_PCT(75), LV_PCT(100));
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(60));
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // panorama
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.panorama = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // panorama_image
                                    lv_obj_t *obj = lv_image_create(parent_obj);
                                    objects.panorama_image = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
                                }
                            }
                        }
                    }
                }
                {
                    // anypoint
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.anypoint = obj;
                    lv_obj_set_pos(obj, 80, 308);
                    lv_obj_set_size(obj, LV_PCT(100), 200);
                    lv_obj_set_style_flex_grow(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // anypoint_1
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.anypoint_1 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_PCT(24), LV_PCT(100));
                            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // anypoint_1_image
                                    lv_obj_t *obj = lv_image_create(parent_obj);
                                    objects.anypoint_1_image = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
                                }
                            }
                        }
                        {
                            // anypoint_2
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.anypoint_2 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_PCT(24), LV_PCT(100));
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // anypoint_2_image
                                    lv_obj_t *obj = lv_image_create(parent_obj);
                                    objects.anypoint_2_image = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
                                }
                            }
                        }
                        {
                            // anypoint_3
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.anypoint_3 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_PCT(24), LV_PCT(100));
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // anypoint_3_image
                                    lv_obj_t *obj = lv_image_create(parent_obj);
                                    objects.anypoint_3_image = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
                                }
                            }
                        }
                        {
                            // anypoint_4
                            lv_obj_t *obj = lv_obj_create(parent_obj);
                            objects.anypoint_4 = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_PCT(24), LV_PCT(100));
                            {
                                lv_obj_t *parent_obj = obj;
                                {
                                    // anypoint_4_image
                                    lv_obj_t *obj = lv_image_create(parent_obj);
                                    objects.anypoint_4_image = obj;
                                    lv_obj_set_pos(obj, 0, 0);
                                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
                                }
                            }
                        }
                    }
                }
            }
        }
        {
            // Sidebar
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.sidebar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, LV_PCT(25), LV_PCT(100));
            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // open_camera
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.open_camera = obj;
                    lv_obj_set_pos(obj, 150, 21);
                    lv_obj_set_size(obj, 278, 44);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text(obj, "Open Camera");
                        }
                    }
                }
                {
                    // moil_image
                    lv_obj_t *obj = lv_image_create(parent_obj);
                    objects.moil_image = obj;
                    lv_obj_set_pos(obj, LV_PCT(0), LV_PCT(0));
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(30));
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // mode_panel
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.mode_panel = obj;
                    lv_obj_set_pos(obj, 11, 283);
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(20));
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -8, -11);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Mode");
                        }
                        {
                            // btn_original
                            lv_obj_t *obj = lv_checkbox_create(parent_obj);
                            objects.btn_original = obj;
                            lv_obj_set_pos(obj, -8, 9);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_checkbox_set_text(obj, "Original");
                        }
                        {
                            // btn_panorama
                            lv_obj_t *obj = lv_checkbox_create(parent_obj);
                            objects.btn_panorama = obj;
                            lv_obj_set_pos(obj, -8, 38);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_checkbox_set_text(obj, "Panorama");
                        }
                        {
                            // btn_anypoint
                            lv_obj_t *obj = lv_checkbox_create(parent_obj);
                            objects.btn_anypoint = obj;
                            lv_obj_set_pos(obj, -8, 66);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_checkbox_set_text(obj, "Anypoint");
                        }
                    }
                }
                {
                    // config
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.config = obj;
                    lv_obj_set_pos(obj, 11, 462);
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(26));
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -7, -5);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Parameter Config");
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -6, 19);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Alpha");
                        }
                        {
                            // alpha_conf
                            lv_obj_t *obj = lv_slider_create(parent_obj);
                            objects.alpha_conf = obj;
                            lv_obj_set_pos(obj, -7, 41);
                            lv_obj_set_size(obj, 240, 10);
                            lv_slider_set_value(obj, 25, LV_ANIM_OFF);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -6, 58);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Beta");
                        }
                        {
                            // beta_conf
                            lv_obj_t *obj = lv_slider_create(parent_obj);
                            objects.beta_conf = obj;
                            lv_obj_set_pos(obj, -7, 80);
                            lv_obj_set_size(obj, 240, 10);
                            lv_slider_set_value(obj, 25, LV_ANIM_OFF);
                        }
                        {
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            lv_obj_set_pos(obj, -6, 98);
                            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                            lv_label_set_text(obj, "Zoom");
                        }
                        {
                            // zoom_conf
                            lv_obj_t *obj = lv_slider_create(parent_obj);
                            objects.zoom_conf = obj;
                            lv_obj_set_pos(obj, -7, 120);
                            lv_obj_set_size(obj, 240, 10);
                            lv_slider_set_value(obj, 25, LV_ANIM_OFF);
                        }
                    }
                }
            }
        }
    }
    
    tick_screen_main_view();
}

void tick_screen_main_view() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main_view,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main_view();
}