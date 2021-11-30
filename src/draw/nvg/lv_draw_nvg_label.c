/**
 * @file lv_templ.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../core/lv_refr.h"
#include "../../misc/lv_area.h"
#include "../../core/lv_disp.h"
#include "../../draw/lv_draw_rect.h"
#include "lv_draw_nvg.h"
/*********************
 *      DEFINES
 *********************/

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

void lv_draw_nvg_label(const lv_point_t *pos_p, const lv_area_t *clip_area, const lv_font_t *font_p, uint32_t letter,
                       lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
//    nvgBeginPath(ctx->nvg);
//    nvgRect(ctx->nvg, coords->x1, coords->y1, lv_area_get_width(coords), lv_area_get_height(coords));
//    nvgFillColor(ctx->nvg, nvgRGBA(dsc->bg_color.ch.red, dsc->bg_color.ch.green, dsc->bg_color.ch.blue, dsc->bg_opa));
//    nvgFill(ctx->nvg);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
