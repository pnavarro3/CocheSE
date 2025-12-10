# 🚗 CocheSE - Biblioteca para Coches Robot ESP8266

## 📖 Descripción

Biblioteca Arduino para controlar coches robot basados en **ESP8266** con sensores y comunicación **ESP-NOW**. Incluye soporte para control automático de distancia, servidor web, y sistema de sincronización maestro/esclavo con alternancia dinámica.

## ✨ Características

- ✅ **Control de motores** con driver L9110S
- ✅ **Sensor ultrasónico** HC-SR04 para medición de distancia
- ✅ **Sensor de temperatura** LM35
- ✅ **Sensor de luz** LM393
- ✅ **Sensores compartidos** - Un solo sensor para ambos coches vía ESP-NOW
- ✅ **Control automático de luces** según luminosidad detectada
- ✅ **Control automático** de distancia con zona muerta
- ✅ **Servidor web** con interfaz responsive
- ✅ **Comunicación ESP-NOW** para sincronización entre coches
- ✅ **Modo dual maestro/esclavo** con alternancia desde web
- ✅ **Sincronización automática** bidireccional
- ✅ **Visualización de origen de datos** (📡 local / 📶 remoto / ❌ sin datos)

## 🔧 Hardware Requerido

### Por cada coche:
- 1x ESP8266 (LOLIN D1, ESP-WROOM-02, NodeMCU, etc.)
- 1x Driver de motores L9110S
- 2x Motores DC
- 1x Sensor ultrasónico HC-SR04
- 1x Sensor de temperatura LM35 (opcional, puede compartirse entre coches)
- 1x Sensor de luz LM393 (opcional, puede compartirse entre coches)
- LEDs para luces (opcional)
- 1x Divisor de voltaje para ECHO (5V → 3.3V)
- Fuente de alimentación (baterías)

### Conexiones ESP8266

```
Motor Izquierdo:
  Motor 1A → D1 (GPIO5)
  Motor 1B → D2 (GPIO4)

Motor Derecho:
  Motor 2A → D3 (GPIO0)
  Motor 2B → D4 (GPIO2)

HC-SR04:
  TRIG → D5 (GPIO14)
  ECHO → D6 (GPIO12) ⚠️ CON DIVISOR DE VOLTAJE

LM35:
  OUT → A0

LM393:
  DO → D7 (GPIO13)

LEDs/Luces (opcional):
  Ánodo → D8 (GPIO15) → Resistencia 220Ω
  Cátodo → GND
```

## 📦 Instalación

### Método 1: Arduino Library Manager (Recomendado)
1. Abrir Arduino IDE
2. Ir a **Sketch → Include Library → Manage Libraries**
3. Buscar "CocheSE"
4. Clic en **Install**

### Método 2: Manual
1. Descargar este repositorio como ZIP
2. En Arduino IDE: **Sketch → Include Library → Add .ZIP Library**
3. Seleccionar el archivo ZIP descargado

### Método 3: Git Clone
```bash
cd ~/Arduino/libraries/
git clone https://github.com/pnavarro3/CocheSE.git
```

## 🚀 Uso Rápido

### Sistema con ESP-NOW (2 coches sincronizados)

```cpp
#include "Coche.h"

// Configuración WiFi
const char* WIFI_SSID = "tu_wifi";        
const char* WIFI_PASSWORD = "tu_password"; 

// MAC del otro coche (obtener de Serial Monitor)
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x12, 0x34, 0x56};

// Este coche empieza como maestro (true) o esclavo (false)
bool EMPEZAR_COMO_MAESTRO = true;

// Definir pines
#define MOTOR1_A D1
#define MOTOR1_B D2
#define MOTOR2_A D3
#define MOTOR2_B D4
#define TRIG_PIN D5
#define ECHO_PIN D6

// SENSORES: Usa -1 si NO tiene sensores físicos (recibirá datos del otro coche)
#define TEMP_PIN A0     // o -1 si sin sensor
#define LIGHT_PIN D7    // o -1 si sin sensor
#define LUCES_PIN D8    // o -1 si sin LEDs

Coche miCoche(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B,
              TRIG_PIN, ECHO_PIN, TEMP_PIN, LIGHT_PIN, LUCES_PIN);

void setup() {
  Serial.begin(115200);
  
  miCoche.inicializar();
  miCoche.setRangoDistancia(18.0, 22.0);
  miCoche.inicializarESPNowDual(MAC_OTRO_COCHE, EMPEZAR_COMO_MAESTRO);
  miCoche.inicializarWiFi(WIFI_SSID, WIFI_PASSWORD);
  miCoche.inicializarServidorWeb();
}

void loop() {
  miCoche.atenderClientes();
  miCoche.controlarDistancia();
  miCoche.controlarLucesAutomaticas();  // Control automático de luces
  
  if (miCoche.obtenerModo() && miCoche.obtenerModoAutomatico()) {
    miCoche.enviarComandoESPNow();
  }
  
  delay(50);
}
```

## 📱 Interfaz Web

Una vez conectado al WiFi, abre la IP mostrada en el Serial Monitor:

```
┌─────────────────────────────────┐
│  🚗 Control Coche Robot         │
├─────────────────────────────────┤
│      👑 MAESTRO                 │
├─────────────────────────────────┤
│  [👑 Maestro] [🤖 Esclavo]      │  ← Cambiar modo
│  [🤖 AUTOMÁTICO]                │  ← Toggle auto/manual
├─────────────────────────────────┤
│  ⬆️ AVANZANDO                   │
│  📏 Distancia: 20.5 cm          │
│  🌡️ Temperatura: 24.3 °C 📡     │  ← Datos locales
│  💡 Luz: ☀️ Detectada 📶        │  ← Datos remotos
├─────────────────────────────────┤
│  💡 LUCES ENCENDIDAS            │
│  [🔄 Toggle] [🤖 Auto]          │  ← Control luces
└─────────────────────────────────┘

📡 = LOCAL   📶 = REMOTO   ❌ = SIN_DATOS
```

## 📚 Ejemplos

### `espnow_control`
Sistema completo con ESP-NOW para 2 coches que pueden alternar entre maestro/esclavo desde la interfaz web.

**Características:**
- Un solo código para ambos coches
- Alternancia maestro/esclavo desde web
- Sincronización automática bidireccional
- Control automático de distancia
- Modo automático/manual
- **Sensores compartidos** entre coches
- **Luces automáticas** según luminosidad

**Ver documentación completa:** [`examples/README.md`](examples/README.md)

**Guía rápida:** [`examples/GUIA_RAPIDA.md`](examples/GUIA_RAPIDA.md)

**Sensores compartidos:** [`examples/SENSORES_COMPARTIDOS.md`](examples/SENSORES_COMPARTIDOS.md)

**Control de luces:** [`examples/CONTROL_LUCES.md`](examples/CONTROL_LUCES.md)

## 🎯 Configuración en 3 Pasos

### 1️⃣ Primer Coche
```cpp
bool EMPEZAR_COMO_MAESTRO = true;
uint8_t MAC_OTRO_COCHE[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```
Subir → Serial Monitor → **Anotar MAC**

### 2️⃣ Segundo Coche
```cpp
bool EMPEZAR_COMO_MAESTRO = false;
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x11, 0x22, 0x33}; // MAC del primer coche

// RECOMENDADO: Si el primer coche tiene sensores, usa:
#define TEMP_PIN -1    // Recibirá datos del otro coche
#define LIGHT_PIN -1   // Recibirá datos del otro coche
```
Subir → Serial Monitor → **Anotar MAC**

> 💡 **Tip:** Solo necesitas conectar sensores de temperatura y luz en UN coche. El otro recibirá los datos automáticamente por ESP-NOW.

### 3️⃣ Actualizar Primer Coche
```cpp
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x44, 0x55, 0x66}; // MAC del segundo coche
```
Volver a subir

## 📖 API Reference

### Inicialización

```cpp
void inicializar();
void inicializarWiFi(const char* ssid, const char* password);
void inicializarServidorWeb();
void inicializarESPNowDual(uint8_t macOtroCoche[6], bool empezarComoMaestro);
```

### Control de Movimiento

```cpp
void avanzar(int velocidad);
void retroceder(int velocidad);
void girarIzquierda(int velocidad);
void girarDerecha(int velocidad);
void detener();
void controlarDistancia();
```

### Sensores

```cpp
float leerDistancia();
float leerTemperatura();
int leerLuz();
String obtenerEstadoMovimiento();

// Sensores compartidos
bool tieneSensores();
float obtenerTemperaturaActual();  // Local o remota
int obtenerLuminosidadActual();    // Local o remota
String obtenerOrigenDatos();       // "LOCAL", "REMOTO", "SIN_DATOS"
```

### Control de Luces

```cpp
void controlarLucesAutomaticas();
void encenderLuces();
void apagarLuces();
void toggleLuces();
void setLucesAutomaticas(bool automatico);
bool obtenerEstadoLuces();
bool obtenerLucesAutomaticas();
```

### Configuración

```cpp
void setDistanciaObjetivo(float distancia);
void setRangoDistancia(float minDist, float maxDist);
void setConstanteProporcional(float kp_value);
void setModoAutomatico(bool automatico);
```

### ESP-NOW

```cpp
void cambiarModo(bool nuevoModoMaestro);
void enviarComandoESPNow();
bool obtenerModo();
String obtenerModoTexto();
bool obtenerModoAutomatico();
```

### Servidor Web

```cpp
void atenderClientes();
String obtenerDatosJSON();
```

## 🔧 Parámetros Configurables

| Parámetro | Método | Valor Por Defecto | Descripción |
|-----------|--------|-------------------|-------------|
| Distancia mínima | `setRangoDistancia(min, max)` | 7.0 cm | Límite inferior zona muerta |
| Distancia máxima | `setRangoDistancia(min, max)` | 13.0 cm | Límite superior zona muerta |
| Constante proporcional | `setConstanteProporcional(kp)` | 8.0 | Suavidad del control |
| Modo automático | `setModoAutomatico(bool)` | true | Control automático activo |

## 🐛 Solución de Problemas

### El coche no se mueve
- Verifica las conexiones de los motores
- Comprueba la alimentación
- Revisa que los pines estén correctos

### ESP-NOW no funciona
- Verifica que las MACs sean correctas
- Asegúrate de que ambos ESP8266 estén encendidos
- Mantén los coches a menos de 50m

### El servidor web no responde
- Verifica la conexión WiFi
- Comprueba la IP en el Serial Monitor
- Refresca la página del navegador

### Sensor de distancia da lecturas incorrectas
- Verifica el divisor de voltaje en ECHO
- Asegúrate de que el sensor tenga alimentación
- Comprueba las conexiones TRIG y ECHO

## 📊 Especificaciones Técnicas

- **Plataforma:** ESP8266
- **Frecuencia de control:** 20 Hz (cada 50ms)
- **Latencia ESP-NOW:** < 10ms
- **Alcance ESP-NOW:** 50-200 metros
- **Servidor web:** Puerto 80
- **Baudrate Serial:** 115200

## 🎓 Casos de Uso

1. **Convoy sincronizado** - Múltiples coches siguiendo al líder
2. **Exploración colaborativa** - Alternancia de roles según el terreno
3. **Educación robótica** - Aprendizaje de comunicación inalámbrica
4. **Competencias** - Seguimiento de línea o evitación de obstáculos
5. **Demostración** - Sincronización en tiempo real

## 🤝 Contribuir

Las contribuciones son bienvenidas. Por favor:

1. Fork el repositorio
2. Crea una rama para tu feature (`git checkout -b feature/nueva-funcionalidad`)
3. Commit tus cambios (`git commit -m 'Agregar nueva funcionalidad'`)
4. Push a la rama (`git push origin feature/nueva-funcionalidad`)
5. Abre un Pull Request

## 📄 Licencia

Este proyecto está bajo licencia MIT. Ver archivo `LICENSE` para más detalles.

## 👨‍💻 Autor

**Pablo Navarro** - [pnavarro3](https://github.com/pnavarro3)

## 🙏 Agradecimientos

- Comunidad Arduino
- Espressif Systems (ESP8266)
- Contribuidores del proyecto

## 📞 Soporte

- 📧 Email: Contacto a través de GitHub
- 🐛 Issues: [GitHub Issues](https://github.com/pnavarro3/CocheSE/issues)
- 📖 Wiki: [Documentación completa](https://github.com/pnavarro3/CocheSE/wiki)

---

**¡Disfruta construyendo tu coche robot! 🎉**
