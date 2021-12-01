/**
 * @file lv_draw_nvg.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_draw_nvg.h"
#include "lv_draw_nvg_priv.h"
#include "../../core/lv_refr.h"
#include "lv_draw_nvg_img_cache.h"

/*********************
 *      DEFINES
 *********************/

#define FRAME_DEPTH_MAX 8

void lv_draw_nvg_rect(const lv_area_t *coords, const lv_area_t *clip, const lv_draw_rect_dsc_t *dsc);

void lv_draw_nvg_label(const lv_point_t *pos_p, const lv_area_t *clip_area, const lv_font_t *font_p, uint32_t letter,
                       lv_color_t color, lv_opa_t opa, lv_blend_mode_t blend_mode);

lv_res_t lv_draw_nvg_img_core(const lv_area_t *coords, const lv_area_t *mask, const void *src,
                              const lv_draw_img_dsc_t *draw_dsc);

void lv_draw_nvg_line(const lv_point_t *point1, const lv_point_t *point2, const lv_area_t *clip,
                      const lv_draw_line_dsc_t *dsc);

void lv_draw_nvg_arc(lv_coord_t center_x, lv_coord_t center_y, uint16_t radius, uint16_t start_angle,
                     uint16_t end_angle, const lv_area_t *clip_area, const lv_draw_arc_dsc_t *dsc);

void lv_draw_nvg_polygon(const lv_point_t points[], uint16_t point_cnt, const lv_area_t *clip,
                         const lv_draw_rect_dsc_t *dsc);

/**********************
 *      TYPEDEFS
 **********************/
typedef struct lv_draw_nvg_context_states_t {
    int frame_depth;
    lv_draw_nvg_buffer_index frame_indices[FRAME_DEPTH_MAX];
    lv_area_t frame_areas[FRAME_DEPTH_MAX];
} lv_draw_nvg_context_states_t;

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

void lv_draw_nvg_context_init(lv_draw_nvg_context_t *context, NVGcontext *nvg,
                              const lv_draw_nvg_callbacks_t *callbacks) {
    lv_memset_00(context, sizeof(lv_draw_nvg_context_t));
    context->nvg = nvg;
    lv_memcpy(&context->callbacks, callbacks, sizeof(lv_draw_nvg_callbacks_t));
    lv_draw_nvg_context_states_t *states = lv_mem_alloc(sizeof(lv_draw_nvg_context_states_t));
    lv_memset_00(states, sizeof(lv_draw_nvg_context_states_t));
    context->states = states;
}

void lv_draw_nvg_init(lv_draw_backend_t *backend) {
    lv_draw_backend_init(backend);
    backend->draw_rect = lv_draw_nvg_rect;
    backend->draw_letter = lv_draw_nvg_label;
    backend->draw_img_core = lv_draw_nvg_img_core;
    backend->draw_line = lv_draw_nvg_line;
    backend->draw_arc = lv_draw_nvg_arc;
    backend->draw_polygon = lv_draw_nvg_polygon;
}

lv_draw_nvg_context_t *lv_draw_nvg_current_context() {
    lv_disp_t *disp = _lv_refr_get_disp_refreshing();
    return disp->driver->user_data;
}

void lv_draw_nvg_begin_frame(lv_draw_nvg_context_t *context, lv_draw_nvg_buffer_index index, bool clear) {
    lv_draw_nvg_context_states_t *states = context->states;
    LV_ASSERT(states->frame_depth < FRAME_DEPTH_MAX);
    if (states->frame_depth > 0) {
        if (states->frame_indices[states->frame_depth - 1] == index)
            return;
        nvgEndFrame(context->nvg);
    }
    lv_disp_t *disp = _lv_refr_get_disp_refreshing();
    lv_area_t area = {0, 0, disp->driver->hor_res - 1, disp->driver->ver_res - 1};
    context->callbacks.set_render_buffer(context, index, clear);
    nvgBeginFrame(context->nvg, lv_area_get_width(&area), lv_area_get_height(&area), 1);
    lv_memcpy(&states->frame_areas[states->frame_depth], &area, sizeof(lv_area_t));
    states->frame_indices[states->frame_depth] = index;
    states->frame_depth++;
}

void lv_draw_nvg_end_frame(lv_draw_nvg_context_t *context) {
    lv_draw_nvg_context_states_t *states = context->states;
    if (states->frame_depth == 0) return;
    nvgEndFrame(context->nvg);
    states->frame_depth--;
    if (states->frame_depth > 0) {
        const lv_area_t *area = &states->frame_areas[states->frame_depth - 1];
        context->callbacks.set_render_buffer(context, states->frame_indices[states->frame_depth - 1], false);
        nvgBeginFrame(context->nvg, lv_area_get_width(area), lv_area_get_height(area), 1);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

