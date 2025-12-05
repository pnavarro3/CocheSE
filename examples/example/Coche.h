#ifndef COCHE_H
#define COCHE_H

#include <Arduino.h>

class Coche {
private:
    // Pines del driver L9110S
    int motor1A;
    int motor1B;
    int motor2A;
    int motor2B;
    
    // Pines del sensor HC-SR04
    int trigPin;
    int echoPin;
    
    // Pin del sensor LM35 (temperatura)
    int tempPin;
    
    // Pin del sensor LM393 (luz)
    int lightPin;
    
    // Variables de control
    float distanciaObjetivo;
    float kp; // Constante proporcional para control
    
    // Funciones privadas
    void moverMotores(int velocidadIzq, int velocidadDer);
    void detenerMotores();
    
public:
    // Constructor
    Coche(int m1A, int m1B, int m2A, int m2B, 
          int trig, int echo, int temp, int light);
    
    // Inicialización
    void inicializar();
    
    // Control de movimiento
    void controlarDistancia();
    void avanzar(int velocidad);
    void retroceder(int velocidad);
    void girarIzquierda(int velocidad);
    void girarDerecha(int velocidad);
    void detener();
    
    // Sensores
    float leerDistancia();
    float leerTemperatura();
    int leerLuz();
    
    // Configuración
    void setDistanciaObjetivo(float distancia);
    void setConstanteProporcional(float kp_value);
};

#endif