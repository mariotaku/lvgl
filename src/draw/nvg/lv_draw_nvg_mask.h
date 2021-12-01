/**
 * @file lv_draw_nvg_mask.h
 *
 */

#ifndef LV_DRAW_NVG_MASK_H
#define LV_DRAW_NVG_MASK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../../lv_conf_internal.h"
#include "../../misc/lv_area.h"

#include "lv_draw_nvg.h"

#include <stdbool.h>

#include <nanovg.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
bool lv_draw_nvg_mask_begin(lv_draw_nvg_context_t *ctx, const lv_area_t *a);

void lv_draw_nvg_mask_end(lv_draw_nvg_context_t *ctx, const lv_area_t *a);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_NVG_MASK_H*/
