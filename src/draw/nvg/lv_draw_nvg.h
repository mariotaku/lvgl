/**
 * @file lv_draw_nvg.h
 *
 */

#ifndef LV_TEMPL_H
#define LV_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_draw.h"
#include <nanovg.h>
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

struct lv_draw_nvg_context_states_t;
struct lv_draw_nvg_context_userdata_t;
struct lv_draw_nvg_context_t;

typedef enum lv_draw_nvg_buffer_index {
    LV_DRAW_NVG_BUFFER_SCREEN = 0,
    LV_DRAW_NVG_BUFFER_FRAME,
    LV_DRAW_NVG_BUFFER_TEMP,
    LV_DRAW_NVG_BUFFER_COUNT,
} lv_draw_nvg_buffer_index;

typedef struct lv_draw_nvg_callbacks_t {
    void (*set_render_buffer)(struct lv_draw_nvg_context_t *context, lv_draw_nvg_buffer_index dst);

    void (*submit_buffer)(struct lv_draw_nvg_context_t *context, lv_draw_nvg_buffer_index src, const lv_area_t *a,
                          bool clear);

    void (*swap_window)(struct lv_draw_nvg_context_t *context);
} lv_draw_nvg_callbacks_t;

typedef struct lv_draw_nvg_context_t {
    NVGcontext *nvg;
    lv_draw_nvg_callbacks_t callbacks;
    struct lv_draw_nvg_context_userdata_t *userdata;
    struct lv_draw_nvg_context_states_t *states;
} lv_draw_nvg_context_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_nvg_context_init(lv_draw_nvg_context_t *context, NVGcontext *nvg,
                              const lv_draw_nvg_callbacks_t *callbacks);

void lv_draw_nvg_init(lv_draw_backend_t *backend);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEMPL_H*/
