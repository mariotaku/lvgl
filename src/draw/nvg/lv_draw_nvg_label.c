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
    if((g.box_h == 0) || (g.box_w == 0)) return;

    int32_t pos_x = pos_p->x + g.ofs_x;
    int32_t pos_y = pos_p->y + (font_p->line_height - font_p->base_line) - g.box_h - g.ofs_y;

    /*If the letter is completely out of mask don't draw it*/
    if(pos_x + g.box_w < clip_area->x1 ||
       pos_x > clip_area->x2 ||
       pos_y + g.box_h < clip_area->y1 ||
       pos_y > clip_area->y2) {
        return;
    }

    nvgBeginPath(ctx->nvg);
    nvgRect(ctx->nvg, pos_p->x, pos_p->y, g.box_w, g.box_h);
    nvgFillColor(ctx->nvg, nvgRGBA(color.ch.red, color.ch.green, color.ch.blue, opa));
    nvgFill(ctx->nvg);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
