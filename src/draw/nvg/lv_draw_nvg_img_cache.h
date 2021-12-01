/**
 * @file lv_draw_sdl_texture_cache.h
 *
 */

#ifndef LV_DRAW_NVG_IMG_CACHE_H
#define LV_DRAW_NVG_IMG_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include "../../lv_conf_internal.h"

#include "../../draw/lv_img_decoder.h"
#include "../../misc/lv_area.h"
#include "../../misc/lv_lru.h"

#include <nanovg.h>

/*********************
 *      DEFINES
 *********************/

#define LV_DRAW_SDL_DEC_DSC_TEXTURE_HEAD "@LVNVGImg"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    char head[8];
    int img;
} lv_draw_nvg_dec_dsc_userdata_t;

typedef enum {
    LV_GPU_CACHE_KEY_MAGIC_IMG = 0x01,
    LV_GPU_CACHE_KEY_TEMP = 0xFF,
} lv_gpu_cache_key_magic_t;

typedef enum {
    LV_DRAW_SDL_CACHE_FLAG_NONE = 0,
    LV_DRAW_SDL_CACHE_FLAG_MANAGED = 1,
} lv_draw_sdl_cache_flag_t;

typedef struct {
    lv_gpu_cache_key_magic_t magic;
    lv_img_src_t type;
    int32_t frame_id;
} lv_draw_sdl_cache_key_head_img_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_nvg_draw_cache_init(NVGcontext *nvg);

void lv_nvg_draw_cache_deinit();

int lv_nvg_draw_cache_get(const void *key, size_t key_length, bool *found);

int lv_nvg_draw_cache_get_with_userdata(const void *key, size_t key_length, bool *found, void **userdata);

void lv_nvg_draw_cache_put(const void *key, size_t key_length, int img);

void lv_nvg_draw_cache_put_advanced(const void *key, size_t key_length, int img, void *userdata,
                                    lv_lru_free_t userdata_free, lv_draw_sdl_cache_flag_t flags);

lv_draw_sdl_cache_key_head_img_t *lv_nvg_img_cache_key_create(const void *src, int32_t frame_id, size_t *size);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DRAW_NVG_IMG_CACHE_H*/
