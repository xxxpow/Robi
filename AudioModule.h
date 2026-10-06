#pragma once
#include <Arduino.h>
#include "Config.h"

class AudioModule {
public:
  AudioModule();

  void init();
  void update();

  // Control de reproduccion
  void playTrack(uint16_t pista);
  void setVolume(uint8_t volume);

  // Consultas de estado
  bool isPlaying() const { return _reproduciendo; }
  bool justStartedPlaying() const { return _justStarted; }
  bool justFinishedPlaying() const { return _justFinished; }

  // Selectores de listas de reproduccion
  uint16_t getNextTouchTrack();
  uint16_t getNextAngryTrack();
  uint16_t getNextClapTrack();
  uint16_t getConversationTrack(int pregunta);

private:
  HardwareSerial _serial;
  bool _reproduciendo;
  int _estadoBusyAnterior;

  bool _justStarted;
  bool _justFinished;

  // LED de la boca
  unsigned long _ultimoParpadeoLED;
  bool _estadoLEDVoz;

  // Indices de rotacion de listas
  int _indiceTTP;
  int _indiceEnojo;
  int _indicePalmada;

  void sendCommand(byte comando, uint16_t parametro);
};
