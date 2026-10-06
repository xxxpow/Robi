#pragma once
#include <Arduino.h>
#include <driver/i2s.h>
#include "Config.h"

class MicrophoneModule {
public:
  MicrophoneModule();

  void init();
  void calibrate();

  float readLevel();
  bool detectClap();

  // Bloqueo y proteccion contra eco/voz propia del robot
  void lock();
  void unlock();
  bool isLocked() const { return _bloqueoMicrofono; }
  void updateLock(bool robotHablando);
  void flushDma();

  float getAmbientNoise() const { return _ruidoAmbiente; }
  float getLastLevel() const { return _ultimoNivel; }

private:
  int32_t _muestrasI2S[256];
  float _ruidoAmbiente;
  float _ultimoNivel;
  unsigned long _ultimaPalmada;

  bool _bloqueoMicrofono;
  unsigned long _inicioBloqueoMicrofono;
};
