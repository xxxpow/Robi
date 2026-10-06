#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "Types.h"
#include "Config.h"

enum ModoOjos {
  MODO_NORMAL,
  MODO_ALEGRE,
  MODO_ENOJADO,
  MODO_SORPRESA,
  MODO_CERRADOS
};

struct RectArea {
  int x;
  int y;
  int w;
  int h;
  bool active;
};

class DisplayModule {
public:
  DisplayModule();

  void init();
  void clear();
  void update();

  // Ojos Normales y Mirada Fluida
  void showDirection(Direccion direccion);
  Direccion getCurrentDirection() const { return _currentDirection; }
  void resetDirectionTracking();

  // Parpadeo suave y realista (Eyelid animation)
  void startBlink(unsigned long durationMs = 140);
  bool isBlinking() const { return _isBlinking; }

  // Ojos Alegres
  void showHappy(bool arcUp);
  void showHappyCenter();

  // Ojos Enojados
  void showAngryDirection(Direccion direccion);
  void showAngryFigure(bool figuraGrande);
  void showAngryCenter();

  // Ojos de Palmada / Expresiones Especiales
  void showSurprise();
  void showClosedEyes();

  ModoOjos getCurrentMode() const { return _modoActual; }

private:
  Adafruit_ST7789 _tft;
  ModoOjos _modoActual;
  Direccion _currentDirection;
  Direccion _currentAngryDirection;

  // Variables de interpolacion suave de mirada (Easing)
  float _currentOffsetX;
  float _currentOffsetY;
  float _targetOffsetX;
  float _targetOffsetY;

  // Variables de parpadeo realista
  bool _isBlinking;
  unsigned long _blinkStartTime;
  unsigned long _blinkDuration;
  int _currentEyeHeight;

  // Temporizador de refresco
  unsigned long _lastUpdateMs;

  // Rectangulos previos para borrado selectivo sin parpadeo (Dirty Rectangles)
  RectArea _prevLeftArea;
  RectArea _prevRightArea;

  void erasePreviousAreas();
  void renderNormalEyes(int offsetX, int offsetY, int altura);
  void drawPolygon(Punto puntos[], int cantidad, uint16_t color);
  void drawAngryEye(int centroX, int centroY, int ancho, int alto, bool izquierdo, bool figuraGrande);
  void drawSingleArc(int centroX, int centroY);
};

