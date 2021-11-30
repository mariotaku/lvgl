CSRCS += lv_draw_nvg_rect.c
CSRCS += lv_draw_nvg.c

DEPPATH += --dep-path $(LVGL_DIR)/$(LVGL_DIR_NAME)/src/draw/nvg
VPATH += :$(LVGL_DIR)/$(LVGL_DIR_NAME)/src/draw/nvg

CFLAGS += "-I$(LVGL_DIR)/$(LVGL_DIR_NAME)/src/draw/nvg"
