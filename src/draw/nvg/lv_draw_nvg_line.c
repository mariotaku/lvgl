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
#include "lv_draw_nvg_mask.h"
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

void lv_draw_nvg_line(const lv_point_t *point1, const lv_point_t *point2, const lv_area_t *clip,
                      const lv_draw_line_dsc_t *dsc) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
    lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_FRAME, false);

    // Draw background color
    bool has_mask = lv_draw_nvg_mask_begin(ctx, clip);

    nvgSave(ctx->nvg);

    nvgScissor(ctx->nvg, clip->x1, clip->y1, lv_area_get_width(clip), lv_area_get_height(clip));

    nvgBeginPath(ctx->nvg);
    nvgMoveTo(ctx->nvg, point1->x, point1->y);
    nvgLineTo(ctx->nvg, point2->x, point2->y);
    nvgStrokeWidth(ctx->nvg, dsc->width);
    nvgStrokeColor(ctx->nvg, nvgRGBA(dsc->color.ch.red, dsc->color.ch.green, dsc->color.ch.blue, dsc->opa));

    nvgStroke(ctx->nvg);

    nvgRestore(ctx->nvg);

    if (has_mask) {
        lv_draw_nvg_mask_end(ctx, clip);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
