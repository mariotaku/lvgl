/**
 * @file lv_templ.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../misc/lv_area.h"
#include "../../core/lv_disp.h"
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

static void draw_bg(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx);

static void draw_border(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/


static void rect_path(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_nvg_rect(const lv_area_t *coords, const lv_area_t *clip, const lv_draw_rect_dsc_t *dsc) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
    lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_FRAME, false);

    // Draw background color
    lv_area_t mask_area;
    bool has_mask = _lv_area_intersect(&mask_area, coords, clip) && lv_draw_nvg_mask_begin(ctx, &mask_area);

    nvgSave(ctx->nvg);

    nvgReset(ctx->nvg);
    nvgScissor(ctx->nvg, clip->x1, clip->y1, lv_area_get_width(clip), lv_area_get_height(clip));

    draw_bg(coords, dsc, ctx);
    _lv_draw_rect_bg_img(coords, clip, dsc);
    draw_border(coords, dsc, ctx);

    nvgRestore(ctx->nvg);

    if (has_mask) {
        lv_draw_nvg_mask_end(ctx, &mask_area);
    }

}

/**********************
 *   STATIC FUNCTIONS
 **********************/


static void draw_bg(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx) {
    if (dsc->bg_opa <= LV_OPA_TRANSP) return;
    nvgBeginPath(ctx->nvg);
    rect_path(coords, dsc, ctx);
    nvgFillColor(ctx->nvg, nvgRGBA(dsc->bg_color.ch.red, dsc->bg_color.ch.green, dsc->bg_color.ch.blue, dsc->bg_opa));
    nvgFill(ctx->nvg);
}


static void draw_border(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx) {
    if (dsc->border_opa <= LV_OPA_TRANSP) return;
    nvgBeginPath(ctx->nvg);
    lv_area_t border_area = *coords;
    lv_area_increase(&border_area, -dsc->border_width / 2, -dsc->border_width / 2);
    rect_path(&border_area, dsc, ctx);
    nvgStrokeWidth(ctx->nvg, dsc->border_width);
    nvgStrokeColor(ctx->nvg, nvgRGBA(dsc->border_color.ch.red, dsc->border_color.ch.green, dsc->border_color.ch.blue,
                                     dsc->border_opa));
    nvgStroke(ctx->nvg);
}

static void rect_path(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx) {
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
}
