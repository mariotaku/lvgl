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

struct lv_draw_nvg_context_userdata_t;
struct lv_draw_nvg_context_t;

typedef struct lv_draw_nvg_callbacks_t {
    void (*set_offscreen)(struct lv_draw_nvg_context_t *context, bool offscreen);

    void (*submit_buffer)(struct lv_draw_nvg_context_t *context);

    void (*swap_window)(struct lv_draw_nvg_context_t *context);
} lv_draw_nvg_callbacks_t;

typedef struct lv_draw_nvg_context_t {
    NVGcontext *nvg;
    lv_draw_nvg_callbacks_t callbacks;
    bool frame_begun;
    struct lv_draw_nvg_context_userdata_t *userdata;
} lv_draw_nvg_context_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_draw_nvg_init(lv_draw_backend_t *backend);

lv_draw_nvg_context_t *lv_draw_nvg_current_context();

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_TEMPL_H*/
