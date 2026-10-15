# 🤖 Robi - Día 3: Creando Mi Propio Robot Desde Cero

¡Bienvenido al repositorio oficial del **Día 3** de la serie de YouTube: *Creando mi propio robot desde cero*! En este capítulo Robi cobra vida con síntesis y reproducción de audio, interacción táctil, sensores y la primera integración con nuestra aplicación móvil desarrollada en **Android Studio**.

---

## 📁 Estructura del Repositorio

* **`Robi.ino` y módulos C++ (`*.cpp`, `*.h`)**:
  * `AudioModule`: Control del reproductor MP3-TF-18P.
  * `DisplayModule`: Renderizado de expresiones y miradas en pantalla ST7789.
  * `ImuModule`: Detección de orientación y movimientos.
  * `MicrophoneModule` e `TouchModule`: Sensores de sonido y toque TTP223.
  * `WifiServerModule`: Servidor web local en ESP32 para recibir comandos.
* **[`android-app/`](./android-app/)**:
  * Proyecto completo de Android Studio (`RobiVozz`) en Kotlin para enviar los comandos de voz y texto a Robi por red local.
* **[`pistas-audio/`](./pistas-audio/)**:
  * Pistas de audio `.mp3` utilizadas por Robi para sus respuestas (`hola creador`, `adios`, `chiste`, `cancion`, `palmadas`, etc.).
* **[`guion/`](./guion/)**:
  * Documento con el guion del Día 3.

---

## 🚀 Cómo Empezar

### Firmware (ESP32)
1. Abre `Robi.ino` en el Arduino IDE.
2. Configura los pines y las credenciales Wi-Fi en `Config.h`.
3. Sube el código a tu ESP32.

### App Móvil (Android Studio)
1. Abre **Android Studio** y selecciona **Open**.
2. Abre la carpeta `android-app/`.
3. Compila y ejecuta la aplicación en tu teléfono.

---

## 📺 Serie en YouTube
¡Sigue la serie completa en YouTube y apoya el proyecto dejando una estrella ⭐ en este repositorio!
