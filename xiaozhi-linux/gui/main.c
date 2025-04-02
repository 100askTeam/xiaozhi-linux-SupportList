#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>


#include "lv_100ask_xz_ai/lv_100ask_xz_ai.h"
#include "lv_100ask_xz_ai/src/ui_system.h"


lv_coord_t lcd_w;
lv_coord_t lcd_h;
uint32_t lcd_dpi;

static const wchar_t * title = L"百问网LVGL课程案例  © Copyright 2025, Shenzhen Baiwenwang Technology Co., Ltd.   https://www.100ask.net | https://lvgl.100ask.net";
static pthread_mutex_t lvgl_mutex;
static const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ? : dflt;
}

static void dump_maps(void) {
    int fd = open("/proc/self/maps", O_RDONLY);
    char buffer[4096];
    ssize_t size;

    read:
    size = read(fd, buffer, sizeof(buffer));
    if (size > 0) {
        write(STDOUT_FILENO, buffer, size);
        goto read;
    }
    close(fd);
}

static void test_mem_speed(void* buffer, size_t size) {
    struct timeval tv, tv2;
    uint64_t dut;
    void* tmp = malloc(size);
    gettimeofday(&tv, NULL);
    for (int i = 0; i < 256; i++) {
        memset(buffer, i, size);
    }
    gettimeofday(&tv2, NULL);
    dut = 1000000 * (tv2.tv_sec - tv.tv_sec) + tv2.tv_usec - tv.tv_usec;
    printf("memset %p: %.3f MB/s, elapsed %lu us\n", buffer, 1000000. * size * 256 / dut / 1024 / 1024, dut);

    gettimeofday(&tv, NULL);
    for (int i = 0; i < 256; i++) {
        memcpy(tmp, buffer, size);
    }
    gettimeofday(&tv2, NULL);
    dut = 1000000 * (tv2.tv_sec - tv.tv_sec) + tv2.tv_usec - tv.tv_usec;
    printf("memcpy %p: %.3f MB/s, elapsed %lu us\n", buffer, 1000000. * size * 256 / dut / 1024 / 1024, dut);
    free(tmp);
}

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t * disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t * disp = lv_linux_drm_create();

    lv_linux_drm_set_file(disp, device, -1);
}

#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    const int width = atoi(getenv("LV_SDL_VIDEO_WIDTH") ? : "800");
    const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ? : "480");

    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif

void lvgl_lock(void)
{
    pthread_mutex_lock(&lvgl_mutex);
}

void lvgl_unlock(void)
{
    pthread_mutex_unlock(&lvgl_mutex);
}

int main(void)
{
    pthread_mutex_init(&lvgl_mutex, NULL);
    lv_init();

    /*Linux display device init*/
    lv_linux_disp_init();

    /* 初始化UI交互系统 */
    ui_system_init();

    /*Create a Demo*/
    //lv_demo_widgets();
   // lv_demo_widgets_start_slideshow();
   // lv_indev_t * indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event1");    

    lv_100ask_xz_ai_main();

    /*Handle LVGL tasks*/
    while(1) {
        lvgl_lock();
        lv_timer_handler();
        lvgl_unlock();
        usleep(5000);
    }

    return 0;
}
