#ifndef __STARTUP_PRESENTER_H
#define __STARTUP_PRESENTER_H

#include "StartUpView.h"
#include "StartUpModel.h"

#if defined(DEBUG)
#define PAGE_STARTUP_PRINTF(fmt, ...) printf("Startup::" fmt "\n", ##__VA_ARGS__)
#else
#define PAGE_STARTUP_PRINTF(fmt, ...)
#endif

namespace Page
{

class Startup : public PageBase
{
public:

public:
    Startup();
    virtual ~Startup();

    virtual void onCustomAttrConfig();
    virtual void onViewLoad();
    virtual void onViewDidLoad();
    virtual void onViewWillAppear();
    virtual void onViewDidAppear();
    virtual void onViewWillDisappear();
    virtual void onViewDidDisappear();
    virtual void onViewUnload();
    virtual void onViewDidUnload();

private:
    static void onTimer(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);

private:
    StartupView View;
    StartupModel Model;

    lv_timer_t* StartupTimer;
};

}

#endif
