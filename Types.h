#pragma once
#include <Arduino.h>

// =====================================================
// DEFINICION DE TIPOS Y ESTRUCTURAS DE ROBI
// =====================================================

enum Direccion {
  DIR_INVALIDA = -1,
  NORMAL = 0,
  MIRA_ARRIBA,
  MIRA_ABAJO,
  MIRA_IZQUIERDA,
  MIRA_DERECHA
};

struct Punto {
  int x;
  int y;
};
