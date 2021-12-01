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
#include "lv_draw_nvg_img_cache.h"
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

static int upload_img(NVGcontext *nvg, lv_img_decoder_dsc_t *dsc);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_res_t lv_draw_nvg_img_core(const lv_area_t *coords, const lv_area_t *mask, const void *src,
                              const lv_draw_img_dsc_t *draw_dsc) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
    lv_draw_nvg_begin_frame(ctx, LV_DRAW_NVG_BUFFER_FRAME, false);

    size_t key_size;
    lv_draw_sdl_cache_key_head_img_t *key = lv_nvg_img_cache_key_create(src, draw_dsc->frame_id, &key_size);
    bool cache_found = false;
    int img = lv_nvg_draw_cache_get(key, key_size, &cache_found);
    if (!cache_found) {
        _lv_img_cache_entry_t *cdsc = _lv_img_cache_open(src, draw_dsc->recolor, draw_dsc->frame_id);
        lv_draw_sdl_cache_flag_t tex_flags = 0;
        if (cdsc) {
            lv_img_decoder_dsc_t *dsc = &cdsc->dec_dsc;
            if (dsc->user_data && memcmp(dsc->user_data, LV_DRAW_SDL_DEC_DSC_TEXTURE_HEAD, 8) == 0) {
                img = ((lv_draw_nvg_dec_dsc_userdata_t *) dsc->user_data)->img;
                tex_flags |= LV_DRAW_SDL_CACHE_FLAG_MANAGED;
            } else {
                img = upload_img(ctx->nvg, dsc);
            }
#if LV_IMG_CACHE_DEF_SIZE == 0
            lv_img_decoder_close(dsc);
#endif
        }
        if (img && cdsc) {
            lv_img_header_t *header = lv_mem_alloc(sizeof(lv_img_header_t));
            lv_memcpy(header, &cdsc->dec_dsc.header, sizeof(lv_img_header_t));
            lv_nvg_draw_cache_put_advanced(key, key_size, img, header, lv_mem_free, tex_flags);
        } else {
            lv_nvg_draw_cache_put(key, key_size, 0);
        }
    }
    lv_mem_free(key);
    if (!img) {
        return LV_RES_INV;
    }

    // Draw background color
    lv_area_t mask_area;
    bool has_mask = _lv_area_intersect(&mask_area, coords, mask) && lv_draw_nvg_mask_begin(ctx, &mask_area);

    nvgSave(ctx->nvg);

    nvgScissor(ctx->nvg, mask->x1, mask->y1, lv_area_get_width(mask), lv_area_get_height(mask));

    nvgTranslate(ctx->nvg, coords->x1 + draw_dsc->pivot.x, coords->y1 + draw_dsc->pivot.y);

    nvgBeginPath(ctx->nvg);
    nvgRect(ctx->nvg, -draw_dsc->pivot.x, -draw_dsc->pivot.y, lv_area_get_width(coords), lv_area_get_height(coords));

    int iw = 0, ih = 0;
    if (draw_dsc->angle != 0) {
        nvgRotate(ctx->nvg, nvgDegToRad(draw_dsc->angle));
    }
    if (draw_dsc->zoom != LV_IMG_ZOOM_NONE) {
        float scale = draw_dsc->zoom / ((float) LV_IMG_ZOOM_NONE);
        nvgScale(ctx->nvg, scale, scale);
    }
    nvgImageSize(ctx->nvg, img, &iw, &ih);
    NVGpaint paint = nvgImagePattern(ctx->nvg, -draw_dsc->pivot.x, -draw_dsc->pivot.y, iw, ih, 0, img,
                                     draw_dsc->opa / 255.0f);
    if (draw_dsc->recolor_opa > 0) {
        paint.innerColor = paint.outerColor = nvgRGBA(draw_dsc->recolor.ch.red, draw_dsc->recolor.ch.green,
                                                      draw_dsc->recolor.ch.blue, draw_dsc->recolor_opa);
    } else {
        paint.innerColor = paint.outerColor = nvgRGBA(255, 255, 255, 255);
    }
    nvgFillPaint(ctx->nvg, paint);
    nvgFill(ctx->nvg);

    nvgRestore(ctx->nvg);

    if (has_mask) {
        lv_draw_nvg_mask_end(ctx, &mask_area);
    }
    return LV_RES_OK;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

int upload_img(NVGcontext *nvg, lv_img_decoder_dsc_t *dsc) {
    uint8_t bpp = lv_img_cf_get_px_size(dsc->header.cf) / 8;
    char *rgba = lv_mem_alloc(dsc->header.w * dsc->header.h * 4);
    if (bpp == 4 && dsc->img_data) {
        for (int y = 0; y < dsc->header.h; y++) {
            const lv_color32_t *src_line = (lv_color32_t *) &dsc->img_data[y * dsc->header.w * 4];
            char *dst_line = &rgba[y * dsc->header.w * 4];
            for (int x = 0; x < dsc->header.w; x++) {
                lv_color_t pixel = src_line[x];
                dst_line[x * 4 + 0] = (char) pixel.ch.red;
                dst_line[x * 4 + 1] = (char) pixel.ch.green;
                dst_line[x * 4 + 2] = (char) pixel.ch.blue;
                dst_line[x * 4 + 3] = (char) pixel.ch.alpha;
            }
        }
    } else if (bpp == 3 && dsc->img_data) {
        for (int y = 0; y < dsc->header.h; y++) {
            const uint8_t *src_line = &dsc->img_data[y * dsc->header.w * 3];
            char *dst_line = &rgba[y * dsc->header.w * 4];
            for (int x = 0; x < dsc->header.w; x++) {
                dst_line[x * 4 + 0] = (char) src_line[x * 3 + 0];
                dst_line[x * 4 + 1] = (char) src_line[x * 3 + 1];
                dst_line[x * 4 + 2] = (char) src_line[x * 3 + 2];
                dst_line[x * 4 + 3] = (char) 0xFF;
            }
        }
    } else {
        for (int y = 0; y < dsc->header.h; y++) {
            uint8_t *line = (uint8_t *) &rgba[y * dsc->header.w * 4];
            if (lv_img_decoder_read_line(dsc, 0, y, dsc->header.w, line) != LV_RES_OK) {
                lv_mem_free(rgba);
                return 0;
            }
            for (int x = 0; x < dsc->header.w; x++) {
                lv_color_t pixel = *((lv_color_t *) &line[x * 4]);
                line[x * 4 + 0] = pixel.ch.red;
                line[x * 4 + 1] = pixel.ch.green;
                line[x * 4 + 2] = pixel.ch.blue;
                line[x * 4 + 3] = pixel.ch.alpha;
            }
        }
    }
    int img = nvgCreateImageRGBA(nvg, dsc->header.w, dsc->header.h, 0, rgba);
    lv_mem_free(rgba);
    return img;
}
