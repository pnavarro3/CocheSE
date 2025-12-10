#ifndef COCHE_H
#define COCHE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <espnow.h>

// Estructura de datos para enviar comandos por ESP-NOW
typedef struct struct_mensaje {
    int velocidadIzq;  // Velocidad motor izquierdo (-255 a 255)
    int velocidadDer;  // Velocidad motor derecho (-255 a 255)
    char comando[20];  // Comando: "AVANZAR", "RETROCEDER", "DETENER", etc.
    float temperatura; // Temperatura del sensor LM35 (grados Celsius)
    int luminosidad;   // Luminosidad del sensor LM393 (0=oscuro, 1=claro)
    bool tieneSensores; // true si este coche tiene sensores físicos conectados
} struct_mensaje;

// Estructura para comandos de control (cambio de modo)
typedef struct struct_control {
    char tipoComando[20];  // "CAMBIAR_MODO"
    char nuevoModo[20];    // "MAESTRO" o "ESCLAVO"
} struct_control;

// Estructura para respuesta con datos de sensores (comunicación bidireccional)
typedef struct struct_respuesta {
    float temperatura;     // Temperatura del sensor LM35
    int luminosidad;       // Luminosidad del sensor LM393
    bool tieneSensores;    // true si tiene sensores físicos
    char origen[20];       // "MAESTRO" o "ESCLAVO" para identificar quién envía
} struct_respuesta;

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
    
    // Pin para LEDs/Luces del coche
    int pinLuces;
    
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
    
    // Variables para sensores compartidos
    float temperaturaRemota;  // Temperatura recibida del otro coche
    int luminosidadRemota;    // Luminosidad recibida del otro coche
    bool tieneSensoresLocales; // true si este coche tiene sensores conectados
    bool datosRemotos Validos; // true si hemos recibido datos del otro coche
    unsigned long ultimosDatosRemotos; // timestamp de última recepción
    
    // Variables para ESP-NOW
    bool esMaestro;  // true = maestro, false = esclavo
    uint8_t macRemota[6];  // MAC del dispositivo remoto
    int ultimaVelocidadIzq;
    int ultimaVelocidadDer;
    bool espnowInicializado;
    bool modoAutomatico;  // true = control automático con sensor, false = manual
    bool lucesAutomaticas;  // true = luces se encienden/apagan según sensor
    bool estadoLuces;  // true = luces encendidas, false = apagadas
    
    // Variables para log de mensajes
    unsigned long mensajesEnviados;  // Contador de mensajes enviados
    unsigned long mensajesRecibidos;  // Contador de mensajes recibidos
    unsigned long mensajesFallidos;  // Contador de mensajes fallidos
    bool esperandoACK;  // true si está esperando confirmación
    unsigned long ultimoEnvio;  // Timestamp del último envío (throttling 100ms)
    
    // Funciones privadas
    void moverMotores(int velocidadIzq, int velocidadDer);
    float leerDistanciaFiable();
    void detenerMotores();
    
public:
    // Constructor
    Coche(int m1A, int m1B, int m2A, int m2B, 
          int trig, int echo, int temp, int light, int luces = -1);
    
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
    
    // ESP-NOW
    void inicializarESPNowDual(uint8_t macOtroCoche[6], bool empezarComoMaestro = true);
    void cambiarModo(bool nuevoModoMaestro);
    void enviarComandoESPNow();
    void procesarComandoRecibido(struct_mensaje* datos);
    void procesarControlRecibido(struct_control* datos);
    void enviarRespuestaSensores();  // Esclavo envía sus sensores al maestro
    void procesarRespuestaSensores(struct_respuesta* datos);  // Maestro recibe datos del esclavo
    void registrarACK(bool exitoso);  // Registrar resultado de envío
    void enviarCambioModo(bool nuevoModoMaestro);
    bool obtenerModo();
    String obtenerModoTexto();
    void setModoAutomatico(bool automatico);
    bool obtenerModoAutomatico();
    uint8_t* obtenerMAC();
    
    // Control de luces
    void controlarLucesAutomaticas();
    void encenderLuces();
    void apagarLuces();
    void toggleLuces();
    void setLucesAutomaticas(bool automatico);
    bool obtenerEstadoLuces();
    bool obtenerLucesAutomaticas();
    
    // Gestión de sensores compartidos
    bool tieneSensores();
    float obtenerTemperaturaActual(); // Devuelve temp local o remota
    int obtenerLuminosidadActual();   // Devuelve luz local o remota
    String obtenerOrigenDatos();      // "LOCAL", "REMOTO" o "SIN_DATOS"
    
    // Estadísticas ESP-NOW
    void agregarLog(String tipo, String detalle);  // Solo para Serial
    unsigned long obtenerMensajesEnviados();
    unsigned long obtenerMensajesRecibidos();
    unsigned long obtenerMensajesFallidos();
    float obtenerTasaExito();  // Porcentaje de mensajes exitosos
    bool puedeEnviar();  // Verifica si puede enviar (ACK + throttling)
};

#endif