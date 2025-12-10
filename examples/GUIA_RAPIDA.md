# ⚡ Guía Rápida - 5 Minutos para Arrancar

## 📋 Requisitos Previos

- Arduino IDE instalado
- Biblioteca CocheSE instalada
- 2x ESP8266 con hardware conectado
- WiFi disponible

---

## 🚀 Paso 1: Primer Coche (2 min)

### Código:
```cpp
// espnow_control.ino
bool EMPEZAR_COMO_MAESTRO = true;
uint8_t MAC_OTRO_COCHE[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Con sensores:
#define TEMP_PIN A0
#define LIGHT_PIN D7
#define LUCES_PIN D8
```

### Acción:
1. Abrir `examples/espnow_control/espnow_control.ino`
2. Verificar configuración WiFi (SSID y password)
3. **Subir código**
4. Abrir Serial Monitor (115200)
5. **✍️ ANOTAR MAC:** `5C:CF:7F:11:22:33`

---

## 🚀 Paso 2: Segundo Coche (2 min)

### Código:
```cpp
bool EMPEZAR_COMO_MAESTRO = false;
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x11, 0x22, 0x33}; // ⬅️ MAC del primer coche

// Sin sensores (ahorro):
#define TEMP_PIN -1
#define LIGHT_PIN -1
#define LUCES_PIN D8
```

### Acción:
1. Editar archivo
2. Poner MAC del primer coche
3. **Subir código**
4. Abrir Serial Monitor
5. **✍️ ANOTAR MAC:** `5C:CF:7F:44:55:66`

---

## 🚀 Paso 3: Actualizar Primer Coche (1 min)

### Código:
```cpp
uint8_t MAC_OTRO_COCHE[] = {0x5C, 0xCF, 0x7F, 0x44, 0x55, 0x66}; // ⬅️ MAC del segundo coche
```

### Acción:
1. Volver al código del primer coche
2. Poner MAC del segundo coche
3. **Subir código nuevamente**

---

## ✅ ¡Listo! Probar

### En Serial Monitor:
```
ESP-NOW inicializado en modo DUAL
Mi MAC: 5C:CF:7F:11:22:33
Modo inicial: MAESTRO
WiFi conectado!
Dirección IP: 192.168.1.100
```

### En Navegador:
1. Abrir: `http://192.168.1.100` (IP del Serial Monitor)
2. Ver interfaz web con controles
3. Probar botones:
   - **[🤖 Esclavo]** → Cambia de maestro a esclavo
   - **[🤖 AUTOMÁTICO]** → Toggle control automático
   - **[🔄 Toggle]** → Encender/apagar luces

---

## 🎯 Prueba Rápida

1. ⚡ Encender ambos coches
2. ⏱️ Esperar 10 segundos (WiFi + ESP-NOW)
3. 🌐 Abrir navegador → IP del maestro
4. 👀 Verificar "👑 MAESTRO"
5. 🎯 Poner obstáculo frente al maestro
6. ✅ Ver que ambos coches se mueven igual
7. 🔄 Clic en [🤖 Esclavo] → ahora el otro controla

---

## 🔍 Verificación Visual

```
MAESTRO                          ESCLAVO
┌──────────────────────┐         ┌──────────────────────┐
│ 👑 MAESTRO           │         │ 🤖 ESCLAVO           │
│ 🌡️ 24.3°C 📡         │         │ 🌡️ 24.3°C 📶         │
│ 💡 Oscuro 📡         │         │ 💡 Oscuro 📶         │
│ 💡 LUCES ON          │         │ 💡 LUCES ON          │
└──────────────────────┘         └──────────────────────┘
   📡 = LOCAL                        📶 = REMOTO
```

---

## 🐛 Si Algo Falla

| Síntoma | Solución Rápida |
|---------|-----------------|
| No conecta WiFi | Verifica SSID y password |
| No encuentra MAC | Abre Serial Monitor a 115200 baudios |
| No cambia modo | Revisa que MACs estén bien copiadas |
| Muestra ❌ | Al menos un coche debe tener sensores ≥ 0 |
| Luces no encienden | Verifica `LUCES_PIN ≥ 0` y conexión GPIO15 |

---

## 💡 Configuraciones Típicas

### Opción A: Ahorro (Recomendado)
```
Coche 1: TEMP_PIN=A0,  LIGHT_PIN=D7  (con sensores)
Coche 2: TEMP_PIN=-1,  LIGHT_PIN=-1  (sin sensores, recibe datos)
```

### Opción B: Redundancia
```
Coche 1: TEMP_PIN=A0,  LIGHT_PIN=D7  (con sensores)
Coche 2: TEMP_PIN=A0,  LIGHT_PIN=D7  (con sensores)
```

### Opción C: Solo Motores
```
Coche 1: TEMP_PIN=-1,  LIGHT_PIN=-1  (sin sensores)
Coche 2: TEMP_PIN=-1,  LIGHT_PIN=-1  (sin sensores)
```

---

## 🎓 Comandos Útiles

### Serial Monitor - Ver estado y debug:
```
// Monitor a 115200 baud - Muestra:
Distancia: 20.5 cm
Modo: MAESTRO
Estado: AVANZANDO

// Log ESP-NOW (tiempo real):
5.234s ENVIO: AVANZANDO V:200,200 T:23.5 L:1
5.347s ENVIO: AVANZANDO V:200,200 T:23.5 L:1
10.123s MODO: MAESTRO → ESCLAVO
12.456s ERROR: Envío fallido  // Normal si < 5%
```

### Cambiar configuración rápido:
```cpp
// En setup(), después de inicializar:
miCoche.setRangoDistancia(18.0, 22.0);  // Ajustar zona muerta
miCoche.setConstanteProporcional(8.0);   // Ajustar suavidad
```

### Verificar comunicación:
✅ **Normal:** 8-10 mensajes/segundo, errores < 5%  
⚠️ **Advertencia:** Errores 5-10%, funciona pero revisar  
❌ **Problema:** Errores > 10%, revisar distancia/alimentación

---

## 📚 Siguiente Paso

- **Documentación completa:** [README.md](README.md)
- **Casos prácticos:** [EJEMPLOS.md](EJEMPLOS.md)

---

**⏱️ Tiempo total: ~5 minutos**

**¡A rodar! 🚗💨**
