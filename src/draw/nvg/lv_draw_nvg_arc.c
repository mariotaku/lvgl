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

void lv_draw_nvg_arc(lv_coord_t center_x, lv_coord_t center_y, uint16_t radius, uint16_t start_angle,
                     uint16_t end_angle, const lv_area_t *clip_area, const lv_draw_arc_dsc_t *dsc) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
    lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_FRAME, false);

    nvgSave(ctx->nvg);

    nvgScissor(ctx->nvg, clip_area->x1, clip_area->y1, lv_area_get_width(clip_area), lv_area_get_height(clip_area));

    nvgBeginPath(ctx->nvg);
    nvgArc(ctx->nvg, center_x, center_y, radius - dsc->width / 2.0f, nvgDegToRad(start_angle), nvgDegToRad(end_angle),
           NVG_CW);

    nvgStrokeWidth(ctx->nvg, dsc->width);
    nvgStrokeColor(ctx->nvg, nvgRGBA(dsc->color.ch.red, dsc->color.ch.green, dsc->color.ch.blue, dsc->opa));
    nvgStroke(ctx->nvg);

    nvgRestore(ctx->nvg);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
