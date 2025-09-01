#include "StartUp.h"

using namespace Page;

Startup::Startup():StartupTimer(nullptr)
{
}

Startup::~Startup()
{

}

void Startup::onCustomAttrConfig()
{
    PAGE_STARTUP_PRINTF("onCustomAttrConfig");
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Startup::onViewLoad()
{
    PAGE_STARTUP_PRINTF("onViewLoad");
    Model.Init();
    Model.SetEncoderEnable(false);
    View.Create(_root);
    StartupTimer = lv_timer_create(onTimer, 10, this);
    lv_timer_set_repeat_count(StartupTimer, -1);
}

void Startup::onViewDidLoad()
{
    PAGE_STARTUP_PRINTF("onViewDidLoad");
}

void Startup::onViewWillAppear()
{
    PAGE_STARTUP_PRINTF("onViewWillAppear");
    Model.PlayMusic("Startup");
    lv_anim_timeline_start(View.ui.anim_timeline);
}

void Startup::onViewDidAppear()
{
    PAGE_STARTUP_PRINTF("onViewDidAppear");
    lv_obj_fade_out(_root, 500, 1500);
}

void Startup::onViewWillDisappear()
{
    PAGE_STARTUP_PRINTF("onViewWillDisappear");
    lv_timer_pause(StartupTimer);
    lv_timer_del(StartupTimer);
    StartupTimer = nullptr;
}

void Startup::onViewDidDisappear()
{
    PAGE_STARTUP_PRINTF("onViewDidDisappear");
    Model.SetStatusBarAppear(true);
}

void Startup::onViewUnload()
{
    PAGE_STARTUP_PRINTF("onViewUnload");
    View.Delete();
    Model.SetEncoderEnable(true);
    Model.Deinit();
}

void Startup::onViewDidUnload()
{
    PAGE_STARTUP_PRINTF("onViewDidUnload");
}

void Startup::onTimer(lv_timer_t* timer)
{
    Startup* instance = (Startup*)timer->user_data;
    static uint32_t last = 0;
    if(lv_tick_get() - last >= 1000)
    {
        last = lv_tick_get();
        PAGE_STARTUP_PRINTF("onTimer, now: %" PRIu32, lv_tick_get());
    }
    // instance->_Manager->Replace("Pages/Dialplate");
}

void Startup::onEvent(lv_event_t* event)
{

}
