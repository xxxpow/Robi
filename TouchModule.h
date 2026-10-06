#pragma once
#include <Arduino.h>
#include "Config.h"

class TouchModule {
public:
  TouchModule();

  void init();
  bool isPressed();
  bool isDown() const;

private:
  int _estadoAnterior;
};
