#include "MicrophoneModule.h"

MicrophoneModule::MicrophoneModule()
  : _ruidoAmbiente(0),
    _ultimoNivel(0),
    _ultimaPalmada(0),
    _bloqueoMicrofono(false),
    _inicioBloqueoMicrofono(0) {
  memset(_muestrasI2S, 0, sizeof(_muestrasI2S));
}

void MicrophoneModule::init() {
  i2s_config_t configuracionI2S = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pinesI2S = {
    .bck_io_num = I2S_SCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD_PIN
  };

  i2s_driver_install(I2S_PORT_NUM, &configuracionI2S, 0, NULL);
  i2s_set_pin(I2S_PORT_NUM, &pinesI2S);
  flushDma();
}

void MicrophoneModule::calibrate() {
  long suma = 0;

  for (int i = 0; i < 20; i++) {
    size_t bytesLeidos = 0;
    i2s_read(I2S_PORT_NUM, _muestrasI2S, sizeof(_muestrasI2S), &bytesLeidos, portMAX_DELAY);

    int cantidad = bytesLeidos / sizeof(int32_t);
    long nivel = 0;

    for (int j = 0; j < cantidad; j++) {
      nivel += abs(_muestrasI2S[j] / 1000);
    }

    if (cantidad > 0) {
      suma += nivel / cantidad;
    }
  }

  _ruidoAmbiente = suma / 20.0f;
  Serial.print("[MICRO] Calibracion completada. Ruido ambiente: ");
  Serial.println(_ruidoAmbiente);
}

void MicrophoneModule::flushDma() {
  i2s_zero_dma_buffer(I2S_PORT_NUM);
}

void MicrophoneModule::lock() {
  _bloqueoMicrofono = true;
  _inicioBloqueoMicrofono = millis();
  flushDma();
}

void MicrophoneModule::unlock() {
  _bloqueoMicrofono = false;
  flushDma();
  Serial.println("[MICRO] Deteccion de audio rehabilitada");
}

void MicrophoneModule::updateLock(bool robotHablando) {
  if (!_bloqueoMicrofono) return;

  if (robotHablando) {
    flushDma();
    _inicioBloqueoMicrofono = millis();
    return;
  }

  // Esperar el periodo de gracia post-voz
  if (millis() - _inicioBloqueoMicrofono < BLOQUEO_MICROFONO_POST_VOZ) {
    flushDma();
    return;
  }

  // Fin del periodo de bloqueo
  unlock();
}

float MicrophoneModule::readLevel() {
  size_t bytesLeidos = 0;
  i2s_read(I2S_PORT_NUM, _muestrasI2S, sizeof(_muestrasI2S), &bytesLeidos, 0);

  int cantidad = bytesLeidos / sizeof(int32_t);
  if (cantidad <= 0) return 0;

  long suma = 0;
  for (int i = 0; i < cantidad; i++) {
    suma += abs(_muestrasI2S[i] / 1000);
  }

  _ultimoNivel = suma / (float)cantidad;
  return _ultimoNivel;
}

bool MicrophoneModule::detectClap() {
  if (_bloqueoMicrofono) {
    return false;
  }

  float nivel = readLevel();

  if (nivel > (_ruidoAmbiente * FACTOR_PALMADA) && nivel > NIVEL_MINIMO_PALMADA) {
    if (millis() - _ultimaPalmada > BLOQUEO_PALMADA) {
      _ultimaPalmada = millis();
      return true;
    }
  }

  return false;
}
