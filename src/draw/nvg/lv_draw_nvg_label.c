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
#include "../../draw/sw/lv_draw_sw.h"
#include "lv_draw_nvg_priv.h"
#include "lv_draw_nvg_img_cache.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_gpu_cache_key_magic_t magic;
    const lv_font_t *font_p;
    uint32_t letter;
} lv_font_glyph_key_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static lv_font_glyph_key_t font_key_glyph_create(const lv_font_t *font_p, uint32_t letter);
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_draw_nvg_label(const lv_point_t *pos_p, const lv_area_t *clip_area, const lv_font_t *font_p, uint32_t letter,
                       lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
//    lv_draw_nvg_ensure_frame(ctx);

    lv_font_glyph_dsc_t g;
    bool g_ret = lv_font_get_glyph_dsc(font_p, &g, letter, '\0');
    if (g_ret == false) {
        /*Add warning if the dsc is not found
         *but do not print warning for non printable ASCII chars (e.g. '\n')*/
        if (letter >= 0x20 &&
            letter != 0xf8ff && /*LV_SYMBOL_DUMMY*/
            letter != 0x200c) { /*ZERO WIDTH NON-JOINER*/
            LV_LOG_WARN("lv_draw_letter: glyph dsc. not found for U+%X", letter);
        }
        return;
    }

    /*Don't draw anything if the character is empty. E.g. space*/
    if ((g.box_h == 0) || (g.box_w == 0)) return;

    int32_t pos_x = pos_p->x + g.ofs_x;
    int32_t pos_y = pos_p->y + (font_p->line_height - font_p->base_line) - g.box_h - g.ofs_y;

    /*If the letter is completely out of mask don't draw it*/
    if (pos_x + g.box_w < clip_area->x1 ||
        pos_x > clip_area->x2 ||
        pos_y + g.box_h < clip_area->y1 ||
        pos_y > clip_area->y2) {
        return;
    }

    lv_font_glyph_key_t glyph_key = font_key_glyph_create(font_p, letter);
    bool glyph_found = false;
    int texture = lv_nvg_draw_cache_get(&glyph_key, sizeof(glyph_key), &glyph_found);
    if (!glyph_found) {
        if (g.resolved_font) {
            font_p = g.resolved_font;
        }
        const uint8_t *bmp = lv_font_get_glyph_bitmap(font_p, letter);
        uint8_t *buf = lv_mem_alloc(g.box_w * g.box_h);
        lv_draw_sw_nbpp_to_opa(buf, bmp, g.box_w, g.box_h, g.box_w, g.bpp);
        texture = lv_nvg_create_alpha_image(buf, g.box_w, g.box_h);
        lv_mem_free(buf);
        lv_nvg_draw_cache_put(&glyph_key, sizeof(glyph_key), texture);
    }
    if (!texture) {
        return;
    }

    nvgSave(ctx->nvg);

    nvgScissor(ctx->nvg, clip_area->x1, clip_area->y1, lv_area_get_width(clip_area), lv_area_get_height(clip_area));

    nvgBeginPath(ctx->nvg);
    nvgRect(ctx->nvg, pos_x, pos_y, g.box_w, g.box_h);
    NVGpaint paint = nvgImagePattern(ctx->nvg, pos_x, pos_y, g.box_w, g.box_h, 0, texture, 1);
    paint.innerColor = paint.outerColor = nvgRGBA(color.ch.red, color.ch.green, color.ch.blue, opa);
    nvgFillPaint(ctx->nvg, paint);
    nvgFill(ctx->nvg);

    nvgRestore(ctx->nvg);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_font_glyph_key_t font_key_glyph_create(const lv_font_t *font_p, uint32_t letter) {
    lv_font_glyph_key_t key;
    /* VERY IMPORTANT! Padding between members is uninitialized, so we have to wipe them manually */
    lv_memset_00(&key, sizeof(key));
    key.magic = LV_GPU_CACHE_KEY_MAGIC_GLYPH;
    key.font_p = font_p;
    key.letter = letter;
    return key;
}
