/*
 * COCHE MAESTRO - Simplificado
 * 
 * Este coche:
 * - Lee TODOS los sensores (distancia, temperatura, luminosidad)
 * - Controla su propio movimiento según la distancia
 * - Envía comandos de movimiento al esclavo
 * - Envía datos de sensores al esclavo
 * 
 * PASOS DE CONFIGURACIÓN:
 * 1. Conecta todos los sensores a este coche
 * 2. Sube primero el código del ESCLAVO y anota su MAC
 * 3. Pon la MAC del esclavo en MAC_ESCLAVO (abajo)
 * 4. Sube este código
 * 5. Abre Serial Monitor para ver la IP
 */

#include "Coche.h"

// ========== CONFIGURACIÓN WIFI ==========
const char* WIFI_SSID = "yiyiyi";        
const char* WIFI_PASSWORD = "xabicrack"; 

// ========== CONFIGURACIÓN ESP-NOW ==========
// IMPORTANTE: Pon aquí la MAC del ESCLAVO (se muestra en su Serial Monitor)
uint8_t MAC_ESCLAVO[] = {0x5C, 0xCF, 0x7F, 0xEC, 0x8F, 0xEE};

// ========== PINES LOLIN D1 ESP-WROOM-02 ==========
#define MOTOR1_A D1  // GPIO5
#define MOTOR1_B D2  // GPIO4
#define MOTOR2_A D3  // GPIO0
#define MOTOR2_B D4  // GPIO2
#define TRIG_PIN D5  // GPIO14 - Sensor ultrasonidos
#define ECHO_PIN D6  // GPIO12 - CON DIVISOR DE VOLTAJE
#define LIGHT_PIN D7 // GPIO13 - Sensor luz LM393
#define LUCES_PIN D8 // GPIO15 - LEDs (opcional)

// Crear objeto Coche MAESTRO
Coche miCoche(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B,
              TRIG_PIN, ECHO_PIN, LIGHT_PIN, LUCES_PIN);

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Inicializar hardware
  miCoche.inicializar();
  
  // Configurar control de distancia: zona muerta 15-20 cm
  miCoche.setRangoDistancia(15.0, 20.0);
  miCoche.setConstanteProporcional(8.0);
  
  // Conectar a WiFi PRIMERO
  miCoche.inicializarWiFi(WIFI_SSID, WIFI_PASSWORD);
  
  // Inicializar ESP-NOW como MAESTRO
  miCoche.inicializarESPNowMaestro(MAC_ESCLAVO);
  
  // Iniciar servidor web
  miCoche.inicializarServidorWeb();
  
  delay(1000);
}

void loop() {
  // Servidor web
  miCoche.atenderClientes();
  
  // Control automático de distancia (15-20 cm)
  miCoche.controlarDistancia();
  
  // Control de luces según sensor
  miCoche.controlarLucesAutomaticas();
  
  // Enviar comandos al esclavo cada 100ms
  miCoche.enviarComandoESPNow();
  
  delay(50);
}
