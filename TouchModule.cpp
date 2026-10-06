#include "TouchModule.h"

TouchModule::TouchModule()
  : _estadoAnterior(LOW) {}

void TouchModule::init() {
  pinMode(TTP223_PIN, INPUT);
  _estadoAnterior = digitalRead(TTP223_PIN);
}

bool TouchModule::isDown() const {
  return digitalRead(TTP223_PIN) == HIGH;
}

bool TouchModule::isPressed() {
  int estadoActual = digitalRead(TTP223_PIN);
  bool flancoSubida = (estadoActual == HIGH && _estadoAnterior == LOW);
  _estadoAnterior = estadoActual;
  return flancoSubida;
}
