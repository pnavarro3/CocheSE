# Guía de Configuración Rápida

Sistema de Vehículos Robot ESP8266 con ESP-NOW

---

## Prerrequisitos

- Arduino IDE instalado (versión 1.8.19 o superior)
- ESP8266 Core instalado
- 2x ESP8266 LOLIN D1 ESP-WROOM-02 con hardware conectado
- Acceso a red WiFi 2.4GHz
- Cable USB para programación

---

## Paso 1: Configurar el Vehículo Maestro

### 1.1 Abrir el sketch
```
Archivo → Abrir → examples/maestro/maestro.ino
```

### 1.2 Configurar WiFi
```cpp
const char* WIFI_SSID = "nombre_de_tu_red";
const char* WIFI_PASSWORD = "tu_contraseña";
```

### 1.3 Dejar MAC temporal
```cpp
uint8_t MAC_ESCLAVO[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

### 1.4 Verificar pines
```cpp
#define MOTOR1_A D1  // GPIO5
#define MOTOR1_B D2  // GPIO4  
#define MOTOR2_A D3  // GPIO0
#define MOTOR2_B D4  // GPIO2
#define TRIG_PIN D5  // GPIO14
#define ECHO_PIN D6  // GPIO12 (con divisor de voltaje)
#define LIGHT_PIN D7 // GPIO13
#define LUCES_PIN D8 // GPIO15
```

### 1.5 Compilar y subir
```
Herramientas → Placa → ESP8266 Boards → LOLIN(WEMOS) D1 R2 & mini
Herramientas → Puerto → [Seleccionar puerto COM]
Sketch → Subir
```

### 1.6 Abrir Serial Monitor
```
Herramientas → Monitor Serie (115200 baud)

Salida esperada:
COCHE MAESTRO
Conectando a WiFi...
WiFi conectado
Dirección IP: 192.168.1.100
ESP-NOW inicializado como MAESTRO
SISTEMA LISTO
```

**ANOTAR:** Dirección IP mostrada

---

## Paso 2: Configurar el Vehículo Esclavo

### 2.1 Abrir el sketch
```
Archivo → Abrir → examples/esclavo/esclavo.ino
```

### 2.2 Configurar WiFi
```cpp
const char* WIFI_SSID = "nombre_de_tu_red";        // Misma red
const char* WIFI_PASSWORD = "tu_contraseña";       // Misma contraseña
```

### 2.3 Configurar pines
```cpp
#define MOTOR1_A D1  // GPIO5
#define MOTOR1_B D2  // GPIO4
#define MOTOR2_A D3  // GPIO0
#define MOTOR2_B D4  // GPIO2
#define TRIG_PIN D5  // GPIO14 - Sensor de seguridad
#define ECHO_PIN D6  // GPIO12 (con divisor de voltaje)
#define LIGHT_PIN -1 // Sin sensor de luz
#define LUCES_PIN D8 // GPIO15
```

Nota: El esclavo no necesita sensor de luz, recibirá datos del maestro.

### 2.4 Compilar y subir
```
Herramientas → Puerto → [Puerto COM del esclavo]
Sketch → Subir
```

### 2.5 Obtener MAC del esclavo
```
Monitor Serie (115200 baud):

COCHE ESCLAVO
Conectando a WiFi...
WiFi conectado
Dirección IP: 192.168.1.101
ESP-NOW inicializado como ESCLAVO
Mi dirección MAC: 5C:CF:7F:EC:8F:EE    <- COPIAR ESTA MAC
Esperando comandos...
```

**ANOTAR:** Dirección MAC completa (formato XX:XX:XX:XX:XX:XX)

---

## Paso 3: Vincular Maestro con Esclavo

### 3.1 Editar código del maestro
Volver a abrir maestro.ino y modificar:

```cpp
// Cambiar:
uint8_t MAC_ESCLAVO[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Por la MAC del esclavo (ejemplo):
uint8_t MAC_ESCLAVO[] = {0x5C, 0xCF, 0x7F, 0xEC, 0x8F, 0xEE};
```

IMPORTANTE: Respetar el formato con 0x antes de cada par.

### 3.2 Recompilar y subir
```
Sketch → Subir
```

### 3.3 Verificar vinculación
```
Monitor Serie del maestro:

ESP-NOW inicializado como MAESTRO
Peer añadido: 5C:CF:7F:EC:8F:EE
```

---

## Verificación del Sistema

### Test 1: Comunicación
Abrir Monitor Serie de ambos vehículos. Deben mostrar actividad de envío/recepción cada ~20ms.

### Test 2: Control de motores
1. Colocar obstáculo a 25cm del sensor del maestro
2. Ambos vehículos avanzan
3. Acercar a 17cm → ambos se detienen
4. Acercar a 10cm → ambos retroceden

### Test 3: Seguridad del esclavo
1. Acercar objeto a menos de 5cm del sensor del esclavo
2. El esclavo se detiene inmediatamente
3. Alejar el objeto → el esclavo se reactiva

### Test 4: Interfaz web
1. Abrir navegador
2. Ir a: http://192.168.1.100 (IP del maestro)
3. Verificar visualización de datos en tiempo real

### Test 5: Control de luces
1. Tapar sensor de luz → LEDs se encienden (después de 5 segundos)
2. Destapar sensor → LEDs se apagan (después de 5 segundos)

---

## Solución de Problemas Comunes

### No conecta a WiFi
- Verificar SSID y contraseña correctos
- Confirmar que router usa 2.4GHz (no 5GHz)
- Acercar ESP8266 al router

### No se ve la MAC en Serial Monitor
- Verificar velocidad: 115200 baud
- Presionar botón RESET en el ESP8266
- Revisar que el código se subió sin errores

### Esclavo no recibe comandos
- Verificar MAC correctamente copiada en maestro.ino
- Confirmar ambos en la misma red WiFi
- Reducir distancia entre vehículos (prueba a menos de 5m)

### Motores no se mueven
- Verificar alimentación independiente de motores
- Confirmar conexiones del driver L9110S
- Probar motores con conexión directa a batería

### Sensor HC-SR04 da lecturas erráticas
- CRÍTICO: Verificar divisor de voltaje en ECHO (resistencias 1kΩ + 2kΩ)
- Verificar cableado: VCC→5V, GND→GND, TRIG→D5, ECHO→Divisor→D6
- Asegurar sensor firmemente montado

### Luces no responden
- Verificar LUCES_PIN configurado como D8
- Confirmar conexión LEDs: Ánodo→D8→220Ω, Cátodo→GND
- Recordar retardo de 5 segundos (anti-parpadeo)

---

## Parámetros Configurables

### Zona muerta de control
Ubicación: maestro.ino y esclavo.ino, función setup()
```cpp
miCoche.setRangoDistancia(15.0, 20.0);  // Por defecto
miCoche.setRangoDistancia(12.0, 18.0);  // Zona más amplia
```

### Velocidades PWM
Ubicación: Coche.cpp, función controlarDistancia()
```cpp
velocidad = 80;   // Mínima retroceso (a 15cm)
velocidad = 180;  // Máxima retroceso (a 5cm)
velocidad = -100; // Mínima avance (a 20cm)
velocidad = -255; // Máxima avance (a 40cm)
```

### Frecuencia de comunicación
Ubicación: Coche.cpp, función enviarComandoESPNow()
```cpp
if (millis() - ultimoEnvio < 20) return;  // 50 msg/s (defecto)
if (millis() - ultimoEnvio < 10) return;  // 100 msg/s (más rápido)
if (millis() - ultimoEnvio < 50) return;  // 20 msg/s (ahorro energía)
```

### Boost de arranque
Ubicación: Coche.cpp, función moverMotores()
```cpp
int velocidadReal = boostIzq ? 210 : velocidadIzq;  // Intensidad
delay(100);  // Duración en milisegundos
```

### Distancia de seguridad
Ubicación: Coche.cpp, función ejecutarComandoRecibido()
```cpp
if (distanciaPropia > 2 && distanciaPropia < 5.0) {  // 5cm (defecto)
if (distanciaPropia > 2 && distanciaPropia < 8.0) {  // 8cm (más seguro)
```

### Anti-parpadeo de luces
Ubicación: Coche.cpp, función controlarLucesAutomaticas()
```cpp
if (millis() - ultimoCambioLuces < 5000) return;  // 5 segundos (defecto)
if (millis() - ultimoCambioLuces < 2000) return;  // 2 segundos (más rápido)
```

---

## Métricas de Rendimiento Esperadas

- Latencia maestro a esclavo: 20-30ms
- Frecuencia de actualización: 50 mensajes/segundo
- Precisión de seguimiento: ±2cm
- Tiempo de respuesta seguridad: menos de 5ms
- Tasa de pérdida de mensajes: menos de 1%

---

Tiempo estimado de configuración: 10-15 minutos

Versión: 2.0  
Diciembre 2025
