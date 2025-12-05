#include "Coche.h"

// Constructor
Coche::Coche(int m1A, int m1B, int m2A, int m2B, 
             int trig, int echo, int temp, int light) {
    motor1A = m1A;
    motor1B = m1B;
    motor2A = m2A;
    motor2B = m2B;
    trigPin = trig;
    echoPin = echo;
    tempPin = temp;
    lightPin = light;
    distanciaObjetivo = 10.0; // 10 cm por defecto
    kp = 15.0; // Constante proporcional por defecto
}

// Inicialización de pines
void Coche::inicializar() {
    // Configurar pines del motor como salidas
    pinMode(motor1A, OUTPUT);
    pinMode(motor1B, OUTPUT);
    pinMode(motor2A, OUTPUT);
    pinMode(motor2B, OUTPUT);
    
    // Configurar pines del sensor HC-SR04
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    // Configurar pin del sensor de luz como entrada
    pinMode(lightPin, INPUT);
    
    // El pin analógico del LM35 no necesita configuración
    
    // Detener motores inicialmente
    detenerMotores();
}

// Leer distancia del sensor HC-SR04 en cm
float Coche::leerDistancia() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duracion = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
    float distancia = duracion * 0.034 / 2.0; // velocidad del sonido
    
    // Si no hay lectura válida, retornar valor alto
    if (distancia == 0) {
        distancia = 400;
    }
    
    return distancia;
}

// Leer temperatura del sensor LM35 en grados Celsius
float Coche::leerTemperatura() {
    int lectura = analogRead(tempPin);
    // LM35: 10mV/°C, con Vref 5V y ADC de 10 bits (1024)
    // Temperatura = (lectura * 5000mV / 1024) / 10
    float temperatura = (lectura * 5000.0 / 1024.0) / 10.0;
    return temperatura;
}

// Leer estado del sensor de luz LM393 (digital)
// Retorna 1 si hay luz, 0 si está oscuro
int Coche::leerLuz() {
    return digitalRead(lightPin);
}

// Control proporcional de distancia
void Coche::controlarDistancia() {
    float distanciaActual = leerDistancia();
    
    // Calcular error (distancia actual - distancia deseada)
    float error = distanciaActual - distanciaObjetivo;
    
    // Calcular velocidad proporcional al error
    int velocidad = (int)(kp * error);
    
    // Limitar velocidad entre -255 y 255
    if (velocidad > 255) velocidad = 255;
    if (velocidad < -255) velocidad = -255;
    
    // Si el error es muy pequeño, detener
    if (abs(error) < 1.0) {
        detenerMotores();
    }
    // Si está lejos, avanzar
    else if (error > 0) {
        moverMotores(velocidad, velocidad);
    }
    // Si está cerca, retroceder
    else {
        moverMotores(velocidad, velocidad); // velocidad negativa
    }
}

// Mover motores (privado)
void Coche::moverMotores(int velocidadIzq, int velocidadDer) {
    // Motor izquierdo (motor1)
    if (velocidadIzq >= 0) {
        analogWrite(motor1A, velocidadIzq);
        analogWrite(motor1B, 0);
    } else {
        analogWrite(motor1A, 0);
        analogWrite(motor1B, -velocidadIzq);
    }
    
    // Motor derecho (motor2)
    if (velocidadDer >= 0) {
        analogWrite(motor2A, velocidadDer);
        analogWrite(motor2B, 0);
    } else {
        analogWrite(motor2A, 0);
        analogWrite(motor2B, -velocidadDer);
    }
}

// Detener motores (privado y público)
void Coche::detenerMotores() {
    analogWrite(motor1A, 0);
    analogWrite(motor1B, 0);
    analogWrite(motor2A, 0);
    analogWrite(motor2B, 0);
}

void Coche::detener() {
    detenerMotores();
}

// Avanzar
void Coche::avanzar(int velocidad) {
    moverMotores(velocidad, velocidad);
}

// Retroceder
void Coche::retroceder(int velocidad) {
    moverMotores(-velocidad, -velocidad);
}

// Girar a la izquierda
void Coche::girarIzquierda(int velocidad) {
    moverMotores(-velocidad, velocidad);
}

// Girar a la derecha
void Coche::girarDerecha(int velocidad) {
    moverMotores(velocidad, -velocidad);
}

// Configurar distancia objetivo
void Coche::setDistanciaObjetivo(float distancia) {
    distanciaObjetivo = distancia;
}

// Configurar constante proporcional
void Coche::setConstanteProporcional(float kp_value) {
    kp = kp_value;
}