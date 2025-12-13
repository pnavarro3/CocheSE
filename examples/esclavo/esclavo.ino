/*
 * COCHE ESCLAVO - Simplificado
 * 
 * Este coche:
 * - NO necesita sensores conectados (solo motores y luces opcionales)
 * - Recibe comandos de movimiento del maestro
 * - Recibe datos de sensores del maestro
 * - Ejecuta los movimientos que le ordena el maestro
 * - Muestra datos del maestro en su web
 * 
 * PASOS DE CONFIGURACIÓN:
 * 1. Conecta solo los motores (y luces si quieres)
 * 2. Sube este código PRIMERO
 * 3. Abre Serial Monitor y COPIA la MAC que muestra
 * 4. Pon esa MAC en el código del maestro (MAC_ESCLAVO)
 * 5. Anota la IP para acceder a la web
 */

#include "Coche.h"

// ========== CONFIGURACIÓN WIFI ==========
const char* WIFI_SSID = "yiyiyi";        
const char* WIFI_PASSWORD = "xabicrack"; 

// ========== CONFIGURACIÓN ESP-NOW ==========
// Esto se puede dejar así, no es necesario cambiarlo
uint8_t MAC_MAESTRO[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ========== PINES LOLIN D1 ESP-WROOM-02 ==========
#define MOTOR1_A D1  // GPIO5
#define MOTOR1_B D2  // GPIO4
#define MOTOR2_A D3  // GPIO0
#define MOTOR2_B D4  // GPIO2
// NO conectar sensores en el esclavo (usa -1)
#define TRIG_PIN -1  // Sin sensor de distancia
#define ECHO_PIN -1
#define LIGHT_PIN -1 // Sin sensor de luz
#define LUCES_PIN D8 // GPIO15 - LEDs (opcional)

// Crear objeto Coche ESCLAVO
Coche miCoche(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B,
              TRIG_PIN, ECHO_PIN, LIGHT_PIN, LUCES_PIN);

void setup() {
  Serial.begin(115200);
  
  // Inicializar hardware
  miCoche.inicializar();
  
  // Inicializar ESP-NOW como ESCLAVO (mostrará su MAC)
  miCoche.inicializarESPNowEsclavo(MAC_MAESTRO);
  
  // Conectar a WiFi
  miCoche.inicializarWiFi(WIFI_SSID, WIFI_PASSWORD);
  
  // Iniciar servidor web
  miCoche.inicializarServidorWeb();
  
  delay(1000);
}

void loop() {
  // Servidor web
  miCoche.atenderClientes();
  
  // Control de luces según datos del maestro
  miCoche.controlarLucesAutomaticas();
  
  delay(50);
}
