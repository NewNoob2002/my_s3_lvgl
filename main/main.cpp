#include <HAL.h>
#include <lvgl_port/lv_port.h>

lv_anim_t anim;
lv_obj_t *arc;
static uint8_t PowerKey_PressCount = 0;
static uint8_t last_press_count = 0;

// 回调函数类型定义
typedef void (*arc_completion_callback_t)(void);

// 全局回调函数指针
static arc_completion_callback_t completion_callback = NULL;
static bool callback_triggered = false;

static void arc_angle_anim(void * obj, int32_t v)
{
    lv_arc_set_value((lv_obj_t *)obj, v);
    
    // 根据进度值更新颜色
    if (v >= 100) {
        lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);
        // 触发完成回调函数
        if (completion_callback != NULL && !callback_triggered) {
            callback_triggered = true;
            completion_callback();
        }
    } else {
        lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
        // 重置回调触发标志，允许下次触发
        callback_triggered = false;
    }
}

// 启动动画到目标值
void start_animation_to_value(int32_t target_value)
{
    // 获取当前进度
    int32_t current_value = lv_arc_get_value(arc);
    
    // 如果目标值与当前值相同，不需要动画
    if (current_value == target_value) {
        return;
    } 
    // 设置动画参数
    lv_anim_set_values(&anim, current_value, target_value);
    
    // 简化动画时间计算，确保动画流畅
    uint32_t time = 200; // 固定200ms，确保动画流畅
    
    lv_anim_set_time(&anim, time);
    lv_anim_start(&anim);
}
/**
 * Create an arc which acts as a loader.
 */
void lv_example_arc_2(void)
{
    int arc_size = 172 * 0.7; // 使用屏幕高度的70%作为直径
    arc = lv_arc_create(lv_scr_act());
    lv_obj_set_size(arc, arc_size, arc_size);
    lv_obj_align(arc, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, 0);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    
    // 设置进度条样式 - 适配小屏幕
    lv_obj_set_style_arc_width(arc, 6, LV_PART_MAIN);      // 主弧线宽度
    lv_obj_set_style_arc_width(arc, 6, LV_PART_INDICATOR); // 指示器弧线宽度
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x505050), LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
    
    lv_obj_t *label_text = lv_label_create(lv_scr_act());
    lv_label_set_text(label_text, "Hold on");
    lv_obj_align_to(label_text, arc, LV_ALIGN_OUT_RIGHT_MID, 10, 0); // 修复：使用arc而不是label
    lv_obj_set_style_text_font(label_text, &lv_font_montserrat_14, 0);

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, arc);
    lv_anim_set_exec_cb(&anim, arc_angle_anim);
    lv_anim_set_time(&anim, 300); // 默认300ms
    lv_anim_set_playback_time(&anim, 0);
    lv_anim_set_repeat_count(&anim, 0);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 修复：使用线性路径确保能到达目标值
}
// 更新进度条函数
bool update_arc_progress(uint8_t magn)
{  
    uint8_t target_percent = (PowerKey_PressCount * magn);
    target_percent = target_percent > 100 ? 100 : target_percent; // 限制最大为100%
    
    // 只有当目标值发生变化时才启动动画
    static uint8_t last_target_percent = 0;
    if (target_percent != last_target_percent) {
        last_target_percent = target_percent;
        start_animation_to_value(target_percent);
    }

    return false;
}

// 设置进度条完成回调函数
void set_arc_completion_callback(arc_completion_callback_t callback)
{
    completion_callback = callback;
    callback_triggered = false; // 重置触发标志
}

// 清除进度条完成回调函数
void clear_arc_completion_callback(void)
{
    completion_callback = NULL;
    callback_triggered = false;
}

// 重置进度条状态（用于重新开始）
void reset_arc_progress(void)
{
    PowerKey_PressCount = 0;
    last_press_count = 0;
    callback_triggered = false;
    lv_arc_set_value(arc, 0);
    lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);
}

void my_completion_callback(void)
{
    reset_arc_progress();
}

extern "C" void app_main(void)
{
  HAL::Init();
  lv_init();
  lv_port_init();
  lv_example_arc_2();
  set_arc_completion_callback(my_completion_callback);
  xTaskCreate(HAL::Display_Update, "lvgl", 10*1024, nullptr, 12, nullptr);
  
  while (1)
  {
    update_arc_progress(2);
    PowerKey_PressCount++;
    
    delay(100);
  }
}