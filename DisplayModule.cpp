#include "DisplayModule.h"
#include <SPI.h>
#include <math.h>

DisplayModule::DisplayModule()
  : _tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN),
    _modoActual(MODO_NORMAL),
    _currentDirection(DIR_INVALIDA),
    _currentAngryDirection(DIR_INVALIDA),
    _currentOffsetX(0.0f),
    _currentOffsetY(0.0f),
    _targetOffsetX(0.0f),
    _targetOffsetY(0.0f),
    _isBlinking(false),
    _blinkStartTime(0),
    _blinkDuration(140),
    _currentEyeHeight(OJO_ALTO),
    _lastUpdateMs(0),
    _prevLeftArea({0, 0, 0, 0, false}),
    _prevRightArea({0, 0, 0, 0, false}) {}

void DisplayModule::init() {
  SPI.begin(TFT_SCLK_PIN, -1, TFT_MOSI_PIN, TFT_CS_PIN);
  _tft.init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE3);
  _tft.setRotation(1);
  _tft.setSPISpeed(40000000); // 40 MHz para fluidez en ST7789
  clear();
  _modoActual = MODO_NORMAL;
  _currentDirection = DIR_INVALIDA;
  _currentAngryDirection = DIR_INVALIDA;
  _currentOffsetX = 0.0f;
  _currentOffsetY = 0.0f;
  _targetOffsetX = 0.0f;
  _targetOffsetY = 0.0f;
  _currentEyeHeight = OJO_ALTO;
  _isBlinking = false;
  _lastUpdateMs = millis();
}

void DisplayModule::clear() {
  _tft.fillScreen(COLOR_FONDO);
  _prevLeftArea.active = false;
  _prevRightArea.active = false;
}

void DisplayModule::resetDirectionTracking() {
  _currentDirection = DIR_INVALIDA;
  _currentAngryDirection = DIR_INVALIDA;
}

void DisplayModule::erasePreviousAreas() {
  if (_prevLeftArea.active) {
    _tft.fillRect(_prevLeftArea.x, _prevLeftArea.y, _prevLeftArea.w, _prevLeftArea.h, COLOR_FONDO);
    _prevLeftArea.active = false;
  }
  if (_prevRightArea.active) {
    _tft.fillRect(_prevRightArea.x, _prevRightArea.y, _prevRightArea.w, _prevRightArea.h, COLOR_FONDO);
    _prevRightArea.active = false;
  }
}

// =====================================================
// DIBUJAR OJOS NORMALES Y MOTOR DE ANIMACION
// =====================================================

void DisplayModule::renderNormalEyes(int offsetX, int offsetY, int altura) {
  erasePreviousAreas();

  int izquierdaX = 42 + offsetX;
  int derechaX   = 137 + offsetX;
  int centroY    = 120 + offsetY;
  int ojosY      = centroY - (altura / 2);
  int radio      = min((int)RADIO_OJO, max(2, altura / 2));

  _tft.fillRoundRect(izquierdaX, ojosY, OJO_ANCHO, altura, radio, COLOR_NORMAL);
  _tft.fillRoundRect(derechaX,   ojosY, OJO_ANCHO, altura, radio, COLOR_NORMAL);

  _prevLeftArea  = { izquierdaX - 1, ojosY - 1, OJO_ANCHO + 2, altura + 2, true };
  _prevRightArea = { derechaX - 1,   ojosY - 1, OJO_ANCHO + 2, altura + 2, true };
}

void DisplayModule::startBlink(unsigned long durationMs) {
  if (_modoActual == MODO_NORMAL && !_isBlinking) {
    _isBlinking = true;
    _blinkStartTime = millis();
    _blinkDuration = (durationMs > 0) ? durationMs : 140;
  }
}

void DisplayModule::showDirection(Direccion direccion) {
  int targetX = 0;
  int targetY = 0;

  switch (direccion) {
    case NORMAL:         targetX = 0;   targetY = 0;   break;
    case MIRA_ARRIBA:    targetX = 0;   targetY = -20; break;
    case MIRA_ABAJO:     targetX = 0;   targetY = 20;  break;
    case MIRA_IZQUIERDA: targetX = -20; targetY = 0;   break;
    case MIRA_DERECHA:   targetX = 20;  targetY = 0;   break;
    default: return;
  }

  if (_modoActual != MODO_NORMAL) {
    _modoActual = MODO_NORMAL;
    _currentDirection = direccion;
    _targetOffsetX = targetX;
    _targetOffsetY = targetY;
    _currentOffsetX = targetX;
    _currentOffsetY = targetY;
    renderNormalEyes((int)_currentOffsetX, (int)_currentOffsetY, _currentEyeHeight);
    return;
  }

  if (_currentDirection == direccion && _targetOffsetX == targetX && _targetOffsetY == targetY) {
    return;
  }

  _currentDirection = direccion;
  _targetOffsetX = targetX;
  _targetOffsetY = targetY;
}

void DisplayModule::update() {
  if (_modoActual != MODO_NORMAL) {
    return;
  }

  unsigned long ahora = millis();
  if (ahora - _lastUpdateMs < 16) {
    return;
  }
  _lastUpdateMs = ahora;

  bool necesitaRedibujo = false;

  // 1. Manejo del parpadeo realista
  if (_isBlinking) {
    unsigned long transcurrido = ahora - _blinkStartTime;
    if (transcurrido >= _blinkDuration) {
      _isBlinking = false;
      _currentEyeHeight = OJO_ALTO;
      necesitaRedibujo = true;
    } else {
      float progreso = (float)transcurrido / (float)_blinkDuration;
      float factor = fabs(cos(progreso * 3.14159265f));
      int nuevoAlto = 4 + (int)((OJO_ALTO - 4) * factor);
      if (nuevoAlto != _currentEyeHeight) {
        _currentEyeHeight = nuevoAlto;
        necesitaRedibujo = true;
      }
    }
  }

  // 2. Interpolacion de mirada con Easing suave
  float dx = _targetOffsetX - _currentOffsetX;
  float dy = _targetOffsetY - _currentOffsetY;

  if (fabs(dx) > 0.4f || fabs(dy) > 0.4f) {
    _currentOffsetX += dx * 0.35f;
    _currentOffsetY += dy * 0.35f;
    necesitaRedibujo = true;
  } else if (_currentOffsetX != _targetOffsetX || _currentOffsetY != _targetOffsetY) {
    _currentOffsetX = _targetOffsetX;
    _currentOffsetY = _targetOffsetY;
    necesitaRedibujo = true;
  }

  if (necesitaRedibujo) {
    renderNormalEyes((int)round(_currentOffsetX), (int)round(_currentOffsetY), _currentEyeHeight);
  }
}

// =====================================================
// ALGORITMO DE POLIGONOS (PARA OJOS ENOJADOS)
// =====================================================

void DisplayModule::drawPolygon(Punto puntos[], int cantidad, uint16_t color) {
  int minY = puntos[0].y;
  int maxY = puntos[0].y;

  for (int i = 1; i < cantidad; i++) {
    if (puntos[i].y < minY) minY = puntos[i].y;
    if (puntos[i].y > maxY) maxY = puntos[i].y;
  }

  for (int y = minY; y <= maxY; y++) {
    int nodos[20];
    int cantidadNodos = 0;

    for (int i = 0; i < cantidad; i++) {
      int j = (i + 1) % cantidad;
      int yi = puntos[i].y;
      int yj = puntos[j].y;

      if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
        int xi = puntos[i].x;
        int xj = puntos[j].x;
        int x = xi + (long)(y - yi) * (xj - xi) / (yj - yi);
        if (cantidadNodos < 20) {
          nodos[cantidadNodos++] = x;
        }
      }
    }

    for (int i = 0; i < cantidadNodos - 1; i++) {
      for (int j = i + 1; j < cantidadNodos; j++) {
        if (nodos[i] > nodos[j]) {
          int temporal = nodos[i];
          nodos[i] = nodos[j];
          nodos[j] = temporal;
        }
      }
    }

    for (int i = 0; i < cantidadNodos; i += 2) {
      if (i + 1 < cantidadNodos) {
        _tft.drawFastHLine(nodos[i], y, nodos[i + 1] - nodos[i] + 1, color);
      }
    }
  }
}

void DisplayModule::drawAngryEye(int centroX, int centroY, int ancho, int alto, bool izquierdo, bool figuraGrande) {
  int mitadAncho = ancho / 2;
  int mitadAlto  = alto / 2;
  Punto puntos[7];

  if (figuraGrande) {
    if (izquierdo) {
      puntos[0] = { centroX + mitadAncho,            centroY - (mitadAlto * 45) / 100 };
      puntos[1] = { centroX - (mitadAncho * 65) / 100, centroY - mitadAlto };
      puntos[2] = { centroX - (mitadAncho * 95) / 100, centroY - (mitadAlto * 55) / 100 };
      puntos[3] = { centroX - mitadAncho,            centroY };
      puntos[4] = { centroX - (mitadAncho * 95) / 100, centroY + (mitadAlto * 55) / 100 };
      puntos[5] = { centroX - (mitadAncho * 65) / 100, centroY + mitadAlto };
      puntos[6] = { centroX + mitadAncho,            centroY + (mitadAlto * 45) / 100 };
    } else {
      puntos[0] = { centroX - mitadAncho,            centroY - (mitadAlto * 45) / 100 };
      puntos[1] = { centroX + (mitadAncho * 65) / 100, centroY - mitadAlto };
      puntos[2] = { centroX + (mitadAncho * 95) / 100, centroY - (mitadAlto * 55) / 100 };
      puntos[3] = { centroX + mitadAncho,            centroY };
      puntos[4] = { centroX + (mitadAncho * 95) / 100, centroY + (mitadAlto * 55) / 100 };
      puntos[5] = { centroX + (mitadAncho * 65) / 100, centroY + mitadAlto };
      puntos[6] = { centroX - mitadAncho,            centroY + (mitadAlto * 45) / 100 };
    }
  } else {
    if (izquierdo) {
      puntos[0] = { centroX + mitadAncho,            centroY - (mitadAlto * 25) / 100 };
      puntos[1] = { centroX - (mitadAncho * 70) / 100, centroY - mitadAlto };
      puntos[2] = { centroX - (mitadAncho * 95) / 100, centroY - (mitadAlto * 40) / 100 };
      puntos[3] = { centroX - mitadAncho,            centroY };
      puntos[4] = { centroX - (mitadAncho * 95) / 100, centroY + (mitadAlto * 40) / 100 };
      puntos[5] = { centroX - (mitadAncho * 70) / 100, centroY + mitadAlto };
      puntos[6] = { centroX + mitadAncho,            centroY + (mitadAlto * 25) / 100 };
    } else {
      puntos[0] = { centroX - mitadAncho,            centroY - (mitadAlto * 25) / 100 };
      puntos[1] = { centroX + (mitadAncho * 70) / 100, centroY - mitadAlto };
      puntos[2] = { centroX + (mitadAncho * 95) / 100, centroY - (mitadAlto * 40) / 100 };
      puntos[3] = { centroX + mitadAncho,            centroY };
      puntos[4] = { centroX + (mitadAncho * 95) / 100, centroY + (mitadAlto * 40) / 100 };
      puntos[5] = { centroX + (mitadAncho * 70) / 100, centroY + mitadAlto };
      puntos[6] = { centroX - mitadAncho,            centroY + (mitadAlto * 25) / 100 };
    }
  }

  drawPolygon(puntos, 7, COLOR_ENOJO);
}

void DisplayModule::showAngryDirection(Direccion nuevaDireccion) {
  if (_modoActual == MODO_ENOJADO && _currentAngryDirection == nuevaDireccion) {
    return;
  }
  _modoActual = MODO_ENOJADO;
  _currentAngryDirection = nuevaDireccion;

  erasePreviousAreas();

  int desplazamientoX = 0;
  int desplazamientoY = 0;

  switch (nuevaDireccion) {
    case MIRA_ARRIBA:    desplazamientoY = -40; break;
    case MIRA_ABAJO:     desplazamientoY = 40;  break;
    case MIRA_IZQUIERDA: desplazamientoX = -40; break;
    case MIRA_DERECHA:   desplazamientoX = 30;  break;
    case NORMAL:
    default:
      break;
  }

  int centroX = ENOJO_POSICION_X + desplazamientoX;
  int centroY = ENOJO_POSICION_Y + desplazamientoY;
  int distancia = (ENOJO_FIG1_ANCHO / 2) + (ENOJO_SEPARACION / 1);

  int leftCenterX = centroX - distancia;
  int rightCenterX = centroX + distancia;

  drawAngryEye(leftCenterX, centroY, ENOJO_FIG1_ANCHO, ENOJO_FIG1_ALTO, true, true);
  drawAngryEye(rightCenterX, centroY, ENOJO_FIG1_ANCHO, ENOJO_FIG1_ALTO, false, true);

  _prevLeftArea  = { leftCenterX - (ENOJO_FIG1_ANCHO / 2) - 2, centroY - (ENOJO_FIG1_ALTO / 2) - 2, ENOJO_FIG1_ANCHO + 4, ENOJO_FIG1_ALTO + 4, true };
  _prevRightArea = { rightCenterX - (ENOJO_FIG1_ANCHO / 2) - 2, centroY - (ENOJO_FIG1_ALTO / 2) - 2, ENOJO_FIG1_ANCHO + 4, ENOJO_FIG1_ALTO + 4, true };
}

void DisplayModule::showAngryFigure(bool figuraGrande) {
  _modoActual = MODO_ENOJADO;
  erasePreviousAreas();

  int ancho = figuraGrande ? ENOJO_FIG1_ANCHO : ENOJO_FIG2_ANCHO;
  int alto  = figuraGrande ? ENOJO_FIG1_ALTO  : ENOJO_FIG2_ALTO;
  int distancia = (ancho / 2) + (ENOJO_SEPARACION / 2);

  int leftCenterX = ENOJO_POSICION_X - distancia;
  int rightCenterX = ENOJO_POSICION_X + distancia;

  drawAngryEye(leftCenterX, ENOJO_POSICION_Y, ancho, alto, true, figuraGrande);
  drawAngryEye(rightCenterX, ENOJO_POSICION_Y, ancho, alto, false, figuraGrande);

  _prevLeftArea  = { leftCenterX - (ancho / 2) - 2, ENOJO_POSICION_Y - (alto / 2) - 2, ancho + 4, alto + 4, true };
  _prevRightArea = { rightCenterX - (ancho / 2) - 2, ENOJO_POSICION_Y - (alto / 2) - 2, ancho + 4, alto + 4, true };
}

void DisplayModule::showAngryCenter() {
  if (_modoActual == MODO_ENOJADO && _currentAngryDirection == NORMAL) {
    return;
  }
  _currentAngryDirection = NORMAL;
  showAngryFigure(true);
}

// =====================================================
// OJOS ALEGRES (ARCOS)
// =====================================================

void DisplayModule::drawSingleArc(int centroX, int centroY) {
  float inicio = 200.0f;
  float fin    = 340.0f;

  for (int grosor = 0; grosor < ARCO_GROSOR; grosor++) {
    int radioX = (ARCO_ANCHO / 2) - grosor;
    int radioY = (ARCO_ALTO / 2) - grosor;

    int xAnterior = 0;
    int yAnterior = 0;
    bool primerPunto = true;

    for (float angulo = inicio; angulo <= fin; angulo += 2.0f) {
      float radianes = angulo * PI / 180.0f;
      int x = centroX + (int)(cos(radianes) * radioX);
      int y = centroY + (int)(sin(radianes) * radioY);

      if (!primerPunto) {
        _tft.drawLine(xAnterior, yAnterior, x, y, COLOR_ALEGRE);
      }
      xAnterior = x;
      yAnterior = y;
      primerPunto = false;
    }
  }
}

void DisplayModule::showHappy(bool arcUp) {
  _modoActual = MODO_ALEGRE;
  erasePreviousAreas();

  int posY = arcUp ? ARCO_ARRIBA_Y : ARCO_ABAJO_Y;
  drawSingleArc(82, posY);
  drawSingleArc(178, posY);

  _prevLeftArea  = { 82 - (ARCO_ANCHO / 2) - 2, posY - (ARCO_ALTO / 2) - 4, ARCO_ANCHO + 4, ARCO_ALTO + 8, true };
  _prevRightArea = { 178 - (ARCO_ANCHO / 2) - 2, posY - (ARCO_ALTO / 2) - 4, ARCO_ANCHO + 4, ARCO_ALTO + 8, true };
}

void DisplayModule::showHappyCenter() {
  _modoActual = MODO_ALEGRE;
  erasePreviousAreas();

  drawSingleArc(82, ARCO_CENTRO_Y);
  drawSingleArc(178, ARCO_CENTRO_Y);

  _prevLeftArea  = { 82 - (ARCO_ANCHO / 2) - 2, ARCO_CENTRO_Y - (ARCO_ALTO / 2) - 4, ARCO_ANCHO + 4, ARCO_ALTO + 8, true };
  _prevRightArea = { 178 - (ARCO_ANCHO / 2) - 2, ARCO_CENTRO_Y - (ARCO_ALTO / 2) - 4, ARCO_ANCHO + 4, ARCO_ALTO + 8, true };
}

// =====================================================
// OJOS DE PALMADA / EXPRESIONES ESPECIALES
// =====================================================

void DisplayModule::showSurprise() {
  _modoActual = MODO_SORPRESA;
  erasePreviousAreas();

  // Ojos circulares grandes
  _tft.fillCircle(SORPRESA_CENTRO_IZQ_X, SORPRESA_CENTRO_Y, RADIO_OJO_SORPRESA, COLOR_NORMAL);
  _tft.fillCircle(SORPRESA_CENTRO_DER_X, SORPRESA_CENTRO_Y, RADIO_OJO_SORPRESA, COLOR_NORMAL);

  // Pupilas negras centradas
  _tft.fillCircle(SORPRESA_CENTRO_IZQ_X, SORPRESA_CENTRO_Y, RADIO_PUPILA_SORPRESA, COLOR_FONDO);
  _tft.fillCircle(SORPRESA_CENTRO_DER_X, SORPRESA_CENTRO_Y, RADIO_PUPILA_SORPRESA, COLOR_FONDO);

  _prevLeftArea  = { SORPRESA_CENTRO_IZQ_X - RADIO_OJO_SORPRESA - 2, SORPRESA_CENTRO_Y - RADIO_OJO_SORPRESA - 2, (RADIO_OJO_SORPRESA * 2) + 4, (RADIO_OJO_SORPRESA * 2) + 4, true };
  _prevRightArea = { SORPRESA_CENTRO_DER_X - RADIO_OJO_SORPRESA - 2, SORPRESA_CENTRO_Y - RADIO_OJO_SORPRESA - 2, (RADIO_OJO_SORPRESA * 2) + 4, (RADIO_OJO_SORPRESA * 2) + 4, true };
}

void DisplayModule::showClosedEyes() {
  _modoActual = MODO_CERRADOS;
  erasePreviousAreas();

  // Lineas horizontales gruesas
  for (int i = -4; i <= 4; i++) {
    _tft.drawLine(42, 120 + i, 117, 120 + i, COLOR_NORMAL);
    _tft.drawLine(137, 120 + i, 212, 120 + i, COLOR_NORMAL);
  }

  _prevLeftArea  = { 40, 114, 79, 14, true };
  _prevRightArea = { 135, 114, 79, 14, true };
}
