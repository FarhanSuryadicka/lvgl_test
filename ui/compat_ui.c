/* Compatibility glue: provide the simple globals expected by older code
 * (ui_img_video, ui_lbl_fps) while the project uses the generated
 * screens.c / objects.* layout. This file is intentionally minimal:
 * - defines the globals declared in ui.h
 * - provides ui_post_init() that hooks them to generated objects
 */

#include "ui.h"
#include "screens.h"
#include <lvgl/lvgl.h>

/* Define the globals expected by existing code (previously in ui.c) */
lv_obj_t * ui_img_video;
lv_obj_t * ui_lbl_fps;

/* Called after the generated UI has been created (after ui_init()).
 * This function assigns `ui_img_video` to the generated image object
 * (objects.panorama_image) and creates a small FPS label placed at
 * top-right so existing code that updates `ui_lbl_fps` will work.
 */
void ui_post_init(void) {
    /* Assign the image object to the generated moil image if available. */
    if(&objects && objects.moil_image) {
        ui_img_video = objects.moil_image;
    } else {
        ui_img_video = NULL;
    }

    /* Create an FPS label on the active screen if none exists. */
    lv_obj_t * scr = lv_scr_act();
    if(scr) {
        ui_lbl_fps = lv_label_create(scr);
        lv_label_set_text(ui_lbl_fps, "FPS: 0.0");
        lv_obj_align(ui_lbl_fps, LV_ALIGN_TOP_RIGHT, -10, 10);
        /* Make sure it's on top */
        lv_obj_move_foreground(ui_lbl_fps);
    } else {
        ui_lbl_fps = NULL;
    }
}
