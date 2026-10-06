# Robi - Robot Interactivo ESP32 🤖

Proyecto de robot interactivo basado en microcontrolador ESP32 con pantalla TFT, módulo de audio, micrófono I2S, IMU y servidor Web integrado.

## 📦 Características

- **Pantalla TFT ST7789 (240x240 SPI)**: Expresiones faciales animadas (ojos normales, alegres, enojados, parpadeo y seguimiento de dirección).
- **Módulo de Audio (DAC / MAX98357A / DFPlayer)**: Efectos sonoros y voz interactiva.
- **Sensor IMU**: Detección de sacudidas, inclinación y movimiento para reacciones gestuales.
- **Micrófono**: Detección acústica (palmadas / aplausos / voz).
- **Sensor de Tacto (Touch)**: Reacción reactiva a caricias.
- **Servidor Web Soft-AP**: Interfaz web integrada para interactuar con Robi mediante Wi-Fi.

## 📁 Estructura del Proyecto

```
Robi/
├── Robi.ino              # Archivo principal del sketch de Arduino
├── Config.h              # Definiciones de hardware, pines y configuración
├── Types.h               # Tipos de datos y enumeraciones
├── DisplayModule.h/.cpp  # Control de expresiones gráficas en pantalla
├── AudioModule.h/.cpp    # Reproducción de sonido y efectos
├── MicrophoneModule.h/.cpp # Adquisición y procesamiento de micrófono
├── ImuModule.h/.cpp      # Sensor de movimiento e inercia
├── TouchModule.h/.cpp    # Sensor táctil capacitivo
└── WifiServerModule.h/.cpp # Servidor web y control vía Wi-Fi
```

## 🚀 Requisitos

- **Arduino IDE** (v2.x recomendado) o PlatformIO
- **Placa**: ESP32 Dev Module
- **Librerías principales**:
  - Adafruit GFX / Adafruit ST7789 (o equivalente)
  - Librerías para IMU (ej. MPU6050 / LSM6DS3)
  - WiFi / WebServer (incluidas en el core ESP32)
