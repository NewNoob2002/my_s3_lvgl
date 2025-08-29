#pragma once 
/**
 * \file
 * \brief LVGL port include file.
 */
#include "lvgl.h"
#include "lv_demos.h"

void lv_port_disp_init();
void lv_port_indev_init();
void lv_port_fs_init();

inline void lv_port_init(){
    lv_port_disp_init();
    lv_port_indev_init();
    lv_port_fs_init();
}