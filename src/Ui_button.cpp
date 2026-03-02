//************************************************************************************************
// #include <Arduino.h>
#include "config.h"
#include "Ui_button.h"

#define IO_BUTTON 0

// 콜백을 써서 UI 함수에서 불러 쓸 수 있도록 하자.
UiButton btn;

//************************************************************************************************
void UiButton::init()
{
  pinMode(IO_BUTTON, INPUT_PULLUP);
}

//************************************************************************************************
void UiButton::RegisterCallback(InputEvent InEvent)
{
  Event = InEvent;
}

//************************************************************************************************
void UiButton::regCallback(pressCallback step0, pressCallback step1, pressCallback step2, pressCallback step3)
{
  pressShort = step0;
  pressLongStep_1 = step1;
  pressLongStep_2 = step2;
  pressLongStep_3 = step3;
}

//************************************************************************************************
void UiButton::loop()
{
  this->millis_now = millis();

  if ((this->millis_now - this->millis_old) > 35)
  {
    switch (this->step)
    {
    //------------------------------------
    case 0:
    {
      if (digitalRead(IO_BUTTON) == LOW)
      {
        this->millis_pressed = this->millis_now;
        this->step++;
      }
      break;
    }
    //------------------------------------
    case 1:
    {
      if ((this->millis_now - this->millis_pressed) > 1000)
      {
        pressLongStep_1();
        this->step++;
      }
      else if (digitalRead(IO_BUTTON) == HIGH)
      {
        pressShort();
        this->step = 0;
      }
      break;
    }
    //------------------------------------
    case 2:
    {
      if ((this->millis_now - this->millis_pressed) > 3000)
      {
        pressLongStep_2();
        this->step++;
      }
      else if (digitalRead(IO_BUTTON) == HIGH)
      {
        this->step = 0;
      }
      break;
    }
    //------------------------------------
    case 3:
    {
      if ((this->millis_now - this->millis_pressed) > 5000)
      {
        pressLongStep_3();
        this->step++;
      }
      else if (digitalRead(IO_BUTTON) == HIGH)
      {
        this->step = 0;
      }
      break;
    }
    //------------------------------------
    case 4:
    {
      if (digitalRead(IO_BUTTON) == HIGH)
      {
        this->step = 0;
      }
      break;
    }
    default:
    {
      this->step = 0;
    }
    }

    // debounce time(ms)
    this->millis_old = this->millis_now;
  }
}

//************************************************************************************************
