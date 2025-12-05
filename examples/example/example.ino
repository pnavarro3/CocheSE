/*
 * Ejemplo de uso de la clase Coche
 * Control automático de distancia con sensores
 */

#include "Coche.h"

// Definir pines para LOLIN D1 ESP-WROOM-02 (ESP8266)
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
  Serial.begin(9600);
  Serial.println("Iniciando sistema del coche...");
  
  // Inicializar el coche
  miCoche.inicializar();
  
  // Configurar distancia objetivo (10 cm por defecto)
  miCoche.setDistanciaObjetivo(10.0);
  
  // Configurar constante proporcional (ajustar según necesidad)
  miCoche.setConstanteProporcional(15.0);
  
  Serial.println("Sistema inicializado");
  delay(2000);
}

void loop() {
  // Leer sensores
  float distancia = miCoche.leerDistancia();
  float temperatura = miCoche.leerTemperatura();
  int estadoLuz = miCoche.leerLuz();
  
  // Mostrar lecturas en el monitor serial
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.print(" cm | Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" °C | Luz: ");
  Serial.println(estadoLuz ? "Detectada" : "Oscuro");
  
  // Control automático de distancia
  miCoche.controlarDistancia();
  
  // Pequeña pausa para estabilidad
  delay(100);
}

// Funciones adicionales de ejemplo (comentadas)
/*
void ejemplo_movimientos_manuales() {
  // Avanzar
  miCoche.avanzar(150);
  delay(2000);
  
  // Detener
  miCoche.detener();
  delay(1000);
  
  // Retroceder
  miCoche.retroceder(150);
  delay(2000);
  
  // Girar derecha
  miCoche.girarDerecha(150);
  delay(1000);
  
  // Girar izquierda
  miCoche.girarIzquierda(150);
  delay(1000);
  
  // Detener
  miCoche.detener();
}

void ejemplo_lectura_sensores() {
  float dist = miCoche.leerDistancia();
  float temp = miCoche.leerTemperatura();
  int luz = miCoche.leerLuz();
  
  Serial.print("Distancia: "); Serial.print(dist); Serial.println(" cm");
  Serial.print("Temperatura: "); Serial.print(temp); Serial.println(" °C");
  Serial.print("Estado luz: "); Serial.println(luz);
}
*/