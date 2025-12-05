#ifndef COCHE_H
#define COCHE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

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
    float distanciaMin;  // Límite inferior zona muerta
    float distanciaMax;  // Límite superior zona muerta
    float kp; // Constante proporcional para control
    
    // Servidor web
    ESP8266WebServer* servidor;
    
    // Variables para datos de sensores
    float ultimaDistancia;
    float ultimaTemperatura;
    int ultimaLuz;
    String estadoMovimiento;  // "PARADO", "AVANZANDO", "RETROCEDIENDO"
    unsigned long ultimaLecturaDistancia;
    
    // Funciones privadas
    void moverMotores(int velocidadIzq, int velocidadDer);
    float leerDistanciaFiable();
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
    String obtenerEstadoMovimiento();
    
    // Configuración
    void setDistanciaObjetivo(float distancia);
    void setRangoDistancia(float minDist, float maxDist);
    void setConstanteProporcional(float kp_value);
    
    // WiFi y servidor web
    void inicializarWiFi(const char* ssid, const char* password);
    void inicializarServidorWeb();
    void atenderClientes();
    String obtenerDatosJSON();
};

#endif