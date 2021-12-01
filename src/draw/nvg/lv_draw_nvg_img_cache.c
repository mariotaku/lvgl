/**
 * @file lv_draw_sdl_texture_cache.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "../../lv_conf_internal.h"


#include "lv_draw_nvg_img_cache.h"

#include "../../misc/lv_log.h"
#include "../../draw/lv_draw_label.h"
#include "../../draw/lv_draw_img.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    int img;
    void * userdata;
    lv_lru_free_t * userdata_free;
    lv_draw_sdl_cache_flag_t flags;
} draw_cache_value_t;

typedef struct {
    lv_gpu_cache_key_magic_t magic;
} temp_texture_key_t;

typedef struct {
    lv_coord_t width, height;
} temp_texture_userdata_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void draw_cache_free_value(draw_cache_value_t *);

/**********************
 *  STATIC VARIABLES
 **********************/

static lv_lru_t * lv_nvg_img_cache;
NVGcontext *lv_nvg_context;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_nvg_draw_cache_init(NVGcontext *nvg)
{
    lv_nvg_img_cache = lv_lru_new(1024 * 1024 * 8, 65536, (lv_lru_free_t *) draw_cache_free_value,
                                  NULL);
    lv_nvg_context = nvg;
}

void lv_nvg_draw_cache_deinit()
{
    lv_lru_free(lv_nvg_img_cache);
    lv_nvg_context = NULL;
}

int lv_nvg_draw_cache_get(const void * key, size_t key_length, bool * found)
{
    return lv_nvg_draw_cache_get_with_userdata(key, key_length, found, NULL);
}

int lv_nvg_draw_cache_get_with_userdata(const void * key, size_t key_length, bool * found, void ** userdata)
{
    draw_cache_value_t * value = NULL;
    lv_lru_get(lv_nvg_img_cache, key, key_length, (void **) &value);
    if(!value) {
        if(found) {
            *found = false;
        }
        return 0;
    }
    else {
        if(userdata) {
            *userdata = value->userdata;
        }
    }
    if(found) {
        *found = true;
    }
    return value->img;
}

void lv_nvg_draw_cache_put(const void * key, size_t key_length, int img)
{
    lv_nvg_draw_cache_put_advanced(key, key_length, img, NULL, NULL, 0);
}

void lv_nvg_draw_cache_put_advanced(const void * key, size_t key_length, int img, void * userdata,
                                         lv_lru_free_t userdata_free, lv_draw_sdl_cache_flag_t flags)
{
    draw_cache_value_t * value = lv_mem_alloc(sizeof(draw_cache_value_t));
    value->img = img;
    value->userdata = userdata;
    value->userdata_free = userdata_free;
    value->flags = flags;
    if(!img) {
        lv_lru_set(lv_nvg_img_cache, key, key_length, value, 1);
        return;
    }
    if(flags & LV_DRAW_SDL_CACHE_FLAG_MANAGED) {
        /* Managed texture doesn't count into cache size */
        LV_LOG_INFO("cache texture %p, %d*%d@%dbpp", texture, width, height, SDL_BITSPERPIXEL(format));
        lv_lru_set(lv_nvg_img_cache, key, key_length, value, 1);
        return;
    }
    int width = 0, height = 0;
    nvgImageSize(lv_nvg_context, img, &width, &height);
    if(width * height <= 0) {
        return;
    }
    LV_LOG_INFO("cache image %d, %d*%d@ARGB", img, width, height);
    lv_lru_set(lv_nvg_img_cache, key, key_length, value, width * height * 4);
}

lv_draw_sdl_cache_key_head_img_t * lv_nvg_img_cache_key_create(const void * src, int32_t frame_id, size_t * size)
{
    lv_draw_sdl_cache_key_head_img_t header;
    /* VERY IMPORTANT! Padding between members is uninitialized, so we have to wipe them manually */
    lv_memset(&header, 0, sizeof(header));
    header.magic = LV_GPU_CACHE_KEY_MAGIC_IMG;
    header.type = lv_img_src_get_type(src);
    header.frame_id = frame_id;
    void * key;
    size_t key_size;
    if(header.type == LV_IMG_SRC_FILE || header.type == LV_IMG_SRC_SYMBOL) {
        size_t srclen = strlen(src);
        key_size = sizeof(header) + srclen;
        key = lv_mem_alloc(key_size);
        lv_memcpy(key, &header, sizeof(header));
        /*Copy string content as key value*/
        lv_memcpy(key + sizeof(header), src, srclen);
    }
    else {
        key_size = sizeof(header) + sizeof(void *);
        key = lv_mem_alloc(key_size);
        lv_memcpy(key, &header, sizeof(header));
        /*Copy address number as key value*/
        lv_memcpy(key + sizeof(header), &src, sizeof(void *));
    }
    *size = key_size;
    return (lv_draw_sdl_cache_key_head_img_t *) key;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void draw_cache_free_value(draw_cache_value_t * value)
{
    if(value->img && !(value->flags & LV_DRAW_SDL_CACHE_FLAG_MANAGED)) {
        LV_LOG_INFO("destroy texture %p", value->texture);
        nvgDeleteImage(lv_nvg_context, value->img);
    }
    if(value->userdata_free) {
        value->userdata_free(value->userdata);
    }
    lv_mem_free(value);
}



