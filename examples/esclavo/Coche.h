#ifndef COCHE_H
#define COCHE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <espnow.h>

// Estructura de datos ESP-NOW
typedef struct struct_mensaje {
    int velocidadIzq;
    int velocidadDer;
    char comando[20];
    int luminosidad;
    float distancia;
} struct_mensaje;

class Coche {
private:
    int motor1A, motor1B, motor2A, motor2B;
    int trigPin, echoPin, lightPin, pinLuces;
    float distanciaMin, distanciaMax, kp;
    ESP8266WebServer* servidor;
    float ultimaDistancia;
    int ultimaLuz;
    String estadoMovimiento;
    unsigned long ultimaLecturaDistancia;
    float distanciaRecibida;
    int luminosidadRecibida;
    bool datosRecibidos, esMaestro;
    uint8_t macRemota[6];
    int ultimaVelocidadIzq, ultimaVelocidadDer;
    bool espnowInicializado, lucesAutomaticas, estadoLuces;
    unsigned long ultimoEnvio;
    
    void moverMotores(int velocidadIzq, int velocidadDer);
    void detenerMotores();
    float leerDistanciaFiable();
    
public:
    Coche(int m1A, int m1B, int m2A, int m2B, 
          int trig, int echo, int light, int luces = -1);
    
    void inicializar();
    void controlarDistancia();
    void detener();
    float leerDistancia();
    int leerLuz();
    void setRangoDistancia(float minDist, float maxDist);
    void setConstanteProporcional(float kp_value);
    void inicializarWiFi(const char* ssid, const char* password);
    void inicializarServidorWeb();
    void atenderClientes();
    String obtenerDatosJSON();
    void inicializarESPNowMaestro(uint8_t macEsclavo[6]);
    void inicializarESPNowEsclavo(uint8_t macMaestro[6]);
    void enviarComandoESPNow();
    void procesarComandoRecibido(struct_mensaje* datos);
    void controlarLucesAutomaticas();
    void encenderLuces();
    void apagarLuces();
    bool obtenerEstadoLuces();
};

#endif
