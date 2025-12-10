/*
 * Ejemplo COCHE DUAL - Control Maestro/Esclavo Intercambiable
 * CON SENSORES COMPARTIDOS (Temperatura y Luminosidad)
 * 
 * Este código sirve para AMBOS coches. Cada coche puede cambiar entre
 * maestro y esclavo desde su interfaz web. Cuando uno cambia a maestro,
 * automáticamente el otro se convierte en esclavo y viceversa.
 * 
 * NOVEDAD: Los sensores de temperatura y luminosidad se comparten automáticamente
 * a través de ESP-NOW. Solo necesitas conectar estos sensores a UN coche, y el
 * otro recibirá los datos automáticamente. Las luces funcionarán en ambos coches.
 * 
 * INSTRUCCIONES INICIALES:
 * =========================
 * 
 * 1. PRIMER COCHE (por ejemplo, el que quieres que empiece como MAESTRO):
 *    - Configura EMPEZAR_COMO_MAESTRO = true
 *    - Deja MAC_OTRO_COCHE como está por ahora
 *    - Si este coche TIENE sensores: TEMP_PIN = A0, LIGHT_PIN = D7
 *    - Si NO tiene sensores: TEMP_PIN = -1, LIGHT_PIN = -1
 *    - Sube el código
 *    - Abre Serial Monitor y anota su MAC
 * 
 * 2. SEGUNDO COCHE (empezará como ESCLAVO):
 *    - Configura EMPEZAR_COMO_MAESTRO = false
 *    - En MAC_OTRO_COCHE pon la MAC del primer coche
 *    - Si este coche TIENE sensores: TEMP_PIN = A0, LIGHT_PIN = D7
 *    - Si NO tiene sensores: TEMP_PIN = -1, LIGHT_PIN = -1
 *    - Sube el código
 *    - Abre Serial Monitor y anota su MAC
 * 
 * 3. ACTUALIZAR PRIMER COCHE:
 *    - Edita el código del primer coche
 *    - En MAC_OTRO_COCHE pon la MAC del segundo coche
 *    - Vuelve a subir el código
 * 
 * 4. ¡LISTO! Ahora:
 *    - Conecta ambos coches al WiFi configurado
 *    - Abre la IP de cualquier coche en tu navegador
 *    - Usa los botones para cambiar entre maestro/esclavo
 *    - El otro coche cambiará automáticamente al modo contrario
 *    - La interfaz web mostrará si los datos son LOCAL 📡 o REMOTO 📶
 *    - Las luces funcionarán en ambos coches según el sensor compartido
 * 
 * CONFIGURACIÓN DE SENSORES:
 * ==========================
 * 
 * OPCIÓN A - Un coche con sensores, otro sin sensores:
 *   COCHE 1 (con sensores):  TEMP_PIN = A0, LIGHT_PIN = D7
 *   COCHE 2 (sin sensores):  TEMP_PIN = -1, LIGHT_PIN = -1
 *   → El coche 1 lee sensores y envía datos al coche 2
 * 
 * OPCIÓN B - Ambos coches con sensores:
 *   COCHE 1: TEMP_PIN = A0, LIGHT_PIN = D7
 *   COCHE 2: TEMP_PIN = A0, LIGHT_PIN = D7
 *   → Cada coche usa sus propios sensores locales
 * 
 * OPCIÓN C - Ningún coche con sensores (solo para pruebas):
 *   COCHE 1: TEMP_PIN = -1, LIGHT_PIN = -1
 *   COCHE 2: TEMP_PIN = -1, LIGHT_PIN = -1
 *   → No habrá datos de sensores disponibles
 */

#include "Coche.h"

// ========== CONFIGURACIÓN WIFI ==========
const char* WIFI_SSID = "yiyiyi";        
const char* WIFI_PASSWORD = "xabicrack"; 

// ========== CONFIGURACIÓN ESP-NOW ==========
// IMPORTANTE: Reemplaza con la MAC del OTRO coche
uint8_t MAC_OTRO_COCHE[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// Ejemplo: {0x5C, 0xCF, 0x7F, 0x12, 0x34, 0x56}

// Define con qué modo empieza este coche
bool EMPEZAR_COMO_MAESTRO = true;  // true = maestro, false = esclavo

// ========== DEFINIR PINES PARA LOLIN D1 ESP-WROOM-02 (ESP8266) ==========
// Driver L9110S - Motor Izquierdo (Motor 1)
#define MOTOR1_A D1  // GPIO5
#define MOTOR1_B D2  // GPIO4

// Driver L9110S - Motor Derecho (Motor 2)
#define MOTOR2_A D3  // GPIO0
#define MOTOR2_B D4  // GPIO2

// Sensor HC-SR04 (¡ECHO necesita divisor de voltaje 5V->3.3V!)
#define TRIG_PIN D5  // GPIO14
#define ECHO_PIN D6  // GPIO12 (CON DIVISOR DE VOLTAJE)

// Sensor LM35 (Temperatura)
#define TEMP_PIN A0  // Único pin analógico - USA -1 si este coche NO tiene sensor

// Sensor LM393 (Luz)
#define LIGHT_PIN D7  // GPIO13 - USA -1 si este coche NO tiene sensor

// LEDs/Luces del coche (OPCIONAL - usa -1 si no tienes LEDs conectados)
#define LUCES_PIN D8  // GPIO15 - Puedes usar cualquier GPIO disponible
// Si no tienes LEDs, usa: #define LUCES_PIN -1

// Crear objeto Coche
Coche miCoche(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B,
              TRIG_PIN, ECHO_PIN, TEMP_PIN, LIGHT_PIN, LUCES_PIN);

void setup() {
  // Inicializar comunicación serial
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("  COCHE DUAL - MODO INTERCAMBIABLE");
  Serial.println("=================================");
  
  // Inicializar el coche
  miCoche.inicializar();
  
  // Configurar rango de distancia: 18-22 cm (zona muerta)
  miCoche.setRangoDistancia(18.0, 22.0);
  
  // Configurar constante proporcional más suave
  miCoche.setConstanteProporcional(8.0);
  
  // Inicializar ESP-NOW en modo dual
  Serial.println("\n=== INICIALIZANDO ESP-NOW ===");
  miCoche.inicializarESPNowDual(MAC_OTRO_COCHE, EMPEZAR_COMO_MAESTRO);
  
  // Conectar a WiFi
  Serial.println("\n=== CONEXIÓN WIFI ===");
  miCoche.inicializarWiFi(WIFI_SSID, WIFI_PASSWORD);
  
  // Iniciar servidor web
  Serial.println("\n=== SERVIDOR WEB ===");
  miCoche.inicializarServidorWeb();
  
  Serial.println("\n=== SISTEMA LISTO ===");
  Serial.println("Abre tu navegador y visita la IP mostrada arriba");
  Serial.println("Usa los botones para cambiar entre MAESTRO/ESCLAVO");
  Serial.println("====================================\n");
  
  delay(1000);
}

void loop() {
  // Atender peticiones web
  miCoche.atenderClientes();
  
  // Control automático de distancia (solo si es maestro y modo automático)
  miCoche.controlarDistancia();
  
  // Control automático de luces según luminosidad
  miCoche.controlarLucesAutomaticas();
  
  // Enviar comandos ESP-NOW (solo si es maestro)
  if (miCoche.obtenerModo() && miCoche.obtenerModoAutomatico()) {
    miCoche.enviarComandoESPNow();
  }
  
  // Pequeña pausa para estabilidad
  delay(50);
}
