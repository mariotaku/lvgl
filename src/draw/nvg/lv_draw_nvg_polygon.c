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
#include "lv_draw_nvg_priv.h"
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

void lv_draw_nvg_polygon(const lv_point_t points[], uint16_t point_cnt, const lv_area_t *clip,
                         const lv_draw_rect_dsc_t *dsc) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
    lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_FRAME, false);

    nvgSave(ctx->nvg);

    nvgScissor(ctx->nvg, clip->x1, clip->y1, lv_area_get_width(clip), lv_area_get_height(clip));

    nvgBeginPath(ctx->nvg);
    nvgMoveTo(ctx->nvg, points[0].x, points[0].y);
    for (int i = 1; i < point_cnt; i++) {
        nvgLineTo(ctx->nvg, points[i].x, points[i].y);
    }
    nvgClosePath(ctx->nvg);

    nvgFillColor(ctx->nvg, nvgRGBA(dsc->bg_color.ch.red, dsc->bg_color.ch.green, dsc->bg_color.ch.blue, dsc->bg_opa));
    nvgFill(ctx->nvg);

    nvgRestore(ctx->nvg);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
