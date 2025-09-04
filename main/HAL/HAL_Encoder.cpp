#include "HAL.h"
#include "ButtonEvent.h"

static bool EncoderEnable = true;
static volatile int32_t EncoderDiff = 0;
static bool EncoderDiffDisable = false;

static ButtonEvent EncoderButton;

static void Encoder_PushHandler(ButtonEvent *btn, int event)
{
    if (event == ButtonEvent::EVENT_PRESSED)
    {
        EncoderDiffDisable = true;
    }
    else if (event == ButtonEvent::EVENT_DOUBLE_CLICKED)
    {
    }
    else if (event == ButtonEvent::EVENT_RELEASED)
    {
        EncoderDiffDisable = false;
    }
    else if (event == ButtonEvent::EVENT_LONG_PRESSED)
    {
    }
}

static void EncoderA_ISRHandler(void)
{
    if (!EncoderEnable || EncoderDiffDisable)
    {
        return;
    }
    int dir = (digitalRead(CONFIG_ENCODER_B_PIN) == LOW ? -1 : +1);
    EncoderDiff += dir;
}

void HAL::Encoder_Init()
{
    pinMode(CONFIG_ENCODER_PUSH_PIN, INPUT_PULLUP);
    pinMode(CONFIG_ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(CONFIG_ENCODER_B_PIN, INPUT_PULLUP);
    EncoderButton.EventAttach(Encoder_PushHandler);
    attachInterrupt(CONFIG_ENCODER_A_PIN, EncoderA_ISRHandler, FALLING);
}

void HAL::Encoder_Update(void *e)
{
    EncoderButton.EventMonitor(digitalRead(CONFIG_ENCODER_PUSH_PIN) == LOW);
}

int32_t HAL::Encoder_GetDiff()
{
    if (!EncoderEnable || EncoderDiffDisable)
    {
        return 0;
    }
    int32_t diff = EncoderDiff;
    EncoderDiff = 0;
    return diff;
}

bool HAL::Encoder_GetIsPush()
{
    if (!EncoderEnable)
    {
        return false;
    }

    return (digitalRead(CONFIG_ENCODER_PUSH_PIN) == LOW);
}

void HAL::Encoder_SetEnable(bool en)
{
    EncoderEnable = en;
}