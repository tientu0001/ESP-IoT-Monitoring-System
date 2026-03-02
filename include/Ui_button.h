#ifndef UI_BTN_H
#define UI_BTN_H

#include <stdint.h>

//************************************************************************************************
class UiButton
{
  typedef void (*InputEvent)(const char *);
  typedef void (*pressCallback)(void);

public:
  void init();
  void loop();
  void RegisterCallback(InputEvent InEvent);
  void regCallback(pressCallback step0, pressCallback step1, pressCallback step2, pressCallback step3);

private:
  InputEvent Event;
  pressCallback pressShort, pressLongStep_1, pressLongStep_2, pressLongStep_3;
  char buffer[10];

  uint8_t step;
  uint32_t millis_now, millis_old, millis_pressed;
};

//************************************************************************************************
extern UiButton btn;

//************************************************************************************************
#endif // UI_BTN_H