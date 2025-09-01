#ifndef __POWER_CHECK_VIEW_H
#define __POWER_CHECK_VIEW_H

#include "../Page.h"

namespace Page
{

class PowerCheckView
{
public:
    void Create(lv_obj_t* root);
    void Delete();

public:
    struct
    {
        lv_obj_t* cont;
        lv_obj_t* label_battery;
        lv_obj_t* label_useage_battery;
        lv_obj_t* label_text_battery;
        lv_obj_t* label_charging_battery;

        lv_anim_timeline_t* anim_timeline;
    } ui;

private:
};

}

#endif // !__VIEW_H
