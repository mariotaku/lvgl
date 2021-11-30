/**
 * @file lv_templ.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../misc/lv_area.h"
#include "../../core/lv_disp.h"
#include "lv_draw_nvg.h"
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

lv_res_t lv_draw_nvg_img_core(const lv_area_t *coords, const lv_area_t *mask, const void *src,
                              const lv_draw_img_dsc_t *draw_dsc) {
    lv_draw_nvg_context_t *ctx = lv_draw_nvg_current_context();
    nvgSave(ctx->nvg);

    nvgReset(ctx->nvg);
    nvgScissor(ctx->nvg, mask->x1, mask->y1, lv_area_get_width(mask), lv_area_get_height(mask));

    nvgBeginPath(ctx->nvg);
    nvgRect(ctx->nvg, coords->x1, coords->y1, lv_area_get_width(coords), lv_area_get_height(coords));
    nvgFillColor(ctx->nvg, nvgRGB(255, 0, 0));
    nvgFill(ctx->nvg);

    nvgRestore(ctx->nvg);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
