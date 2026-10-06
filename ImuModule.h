#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "Types.h"
#include "Config.h"

class ImuModule {
public:
  ImuModule();

  void init();
  void calibrate();
  void update();

  Direccion getDirection() const { return _direccionInstantanea; }
  bool isMovementConfirmed() const { return _movimientoConfirmado; }
  void resetStability();

  float getAngleX() const { return _anguloX; }
  float getAngleY() const { return _anguloY; }

private:
  float _offsetX;
  float _offsetY;
  float _anguloX;
  float _anguloY;

  Direccion _direccionInstantanea;
  bool _movimientoConfirmado;

  unsigned long _inicioMovimientoDetectado;
  unsigned long _inicioNormalDetectado;

  void writeRegister(byte registro, byte valor);
  int16_t readRegister16(byte registro);
  void readAngles();
};
