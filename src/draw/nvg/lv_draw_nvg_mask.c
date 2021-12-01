//
// Created by Mariotaku on 2021/11/30.
//

#include "lv_draw_nvg_mask.h"

#include "../lv_draw_mask.h"
#include "../../misc/lv_gc.h"

static bool mask_apply_fade(NVGcontext *nvg, const lv_draw_mask_fade_param_t *param);

static bool mask_apply_radius(NVGcontext *nvg, const lv_draw_mask_radius_param_t *param);

static bool mask_apply_line(NVGcontext *nvg, const lv_draw_mask_line_param_t *param, const lv_area_t *a);


bool lv_draw_nvg_mask_begin(lv_draw_nvg_context_t *ctx, const lv_area_t *a) {
    if (!lv_draw_mask_is_any(a) || true) {
        return false;
    }
    // Set render target to texture buffer
    ctx->callbacks.set_render_buffer(ctx, LV_DRAW_NVG_BUFFER_TEMP);

    nvgSave(ctx->nvg);
    nvgReset(ctx->nvg);

    nvgReset(ctx->nvg);
    return true;
}

void lv_draw_nvg_mask_end(lv_draw_nvg_context_t *ctx, const lv_area_t *a) {
    nvgReset(ctx->nvg);
    bool mask_applied = false;
    for (uint8_t i = 0; i < _LV_MASK_MAX_NUM; i++) {
        _lv_draw_mask_common_dsc_t *comm_param = LV_GC_ROOT(_lv_draw_mask_list[i]).param;
        if (comm_param == NULL) continue;
        nvgScissor(ctx->nvg, a->x1, a->y1, lv_area_get_width(a), lv_area_get_height(a));
        switch (comm_param->type) {
            case LV_DRAW_MASK_TYPE_LINE: {
                mask_applied |= mask_apply_line(ctx->nvg, (const lv_draw_mask_line_param_t *) comm_param, a);
                break;
            }
            case LV_DRAW_MASK_TYPE_RADIUS: {
                mask_applied |= mask_apply_radius(ctx->nvg, (const lv_draw_mask_radius_param_t *) comm_param);
                break;
            }
            case LV_DRAW_MASK_TYPE_FADE: {
                mask_applied |= mask_apply_fade(ctx->nvg, (const lv_draw_mask_fade_param_t *) comm_param);
                break;
            }
            default: {
                break;
            }
        }
    }
//    nvgGlobalCompositeOperation(ctx->nvg, NVG_SOURCE_OVER);

    // Set render target to texture buffer
    ctx->callbacks.set_render_buffer(ctx, LV_DRAW_NVG_BUFFER_FRAME);
    ctx->callbacks.submit_buffer(ctx, LV_DRAW_NVG_BUFFER_TEMP, a, false);

    nvgRestore(ctx->nvg);
}

static bool mask_apply_fade(NVGcontext *nvg, const lv_draw_mask_fade_param_t *param) {
    nvgBeginPath(nvg);
    nvgRect(nvg, param->cfg.coords.x1, param->cfg.coords.y1, lv_area_get_width(&param->cfg.coords),
            lv_area_get_height(&param->cfg.coords));
    NVGpaint paint = nvgLinearGradient(nvg, param->cfg.coords.x1, param->cfg.y_top, param->cfg.coords.x1,
                                       param->cfg.y_bottom,
                                       nvgRGBA(255, 255, 0, param->cfg.opa_top),
                                       nvgRGBA(255, 255, 0, param->cfg.opa_bottom));
    nvgFillPaint(nvg, paint);
    nvgFill(nvg);
    return true;
}

static bool mask_apply_radius(NVGcontext *nvg, const lv_draw_mask_radius_param_t *param) {
    nvgBeginPath(nvg);
    nvgRoundedRect(nvg, param->cfg.rect.x1, param->cfg.rect.y1, lv_area_get_width(&param->cfg.rect),
                   lv_area_get_height(&param->cfg.rect), param->cfg.radius);
    nvgFillColor(nvg, nvgRGBA(0, 255, 255, 255));
    nvgFill(nvg);
    return true;
}

static bool mask_apply_line(NVGcontext *nvg, const lv_draw_mask_line_param_t *param, const lv_area_t *a) {
    nvgBeginPath(nvg);
    // TODO: inverted mode
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
    return true;
}
