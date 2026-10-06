#include "AudioModule.h"

AudioModule::AudioModule()
  : _serial(2),
    _reproduciendo(false),
    _estadoBusyAnterior(HIGH),
    _justStarted(false),
    _justFinished(false),
    _ultimoParpadeoLED(0),
    _estadoLEDVoz(false),
    _indiceTTP(0),
    _indiceEnojo(0),
    _indicePalmada(0) {}

void AudioModule::init() {
  pinMode(MP3_BUSY_PIN, INPUT);
  pinMode(LED_VOZ_PIN, OUTPUT);
  digitalWrite(LED_VOZ_PIN, LOW);

  _estadoBusyAnterior = digitalRead(MP3_BUSY_PIN);
  _reproduciendo = (_estadoBusyAnterior == MP3_REPRODUCIENDO);

  _serial.begin(9600, SERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN);
  delay(500);

  sendCommand(MP3_CMD_SEL_DEV, MP3_DEV_TF);
  delay(500);

  setVolume(MP3_VOLUMEN_DEFECTO);
  delay(500);
}

void AudioModule::sendCommand(byte comando, uint16_t parametro) {
  byte buffer[10];

  buffer[0] = 0x7E;
  buffer[1] = 0xFF;
  buffer[2] = 0x06;
  buffer[3] = comando;
  buffer[4] = 0x00;
  buffer[5] = (byte)(parametro >> 8);
  buffer[6] = (byte)(parametro & 0xFF);
  buffer[7] = 0x00;
  buffer[8] = 0x00;
  buffer[9] = 0xEF;

  uint16_t checksum = 0 - (
    0xFF + 0x06 + comando + 0x00 +
    (parametro >> 8) + (parametro & 0xFF)
  );

  buffer[7] = (byte)(checksum >> 8);
  buffer[8] = (byte)(checksum & 0xFF);

  _serial.write(buffer, 10);
}

void AudioModule::playTrack(uint16_t pista) {
  sendCommand(MP3_CMD_PLAY_INDEX, pista);
}

void AudioModule::setVolume(uint8_t volume) {
  if (volume > 30) volume = 30;
  sendCommand(MP3_CMD_SET_VOLUME, volume);
}

void AudioModule::update() {
  int estadoBusy = digitalRead(MP3_BUSY_PIN);
  bool reproduciendo = (estadoBusy == MP3_REPRODUCIENDO);

  _justStarted = (! _reproduciendo && reproduciendo);
  _justFinished = (_reproduciendo && !reproduciendo);
  _reproduciendo = reproduciendo;

  // Actualizar LED de la boca (parpadeo mientras habla)
  if (_reproduciendo) {
    if (millis() - _ultimoParpadeoLED >= INTERVALO_LED_VOZ) {
      _ultimoParpadeoLED = millis();
      _estadoLEDVoz = !_estadoLEDVoz;
      digitalWrite(LED_VOZ_PIN, _estadoLEDVoz ? HIGH : LOW);
    }
  } else {
    _estadoLEDVoz = false;
    digitalWrite(LED_VOZ_PIN, LOW);
  }

  // Notificacion de cambio de estado en puerto Serial
  if (estadoBusy != _estadoBusyAnterior) {
    _estadoBusyAnterior = estadoBusy;
    if (_reproduciendo) {
      Serial.println("[AUDIO] MP3 reproduciendo -> Boca activa");
    } else {
      Serial.println("[AUDIO] MP3 libre -> Boca apagada");
    }
  }
}

uint16_t AudioModule::getNextTouchTrack() {
  uint16_t pista = PISTAS_TTP223[_indiceTTP];
  _indiceTTP = (_indiceTTP + 1) % CANTIDAD_PISTAS_TTP;
  return pista;
}

uint16_t AudioModule::getNextAngryTrack() {
  uint16_t pista = PISTAS_ENOJO[_indiceEnojo];
  _indiceEnojo = (_indiceEnojo + 1) % CANTIDAD_PISTAS_ENOJO;
  return pista;
}

uint16_t AudioModule::getNextClapTrack() {
  uint16_t pista = PISTAS_PALMADA[_indicePalmada];
  _indicePalmada = (_indicePalmada + 1) % CANTIDAD_PISTAS_PALMADA;
  return pista;
}

uint16_t AudioModule::getConversationTrack(int pregunta) {
  switch (pregunta) {
    case 1: return PISTA_HOLA;
    case 2: return PISTA_NOMBRE;
    case 3: return PISTA_QUE_PUEDE_HACER;
    case 4: return PISTA_CANCION;
    case 5: return PISTA_CHISTE;
    case 6: return PISTA_NUEVO_CUERPO;
    case 7: return PISTA_SUSCRIPTORES;
    case 8: return PISTA_ADIOS;
    default: return 0;
  }
}
