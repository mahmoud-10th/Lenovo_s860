include $(MTK_ROOT_BUILD)/Makefile
LEDS_PLATFORM_DRIVER_DIR :=  $(srctree)/mediatek/platform/$(call lc,$(MTK_PLATFORM))/kernel/drivers/leds
obj-$(CONFIG_MTK_LEDS) += leds_drv.o
ifeq ($(LENOVO_SN3193_SUPPORT),yes)
obj-y += sn3193_leds.o
endif
ifeq ($(LENOVO_SN3199_SUPPORT),yes)
obj-y += sn3199_leds.o
endif
ccflags-y  += -I$(LEDS_PLATFORM_DRIVER_DIR)

