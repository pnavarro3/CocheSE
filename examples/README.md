# 🚗 CocheSE - Sistema Dual Maestro/Esclavo con ESP-NOW

## 📖 Descripción

Sistema de control para **2 coches robot ESP8266** que se sincronizan mediante **ESP-NOW**. Los coches pueden alternar entre maestro/esclavo desde la interfaz web, compartir sensores de temperatura y luz, y controlar luces automáticamente.

## ✨ Características

- ✅ **Alternancia maestro/esclavo** desde navegador web
- ✅ **Sincronización automática** bidireccional
- ✅ **Sensores compartidos** - Solo necesitas conectar temperatura/luz en un coche
- ✅ **Luces automáticas** según luminosidad (LM393)
- ✅ **Control de distancia** automático con HC-SR04
- ✅ **Mismo código** para ambos coches
- ✅ **Interfaz web** responsive con actualización en tiempo real

## 🔧 Hardware

### Por cada coche:
- ESP8266 (LOLIN D1, ESP-WROOM-02, NodeMCU)
- Driver L9110S + 2 motores DC
- Sensor HC-SR04 (distancia)
- **Opcional:** Sensor LM35 (temperatura) - puede estar solo en 1 coche
- **Opcional:** Sensor LM393 (luz) - puede estar solo en 1 coche
- **Opcional:** LEDs para luces
- Divisor de voltaje 5V→3.3V para ECHO
- Baterías

### Conexiones (LOLIN D1)

```
Motores (L9110S):
  Izq:  Motor1A→D1 (GPIO5)  Motor1B→D2 (GPIO4)
  Der:  Motor2A→D3 (GPIO0)  Motor2B→D4 (GPIO2)

Sensores:
  HC-SR04:  TRIG→D5 (GPIO14)  ECHO→D6 (GPIO12) ⚠️ con divisor
  LM35:     OUT→A0
  LM393:    DO→D7 (GPIO13)
  LEDs:     Ánodo→D8 (GPIO15)→220Ω  Cátodo→GND
```

## 🚀 Configuración en 3 Pasos

### 1️⃣ Primer Coche

```cpp
// En espnow_control.ino
bool EMPEZAR_COMO_MAESTRO = true;
uint8_t MAC_OTRO_COCHE[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Si TIENE sensores:
#define TEMP_PIN A0
#define LIGHT_PIN D7
#define LUCES_PIN D8

// Si NO tiene sensores (recibirá datos del otro):
// #define TEMP_PIN -1
// #define LIGHT_PIN -1
// #define LUCES_PIN D8
```

**➡️ Subir código → Serial Monitor → Anotar MAC** (ej: `5C:CF:7F:11:22:33`)

---

### 2️⃣ Segundo Coche

```cpp
bool EMPEZAR_COMO_MAESTRO = false;
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x11, 0x22, 0x33}; // MAC del primer coche

// RECOMENDADO si el otro tiene sensores:
#define TEMP_PIN -1     // Recibirá datos remotos
#define LIGHT_PIN -1    // Recibirá datos remotos
#define LUCES_PIN D8    // Puede tener LEDs propios
```

**➡️ Subir código → Serial Monitor → Anotar MAC** (ej: `5C:CF:7F:44:55:66`)

---

### 3️⃣ Actualizar Primer Coche

```cpp
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x44, 0x55, 0x66}; // MAC del segundo
```

**➡️ Volver a subir**

---

## 📱 Interfaz Web

Abre la IP mostrada en Serial Monitor:

```
┌──────────────────────────────┐
│  🚗 Control Coche Robot      │
│      👑 MAESTRO              │
│  [👑 Maestro] [🤖 Esclavo]   │
│  [🤖 AUTOMÁTICO]             │
├──────────────────────────────┤
│  ⬆️ AVANZANDO                │
│  📏 Distancia: 20.5 cm       │
│  🌡️ Temp: 24.3°C 📡         │  📡=local 📶=remoto
│  💡 Luz: ☀️ 📶              │
│  💡 LUCES ON [🔄][🤖 Auto]  │
└──────────────────────────────┘
```

## 💡 Sensores Compartidos

### ¿Cómo funciona?

Solo necesitas conectar sensores de **temperatura y luz en UN coche**. El otro recibirá los datos automáticamente por ESP-NOW.

**Configuración recomendada:**
- **Coche 1:** `TEMP_PIN = A0`, `LIGHT_PIN = D7` → Lee sensores
- **Coche 2:** `TEMP_PIN = -1`, `LIGHT_PIN = -1` → Recibe datos

**Ventajas:**
- ✅ Ahorro de costos (1 solo juego de sensores)
- ✅ Menos cableado
- ✅ Ambos coches tienen datos de temperatura y luz
- ✅ Las luces funcionan en ambos coches

**Detección automática:**
El sistema detecta automáticamente si tiene sensores evaluando: `(TEMP_PIN >= 0 && LIGHT_PIN >= 0)`

**Validación:**
Los datos remotos son válidos durante 5 segundos. Si no llegan nuevos datos, el sistema marca como ❌ SIN_DATOS.

### Opciones de configuración:

| Configuración | Coche 1 | Coche 2 | Resultado |
|---------------|---------|---------|-----------|
| **A (recomendado)** | Sensores: ✅ | Sensores: ❌ | C1 envía → C2 recibe |
| **B (redundante)** | Sensores: ✅ | Sensores: ✅ | Cada uno usa los suyos |
| **C (sin sensores)** | Sensores: ❌ | Sensores: ❌ | Sin datos (solo luces manuales) |

## 🔦 Control de Luces

### Automático (por defecto)

Las luces se encienden/apagan automáticamente según el sensor LM393:
- **Oscuro (LM393=0)** → Luces ON
- **Luz (LM393=1)** → Luces OFF

Funciona con datos locales o remotos.

### Manual (desde web)

1. **Toggle:** `/luces/toggle` - Encender/apagar manualmente
2. **Auto/Manual:** `/luces/auto` - Cambiar entre modo automático y manual

## 📊 Comunicación ESP-NOW

### Control de Flujo Fiable

El sistema implementa **control de flujo estricto** para garantizar comunicación fiable:

✅ **Bloqueo por ACK** - No se envía otro mensaje hasta recibir confirmación  
✅ **Throttling 100ms** - Mínimo 100 milisegundos entre mensajes  
✅ **Máximo 10 msg/s** - Tasa controlada para evitar congestión  
✅ **Detección de fallos** - Registra mensajes perdidos en Serial

**Ventajas:**
- Evita pérdida de mensajes
- Sincronización precisa
- Rendimiento estable
- Fácil depuración via Serial Monitor

### Estructura de mensajes:

```cpp
struct_mensaje {
    int velocidadIzq;     // -255 a 255
    int velocidadDer;     // -255 a 255
    char comando[20];     // "AVANZANDO", "RETROCEDIENDO", "PARADO"
    float temperatura;    // Temperatura LM35 (°C)
    int luminosidad;      // Luminosidad LM393 (0=oscuro, 1=luz)
    bool tieneSensores;   // true si tiene sensores físicos
}

struct_control {
    char tipoComando[20];  // "CAMBIAR_MODO"
    char nuevoModo[20];    // "MAESTRO" o "ESCLAVO"
}

struct_respuesta {
    float temperatura;     // Temperatura del esclavo
    int luminosidad;       // Luminosidad del esclavo
    bool tieneSensores;    // Si el esclavo tiene sensores
    char origen[20];       // "ESCLAVO"
}
```

### Flujo bidireccional:

```
MAESTRO                          ESCLAVO
   │                                │
   │  Lee sensores (temp, luz)      │
   │  Calcula velocidades           │
   │  Mueve motores                 │
   │                                │
   ├──────────────────────────────>│
   │  {velocidades, sensores}       │  Espera ACK (bloqueado)
   │                                │
   │<───────────────────────────────┤
   │          ACK recibido           │  Recibe comando
   │  (libera bloqueo)              │  Mueve motores
   │                                │  Almacena sensores
   │                                │
   │<───────────────────────────────┤
   │  {sensores del esclavo}        │  Envía respuesta (si tiene sensores)
   │                                │  Espera ACK
   ├──────────────────────────────>│
   │          ACK                    │  (libera bloqueo)
   │                                │
   │  Almacena sensores remotos     │
   │  Controla luces                │  Controla luces
   │                                │
   │  Espera 100ms (throttling)     │  Espera 100ms
   │                                │
   └─ Puede enviar nuevo mensaje    └─ Puede enviar respuesta
```

### Log en Serial Monitor (115200 baud):

```
5.234s ENVIO: AVANZANDO V:200,200 T:23.5 L:1
5.247s ✓ ACK recibido
5.250s RECEP: AVANZANDO V:200,200 T:23.5 L:1
5.265s RESP: T:24.2 L:0
10.123s MODO: MAESTRO → ESCLAVO
12.456s ERROR: Envío fallido
```

**Tipos de log:**
- `ENVIO` - Maestro envía comando
- `RECEP` - Esclavo recibe comando
- `RESP` - Esclavo envía sus sensores
- `RESP_RX` - Maestro recibe sensores del esclavo
- `MODO` - Cambio maestro ↔ esclavo
- `ERROR` - Mensaje sin ACK

### Implementación técnica:

```cpp
// Variables de control
bool esperandoACK;         // Flag de bloqueo
unsigned long ultimoEnvio; // Timestamp para throttling

// Verificación antes de enviar
bool puedeEnviar() {
    if (esperandoACK) return false;           // Esperando confirmación
    if (millis() - ultimoEnvio < 100) return false;  // Throttling 100ms
    return true;
}

// Al enviar
if (!puedeEnviar()) return;
esperandoACK = true;
ultimoEnvio = millis();
esp_now_send(...);

// Callback ACK
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    esperandoACK = false;  // Liberar bloqueo
    if (sendStatus != 0) {
        mensajesFallidos++;
        Serial.println("✗ FALLO");
    }
}
```

## 🎮 Modos de Operación

| Modo | Función | Comportamiento |
|------|---------|----------------|
| **👑 Maestro** | Controla | Lee distancia → calcula velocidades → envía comandos |
| **🤖 Esclavo** | Replica | Recibe comandos → mueve motores igual al maestro |
| **🤖 Automático** | Control activo | Maestro usa sensor HC-SR04 automáticamente |
| **🎮 Manual** | Control inactivo | Maestro no usa sensores (preparado para web) |

## 🔧 API Web

| Endpoint | Descripción |
|----------|-------------|
| `/` | Interfaz principal HTML |
| `/datos` | JSON con estado completo |
| `/modo?maestro=0/1` | Cambiar a maestro (1) o esclavo (0) |
| `/automatico` | Toggle control automático/manual |
| `/luces/toggle` | Encender/apagar luces |
| `/luces/auto` | Toggle luces automáticas/manuales |

### JSON de respuesta:

```json
{
  "distancia": 20.5,
  "temperatura": 24.3,
  "luz": 0,
  "estado": "AVANZANDO",
  "modo": "MAESTRO",
  "automatico": true,
  "lucesDisponibles": true,
  "lucesEncendidas": true,
  "lucesAutomaticas": true,
  "tieneSensores": true,
  "origenDatos": "LOCAL",
  "sensorUltrasonico": true,
  "sensorTemperatura": true,
  "sensorLuminosidad": true,
  "sensorLuces": true,
  "mensajesEnviados": 125,
  "mensajesRecibidos": 123,
  "mensajesFallidos": 2,
  "tasaExito": 98.4
}
```

**Campos adicionales para monitoreo:**
- `mensajesEnviados` - Total de mensajes enviados exitosamente
- `mensajesRecibidos` - Total de mensajes recibidos
- `mensajesFallidos` - Mensajes sin ACK
- `tasaExito` - Porcentaje de mensajes exitosos (100% ideal)

## 🐛 Troubleshooting

| Problema | Solución |
|----------|----------|
| No cambia de modo | Verifica MACs en Serial Monitor |
| No replica movimientos | Confirma que uno es maestro y otro esclavo |
| Muestra ❌ SIN_DATOS | Verifica que al menos un coche tenga sensores >= 0 |
| Luces no funcionan | Define `LUCES_PIN` ≥ 0 y conecta LEDs |
| Datos remotos no llegan | Mantén coches < 50m, verifica ESP-NOW inicializado |
| Mensajes perdidos (ERROR en Serial) | Normal si < 5%, verifica distancia y obstáculos |
| Alta tasa de fallos (> 10%) | Reduce distancia, revisa alimentación, elimina interferencias WiFi |
| Movimientos entrecortados | Verifica Serial Monitor: debe mostrar ~5-10 msg/s |

### Depuración por Serial Monitor

Conecta a **115200 baud** y observa:

```
✅ Funcionamiento normal:
5.234s ENVIO: AVANZANDO V:200,200
5.347s ENVIO: AVANZANDO V:200,200
5.456s ENVIO: AVANZANDO V:200,200
(~8-10 mensajes/segundo, sin errores)

⚠️ Advertencia (aceptable < 5%):
10.123s ERROR: Envío fallido
10.234s ENVIO: AVANZANDO V:200,200
(errores ocasionales, recuperación automática)

❌ Problema grave (> 10% fallos):
15.123s ERROR: Envío fallido
15.234s ERROR: Envío fallido
15.345s ERROR: Envío fallido
(muchos errores consecutivos → revisar hardware)
```

## 📝 Casos de Uso

1. **Ahorro de sensores** - Conecta temp/luz solo en el coche líder
2. **Convoy sincronizado** - El trasero replica al delantero perfectamente
3. **Alternancia de liderazgo** - Cambia líder según obstáculos
4. **Luces coordinadas** - Ambos coches encienden luces a la vez

## 📄 Archivos

```
examples/
├── espnow_control/
│   ├── espnow_control.ino  ← Código principal
│   ├── Coche.h
│   └── Coche.cpp
├── README.md               ← Este archivo
├── GUIA_RAPIDA.md          ← Quick start
└── EJEMPLOS.md             ← Casos prácticos
```

## 🎓 Referencia Rápida

**Configurar sensores:**
- Con sensores: `#define TEMP_PIN A0` y `#define LIGHT_PIN D7`
- Sin sensores: `#define TEMP_PIN -1` y `#define LIGHT_PIN -1`

**Habilitar/deshabilitar luces:**
- Con LEDs: `#define LUCES_PIN D8`
- Sin LEDs: `#define LUCES_PIN -1`

**Inicialización en setup():**
```cpp
miCoche.inicializar();
miCoche.inicializarESPNowDual(MAC_OTRO_COCHE, EMPEZAR_COMO_MAESTRO);
miCoche.inicializarWiFi(SSID, PASSWORD);
miCoche.inicializarServidorWeb();
```

**Loop básico:**
```cpp
miCoche.atenderClientes();
miCoche.controlarDistancia();
miCoche.controlarLucesAutomaticas();
if (miCoche.obtenerModo() && miCoche.obtenerModoAutomatico()) {
    miCoche.enviarComandoESPNow();
}
```

---

**Ver también:**
- [Guía Rápida](GUIA_RAPIDA.md) - Configuración en 5 minutos
- [Ejemplos Prácticos](EJEMPLOS.md) - Casos de uso detallados

---

**¡Listo para usar! 🎉**
