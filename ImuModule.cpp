#include "ImuModule.h"
#include <math.h>

ImuModule::ImuModule()
  : _offsetX(0),
    _offsetY(0),
    _anguloX(0),
    _anguloY(0),
    _direccionInstantanea(NORMAL),
    _movimientoConfirmado(false),
    _inicioMovimientoDetectado(0),
    _inicioNormalDetectado(0) {}

void ImuModule::init() {
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Despertar el sensor MPU6050
  writeRegister(MPU_REG_PWR_MGMT_1, 0x00);
  delay(100);

  // Configurar acelerometro (+- 2g)
  writeRegister(MPU_REG_ACCEL_CONFIG, 0x00);
  delay(100);
}

void ImuModule::writeRegister(byte registro, byte valor) {
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(registro);
  Wire.write(valor);
  Wire.endTransmission();
}

int16_t ImuModule::readRegister16(byte registro) {
  Wire.beginTransmission(MPU_I2C_ADDR);
  Wire.write(registro);
  Wire.endTransmission(false);

  Wire.requestFrom((uint8_t)MPU_I2C_ADDR, (uint8_t)2);
  if (Wire.available() < 2) {
    return 0;
  }

  int16_t valor = Wire.read() << 8;
  valor |= Wire.read();
  return valor;
}

void ImuModule::readAngles() {
  int16_t ax = readRegister16(MPU_REG_ACCEL_XOUT_H);
  int16_t ay = readRegister16(MPU_REG_ACCEL_YOUT_H);
  int16_t az = readRegister16(MPU_REG_ACCEL_ZOUT_H);

  float fax = ax / 16384.0f;
  float fay = ay / 16384.0f;
  float faz = az / 16384.0f;

  _anguloX = (atan2(fax, sqrt(fay * fay + faz * faz)) * 180.0f / PI) - _offsetX;
  _anguloY = (atan2(fay, sqrt(fax * fax + faz * faz)) * 180.0f / PI) - _offsetY;
}

void ImuModule::calibrate() {
  float sumaX = 0;
  float sumaY = 0;
  const int muestras = 100;

  for (int i = 0; i < muestras; i++) {
    readAngles();
    sumaX += _anguloX;
    sumaY += _anguloY;
    delay(10);
  }

  _offsetX = sumaX / muestras;
  _offsetY = sumaY / muestras;

  Serial.println("[IMU] Calibracion MPU6050 completada.");
}

void ImuModule::resetStability() {
  _movimientoConfirmado = false;
  _inicioMovimientoDetectado = 0;
  _inicioNormalDetectado = 0;
}

void ImuModule::update() {
  readAngles();

  // Determinar direccion de inclinacion segun umbrales
  if (_anguloY > UMBRAL_INCLINACION) {
    _direccionInstantanea = MIRA_ARRIBA;
  } else if (_anguloY < -UMBRAL_INCLINACION) {
    _direccionInstantanea = MIRA_ABAJO;
  } else if (_anguloX < -UMBRAL_INCLINACION) {
    _direccionInstantanea = MIRA_DERECHA;
  } else if (_anguloX > UMBRAL_INCLINACION) {
    _direccionInstantanea = MIRA_IZQUIERDA;
  } else {
    _direccionInstantanea = NORMAL;
  }

  // Filtro de confirmacion de movimiento
  if (_direccionInstantanea != NORMAL) {
    _inicioNormalDetectado = 0;

    if (_inicioMovimientoDetectado == 0) {
      _inicioMovimientoDetectado = millis();
    }

    if (millis() - _inicioMovimientoDetectado >= TIEMPO_CONFIRMACION_MOVIMIENTO) {
      _movimientoConfirmado = true;
    }
  } else {
    _inicioMovimientoDetectado = 0;

    if (_inicioNormalDetectado == 0) {
      _inicioNormalDetectado = millis();
    }

    if (millis() - _inicioNormalDetectado >= TIEMPO_ESTABILIDAD_NORMAL) {
      _movimientoConfirmado = false;
    }
  }
}
