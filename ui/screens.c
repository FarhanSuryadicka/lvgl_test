#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

lv_obj_t *tick_value_change_obj;

/* =========================================================
 * Helpers
 * ========================================================= */
static void reset_container(lv_obj_t *obj) {
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_gap(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void make_black_frame(lv_obj_t *obj, int border_w) {
    reset_container(obj);
    lv_obj_set_style_border_width(obj, border_w, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* =========================================================
 * Screens
 * ========================================================= */
void create_screen_main_view() {
    const int ROOT_W = 1920;
    const int ROOT_H = 1080;

    const int SIDEBAR_W = 340;
    const int MAIN_W = ROOT_W - SIDEBAR_W;

    const int MAIN_PAD = 10;
    const int MAIN_GAP = 10;

    /* Panorama diperkecil */
    const int PANO_H = 550;
    const int ANY_H  = 340;

    /* Sisa ruang dibagi ke anypoint + safety margin */
    const int MAIN_CONTENT_H = ROOT_H - (MAIN_PAD * 2);
    // const int SAFE_BOTTOM = 8;
    // const int ANY_H = MAIN_CONTENT_H - PANO_H - MAIN_GAP - SAFE_BOTTOM;

    lv_obj_t *obj = lv_obj_create(NULL);
    objects.main_view = obj;
    lv_obj_set_size(obj, ROOT_W, ROOT_H);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

    {
        lv_obj_t *parent_obj = obj;

        /* =========================
         * Sidebar
         * ========================= */
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.sidebar = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, SIDEBAR_W, ROOT_H);

            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_obj_set_style_pad_all(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_gap(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

            {
                lv_obj_t *parent_obj = obj;

                /* open_camera */
                {
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.open_camera = obj;
                    lv_obj_set_size(obj, LV_PCT(100), 50);

                    lv_obj_t *lbl = lv_label_create(obj);
                    lv_label_set_text(lbl, "Open Camera");
                    lv_obj_center(lbl);
                }

                /* moil_image */
                {
                    lv_obj_t *obj = lv_image_create(parent_obj);
                    objects.moil_image = obj;
                    lv_obj_set_size(obj, LV_PCT(100), 190);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
                }

                /* mode_panel */
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.mode_panel = obj;
                    lv_obj_set_size(obj, LV_PCT(100), 130);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_all(obj, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

                    lv_obj_t *parent_obj = obj;

                    lv_obj_t *lbl = lv_label_create(parent_obj);
                    lv_obj_set_pos(lbl, 0, 0);
                    lv_label_set_text(lbl, "Mode");

                    objects.btn_original = lv_checkbox_create(parent_obj);
                    lv_obj_set_pos(objects.btn_original, 0, 28);
                    lv_checkbox_set_text(objects.btn_original, "Original");

                    objects.btn_panorama = lv_checkbox_create(parent_obj);
                    lv_obj_set_pos(objects.btn_panorama, 0, 56);
                    lv_checkbox_set_text(objects.btn_panorama, "Panorama");

                    objects.btn_anypoint = lv_checkbox_create(parent_obj);
                    lv_obj_set_pos(objects.btn_anypoint, 0, 84);
                    lv_checkbox_set_text(objects.btn_anypoint, "Anypoint");
                }

                /* config */
                {
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.config = obj;
                    lv_obj_set_size(obj, LV_PCT(100), 210);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_all(obj, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

                    lv_obj_t *parent_obj = obj;

                    {
                        lv_obj_t *lbl = lv_label_create(parent_obj);   // child 0
                        lv_obj_set_pos(lbl, 0, 0);
                        lv_label_set_text(lbl, "Parameter Config");
                    }
                    {
                        lv_obj_t *lbl = lv_label_create(parent_obj);   // child 1
                        lv_obj_set_pos(lbl, 0, 26);
                        lv_label_set_text(lbl, "Alpha");
                    }
                    {
                        objects.alpha_conf = lv_slider_create(parent_obj); // child 2
                        lv_obj_set_pos(objects.alpha_conf, 0, 48);
                        lv_obj_set_size(objects.alpha_conf, 280, 10);
                        lv_slider_set_value(objects.alpha_conf, 25, LV_ANIM_OFF);
                    }
                    {
                        lv_obj_t *lbl = lv_label_create(parent_obj);   // child 3
                        lv_obj_set_pos(lbl, 0, 76);
                        lv_label_set_text(lbl, "Beta");
                    }
                    {
                        objects.beta_conf = lv_slider_create(parent_obj);  // child 4
                        lv_obj_set_pos(objects.beta_conf, 0, 98);
                        lv_obj_set_size(objects.beta_conf, 280, 10);
                        lv_slider_set_value(objects.beta_conf, 25, LV_ANIM_OFF);
                    }
                    {
                        lv_obj_t *lbl = lv_label_create(parent_obj);   // child 5
                        lv_obj_set_pos(lbl, 0, 126);
                        lv_label_set_text(lbl, "Zoom");
                    }
                    {
                        objects.zoom_conf = lv_slider_create(parent_obj);  // child 6
                        lv_obj_set_pos(objects.zoom_conf, 0, 148);
                        lv_obj_set_size(objects.zoom_conf, 280, 10);
                        lv_slider_set_value(objects.zoom_conf, 25, LV_ANIM_OFF);
                    }
                }
            }
        }

        /* =========================
         * Main Panel
         * ========================= */
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_panel = obj;
            lv_obj_set_pos(obj, SIDEBAR_W, 0);
            lv_obj_set_size(obj, MAIN_W, ROOT_H);

            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(obj, MAIN_PAD, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_gap(obj, MAIN_GAP, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_obj_set_style_layout(obj, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(obj, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_cross_place(obj, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

            {
                lv_obj_t *parent_obj = obj;

                /* panorama */
                {
                    objects.panorama = lv_obj_create(parent_obj);
                    lv_obj_set_size(objects.panorama, LV_PCT(100), PANO_H);
                    make_black_frame(objects.panorama, 1);

                    objects.panorama_image = lv_image_create(objects.panorama);
                    lv_obj_set_pos(objects.panorama_image, 0, 0);
                    lv_obj_set_size(objects.panorama_image, LV_PCT(100), LV_PCT(100));
                }

                /* anypoint row */
                {
                    objects.anypoint = lv_obj_create(parent_obj);
                    lv_obj_set_size(objects.anypoint, LV_PCT(100), ANY_H);
                    lv_obj_set_style_layout(objects.anypoint, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_flow(objects.anypoint, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_main_place(objects.anypoint, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_flex_cross_place(objects.anypoint, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_all(objects.anypoint, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_gap(objects.anypoint, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(objects.anypoint, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_clear_flag(objects.anypoint, LV_OBJ_FLAG_SCROLLABLE);

                    /* anypoint_1 */
                    objects.anypoint_1 = lv_obj_create(objects.anypoint);
                    lv_obj_set_width(objects.anypoint_1, 0);
                    lv_obj_set_height(objects.anypoint_1, LV_PCT(100));
                    lv_obj_set_style_flex_grow(objects.anypoint_1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    make_black_frame(objects.anypoint_1, 1);
                    objects.anypoint_1_image = lv_image_create(objects.anypoint_1);
                    lv_obj_set_size(objects.anypoint_1_image, LV_PCT(100), LV_PCT(100));

                    /* anypoint_2 */
                    objects.anypoint_2 = lv_obj_create(objects.anypoint);
                    lv_obj_set_width(objects.anypoint_2, 0);
                    lv_obj_set_height(objects.anypoint_2, LV_PCT(100));
                    lv_obj_set_style_flex_grow(objects.anypoint_2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    make_black_frame(objects.anypoint_2, 1);
                    objects.anypoint_2_image = lv_image_create(objects.anypoint_2);
                    lv_obj_set_size(objects.anypoint_2_image, LV_PCT(100), LV_PCT(100));

                    /* anypoint_3 */
                    objects.anypoint_3 = lv_obj_create(objects.anypoint);
                    lv_obj_set_width(objects.anypoint_3, 0);
                    lv_obj_set_height(objects.anypoint_3, LV_PCT(100));
                    lv_obj_set_style_flex_grow(objects.anypoint_3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    make_black_frame(objects.anypoint_3, 1);
                    objects.anypoint_3_image = lv_image_create(objects.anypoint_3);
                    lv_obj_set_size(objects.anypoint_3_image, LV_PCT(100), LV_PCT(100));

                    /* anypoint_4 */
                    objects.anypoint_4 = lv_obj_create(objects.anypoint);
                    lv_obj_set_width(objects.anypoint_4, 0);
                    lv_obj_set_height(objects.anypoint_4, LV_PCT(100));
                    lv_obj_set_style_flex_grow(objects.anypoint_4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    make_black_frame(objects.anypoint_4, 1);
                    objects.anypoint_4_image = lv_image_create(objects.anypoint_4);
                    lv_obj_set_size(objects.anypoint_4_image, LV_PCT(100), LV_PCT(100));
                }
            }
        }
    }

    lv_obj_update_layout(objects.main_view);
    tick_screen_main_view();
}

void tick_screen_main_view() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main_view,
};

void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < (int)(sizeof(tick_screen_funcs) / sizeof(tick_screen_funcs[0]))) {
        tick_screen_funcs[screen_index]();
    }
}

void tick_screen_by_id(enum ScreensEnum screenId) {
    int idx = screenId - 1;
    if (idx >= 0 && idx < (int)(sizeof(tick_screen_funcs) / sizeof(tick_screen_funcs[0]))) {
        tick_screen_funcs[idx]();
    }
}

/* =========================================================
 * Fonts
 * ========================================================= */
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

uint32_t active_theme_index = 0;

void create_screens() {
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(
        dispp,
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        false,
        LV_FONT_DEFAULT
    );
    lv_display_set_theme(dispp, theme);

    create_screen_main_view();
}