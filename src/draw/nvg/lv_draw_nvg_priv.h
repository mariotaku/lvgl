/**
 * @file lv_draw_nvg_priv.h
 *
 */

#ifndef LV_DRAW_NVG_PRIV_H
#define LV_DRAW_NVG_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#include "lv_draw_nvg.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_draw_nvg_context_t *lv_draw_nvg_current_context();

void lv_draw_nvg_begin_frame(lv_draw_nvg_context_t *context, lv_draw_nvg_buffer_index index, bool clear);

void lv_draw_nvg_end_frame(lv_draw_nvg_context_t *context);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_NVG_PRIV_H*/
