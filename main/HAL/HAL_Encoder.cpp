#include "HAL.h"
#include "ButtonEvent.h"

static volatile int32_t EncoderDiff = 0;
static bool EncoderPush = false;

static ButtonEvent EncoderButton;

static void Encoder_PushHandler(ButtonEvent* btn, int event)
{
    if(event == ButtonEvent::EVENT_CLICKED)
    {
        // EncoderDiffDisable = true;
        EncoderDiff += 1;
    }
    else if(event == ButtonEvent::EVENT_DOUBLE_CLICKED)
    {
        EncoderDiff -= 1;
    }
    else if(event == ButtonEvent::EVENT_RELEASED)
    {
        // EncoderDiffDisable = false;
        EncoderPush = false;
    }
    else if(event == ButtonEvent::EVENT_LONG_PRESSED)
    {
        EncoderPush = true;
        // HAL::Power_Shutdown();
        // HAL::Audio_PlayMusic("Shutdown");
    }
}

void HAL::Encoder_Init()
{
    pinMode(0, INPUT_PULLUP);
    EncoderButton.EventAttach(Encoder_PushHandler);
}

void HAL::Encoder_Update(void *e)
{
    EncoderButton.EventMonitor(digitalRead(0) == LOW);

}

int32_t HAL::Encoder_GetDiff()
{
    int32_t diff = EncoderDiff;
    EncoderDiff = 0;
    return diff;
}

bool HAL::Encoder_GetIsPush()
{
    // if(!EncoderEnable)
    // {
    //     return false;
    // }
    
    return EncoderPush;
}