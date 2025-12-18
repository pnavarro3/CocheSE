# Sistema de Control Autónomo para Vehículos Robot ESP8266

Sistema de coordinación maestro-esclavo para vehículos robot basado en ESP8266 con comunicación ESP-NOW.

---

## Resumen

Sistema de control distribuido que coordina dos vehículos robot mediante ESP-NOW. El maestro lee sensores y toma decisiones de navegación, transmitiendo comandos al esclavo con latencia inferior a 30ms.

**Características:**
- Comunicación ESP-NOW: 50 mensajes/segundo
- Control autónomo de distancia con sensor HC-SR04
- Servidor web para monitorización en tiempo real
- Sistema de seguridad: parada de emergencia a <5cm
- Control automático de iluminación

---

## Hardware

### Componentes por Vehículo
- ESP8266 LOLIN D1 ESP-WROOM-02
- Driver L9110S + 2 motores DC
- HC-SR04 (sensor ultrasónico)
- LM393 (sensor de luz, solo maestro)
- LEDs (opcional)
- Divisor de voltaje 5V→3.3V para ECHO

### Conexiones

```
Motores:  D1-D2 (Motor Izq)  D3-D4 (Motor Der)
Sensores: D5 (TRIG)  D6 (ECHO+divisor)  D7 (LM393)  D8 (LEDs)
```

IMPORTANTE: ECHO requiere divisor de voltaje (1kΩ + 2kΩ).

---

## Instalación

### Software
```bash
# Instalar ESP8266 Core en Arduino IDE
# Board Manager → esp8266

# Clonar repositorio
cd ~/Arduino/libraries/
git clone https://github.com/pnavarro3/CocheSE.git
```

### Configuración

**1. Maestro (maestro.ino):**
```cpp
const char* WIFI_SSID = "tu_red";
const char* WIFI_PASSWORD = "tu_password";
uint8_t MAC_ESCLAVO[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Temporal
```

**2. Esclavo (esclavo.ino):**
```cpp
const char* WIFI_SSID = "tu_red";
const char* WIFI_PASSWORD = "tu_password";
#define LIGHT_PIN -1  // Sin sensor de luz
```

**3. Vincular:**
- Subir código al esclavo → Anotar MAC del Serial Monitor
- Editar maestro.ino con la MAC del esclavo
- Resubir maestro

---

## Especificaciones Técnicas

### Comunicación
- Protocolo: Mensajes texto "luz=%d,dist=%.1f"
- Frecuencia: 50 Hz (20ms)
- Latencia: 15-25ms
- Alcance: 30-50m interiores

### Control de Distancia

```
Zona muerta: 15-20cm (detenido)
Retroceso (<15cm): 80-180 PWM
Avance (>20cm): 100-255 PWM
Boost arranque: 210 PWM × 100ms
```

Ecuaciones:
```
Retroceso: v = 80 + (15-d) × 10
Avance: v = -(100 + (d-20) × 7.75)
```

### Seguridad
- Esclavo: parada inmediata si obstáculo <5cm
- Timeout comunicación: 1 segundo
- Validación lecturas sensores

### Control de Luces
- Sensor LM393 con histéresis de 5 segundos
- luz=1 (claro) → LEDs ON
- luz=0 (oscuro) → LEDs OFF

---

## Uso

### Despliegue
1. Configurar y subir código a ambos vehículos
2. Abrir Serial Monitor (115200 baud) para verificar conexión
3. Acceder a interfaz web (IP mostrada en Serial)
4. Colocar obstáculo frente al maestro para probar funcionamiento

### Monitorización
Interfaz web en `http://[IP_MAESTRO]`:
- Estado: PARADO/AVANZANDO/RETROCEDIENDO
- Distancia en tiempo real
- Estado de luces
- Actualización automática cada 500ms

---

## Resultados

### Métricas de Rendimiento
- Latencia total: 22-35ms
- Error de seguimiento: ±2cm
- Mensajes perdidos: <0.5%
- Tiempo detección emergencia: <5ms

### Limitaciones
- Alcance ESP-NOW: <50m interiores
- WiFi congestionado puede aumentar latencia
- Sensor ultrasónico impreciso en ángulos >30°
- Autonomía dependiente de capacidad de batería

---

## Configuración Avanzada

### Ajustar Zona Muerta
```cpp
miCoche.setRangoDistancia(15.0, 20.0);  // Por defecto
```

### Ajustar Velocidades
En `Coche.cpp`, función `controlarDistancia()`:
```cpp
velocidad = 80;   // Mín retroceso
velocidad = 180;  // Máx retroceso
velocidad = -100; // Mín avance
velocidad = -255; // Máx avance
```

### Ajustar Frecuencia Comunicación
En `Coche.cpp`, función `enviarComandoESPNow()`:
```cpp
if (millis() - ultimoEnvio < 20) return;  // 50 Hz (defecto)
```

---

## Conclusiones

Implementación exitosa de sistema de control distribuido con ESP-NOW, logrando:
- Latencia comparable a sistemas industriales (<50ms)
- Coste reducido (~25€/unidad)
- Robustez mediante replicación de lógica de control
- Interfaz web integrada para monitorización

### Aplicaciones Futuras
- Expansión a múltiples vehículos (mesh ESP-NOW)
- Navegación autónoma con GPS
- Integración de visión artificial (ESP32-CAM)

---

## Referencias

1. Espressif Systems. *ESP8266 Technical Reference Manual*. https://www.espressif.com/
2. Espressif Systems. *ESP-NOW User Guide*.
3. Arduino Reference. *ESP8266 Core Documentation*. https://arduino-esp8266.readthedocs.io/

---

**Autor:** Pablo Navarro  
**Asignatura:** Sistemas Embebidos 
**Fecha:** Diciembre 2025  

---

**Documentación completa:** Ver `examples/GUIA_RAPIDA.md`  

