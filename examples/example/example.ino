/*
 * Ejemplo de uso de la clase Coche con servidor web
 * Control automático de distancia con sensores
 * Visualización web de datos en tiempo real
 */

#include "Coche.h"

// ========== CONFIGURACIÓN WIFI ==========
const char* WIFI_SSID = "yiyiyi";        
const char* WIFI_PASSWORD = "xabicrack"; 

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
#define TEMP_PIN A0  // Único pin analógico

// Sensor LM393 (Luz)
#define LIGHT_PIN D7  // GPIO13

// Crear objeto Coche
Coche miCoche(MOTOR1_A, MOTOR1_B, MOTOR2_A, MOTOR2_B,
              TRIG_PIN, ECHO_PIN, TEMP_PIN, LIGHT_PIN);

void setup() {
  // Inicializar comunicación serial
  Serial.begin(115200);
  Serial.println("\n\nIniciando sistema del coche...");
  
  // Inicializar el coche
  miCoche.inicializar();
  
  // Configurar rango de distancia: 18-22 cm (zona muerta)
  miCoche.setRangoDistancia(18.0, 22.0);
  
  // Configurar constante proporcional más suave
  miCoche.setConstanteProporcional(8.0);
  
  // Conectar a WiFi
  Serial.println("\n=== CONEXIÓN WIFI ===");
  miCoche.inicializarWiFi(WIFI_SSID, WIFI_PASSWORD);
  
  // Iniciar servidor web
  Serial.println("\n=== SERVIDOR WEB ===");
  miCoche.inicializarServidorWeb();
  
  Serial.println("\n=== SISTEMA LISTO ===");
  Serial.println("Abre tu navegador y visita la IP mostrada arriba");
  Serial.println("====================================\n");
  
  delay(1000);
}

void loop() {
  // Atender peticiones web
  miCoche.atenderClientes();
  
  // Control automático de distancia (18-22 cm con zona muerta)
  miCoche.controlarDistancia();
  
  // Pequeña pausa para estabilidad
  delay(50);
}

// ========== FUNCIONES DE EJEMPLO (comentadas) ==========
/*
// Para ver datos en monitor serial
void mostrarDatosSerial() {
  float distancia = miCoche.leerDistancia();
  float temperatura = miCoche.leerTemperatura();
  int estadoLuz = miCoche.leerLuz();
  
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.print(" cm | Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" °C | Luz: ");
  Serial.println(estadoLuz ? "Detectada" : "Oscuro");
}

// Movimientos manuales
void ejemplo_movimientos_manuales() {
  miCoche.avanzar(150);
  delay(2000);
  
  miCoche.detener();
  delay(1000);
  
  miCoche.retroceder(150);
  delay(2000);
  
  miCoche.girarDerecha(150);
  delay(1000);
  
  miCoche.girarIzquierda(150);
  delay(1000);
  
  miCoche.detener();
}
*/