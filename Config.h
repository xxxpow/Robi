#pragma once
#include <Arduino.h>

// =====================================================
// CONFIGURACION GLOBAL DE HARDWARE Y PARAMETROS DE ROBI
// =====================================================

// -----------------------------------------------------
// RED WIFI SOFT-AP
// -----------------------------------------------------
#define WIFI_SSID_NAME     "ROBI"
#define WIFI_PASSWORD_KEY  "12345678"
#define WEB_SERVER_PORT    80

// -----------------------------------------------------
// PANTALLA TFT ST7789 (SPI)
// -----------------------------------------------------
#define TFT_CS_PIN    -1
#define TFT_DC_PIN    27
#define TFT_RST_PIN   4
#define TFT_SCLK_PIN  18
#define TFT_MOSI_PIN  23

#define TFT_WIDTH     240
#define TFT_HEIGHT    240

// -----------------------------------------------------
// COLORES TFT (RGB565)
// -----------------------------------------------------
#define COLOR_FONDO   0x0000 // ST77XX_BLACK
#define COLOR_NORMAL  0x067F
#define COLOR_ALEGRE  0xFFE0
#define COLOR_ENOJO   0xF800

// -----------------------------------------------------
// DIMENSIONES Y PARAMETROS DE OJOS
// -----------------------------------------------------
// Ojos normales
#define OJO_ANCHO 75
#define OJO_ALTO  80
#define RADIO_OJO 17

// Ojos alegres (arcos)
#define ARCO_ANCHO            80
#define ARCO_ALTO             30
#define ARCO_GROSOR           10
#define ARCO_ARRIBA_Y         130
#define ARCO_ABAJO_Y          110
#define ARCO_CENTRO_Y         120
#define ARCO_CENTRO_X         130
#define DURACION_ALEGRIA      3000
#define INTERVALO_ARCO_ALEGRE 20

// Ojos enojados (poligonos)
#define ENOJO_POSICION_X       127
#define ENOJO_POSICION_Y       120
#define ENOJO_SEPARACION       12
#define ENOJO_FIG1_ANCHO       80
#define ENOJO_FIG1_ALTO        70
#define ENOJO_FIG2_ANCHO       80
#define ENOJO_FIG2_ALTO        24
#define DURACION_ENOJO         3000
#define INTERVALO_FIGURA_ENOJO 300

// Ojos de sorpresa (palmada)
#define RADIO_OJO_SORPRESA    48
#define RADIO_PUPILA_SORPRESA 5
#define SORPRESA_CENTRO_IZQ_X 80
#define SORPRESA_CENTRO_DER_X 175
#define SORPRESA_CENTRO_Y     120

// Ojos cerrados (palmada)
#define TIEMPO_OJOS_CERRADOS_PALMADA 180
#define TIEMPO_MAXIMO_ESPERA_MP3_PALMADA 5000

// Parpadeo durante conversacion
#define INTERVALO_PARPADEO_OJOS 1800
#define DURACION_PARPADEO_OJOS  120

// -----------------------------------------------------
// IMU MPU6050 (I2C)
// -----------------------------------------------------
#define MPU_I2C_ADDR                  0x68
#define I2C_SDA_PIN                   21
#define I2C_SCL_PIN                   22
#define IMU_INT_PIN                   34

#define MPU_REG_ACCEL_XOUT_H          0x3B
#define MPU_REG_ACCEL_YOUT_H          0x3D
#define MPU_REG_ACCEL_ZOUT_H          0x3F
#define MPU_REG_PWR_MGMT_1            0x6B
#define MPU_REG_ACCEL_CONFIG          0x1C

#define UMBRAL_INCLINACION            15.0
#define TIEMPO_CONFIRMACION_MOVIMIENTO 100
#define TIEMPO_ESTABILIDAD_NORMAL     80

// -----------------------------------------------------
// SENSOR TACTIL CAPACITIVO TTP223
// -----------------------------------------------------
#define TTP223_PIN 33

// -----------------------------------------------------
// REPRODUCTOR MP3 TF-18P / DFPLAYER (UART2)
// -----------------------------------------------------
#define MP3_RX_PIN        16
#define MP3_TX_PIN        17
#define MP3_BUSY_PIN      19
#define MP3_REPRODUCIENDO LOW

#define MP3_CMD_PLAY_INDEX 0x03
#define MP3_CMD_SET_VOLUME 0x06
#define MP3_CMD_SEL_DEV    0x09
#define MP3_DEV_TF         0x02
#define MP3_VOLUMEN_DEFECTO 25

// LED de la boca (voz sincronizada)
#define LED_VOZ_PIN       5
#define INTERVALO_LED_VOZ 180

// -----------------------------------------------------
// MICROFONO DIGITAL I2S (INMP441)
// -----------------------------------------------------
#define I2S_WS_PIN         25
#define I2S_SCK_PIN        26
#define I2S_SD_PIN         32
#define I2S_PORT_NUM       I2S_NUM_0

#define FACTOR_PALMADA              2
#define NIVEL_MINIMO_PALMADA        40000
#define DURACION_PALMADA            200
#define BLOQUEO_PALMADA             300
#define BLOQUEO_MICROFONO_POST_VOZ  2500

// -----------------------------------------------------
// TABLA Y MAPEO DE PISTAS DE AUDIO
// -----------------------------------------------------
// Conversacion (Preguntas Web)
const uint16_t PISTA_HOLA             = 7;
const uint16_t PISTA_NOMBRE           = 4;
const uint16_t PISTA_QUE_PUEDE_HACER  = 17;
const uint16_t PISTA_CANCION          = 18;
const uint16_t PISTA_CHISTE           = 9;
const uint16_t PISTA_NUEVO_CUERPO     = 10;
const uint16_t PISTA_SUSCRIPTORES     = 11;
const uint16_t PISTA_ADIOS            = 12;

// Tacto (TTP223)
const uint16_t PISTAS_TTP223[] = {6, 14, 15, 16};
const int CANTIDAD_PISTAS_TTP = sizeof(PISTAS_TTP223) / sizeof(PISTAS_TTP223[0]);

// Enojo por movimiento (alterna 2 pistas)
const uint16_t PISTAS_ENOJO[] = {19, 8};
const int CANTIDAD_PISTAS_ENOJO = sizeof(PISTAS_ENOJO) / sizeof(PISTAS_ENOJO[0]);

// Palmada (secuencia de 5 pistas)
const uint16_t PISTAS_PALMADA[] = {1, 3, 2, 13, 5};
const int CANTIDAD_PISTAS_PALMADA = sizeof(PISTAS_PALMADA) / sizeof(PISTAS_PALMADA[0]);
