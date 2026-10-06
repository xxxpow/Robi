// =====================================================
// ROBI - ROBOT INTERACTIVO ESP32
// Codigo Principal Modularizado
// =====================================================

#include <Arduino.h>
#include "Config.h"
#include "Types.h"
#include "DisplayModule.h"
#include "AudioModule.h"
#include "MicrophoneModule.h"
#include "ImuModule.h"
#include "TouchModule.h"
#include "WifiServerModule.h"

// -----------------------------------------------------
// INSTANCIAS DE LOS MODULOS
// -----------------------------------------------------
DisplayModule    pantalla;
AudioModule      audio;
MicrophoneModule microfono;
ImuModule        imu;
TouchModule      tacto;
WifiServerModule servidorWifi;

// -----------------------------------------------------
// VARIABLES DE ESTADO Y CONTROL
// -----------------------------------------------------

// Estado: Tacto / Alegria
bool alegriaActiva       = false;
bool vozTactoEsperando   = false;
bool vozTactoEnCurso     = false;
bool arcoArriba          = true;
unsigned long ultimoCambioArco = 0;
unsigned long inicioAlegria    = 0;

// Estado: Conversacion Web
bool conversacionActiva  = false;
bool ojosParpadeando     = false;
unsigned long ultimoParpadeoOjos = 0;

// Estado: Palmada (Sorpresa)
bool palmadaActiva          = false;
bool palmadaOjosCerrados    = false;
bool palmadaAudioIniciado   = false;
bool palmadaAudioDetectado  = false;
unsigned long inicioPalmada      = 0;
unsigned long inicioPalmadaAudio = 0;

// Estado: Enojo por Movimiento
bool enojoActivo           = false;
bool enojoEsperando        = false;
bool figuraEnojoGrande     = true;
bool audioEnojoDisparado   = false;
unsigned long inicioEnojo           = 0;
unsigned long ultimoCambioFiguraEnojo = 0;

// -----------------------------------------------------
// DECLARACIONES DE FUNCIONES DE COMPORTAMIENTO
// -----------------------------------------------------
void onPreguntaRecibida(int pregunta);
void procesarTacto();
void procesarAlegria();
void procesarConversacion();
void procesarPalmada();
void iniciarPalmada();
void procesarMovimientoEnojo();
void procesarTransicionesAudio();

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println("========================================");
  Serial.println("   INICIANDO SISTEMAS DE ROBI          ");
  Serial.println("========================================");

  // 1. Pantalla TFT
  pantalla.init();
  pantalla.showDirection(NORMAL);
  Serial.println("[OK] Pantalla TFT iniciada");

  // 2. Sensor Tactil TTP223
  tacto.init();
  Serial.println("[OK] Sensor tactil iniciado");

  // 3. Sensor IMU MPU6050
  imu.init();
  delay(100);
  imu.calibrate();
  Serial.println("[OK] IMU MPU6050 calibrado");

  // 4. Modulo de Audio MP3 y Boca
  audio.init();
  Serial.println("[OK] Audio MP3 y LED de voz iniciados");

  // 5. Microfono I2S
  microfono.init();
  delay(500);
  microfono.calibrate();
  microfono.unlock();
  Serial.println("[OK] Microfono I2S calibrado");

  // 6. Servidor WiFi SoftAP
  servidorWifi.setTrackCallback(onPreguntaRecibida);
  servidorWifi.init();
  Serial.println("[OK] Servidor WiFi listo");

  Serial.println("========================================");
  Serial.println("   ROBI - SISTEMA INICIADO CON EXITO   ");
  Serial.println("========================================\n");
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================
void loop() {
  // 0. Actualizar animaciones continuas de pantalla (mirada fluida y parpadeo suave)
  pantalla.update();

  // 1. Atender peticiones HTTP
  servidorWifi.handleClient();

  // 2. Actualizar estado del MP3 y LED de la boca
  audio.update();

  // 3. Manejar eventos de fin de reproduccion de audio
  procesarTransicionesAudio();

  // 4. Lectura del sensor tactil
  procesarTacto();

  // 5. Si esta en modo reaccion a tacto (alegria)
  if (vozTactoEnCurso || vozTactoEsperando) {
    procesarAlegria();
    if (palmadaActiva) {
      procesarPalmada();
    }
    delay(10);
    return;
  }

  // 6. Si esta en conversacion activa
  if (conversacionActiva && audio.isPlaying()) {
    procesarConversacion();
    delay(10);
    return;
  }

  // 7. Si hay una palmada en progreso
  if (palmadaActiva) {
    procesarPalmada();
    delay(10);
    return;
  }

  // 8. Actualizar proteccion contra eco/voz propia
  microfono.updateLock(conversacionActiva || audio.isPlaying());
  if (microfono.isLocked()) {
    delay(10);
    return;
  }

  // 9. Deteccion de palmada por microfono
  if (microfono.detectClap()) {
    iniciarPalmada();
    delay(10);
    return;
  }

  // 10. Actualizar IMU y manejar enojo o mirada
  procesarMovimientoEnojo();

  // 11. Parpadeo espontaneo en reposo (comportamiento organico / vida autonoma)
  static unsigned long ultimoParpadeoReposo = 0;
  static unsigned long intervaloReposo = 4000;
  if (!conversacionActiva && !alegriaActiva && !palmadaActiva && !enojoActivo && !audio.isPlaying()) {
    if (millis() - ultimoParpadeoReposo >= intervaloReposo) {
      ultimoParpadeoReposo = millis();
      intervaloReposo = random(3500, 7500);
      pantalla.startBlink(140);
    }
  }

  delay(10);
}

// =====================================================
// MANEJADOR DE PREGUNTAS DESDE EL CELULAR (WIFI)
// =====================================================
void onPreguntaRecibida(int pregunta) {
  if (audio.isPlaying()) {
    Serial.println("[ROBI] Ocupado reproduciendo otra respuesta.");
    return;
  }

  uint16_t pista = audio.getConversationTrack(pregunta);
  if (pista == 0) {
    Serial.println("[ROBI] Pregunta no valida / sin pista asociada.");
    return;
  }

  conversacionActiva = true;

  // Bloquear microfono antes de disparar el audio
  microfono.lock();

  ojosParpadeando = false;
  ultimoParpadeoOjos = millis();
  pantalla.resetDirectionTracking();

  audio.playTrack(pista);
  Serial.print("[CONVERSACION] Pregunta ");
  Serial.print(pregunta);
  Serial.print(" -> Pista: ");
  Serial.println(pista);
}

// =====================================================
// GESTION DEL SENSOR TACTIL (TTP223)
// =====================================================
void procesarTacto() {
  if (tacto.isPressed()) {
    // No interrumpir si ya esta en conversacion o palmada
    if (!conversacionActiva && !palmadaActiva) {
      alegriaActiva = true;
      vozTactoEsperando = true;
      vozTactoEnCurso = false;
      inicioAlegria = millis();
      arcoArriba = true;

      pantalla.resetDirectionTracking();
      pantalla.showHappy(true);

      uint16_t pistaTTP = audio.getNextTouchTrack();
      audio.playTrack(pistaTTP);

      Serial.print("[TACTO] Reproduciendo pista TTP: ");
      Serial.println(pistaTTP);
    }
  }
}

// =====================================================
// ANIMACION DE ALEGRIA
// =====================================================
void procesarAlegria() {
  unsigned long ahora = millis();

  if (ahora - ultimoCambioArco >= INTERVALO_ARCO_ALEGRE) {
    ultimoCambioArco = ahora;
    pantalla.showHappy(arcoArriba);
    arcoArriba = !arcoArriba;
  }

  // Timeout de seguridad si el MP3 nunca arranca
  if (vozTactoEsperando && (ahora - inicioAlegria >= 5000)) {
    vozTactoEsperando = false;
    alegriaActiva = false;
    pantalla.resetDirectionTracking();
    pantalla.showDirection(NORMAL);
  }
}

// =====================================================
// PARPADEO DE OJOS EN CONVERSACION
// =====================================================
void procesarConversacion() {
  unsigned long ahora = millis();

  if (ahora - ultimoParpadeoOjos >= INTERVALO_PARPADEO_OJOS) {
    ultimoParpadeoOjos = ahora;
    pantalla.startBlink(150);
  }
}

// =====================================================
// SECUENCIA DE PALMADA (SORPRESA)
// =====================================================
void iniciarPalmada() {
  palmadaActiva         = true;
  palmadaOjosCerrados   = true;
  palmadaAudioIniciado  = false;
  palmadaAudioDetectado = false;

  inicioPalmada         = millis();
  inicioPalmadaAudio    = 0;

  pantalla.resetDirectionTracking();
  pantalla.showClosedEyes();

  Serial.println("[PALMADA] Ojos cerrados");
}

void procesarPalmada() {
  unsigned long ahora = millis();

  // 1. Mostrar ojos cerrados durante 180 ms
  if (palmadaOjosCerrados && (ahora - inicioPalmada >= TIEMPO_OJOS_CERRADOS_PALMADA)) {
    palmadaOjosCerrados = false;
    pantalla.showSurprise();

    uint16_t pista = audio.getNextClapTrack();
    audio.playTrack(pista);

    palmadaAudioIniciado  = true;
    palmadaAudioDetectado = false;
    inicioPalmadaAudio    = ahora;

    Serial.print("[PALMADA] Reproduciendo pista: ");
    Serial.println(pista);
  }

  // 2. Esperar confirmacion de que el MP3 empezo
  if (palmadaAudioIniciado && !palmadaAudioDetectado && audio.isPlaying()) {
    palmadaAudioDetectado = true;
    Serial.println("[PALMADA] MP3 iniciado -> Sorpresa activa");
  }

  // 3. Seguridad: timeout si MP3 no responde
  if (palmadaAudioIniciado && !palmadaAudioDetectado && (ahora - inicioPalmadaAudio >= TIEMPO_MAXIMO_ESPERA_MP3_PALMADA)) {
    Serial.println("[PALMADA] Timeout espera MP3 -> Volver a normal");
    palmadaActiva         = false;
    palmadaOjosCerrados   = false;
    palmadaAudioIniciado  = false;
    palmadaAudioDetectado = false;
    pantalla.resetDirectionTracking();
    pantalla.showDirection(NORMAL);
    return;
  }

  // 4. Fin de reproduccion -> Retornar a ojos normales
  if (palmadaAudioDetectado && !audio.isPlaying()) {
    palmadaActiva         = false;
    palmadaOjosCerrados   = false;
    palmadaAudioIniciado  = false;
    palmadaAudioDetectado = false;
    pantalla.resetDirectionTracking();
    pantalla.showDirection(NORMAL);
    Serial.println("[PALMADA] Fin de pista -> Ojos normales");
  }
}

// =====================================================
// TRANSICIONES DE ESTADO DEL AUDIO
// =====================================================
void procesarTransicionesAudio() {
  bool reproduciendo = audio.isPlaying();

  // Transicion tacto: audio empezo
  if (vozTactoEsperando && reproduciendo) {
    vozTactoEsperando = false;
    vozTactoEnCurso = true;
    Serial.println("[TACTO] Robi hablando -> Ojos alegres");
  }

  // Transicion tacto: audio termino
  if (vozTactoEnCurso && !reproduciendo) {
    vozTactoEnCurso = false;
    alegriaActiva = false;
    pantalla.resetDirectionTracking();
    pantalla.showDirection(NORMAL);
    Serial.println("[TACTO] Respuesta terminada -> Ojos normales");
  }

  // Transicion conversacion: audio termino
  if (conversacionActiva && !reproduciendo) {
    conversacionActiva = false;
    microfono.lock(); // Proteccion post-voz contra eco
    ojosParpadeando = false;
    pantalla.resetDirectionTracking();
    pantalla.showDirection(NORMAL);
    Serial.println("[CONVERSACION] Respuesta terminada");
  }
}

// =====================================================
// GESTION DEL MOVIMIENTO Y ENOJO (IMU MPU6050)
// =====================================================
void procesarMovimientoEnojo() {
  imu.update();

  Direccion dirInclinacion = imu.getDirection();
  bool movConfirmado = imu.isMovementConfirmed();

  if (movConfirmado && dirInclinacion != NORMAL) {
    enojoActivo    = true;
    enojoEsperando = false;
    inicioEnojo    = millis();

    pantalla.showAngryDirection(dirInclinacion);

    if (!audioEnojoDisparado) {
      uint16_t pistaEnojo = audio.getNextAngryTrack();

      // Bloquear microfono para evitar auto-palmada
      microfono.lock();

      audio.playTrack(pistaEnojo);
      audioEnojoDisparado = true;

      Serial.print("[ENOJO] Movimiento detectado -> Pista: ");
      Serial.println(pistaEnojo);
    }
  } else {
    // Si dejo de inclinarse
    if (enojoActivo) {
      if (!enojoEsperando) {
        enojoEsperando = true;
        inicioEnojo = millis();
        figuraEnojoGrande = true;
        ultimoCambioFiguraEnojo = millis();
        pantalla.showAngryCenter();
      } else if (millis() - inicioEnojo < DURACION_ENOJO) {
        // Alternar figura grande/pequena en reposo
        if (millis() - ultimoCambioFiguraEnojo >= INTERVALO_FIGURA_ENOJO) {
          ultimoCambioFiguraEnojo = millis();
          figuraEnojoGrande = !figuraEnojoGrande;
          pantalla.showAngryFigure(figuraEnojoGrande);
        }
      } else {
        // Tiempo de enojo finalizado
        enojoActivo    = false;
        enojoEsperando = false;
        pantalla.resetDirectionTracking();
        pantalla.showDirection(NORMAL);
      }
    } else {
      // Reposo absoluto normal
      pantalla.showDirection(NORMAL);
      audioEnojoDisparado = false;
    }
  }
}
