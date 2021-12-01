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

static void draw_outline(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx);

static void draw_shadow(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx);

static void rect_path(const lv_area_t *coords, lv_coord_t radius, lv_draw_nvg_context_t *ctx);
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/
#define SKIP_BORDER(dsc) ((dsc)->border_opa <= LV_OPA_MIN || (dsc)->border_width == 0 || (dsc)->border_side == LV_BORDER_SIDE_NONE || (dsc)->border_post)
#define SKIP_SHADOW(dsc) ((dsc)->shadow_width == 0 || (dsc)->shadow_opa <= LV_OPA_MIN || ((dsc)->shadow_width == 1 && (dsc)->shadow_spread <= 0 && (dsc)->shadow_ofs_x == 0 && (dsc)->shadow_ofs_y == 0))
#define SKIP_IMAGE(dsc) ((dsc)->bg_img_src == NULL || (dsc)->bg_img_opa <= LV_OPA_MIN)
#define SKIP_OUTLINE(dsc) ((dsc)->outline_opa <= LV_OPA_MIN || (dsc)->outline_width == 0)


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

    draw_shadow(coords, dsc, ctx);

    draw_bg(coords, dsc, ctx);
    _lv_draw_rect_bg_img(coords, clip, dsc);
    draw_border(coords, dsc, ctx);

    draw_outline(coords, dsc, ctx);

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
    rect_path(coords, dsc->radius, ctx);
    nvgFillColor(ctx->nvg, nvgRGBA(dsc->bg_color.ch.red, dsc->bg_color.ch.green, dsc->bg_color.ch.blue, dsc->bg_opa));
    nvgFill(ctx->nvg);
}


static void draw_border(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx) {
    if (dsc->border_opa <= LV_OPA_TRANSP) return;
    if (dsc->border_side == LV_BORDER_SIDE_NONE || dsc->border_width <= 0) return;
    lv_area_t border_area = *coords;
    lv_area_increase(&border_area, -dsc->border_width / 2, -dsc->border_width / 2);
    NVGcolor border_color = nvgRGBA(dsc->border_color.ch.red, dsc->border_color.ch.green, dsc->border_color.ch.blue,
                                    dsc->border_opa);
    nvgBeginPath(ctx->nvg);
    if (dsc->border_side == LV_BORDER_SIDE_FULL) {
        rect_path(&border_area, dsc->radius - dsc->border_width / 2, ctx);
    } else {
        if (dsc->border_side & LV_BORDER_SIDE_LEFT) {
            nvgMoveTo(ctx->nvg, border_area.x1, border_area.y1 - dsc->border_width / 2);
            nvgLineTo(ctx->nvg, border_area.x1, border_area.y2 + dsc->border_width / 2);
        }
        if (dsc->border_side & LV_BORDER_SIDE_BOTTOM) {
            nvgMoveTo(ctx->nvg, border_area.x1 - dsc->border_width / 2, border_area.y2);
            nvgLineTo(ctx->nvg, border_area.x2 + dsc->border_width / 2, border_area.y2);
        }
        if (dsc->border_side & LV_BORDER_SIDE_RIGHT) {
            nvgMoveTo(ctx->nvg, border_area.x2, border_area.y2 + dsc->border_width / 2);
            nvgLineTo(ctx->nvg, border_area.x2, border_area.y1 - dsc->border_width / 2);
        }
        if (dsc->border_side & LV_BORDER_SIDE_TOP) {
            nvgMoveTo(ctx->nvg, border_area.x2 + dsc->border_width / 2, border_area.y1);
            nvgLineTo(ctx->nvg, border_area.x1 - dsc->border_width / 2, border_area.y1);
        }
    }
    nvgStrokeWidth(ctx->nvg, dsc->border_width);
    nvgStrokeColor(ctx->nvg, border_color);
    nvgStroke(ctx->nvg);
}

static void draw_outline(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx) {
    if (dsc->outline_opa <= LV_OPA_MIN) return;
    if (dsc->outline_width == 0) return;

    lv_opa_t opa = dsc->outline_opa;

    if (opa > LV_OPA_MAX) opa = LV_OPA_COVER;

    /*Get the inner radius*/
    lv_area_t border_area;
    lv_area_copy(&border_area, coords);

    lv_area_increase(&border_area, dsc->outline_pad, dsc->outline_pad);
    lv_area_increase(&border_area, dsc->outline_width / 2, dsc->outline_width / 2);


    NVGcolor border_color = nvgRGBA(dsc->outline_color.ch.red, dsc->outline_color.ch.green, dsc->outline_color.ch.blue,
                                    dsc->outline_opa);
    nvgBeginPath(ctx->nvg);
    rect_path(&border_area, dsc->radius + dsc->outline_width / 2, ctx);

    nvgStrokeWidth(ctx->nvg, dsc->border_width);
    nvgStrokeColor(ctx->nvg, border_color);
    nvgStroke(ctx->nvg);
}

static void draw_shadow(const lv_area_t *coords, const lv_draw_rect_dsc_t *dsc, lv_draw_nvg_context_t *ctx) {
    if (SKIP_SHADOW(dsc)) return;

    lv_area_t shadow_coords = *coords;
    lv_area_move(&shadow_coords, dsc->shadow_ofs_x, dsc->shadow_ofs_y);
    lv_area_increase(&shadow_coords, dsc->shadow_spread, dsc->shadow_spread);

    NVGcolor shadow_color = nvgRGB(dsc->shadow_color.ch.red, dsc->shadow_color.ch.green, dsc->shadow_color.ch.blue);
    NVGpaint paint = nvgBoxGradient(ctx->nvg, shadow_coords.x1, shadow_coords.y1, lv_area_get_width(&shadow_coords),
                                    lv_area_get_height(&shadow_coords), dsc->radius, dsc->shadow_width,
                                    nvgTransRGBA(shadow_color, dsc->shadow_opa),
                                    nvgTransRGBA(shadow_color, 0));

    lv_area_increase(&shadow_coords, dsc->shadow_width / 2, dsc->shadow_width / 2);

    nvgBeginPath(ctx->nvg);
    nvgRect(ctx->nvg, shadow_coords.x1, shadow_coords.y1, lv_area_get_width(&shadow_coords),
            lv_area_get_height(&shadow_coords));
    nvgFillPaint(ctx->nvg, paint);
    nvgFill(ctx->nvg);
}

static void rect_path(const lv_area_t *coords, lv_coord_t radius, lv_draw_nvg_context_t *ctx) {
    lv_coord_t w = lv_area_get_width(coords);
    lv_coord_t h = lv_area_get_height(coords);
    if (radius == LV_RADIUS_CIRCLE) {
        nvgRoundedRect(ctx->nvg, coords->x1, coords->y1, w, h, LV_MIN(w, h) / 2.0f);
    } else if (radius > 0) {
        nvgRoundedRect(ctx->nvg, coords->x1, coords->y1, w, h, radius);
    } else {
        nvgRect(ctx->nvg, coords->x1, coords->y1, w, h);
    }
}
