/**
 * @file lv_draw_nvg.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_nvg.h"
#include "../../core/lv_refr.h"

/*********************
 *      DEFINES
 *********************/

void lv_draw_nvg_rect(const lv_area_t *coords, const lv_area_t *clip, const lv_draw_rect_dsc_t *dsc);

void lv_draw_nvg_label(const lv_point_t *pos_p, const lv_area_t *clip_area, const lv_font_t *font_p, uint32_t letter,
                       lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode);

lv_res_t lv_draw_nvg_img_core(const lv_area_t *coords, const lv_area_t *mask, const void *src,
                              const lv_draw_img_dsc_t *draw_dsc);
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_nvg_init(lv_draw_backend_t *backend) {
    lv_draw_backend_init(backend);
    backend->draw_rect = lv_draw_nvg_rect;
    backend->draw_letter = lv_draw_nvg_label;
    backend->draw_img_core = lv_draw_nvg_img_core;
}

lv_draw_nvg_context_t *lv_draw_nvg_current_context() {
    lv_disp_t *disp = _lv_refr_get_disp_refreshing();
    return disp->driver->user_data;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
