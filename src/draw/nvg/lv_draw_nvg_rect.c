/**
 * @file lv_templ.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../misc/lv_area.h"
#include "../../core/lv_disp.h"
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

void lv_draw_nvg_rect(const lv_area_t *coords, const lv_area_t *clip, const lv_draw_rect_dsc_t *dsc) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
    nvgSave(ctx->nvg);

    nvgReset(ctx->nvg);
    nvgScissor(ctx->nvg, clip->x1, clip->y1, lv_area_get_width(clip), lv_area_get_height(clip));

    nvgBeginPath(ctx->nvg);
    if (dsc->radius == LV_RADIUS_CIRCLE) {
        lv_coord_t w = lv_area_get_width(coords);
        lv_coord_t h = lv_area_get_height(coords);
        nvgRoundedRect(ctx->nvg, coords->x1, coords->y1, w, h, LV_MIN(w, h) / 2.0f);
    } else if (dsc->radius > 0) {
        nvgRoundedRect(ctx->nvg, coords->x1, coords->y1, lv_area_get_width(coords), lv_area_get_height(coords),
                       dsc->radius);
    } else {
        nvgRect(ctx->nvg, coords->x1, coords->y1, lv_area_get_width(coords), lv_area_get_height(coords));
    }
    nvgFillColor(ctx->nvg, nvgRGBA(dsc->bg_color.ch.red, dsc->bg_color.ch.green, dsc->bg_color.ch.blue, dsc->bg_opa));
    nvgFill(ctx->nvg);

    nvgRestore(ctx->nvg);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
