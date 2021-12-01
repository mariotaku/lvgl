//
// Created by Mariotaku on 2021/11/30.
//

#include "lv_draw_nvg_mask.h"
#include "lv_draw_nvg_priv.h"

#include "../../misc/lv_gc.h"

static void mask_apply_fade(NVGcontext *nvg, const lv_draw_mask_fade_param_t *param);

static void mask_apply_radius(NVGcontext *nvg, const lv_draw_mask_radius_param_t *param, const lv_area_t *a);

static void mask_apply_line(NVGcontext *nvg, const lv_draw_mask_line_param_t *param, const lv_area_t *a);

static void mask_apply_angle(NVGcontext *nvg, const lv_draw_mask_angle_param_t *param, const lv_area_t *a);

static void render_texture_mask(lv_draw_nvg_context_t *ctx, const lv_area_t *a);


bool lv_draw_nvg_mask_begin(lv_draw_nvg_context_t *ctx, const lv_area_t *a) {
    if (!lv_draw_mask_is_any(a)) {
        return false;
    }

    render_texture_mask(ctx, a);

    lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_COMPOSITE, true);
    return true;
}

void lv_draw_nvg_mask_end(lv_draw_nvg_context_t *ctx, const lv_area_t *a) {
    // Apply mask to composited
    nvgGlobalCompositeBlendFuncSeparate(ctx->nvg, NVG_ZERO, NVG_ONE, NVG_ZERO, NVG_SRC_ALPHA);
    ctx->callbacks.fill_buffer(ctx, LV_DRAW_NVG_BUFFER_MASK, NULL);

    // Restore to last buffer (expect frame)
    lv_draw_nvg_end_frame(ctx);

    nvgSave(ctx->nvg);
    nvgScissor(ctx->nvg, a->x1, a->y1, lv_area_get_width(a), lv_area_get_height(a));
    // Blend composited onto frame
    ctx->callbacks.fill_buffer(ctx, LV_DRAW_NVG_BUFFER_COMPOSITE, NULL);
    nvgRestore(ctx->nvg);
}

static void mask_apply_fade(NVGcontext *nvg, const lv_draw_mask_fade_param_t *param) {
    nvgBeginPath(nvg);
    nvgRect(nvg, param->cfg.coords.x1, param->cfg.coords.y1, lv_area_get_width(&param->cfg.coords),
            lv_area_get_height(&param->cfg.coords));
    NVGpaint paint = nvgLinearGradient(nvg, param->cfg.coords.x1, param->cfg.y_top, param->cfg.coords.x1,
                                       param->cfg.y_bottom,
                                       nvgRGBA(255, 255, 0, param->cfg.opa_top),
                                       nvgRGBA(255, 255, 0, param->cfg.opa_bottom));
    nvgFillPaint(nvg, paint);
    nvgFill(nvg);
}

static void mask_apply_radius(NVGcontext *nvg, const lv_draw_mask_radius_param_t *param, const lv_area_t *a) {
    nvgBeginPath(nvg);
    nvgRoundedRect(nvg, param->cfg.rect.x1, param->cfg.rect.y1, lv_area_get_width(&param->cfg.rect),
                   lv_area_get_height(&param->cfg.rect), param->cfg.radius);
    nvgFillColor(nvg, nvgRGBA(0, 255, 255, 255));
    nvgFill(nvg);
}

static void mask_apply_line(NVGcontext *nvg, const lv_draw_mask_line_param_t *param, const lv_area_t *a) {
    nvgBeginPath(nvg);
    nvgMoveTo(nvg, param->cfg.p1.x, param->cfg.p1.y);
    nvgLineTo(nvg, param->cfg.p2.x, param->cfg.p2.y);
    switch (param->cfg.side) {
        case LV_DRAW_MASK_LINE_SIDE_BOTTOM: {
            nvgLineTo(nvg, param->cfg.p2.x, a->y2);
            nvgLineTo(nvg, param->cfg.p1.x, a->y2);
            break;
        }
        case LV_DRAW_MASK_LINE_SIDE_TOP: {
            nvgLineTo(nvg, param->cfg.p2.x, a->y1);
            nvgLineTo(nvg, param->cfg.p1.x, a->y1);
            break;
        }
        case LV_DRAW_MASK_LINE_SIDE_LEFT: {
            nvgLineTo(nvg, a->x1, a->y2);
            nvgLineTo(nvg, a->x1, a->y1);
            break;
        }
        case LV_DRAW_MASK_LINE_SIDE_RIGHT: {
            nvgLineTo(nvg, a->x2, a->y2);
            nvgLineTo(nvg, a->x2, a->y1);
            break;
        }
    }
    nvgClosePath(nvg);
    nvgFillColor(nvg, nvgRGBA(255, 0, 255, 255));
    nvgFill(nvg);
}

static void mask_apply_angle(NVGcontext *nvg, const lv_draw_mask_angle_param_t *param, const lv_area_t *a) {
    nvgBeginPath(nvg);
    nvgArc(nvg, param->cfg.vertex_p.x, param->cfg.vertex_p.y, 100, nvgDegToRad(param->cfg.start_angle),
           nvgDegToRad(param->cfg.end_angle), NVG_CW);
    nvgFillColor(nvg, nvgRGBA(0, 0, 255, 255));
    nvgFill(nvg);
}

static void render_texture_mask(lv_draw_nvg_context_t *ctx, const lv_area_t *a) {
    bool any_applied = false;
    for (uint8_t i = 0; i < _LV_MASK_MAX_NUM; i++) {
        _lv_draw_mask_common_dsc_t *comm_param = LV_GC_ROOT(_lv_draw_mask_list[i]).param;
        if (comm_param == NULL) continue;
        bool first = !any_applied, handled = false, inverted = false;
        lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_MASK, true);
        switch (comm_param->type) {
            case LV_DRAW_MASK_TYPE_LINE: {
                const lv_draw_mask_line_param_t *param = (const lv_draw_mask_line_param_t *) comm_param;
                mask_apply_line(ctx->nvg, param, a);
                any_applied = true;
                handled = true;
                break;
            }
            case LV_DRAW_MASK_TYPE_RADIUS: {
                const lv_draw_mask_radius_param_t *param = (const lv_draw_mask_radius_param_t *) comm_param;
                inverted = param->cfg.outer;
                mask_apply_radius(ctx->nvg, param, a);
                any_applied = true;
                handled = true;
                break;
            }
            case LV_DRAW_MASK_TYPE_FADE: {
                const lv_draw_mask_fade_param_t *param = (const lv_draw_mask_fade_param_t *) comm_param;
                mask_apply_fade(ctx->nvg, param);
                any_applied = true;
                handled = true;
                break;
            }
            case LV_DRAW_MASK_TYPE_ANGLE: {
                const lv_draw_mask_angle_param_t *param = (const lv_draw_mask_angle_param_t *) comm_param;
                mask_apply_angle(ctx->nvg, param, a);
                any_applied = true;
                handled = true;
                break;
            }
            default: {
                break;
            }
        }
        lv_draw_nvg_end_frame(ctx);
        if (handled) {
            lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_COMPOSITE, first);
            if (inverted) {
                if (first) {
                    nvgBeginPath(ctx->nvg);
                    nvgRect(ctx->nvg, a->x1, a->y1, lv_area_get_width(a), lv_area_get_height(a));
                    nvgFillColor(ctx->nvg, nvgRGBA(0, 255, 255, 255));
                    nvgFill(ctx->nvg);
                }
                nvgGlobalCompositeOperation(ctx->nvg, NVG_DESTINATION_OUT);
            } else if (first) {
                nvgGlobalCompositeOperation(ctx->nvg, NVG_SOURCE_OVER);
            } else {
                nvgGlobalCompositeOperation(ctx->nvg, NVG_DESTINATION_IN);
            }
            // Blend composited onto frame
            ctx->callbacks.fill_buffer(ctx, LV_DRAW_NVG_BUFFER_MASK, NULL);
            lv_draw_nvg_end_frame(ctx);
        }
    }

    // Overwrite mask with composite
    lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_MASK, true);
    ctx->callbacks.fill_buffer(ctx, LV_DRAW_NVG_BUFFER_COMPOSITE, NULL);
    lv_draw_nvg_end_frame(ctx);
}
