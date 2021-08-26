#include "../../lv_conf_internal.h"

#if LV_USE_GPU_SDL

#include "draw/lv_draw_rect.h"
#include "hal/lv_hal_disp.h"
#include "core/lv_refr.h"
#include "lv_gpu_sdl_utils.h"
#include "lv_gpu_sdl_lru.h"
#include "lv_gpu_draw_cache.h"
#include "lv_gpu_sdl_mask.h"

typedef struct {
    lv_gpu_cache_key_magic_t magic;
    lv_coord_t radius;
    lv_coord_t size;
} lv_draw_rect_bg_key_t;

typedef struct {
    lv_gpu_cache_key_magic_t magic;
    lv_coord_t radius;
    lv_coord_t size;
    lv_coord_t blur;
} lv_draw_rect_shadow_key_t;

typedef struct {
    lv_gpu_cache_key_magic_t magic;
    lv_coord_t rout, rin;
    lv_coord_t thickness;
    lv_coord_t size;
    lv_border_side_t side;
} lv_draw_rect_border_key_t;

static void draw_bg_color(SDL_Renderer *renderer, const lv_area_t *coords, const SDL_Rect *mask_rect,
                          const lv_draw_rect_dsc_t *dsc);

static void draw_bg_img(const lv_area_t *coords, const lv_area_t *clip,
                        const lv_draw_rect_dsc_t *dsc);

static void draw_border(SDL_Renderer *renderer, const lv_area_t *coords, const lv_area_t *mask,
                        const lv_draw_rect_dsc_t *dsc);

static void draw_shadow(SDL_Renderer *renderer, const lv_area_t *coords, const SDL_Rect *mask_rect,
                        const lv_draw_rect_dsc_t *dsc);

static void draw_outline(const lv_area_t *coords, const lv_area_t *clip, const lv_draw_rect_dsc_t *dsc);

static void draw_border_generic(const lv_area_t *clip_area, const lv_area_t *outer_area, const lv_area_t *inner_area,
                                lv_coord_t rout, lv_coord_t rin, lv_color_t color, lv_opa_t opa,
                                lv_blend_mode_t blend_mode);

static void draw_border_simple(const lv_area_t *clip, const lv_area_t *outer_area, const lv_area_t *inner_area,
                               lv_color_t color, lv_opa_t opa);

static void draw_bg_compat(SDL_Renderer *renderer, const lv_area_t *coords,
                           const SDL_Rect *coords_rect, const SDL_Rect *mask_rect,
                           const lv_draw_rect_dsc_t *dsc);

static void render_corners(SDL_Renderer *renderer, SDL_Texture *frag, lv_coord_t frag_size, const lv_area_t *coords);

static void render_borders(SDL_Renderer *renderer, SDL_Texture *frag, lv_coord_t frag_size, const lv_area_t *coords);

static void render_center(SDL_Renderer *renderer, SDL_Texture *frag, lv_coord_t frag_size, const lv_area_t *coords);

void lv_draw_rect(const lv_area_t *coords, const lv_area_t *mask, const lv_draw_rect_dsc_t *dsc) {
    lv_disp_t *disp = _lv_refr_get_disp_refreshing();
    SDL_Renderer *renderer = (SDL_Renderer *) disp->driver->user_data;

    SDL_Rect coords_rect, mask_rect, render_rect;
    lv_area_to_sdl_rect(coords, &coords_rect);
    lv_area_to_sdl_rect(mask, &mask_rect);
    SDL_IntersectRect(&coords_rect, &mask_rect, &render_rect);
    if (SDL_RectEmpty(&render_rect)) {
        return;
    }
    if (lv_draw_mask_get_cnt() > 0) {
        draw_bg_compat(renderer, coords, &coords_rect, &mask_rect, dsc);
    } else {
        draw_shadow(renderer, coords, &mask_rect, dsc);
        draw_bg_color(renderer, coords, &mask_rect, dsc);
        draw_bg_img(coords, mask, dsc);
        draw_border(renderer, coords, mask, dsc);

        // Outline
        draw_outline(coords, mask, dsc);
    }
}

void draw_bg_color(SDL_Renderer *renderer, const lv_area_t *coords, const SDL_Rect *mask_rect,
                   const lv_draw_rect_dsc_t *dsc) {
    SDL_Color bg_color;
    lv_color_to_sdl_color(&dsc->bg_color, &bg_color);
    lv_coord_t radius = dsc->radius;
    if (radius > 0) {
        // A small texture with a quarter of the rect is enough
        lv_coord_t bg_w = lv_area_get_width(coords), bg_h = lv_area_get_height(coords), bg_min = LV_MIN(bg_w, bg_h);
        /* If size isn't times of 2, increase 1 px */
        lv_coord_t min_half = bg_min % 2 == 0 ? bg_min / 2 : bg_min / 2 + 1;
        lv_coord_t frag_size = radius == LV_RADIUS_CIRCLE ? min_half : LV_MIN(radius + 1, min_half);
        lv_draw_rect_bg_key_t key = {.magic = LV_GPU_CACHE_KEY_MAGIC_RECT_BG, .radius = radius, .size = min_half};
        lv_area_t coords_frag;
        lv_area_copy(&coords_frag, coords);
        lv_area_set_width(&coords_frag, frag_size);
        lv_area_set_height(&coords_frag, frag_size);
        SDL_Texture *texture = lv_gpu_draw_cache_get(&key, sizeof(key), NULL);
        if (texture == NULL) {
            lv_draw_mask_radius_param_t mask_rout_param;
            lv_draw_mask_radius_init(&mask_rout_param, coords, radius, false);
            int16_t mask_rout_id = lv_draw_mask_add(&mask_rout_param, NULL);
            texture = lv_sdl_gen_mask_texture(renderer, &coords_frag);
            lv_draw_mask_remove_id(mask_rout_id);
            SDL_assert(texture);
            lv_gpu_draw_cache_put(&key, sizeof(key), texture);
        }

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(texture, dsc->bg_opa);
        SDL_SetTextureColorMod(texture, bg_color.r, bg_color.g, bg_color.b);
        SDL_RenderSetClipRect(renderer, mask_rect);
        render_corners(renderer, texture, frag_size, coords);
        render_borders(renderer, texture, frag_size, coords);
        render_center(renderer, texture, frag_size, coords);
    } else {
        SDL_Rect coords_rect;
        lv_area_to_sdl_rect(coords, &coords_rect);
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, dsc->bg_opa);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderSetClipRect(renderer, mask_rect);
        SDL_RenderFillRect(renderer, &coords_rect);
    }
}


void draw_bg_img(const lv_area_t *coords, const lv_area_t *clip,
                 const lv_draw_rect_dsc_t *dsc) {
    if (dsc->bg_img_src == NULL) return;
    if (dsc->bg_img_opa <= LV_OPA_MIN) return;

    lv_img_src_t src_type = lv_img_src_get_type(dsc->bg_img_src);
    if (src_type == LV_IMG_SRC_SYMBOL) {
        lv_point_t size;
        lv_txt_get_size(&size, dsc->bg_img_src, dsc->bg_img_symbol_font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        lv_area_t a;
        a.x1 = coords->x1 + lv_area_get_width(coords) / 2 - size.x / 2;
        a.x2 = a.x1 + size.x - 1;
        a.y1 = coords->y1 + lv_area_get_height(coords) / 2 - size.y / 2;
        a.y2 = a.y1 + size.y - 1;

        lv_draw_label_dsc_t label_draw_dsc;
        lv_draw_label_dsc_init(&label_draw_dsc);
        label_draw_dsc.font = dsc->bg_img_symbol_font;
        label_draw_dsc.color = dsc->bg_img_recolor;
        label_draw_dsc.opa = dsc->bg_img_opa;
        lv_draw_label(&a, clip, &label_draw_dsc, dsc->bg_img_src, NULL);
    } else {
        lv_img_header_t header;
        lv_res_t res = lv_img_decoder_get_info(dsc->bg_img_src, &header);
        if (res != LV_RES_OK) {
            LV_LOG_WARN("Coudn't read the background image");
            return;
        }

        lv_draw_img_dsc_t img_dsc;
        lv_draw_img_dsc_init(&img_dsc);
        img_dsc.blend_mode = dsc->blend_mode;
        img_dsc.recolor = dsc->bg_img_recolor;
        img_dsc.recolor_opa = dsc->bg_img_recolor_opa;
        img_dsc.opa = dsc->bg_img_opa;

        /*Center align*/
        if (dsc->bg_img_tiled == false) {
            lv_area_t area;
            area.x1 = coords->x1 + lv_area_get_width(coords) / 2 - header.w / 2;
            area.y1 = coords->y1 + lv_area_get_height(coords) / 2 - header.h / 2;
            area.x2 = area.x1 + header.w - 1;
            area.y2 = area.y1 + header.h - 1;

            lv_draw_img(&area, clip, dsc->bg_img_src, &img_dsc);
        } else {
            lv_area_t area;
            area.y1 = coords->y1;
            area.y2 = area.y1 + header.h - 1;

            for (; area.y1 <= coords->y2; area.y1 += header.h, area.y2 += header.h) {

                area.x1 = coords->x1;
                area.x2 = area.x1 + header.w - 1;
                for (; area.x1 <= coords->x2; area.x1 += header.w, area.x2 += header.w) {
                    lv_draw_img(&area, clip, dsc->bg_img_src, &img_dsc);
                }
            }
        }
    }
}

void draw_shadow(SDL_Renderer *renderer, const lv_area_t *coords, const SDL_Rect *mask_rect,
                 const lv_draw_rect_dsc_t *dsc) {
    /*Check whether the shadow is visible*/
    if (dsc->shadow_width == 0) return;
    if (dsc->shadow_opa <= LV_OPA_MIN) return;

    if (dsc->shadow_width == 1 && dsc->shadow_ofs_x == 0 &&
        dsc->shadow_ofs_y == 0 && dsc->shadow_spread <= 0) {
        return;
    }

    lv_coord_t sw = dsc->shadow_width;

    lv_area_t sh_rect_area;
    sh_rect_area.x1 = coords->x1 + dsc->shadow_ofs_x - dsc->shadow_spread;
    sh_rect_area.x2 = coords->x2 + dsc->shadow_ofs_x + dsc->shadow_spread;
    sh_rect_area.y1 = coords->y1 + dsc->shadow_ofs_y - dsc->shadow_spread;
    sh_rect_area.y2 = coords->y2 + dsc->shadow_ofs_y + dsc->shadow_spread;

    lv_area_t sh_area;
    sh_area.x1 = sh_rect_area.x1 - sw / 2 - 1;
    sh_area.x2 = sh_rect_area.x2 + sw / 2 + 1;
    sh_area.y1 = sh_rect_area.y1 - sw / 2 - 1;
    sh_area.y2 = sh_rect_area.y2 + sw / 2 + 1;

    lv_opa_t opa = dsc->shadow_opa;

    if (opa > LV_OPA_MAX) opa = LV_OPA_COVER;

    SDL_Rect sh_area_rect;
    lv_area_to_sdl_rect(&sh_area, &sh_area_rect);

    lv_coord_t radius = dsc->radius;
    lv_coord_t sh_width = lv_area_get_width(&sh_rect_area), sh_height = lv_area_get_height(&sh_rect_area),
            sh_min = LV_MIN(sh_width, sh_height);
    /* If size isn't times of 2, increase 1 px */
    lv_coord_t min_half = sh_min % 2 == 0 ? sh_min / 2 : sh_min / 2 + 1;
    /* No matter how big the shadow is, what we need is just a corner */
    lv_coord_t frag_size = radius == LV_RADIUS_CIRCLE ? min_half : LV_MIN(radius + 1, min_half);
    /* This is how big the corner is after blurring */
    lv_coord_t blur_frag_size = frag_size + sw + 2;

    lv_draw_rect_shadow_key_t key = {.magic = LV_GPU_CACHE_KEY_MAGIC_RECT_SHADOW, .radius = radius, .size = min_half,
            .blur = sw};

    lv_area_t blur_frag;
    lv_area_copy(&blur_frag, &sh_area);
    lv_area_set_width(&blur_frag, blur_frag_size * 2);
    lv_area_set_height(&blur_frag, blur_frag_size * 2);
    SDL_Texture *texture = lv_gpu_draw_cache_get(&key, sizeof(key), NULL);
    if (texture == NULL) {
        lv_draw_mask_radius_param_t mask_rout_param;
        lv_draw_mask_radius_init(&mask_rout_param, &sh_rect_area, radius, false);
        int16_t mask_rout_id = lv_draw_mask_add(&mask_rout_param, NULL);
        lv_opa_t *mask_buf = lv_draw_mask_dump(&blur_frag);
        lv_draw_mask_blur(mask_buf, lv_area_get_width(&blur_frag), lv_area_get_height(&blur_frag), sw / 2 + 1);
        texture = lv_sdl_create_mask_texture(renderer, mask_buf, blur_frag_size, blur_frag_size,
                                              lv_area_get_width(&blur_frag));
        lv_mem_buf_release(mask_buf);
        lv_draw_mask_remove_id(mask_rout_id);
        SDL_assert(texture);
        lv_gpu_draw_cache_put(&key, sizeof(key), texture);
    }

    SDL_Color shadow_color;
    lv_color_to_sdl_color(&dsc->shadow_color, &shadow_color);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, opa);
    SDL_SetTextureColorMod(texture, shadow_color.r, shadow_color.g, shadow_color.b);
    SDL_RenderSetClipRect(renderer, mask_rect);

    render_corners(renderer, texture, blur_frag_size, &sh_area);
    render_borders(renderer, texture, blur_frag_size, &sh_area);
    render_center(renderer, texture, blur_frag_size, &sh_area);
}


void draw_border(SDL_Renderer *renderer, const lv_area_t *coords, const lv_area_t *mask,
                 const lv_draw_rect_dsc_t *dsc) {
    if (dsc->border_opa <= LV_OPA_MIN) return;
    if (dsc->border_width == 0) return;
    if (dsc->border_side == LV_BORDER_SIDE_NONE) return;
    if (dsc->border_post) return;

    SDL_Color border_color;
    lv_color_to_sdl_color(&dsc->border_color, &border_color);


    if (dsc->border_side != LV_BORDER_SIDE_FULL) {
        SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, dsc->border_opa);
        SDL_Rect mask_rect;
        lv_area_to_sdl_rect(mask, &mask_rect);
        SDL_RenderSetClipRect(renderer, &mask_rect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (int w = 0; w <= dsc->border_width; w++) {
            if (dsc->border_side & LV_BORDER_SIDE_TOP) {
                SDL_RenderDrawLine(renderer, coords->x1, coords->y1 + w, coords->x2, coords->y1 + w);
            }
            if (dsc->border_side & LV_BORDER_SIDE_BOTTOM) {
                SDL_RenderDrawLine(renderer, coords->x1, coords->y2 - w, coords->x2, coords->y2 - w);
            }
            if (dsc->border_side & LV_BORDER_SIDE_LEFT) {
                SDL_RenderDrawLine(renderer, coords->x1 + w, coords->y1, coords->x1 + w, coords->y2);
            }
            if (dsc->border_side & LV_BORDER_SIDE_RIGHT) {
                SDL_RenderDrawLine(renderer, coords->x2 - w, coords->y1, coords->x2 - w, coords->y2);
            }
        }
    } else {
        int32_t coords_w = lv_area_get_width(coords);
        int32_t coords_h = lv_area_get_height(coords);
        int32_t rout = dsc->radius;
        int32_t short_side = LV_MIN(coords_w, coords_h);
        if (rout > short_side >> 1) rout = short_side >> 1;

        /*Get the inner area*/
        lv_area_t area_inner;
        lv_area_copy(&area_inner, coords);
        area_inner.x1 += ((dsc->border_side & LV_BORDER_SIDE_LEFT) ? dsc->border_width : -(dsc->border_width + rout));
        area_inner.x2 -= ((dsc->border_side & LV_BORDER_SIDE_RIGHT) ? dsc->border_width : -(dsc->border_width + rout));
        area_inner.y1 += ((dsc->border_side & LV_BORDER_SIDE_TOP) ? dsc->border_width : -(dsc->border_width + rout));
        area_inner.y2 -= ((dsc->border_side & LV_BORDER_SIDE_BOTTOM) ? dsc->border_width : -(dsc->border_width + rout));

        lv_coord_t rin = rout - dsc->border_width;
        if (rin < 0) rin = 0;
        draw_border_generic(mask, coords, &area_inner, rout, rin, dsc->border_color, dsc->border_opa,
                            dsc->blend_mode);
    }
}

static void draw_outline(const lv_area_t *coords, const lv_area_t *clip, const lv_draw_rect_dsc_t *dsc) {
    if (dsc->outline_opa <= LV_OPA_MIN) return;
    if (dsc->outline_width == 0) return;

    lv_opa_t opa = dsc->outline_opa;

    if (opa > LV_OPA_MAX) opa = LV_OPA_COVER;

    /*Get the inner radius*/
    lv_area_t area_inner;
    lv_area_copy(&area_inner, coords);

    /*Extend the outline into the background area if it's overlapping the edge*/
    lv_coord_t pad = (dsc->outline_pad == 0 ? (dsc->outline_pad - 1) : dsc->outline_pad);
    area_inner.x1 -= pad;
    area_inner.y1 -= pad;
    area_inner.x2 += pad;
    area_inner.y2 += pad;

    lv_area_t area_outer;
    lv_area_copy(&area_outer, &area_inner);

    area_outer.x1 -= dsc->outline_width;
    area_outer.x2 += dsc->outline_width;
    area_outer.y1 -= dsc->outline_width;
    area_outer.y2 += dsc->outline_width;


    int32_t inner_w = lv_area_get_width(&area_inner);
    int32_t inner_h = lv_area_get_height(&area_inner);
    int32_t rin = dsc->radius;
    int32_t short_side = LV_MIN(inner_w, inner_h);
    if (rin > short_side >> 1) rin = short_side >> 1;

    lv_coord_t rout = rin + dsc->outline_width;

    draw_border_generic(clip, &area_outer, &area_inner, rout, rin, dsc->outline_color, dsc->outline_opa,
                        dsc->blend_mode);
}

static void draw_border_generic(const lv_area_t *clip_area, const lv_area_t *outer_area, const lv_area_t *inner_area,
                                lv_coord_t rout, lv_coord_t rin, lv_color_t color, lv_opa_t opa,
                                lv_blend_mode_t blend_mode) {
    opa = opa >= LV_OPA_COVER ? LV_OPA_COVER : opa;

    if (rout == 0 || rin == 0) {
        draw_border_simple(clip_area, outer_area, inner_area, color, opa);
        return;
    }

    lv_disp_t *disp = _lv_refr_get_disp_refreshing();
    SDL_Renderer *renderer = (SDL_Renderer *) disp->driver->user_data;

    lv_coord_t border_width = lv_area_get_width(outer_area), border_height = lv_area_get_height(outer_area),
            border_min = LV_MIN(border_width, border_height);
    lv_coord_t min_half = border_min % 2 == 0 ? border_min / 2 : border_min / 2 + 1;
    lv_coord_t frag_size = rout == LV_RADIUS_CIRCLE ? min_half : LV_MIN(rout + 1, min_half);
    lv_draw_rect_border_key_t key = {
            .magic = LV_GPU_CACHE_KEY_MAGIC_RECT_BORDER,
            .rout = rout, .rin = rin,
            .thickness = inner_area->x1 - outer_area->x1 + 1,
            .size = min_half,
            .side = LV_BORDER_SIDE_FULL,
    };
    SDL_Texture *texture = lv_gpu_draw_cache_get(&key, sizeof(key), NULL);
    if (texture == NULL) {
        /*Create mask for the outer area*/
        int16_t mask_rout_id = LV_MASK_ID_INV;
        lv_draw_mask_radius_param_t mask_rout_param;
        if (rout > 0) {
            lv_draw_mask_radius_init(&mask_rout_param, outer_area, rout, false);
            mask_rout_id = lv_draw_mask_add(&mask_rout_param, NULL);
        }

        /*Create mask for the inner mask*/
        if (rin < 0) rin = 0;
        lv_draw_mask_radius_param_t mask_rin_param;
        lv_draw_mask_radius_init(&mask_rin_param, inner_area, rin, true);
        int16_t mask_rin_id = lv_draw_mask_add(&mask_rin_param, NULL);

        lv_area_t frag_area;
        lv_area_copy(&frag_area, outer_area);
        lv_area_set_width(&frag_area, frag_size);
        lv_area_set_height(&frag_area, frag_size);

        texture = lv_sdl_gen_mask_texture(renderer, &frag_area);

        lv_draw_mask_remove_id(mask_rin_id);
        lv_draw_mask_remove_id(mask_rout_id);
        SDL_assert(texture);
        lv_gpu_draw_cache_put(&key, sizeof(key), texture);
    }

    SDL_Rect clip_rect, outer_rect;
    lv_area_to_sdl_rect(clip_area, &clip_rect);
    lv_area_to_sdl_rect(outer_area, &outer_rect);
    SDL_Color color_sdl;
    lv_color_to_sdl_color(&color, &color_sdl);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, opa);
    SDL_SetTextureColorMod(texture, color_sdl.r, color_sdl.g, color_sdl.b);
    SDL_RenderSetClipRect(renderer, &clip_rect);

    render_corners(renderer, texture, frag_size, outer_area);
    render_borders(renderer, texture, frag_size, outer_area);
}

static void draw_border_simple(const lv_area_t *clip, const lv_area_t *outer_area, const lv_area_t *inner_area,
                               lv_color_t color, lv_opa_t opa) {

    lv_disp_t *disp = _lv_refr_get_disp_refreshing();
    SDL_Renderer *renderer = (SDL_Renderer *) disp->driver->user_data;

    SDL_Color color_sdl;
    lv_color_to_sdl_color(&color, &color_sdl);
    SDL_Rect clip_rect;
    lv_area_to_sdl_rect(clip, &clip_rect);

    SDL_SetRenderDrawColor(renderer, color_sdl.r, color_sdl.g, color_sdl.b, opa);
    SDL_RenderSetClipRect(renderer, &clip_rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect simple_rect;
    simple_rect.w = inner_area->x2 - outer_area->x1 + 1;
    simple_rect.h = inner_area->y1 - outer_area->y1 + 1;
    // Top border
    simple_rect.x = outer_area->x1;
    simple_rect.y = outer_area->y1;
    SDL_RenderFillRect(renderer, &simple_rect);
    // Bottom border
    simple_rect.x = inner_area->x1;
    simple_rect.y = inner_area->y2;
    SDL_RenderFillRect(renderer, &simple_rect);

    simple_rect.w = inner_area->x1 - outer_area->x1 + 1;
    simple_rect.h = inner_area->y2 - outer_area->y1 + 1;
    /* Left border */
    simple_rect.x = outer_area->x1;
    simple_rect.y = inner_area->y1;
    SDL_RenderFillRect(renderer, &simple_rect);
    // Right border
    simple_rect.x = inner_area->x2;
    simple_rect.y = outer_area->y1;
    SDL_RenderFillRect(renderer, &simple_rect);

}

// Slow draw function
void draw_bg_compat(SDL_Renderer *renderer, const lv_area_t *coords, const SDL_Rect *coords_rect,
                    const SDL_Rect *mask_rect, const lv_draw_rect_dsc_t *dsc) {
    SDL_Color bg_color;
    lv_color_to_sdl_color(&dsc->bg_color, &bg_color);

    SDL_Surface *indexed = lv_sdl_apply_mask_surface(coords);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, indexed);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, dsc->bg_opa);
    SDL_SetTextureColorMod(texture, bg_color.r, bg_color.g, bg_color.b);
    SDL_RenderSetClipRect(renderer, mask_rect);
    SDL_RenderCopy(renderer, texture, NULL, coords_rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(indexed);
}


static void render_corners(SDL_Renderer *renderer, SDL_Texture *frag, lv_coord_t frag_size, const lv_area_t *coords) {
    lv_coord_t bg_w = lv_area_get_width(coords), bg_h = lv_area_get_height(coords);
    SDL_Rect srcrect = {0, 0, frag_size, frag_size},
            dstrect = {.x=coords->x1, .y=coords->y1, .w = frag_size, .h=frag_size};
    /* Upper left */
    SDL_RenderCopyEx(renderer, frag, &srcrect, &dstrect, 0, NULL, SDL_FLIP_NONE);
    /* Upper right, clip right edge if too big */
    srcrect.w = dstrect.w = LV_MIN(frag_size, bg_w - frag_size);
    dstrect.x = coords->x2 - srcrect.w + 1;
    SDL_RenderCopyEx(renderer, frag, &srcrect, &dstrect, 0, NULL, SDL_FLIP_HORIZONTAL);
    /* Lower right, clip bottom edge if too big */
    srcrect.h = dstrect.h = LV_MIN(frag_size, bg_h - frag_size);
    dstrect.y = coords->y2 - srcrect.h + 1;
    SDL_RenderCopyEx(renderer, frag, &srcrect, &dstrect, 0, NULL, SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
    /* Lower left, right edge should not be clipped */
    srcrect.w = dstrect.w = frag_size;
    dstrect.x = coords->x1;
    SDL_RenderCopyEx(renderer, frag, &srcrect, &dstrect, 0, NULL, SDL_FLIP_VERTICAL);
}

static void render_borders(SDL_Renderer *renderer, SDL_Texture *frag, lv_coord_t frag_size, const lv_area_t *coords) {
    lv_coord_t bg_w = lv_area_get_width(coords), bg_h = lv_area_get_height(coords);
    SDL_Rect srcrect, dstrect;
    /* For top/bottom edges, stretch pixels on the right */
    srcrect.h = dstrect.h = frag_size;
    dstrect.w = bg_w - frag_size * 2;
    /* Has space to fill */
    if (dstrect.w > 0 && dstrect.h > 0) {
        srcrect.w = 1;
        srcrect.y = 0;
        srcrect.x = frag_size - 1;
        dstrect.x = coords->x1 + frag_size;
        /* Top edge */
        dstrect.y = coords->y1;
        SDL_RenderCopy(renderer, frag, &srcrect, &dstrect);
        /* Bottom edge */
        dstrect.y = coords->y2 - frag_size + 1;
        if (bg_h < frag_size * 2) {
            /* Bottom edge will overlap with top, so decrease it by 1 px */
            srcrect.h = dstrect.h = frag_size - 1;
            dstrect.y += 1;
        }
        if (srcrect.h > 0) {
            SDL_RenderCopyEx(renderer, frag, &srcrect, &dstrect, 0, NULL, SDL_FLIP_VERTICAL);
        }
    }
    /* For left/right edges, stretch pixels on the bottom */
    srcrect.w = dstrect.w = frag_size;
    dstrect.h = bg_h - frag_size * 2;
    if (dstrect.w > 0 && dstrect.h > 0) {
        srcrect.h = 1;
        srcrect.x = 0;
        srcrect.y = frag_size - 1;
        dstrect.y = coords->y1 + frag_size;
        /* Left edge */
        dstrect.x = coords->x1;
        SDL_RenderCopy(renderer, frag, &srcrect, &dstrect);
        /* Right edge */
        dstrect.x = coords->x2 - frag_size + 1;
        if (bg_w < frag_size * 2) {
            /* Right edge will overlap with left, so decrease it by 1 px */
            srcrect.w = dstrect.w = frag_size - 1;
            dstrect.x += 1;
        }
        if (srcrect.w > 0) {
            SDL_RenderCopyEx(renderer, frag, &srcrect, &dstrect, 0, NULL, SDL_FLIP_HORIZONTAL);
        }
    }
}

static void render_center(SDL_Renderer *renderer, SDL_Texture *frag, lv_coord_t frag_size, const lv_area_t *coords) {
    lv_coord_t bg_w = lv_area_get_width(coords), bg_h = lv_area_get_height(coords);
    SDL_Rect dstrect = {coords->x1 + frag_size, coords->y1 + frag_size, bg_w - frag_size * 2, bg_h - frag_size * 2};
    if (dstrect.w > 0 && dstrect.h > 0) {
        SDL_Rect srcrect = {frag_size - 1, frag_size - 1, 1, 1};
        SDL_RenderCopy(renderer, frag, &srcrect, &dstrect);
    }
}

#endif /*LV_USE_GPU_SDL*/