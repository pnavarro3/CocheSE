# 💻 Ejemplos Prácticos y Casos de Uso

## 📖 Código Completo Comentado

### Ejemplo Básico (2 coches sincronizados)

```cpp
#include "Coche.h"

// ========== CONFIGURACIÓN ==========
const char* WIFI_SSID = "MiWiFi";
const char* WIFI_PASSWORD = "MiPassword";

// MAC del otro coche (cambiar por la real)
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x12, 0x34, 0x56};

// true = empieza como maestro, false = esclavo
bool EMPEZAR_COMO_MAESTRO = true;

// ========== PINES ==========
#define MOTOR1_A D1
#define MOTOR1_B D2
#define MOTOR2_A D3
#define MOTOR2_B D4
#define TRIG_PIN D5
#define ECHO_PIN D6
#define TEMP_PIN A0   // o -1 si no tiene
#define LIGHT_PIN D7  // o -1 si no tiene
#define LUCES_PIN D8  // o -1 si no tiene

// Crear objeto coche
Coche miCoche(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B,
              TRIG_PIN, ECHO_PIN, TEMP_PIN, LIGHT_PIN, LUCES_PIN);

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== COCHE DUAL ESP-NOW ===");
  
  // 1. Inicializar hardware
  miCoche.inicializar();
  
  // 2. Configurar control de distancia
  miCoche.setRangoDistancia(18.0, 22.0);  // Zona muerta: 18-22 cm
  miCoche.setConstanteProporcional(8.0);  // Suavidad del control
  
  // 3. Inicializar ESP-NOW (ANTES que WiFi)
  miCoche.inicializarESPNowDual(MAC_OTRO_COCHE, EMPEZAR_COMO_MAESTRO);
  
  // 4. Conectar WiFi
  miCoche.inicializarWiFi(WIFI_SSID, WIFI_PASSWORD);
  
  // 5. Iniciar servidor web
  miCoche.inicializarServidorWeb();
  
  Serial.println("=== SISTEMA LISTO ===");
  delay(1000);
}

void loop() {
  // Atender peticiones web
  miCoche.atenderClientes();
  
  // Control automático de distancia (solo maestro en modo auto)
  miCoche.controlarDistancia();
  
  // Control automático de luces (ambos coches)
  miCoche.controlarLucesAutomaticas();
  
  // Enviar comandos ESP-NOW (solo maestro)
  if (miCoche.obtenerModo() && miCoche.obtenerModoAutomatico()) {
    miCoche.enviarComandoESPNow();
  }
  
  delay(50);  // 20 Hz
}
```

---

## 🎯 Caso 1: Convoy con 1 Sensor

**Escenario:** Ahorrar costos usando un solo juego de sensores.

### Coche Líder (con sensores):
```cpp
bool EMPEZAR_COMO_MAESTRO = true;
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x44, 0x55, 0x66};

#define TEMP_PIN A0      // ✅ Tiene sensor
#define LIGHT_PIN D7     // ✅ Tiene sensor
#define LUCES_PIN D8
```

### Coche Seguidor (sin sensores):
```cpp
bool EMPEZAR_COMO_MAESTRO = false;
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x11, 0x22, 0x33};

#define TEMP_PIN -1      // ❌ Sin sensor (recibe datos)
#define LIGHT_PIN -1     // ❌ Sin sensor (recibe datos)
#define LUCES_PIN D8     // ✅ Tiene LEDs
```

**Resultado:**
- Líder lee sensores → envía datos
- Seguidor recibe datos → controla luces
- Ambos muestran temperatura y luminosidad en web
- Ahorro: ~5-10€ en sensores

---

## 🎯 Caso 2: Alternancia de Liderazgo

**Escenario:** El coche que mejor ve el camino toma el control.

### Setup:
```cpp
// Ambos coches con sensores completos
#define TEMP_PIN A0
#define LIGHT_PIN D7
```

### Uso:
1. Coche A es maestro (controla)
2. Coche A encuentra obstáculo difícil
3. Desde navegador: Clic **[🤖 Esclavo]** en Coche A
4. Coche B automáticamente se vuelve maestro
5. Coche B busca ruta alternativa
6. Cuando esté libre: Cambiar de nuevo

**Ventaja:** Redundancia + flexibilidad

---

## 🎯 Caso 3: Luces Coordinadas

**Escenario:** Ambos coches encienden luces simultáneamente.

### Configuración:
```cpp
// Coche 1: Con sensor de luz
#define LIGHT_PIN D7
#define LUCES_PIN D8

// Coche 2: Sin sensor (recibe datos)
#define LIGHT_PIN -1
#define LUCES_PIN D8
```

### Comportamiento:
```
Coche 1: Lee LM393=0 (oscuro) → Enciende LEDs → Envía dato
Coche 2: Recibe luminosidad=0 → Enciende LEDs
         │
         └─> Ambos encienden a la vez (latencia <100ms)
```

**Casos de uso:**
- Túneles
- Garajes
- Anochecer

---

## 🎯 Caso 4: Modo Demostración

**Escenario:** Mostrar sincronización en presentaciones.

### Código adicional en loop():
```cpp
void loop() {
  miCoche.atenderClientes();
  miCoche.controlarDistancia();
  miCoche.controlarLucesAutomaticas();
  
  // Control manual de velocidades para demo
  if (!miCoche.obtenerModoAutomatico() && miCoche.obtenerModo()) {
    // Patrón de movimiento predefinido
    miCoche.avanzar(150);
    delay(2000);
    miCoche.girarIzquierda(120);
    delay(1000);
    miCoche.detener();
    delay(500);
  }
  
  if (miCoche.obtenerModo() && miCoche.obtenerModoAutomatico()) {
    miCoche.enviarComandoESPNow();
  }
  
  delay(50);
}
```

---

## 🔧 Snippets Útiles

### 1. Verificar origen de datos

```cpp
void loop() {
  miCoche.atenderClientes();
  
  // Verificar cada 2 segundos
  static unsigned long ultimoCheck = 0;
  if (millis() - ultimoCheck > 2000) {
    String origen = miCoche.obtenerOrigenDatos();
    Serial.print("Datos de sensores: ");
    Serial.println(origen);  // "LOCAL", "REMOTO", "SIN_DATOS"
    ultimoCheck = millis();
  }
  
  // ... resto del loop
}
```

### 2. Control manual de luces

```cpp
void setup() {
  // ... inicialización normal
  
  // Desactivar luces automáticas
  miCoche.setLucesAutomaticas(false);
}

void loop() {
  miCoche.atenderClientes();
  
  // Control manual con condición personalizada
  if (miCoche.obtenerTemperaturaActual() > 30.0) {
    miCoche.encenderLuces();  // Advertencia de sobrecalentamiento
  } else {
    miCoche.apagarLuces();
  }
  
  // ... resto del loop
}
```

### 3. Ajuste dinámico de distancia

```cpp
void loop() {
  miCoche.atenderClientes();
  
  // Ajustar rango según temperatura
  float temp = miCoche.obtenerTemperaturaActual();
  if (temp > 35.0) {
    // Mayor distancia en calor (precaución)
    miCoche.setRangoDistancia(25.0, 30.0);
  } else {
    // Distancia normal
    miCoche.setRangoDistancia(18.0, 22.0);
  }
  
  miCoche.controlarDistancia();
  // ... resto del loop
}
```

### 4. Logging de eventos

```cpp
void loop() {
  miCoche.atenderClientes();
  miCoche.controlarDistancia();
  
  // Log de cambios de estado
  static String estadoAnterior = "";
  String estadoActual = miCoche.obtenerEstadoMovimiento();
  
  if (estadoActual != estadoAnterior) {
    Serial.print("[");
    Serial.print(millis());
    Serial.print("] Estado: ");
    Serial.print(estadoAnterior);
    Serial.print(" → ");
    Serial.println(estadoActual);
    estadoAnterior = estadoActual;
  }
  
  // ... resto del loop
}
```

---

## 🐛 Troubleshooting Avanzado

### Problema: Datos remotos intermitentes

**Síntoma:** Aparece y desaparece 📶/❌

**Diagnóstico:**
```cpp
void loop() {
  // ... código normal
  
  // Debug ESP-NOW
  static int mensajesRecibidos = 0;
  static unsigned long ultimoMensaje = 0;
  
  // En procesarComandoRecibido() (modificar Coche.cpp):
  if (datos->tieneSensores) {
    mensajesRecibidos++;
    ultimoMensaje = millis();
    Serial.print("Mensaje #");
    Serial.print(mensajesRecibidos);
    Serial.print(" - Temp: ");
    Serial.println(datos->temperatura);
  }
}
```

**Solución:**
1. Reducir distancia entre coches (< 20m)
2. Verificar que no haya paredes metálicas
3. Aumentar frecuencia de envío (reducir delay en loop)

---

### Problema: Luces no reaccionan

**Diagnóstico:**
```cpp
void setup() {
  // ... inicialización
  
  Serial.print("Tiene sensores locales: ");
  Serial.println(miCoche.tieneSensores() ? "SI" : "NO");
  
  Serial.print("Luces disponibles: ");
  Serial.println(pinLuces >= 0 ? "SI" : "NO");
}

void loop() {
  // ... código normal
  
  // Debug luces
  static unsigned long ultimoDebug = 0;
  if (millis() - ultimoDebug > 1000) {
    int luz = miCoche.obtenerLuminosidadActual();
    bool lucesOn = miCoche.obtenerEstadoLuces();
    bool lucesAuto = miCoche.obtenerLucesAutomaticas();
    
    Serial.print("Luz: ");
    Serial.print(luz);
    Serial.print(" | Luces: ");
    Serial.print(lucesOn ? "ON" : "OFF");
    Serial.print(" | Auto: ");
    Serial.println(lucesAuto ? "SI" : "NO");
    
    ultimoDebug = millis();
  }
}
```

---

### Problema: Consumo excesivo de batería

**Solución:** Optimizar delays y lecturas

```cpp
void loop() {
  miCoche.atenderClientes();
  
  // Leer sensores solo cada 200ms (no cada loop)
  static unsigned long ultimaLectura = 0;
  if (millis() - ultimaLectura > 200) {
    miCoche.controlarDistancia();
    miCoche.controlarLucesAutomaticas();
    ultimaLectura = millis();
  }
  
  // Enviar ESP-NOW solo si hay cambios
  static int velIzqAnterior = 0;
  static int velDerAnterior = 0;
  
  if (miCoche.obtenerModo() && miCoche.obtenerModoAutomatico()) {
    // Solo enviar si cambió la velocidad
    // (implementar getter de velocidades en Coche.h)
    miCoche.enviarComandoESPNow();
  }
  
  delay(50);  // Mantener 20 Hz
}
```

---

## 📊 Métricas de Rendimiento

### Latencias típicas:
```
ESP-NOW envío: 5-15 ms (con ACK)
ESP-NOW bidireccional: 10-30 ms (maestro → esclavo → respuesta)
WiFi HTTP: 50-100 ms
Sensor HC-SR04: 60 ms (incluye delay interno)
Loop completo: ~50-100 ms (10-20 Hz)
Throttling ESP-NOW: 100 ms mínimo entre mensajes
Rate efectivo: 5-10 mensajes/segundo
```

### Consumo:
```
ESP8266 activo: ~80 mA
L9110S (parado): ~10 mA
L9110S (moviendo): 200-500 mA por motor
LEDs: ~20 mA por LED
Total típico: 300-1200 mA dependiendo del uso
```

### Fiabilidad ESP-NOW:
```
✅ Tasa de éxito normal: 95-100%
⚠️ Aceptable: 90-95% (errores ocasionales)
❌ Problema: < 90% (revisar distancia/alimentación)

Factores que afectan:
- Distancia: Óptimo < 20m, máximo ~50m
- Obstáculos: Paredes reducen alcance
- Interferencias: Otros dispositivos WiFi
- Alimentación: Batería baja reduce potencia TX
```

### Control de flujo:
```
Bloqueo por ACK: Garantiza 1 mensaje en vuelo
Throttling 100ms: Evita congestión
Timeout implícito: ESP-NOW tiene ACK de hardware
Recovery: Automática al recibir ACK o fallo
```

---

## 🐛 Debugging Avanzado

### Análisis de log Serial (115200 baud):

```bash
# Mensajes normales (95%+ éxito)
5.234s ENVIO: AVANZANDO V:200,200 T:23.5 L:1
5.347s ENVIO: AVANZANDO V:200,200 T:23.5 L:1
5.456s ENVIO: AVANZANDO V:200,200 T:23.5 L:1
# Rate: ~10 msg/s, sin errores

# Errores ocasionales (aceptable)
10.123s ERROR: Envío fallido
10.234s ENVIO: AVANZANDO V:200,200
# 1 error cada 20-30 mensajes: OK

# Problema crítico (revisar)
15.123s ERROR: Envío fallido
15.234s ERROR: Envío fallido
15.345s ERROR: Envío fallido
15.456s ERROR: Envío fallido
# Muchos errores consecutivos: distancia/batería

# Cambios de modo (esperados)
20.567s MODO: MAESTRO → ESCLAVO
20.580s Recibido cambio de modo a: ESCLAVO
# Sincronización automática

# Comunicación bidireccional
25.123s ENVIO: AVANZANDO V:200,200 T:23.5 L:1
25.136s RECEP: AVANZANDO V:200,200 T:23.5 L:1
25.145s RESP: T:24.2 L:0
25.158s RESP_RX: ESCLAVO T:24.2 L:0
# Maestro envía, esclavo responde con sensores
```

### Verificar tasa de éxito:

```cpp
void loop() {
  miCoche.atenderClientes();
  
  // Log cada 10 segundos
  static unsigned long ultimoLog = 0;
  if (millis() - ultimoLog > 10000) {
    Serial.print("Enviados: ");
    Serial.print(miCoche.obtenerMensajesEnviados());
    Serial.print(" | Fallidos: ");
    Serial.print(miCoche.obtenerMensajesFallidos());
    Serial.print(" | Éxito: ");
    Serial.print(miCoche.obtenerTasaExito(), 1);
    Serial.println("%");
    ultimoLog = millis();
  }
  
  // ... resto del código
}
```

---

## 🎓 Mejores Prácticas

1. **Siempre usar divisor de voltaje en ECHO** (HC-SR04 da 5V, ESP8266 tolera 3.3V)
2. **Configurar sensores con -1 si no existen** (detección automática)
3. **Mantener loop() rápido** (<100ms por iteración)
4. **No usar Serial.print() excesivo** (ralentiza y afecta ESP-NOW)
5. **Validar datos remotos antes de usarlos** (sistema lo hace automático)
6. **Probar con Serial Monitor primero** antes de confiar en web (ver log ESP-NOW)
7. **Alimentar motores con batería separada** del ESP8266
8. **Monitorear tasa de éxito** - debe ser >95% en condiciones normales
9. **Mantener distancia <20m** para comunicación óptima
10. **Evitar obstáculos metálicos** entre coches (reducen señal)

---

## 📚 Referencias Adicionales

- [README.md](README.md) - Documentación completa con detalles de control de flujo
- [GUIA_RAPIDA.md](GUIA_RAPIDA.md) - Quick start

---

**¿Más preguntas? Abre un issue en GitHub** 🚀
