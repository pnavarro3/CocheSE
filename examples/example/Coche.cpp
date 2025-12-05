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
    distanciaMin = 7.0;  // Límite inferior
    distanciaMax = 13.0; // Límite superior
    kp = 8.0; // Constante proporcional más suave
    servidor = nullptr;
    ultimaDistancia = 0;
    ultimaTemperatura = 0;
    ultimaLuz = 0;
    estadoMovimiento = "PARADO";
    ultimaLecturaDistancia = 0;
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

// Leer distancia del sensor HC-SR04 en cm (con caché)
float Coche::leerDistancia() {
    // Solo medir cada 200ms para tener medidas más estables
    unsigned long ahora = millis();
    if (ahora - ultimaLecturaDistancia > 200) {
        ultimaDistancia = leerDistanciaFiable();
        ultimaLecturaDistancia = ahora;
    }
    return ultimaDistancia;
}

// Leer distancia con múltiples mediciones para mayor fiabilidad (privado)
float Coche::leerDistanciaFiable() {
    const int NUM_LECTURAS = 5;
    float lecturas[NUM_LECTURAS];
    float suma = 0;
    int lecturasValidas = 0;
    
    // Tomar múltiples mediciones
    for (int i = 0; i < NUM_LECTURAS; i++) {
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        
        long duracion = pulseIn(echoPin, HIGH, 30000);
        float distancia = duracion * 0.034 / 2.0;
        
        // Filtrar lecturas inválidas (muy cerca o muy lejos)
        if (distancia > 2 && distancia < 400) {
            lecturas[lecturasValidas] = distancia;
            lecturasValidas++;
        }
        
        if (i < NUM_LECTURAS - 1) {
            delay(10); // Pequeña pausa entre mediciones
        }
    }
    
    // Si no hay lecturas válidas, retornar distancia anterior o valor alto
    if (lecturasValidas == 0) {
        return (ultimaDistancia > 0) ? ultimaDistancia : 400;
    }
    
    // Calcular promedio de lecturas válidas
    for (int i = 0; i < lecturasValidas; i++) {
        suma += lecturas[i];
    }
    
    return suma / lecturasValidas;
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

// Control proporcional de distancia con zona muerta
void Coche::controlarDistancia() {
    float distanciaActual = leerDistancia();
    
    // Zona muerta: si está entre distanciaMin y distanciaMax, no hacer nada
    if (distanciaActual >= distanciaMin && distanciaActual <= distanciaMax) {
        detenerMotores();
        estadoMovimiento = "PARADO";
        return;
    }
    
    // Calcular error respecto al centro de la zona muerta
    float error;
    if (distanciaActual < distanciaMin) {
        // Está demasiado cerca, retroceder (velocidad negativa)
        error = distanciaActual - distanciaMin; // Negativo
        estadoMovimiento = "RETROCEDIENDO";
    } else {
        // Está demasiado lejos, avanzar (velocidad positiva)
        error = distanciaActual - distanciaMax; // Positivo
        estadoMovimiento = "AVANZANDO";
    }
    
    // Calcular velocidad proporcional al error (control más suave)
    // INVERTIMOS el signo para corregir la dirección
    int velocidad = -(int)(kp * error);
    
    // Limitar velocidad entre -255 y 255
    if (velocidad > 255) velocidad = 255;
    if (velocidad < -255) velocidad = -255;
    
    // Aplicar velocidad mínima más alta para que los motores se muevan
    if (velocidad > 0 && velocidad < 120) velocidad = 120;
    if (velocidad < 0 && velocidad > -120) velocidad = -120;
    
    // Mover motores
    moverMotores(velocidad, velocidad);
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

// Configurar rango de distancia (zona muerta)
void Coche::setRangoDistancia(float minDist, float maxDist) {
    distanciaMin = minDist;
    distanciaMax = maxDist;
    distanciaObjetivo = (minDist + maxDist) / 2.0;
}

// Configurar constante proporcional
void Coche::setConstanteProporcional(float kp_value) {
    kp = kp_value;
}

// Obtener estado de movimiento actual
String Coche::obtenerEstadoMovimiento() {
    return estadoMovimiento;
}

// Inicializar WiFi
void Coche::inicializarWiFi(const char* ssid, const char* password) {
    Serial.print("Conectando a WiFi");
    WiFi.begin(ssid, password);
    
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
        delay(500);
        Serial.print(".");
        intentos++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi conectado!");
        Serial.print("Dirección IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nNo se pudo conectar a WiFi");
    }
}

// Inicializar servidor web
void Coche::inicializarServidorWeb() {
    servidor = new ESP8266WebServer(80);
    
    // Ruta raíz - página HTML
    servidor->on("/", [this]() {
        String html = "<!DOCTYPE html><html><head>";
        html += "<meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<title>Monitor Coche Robot</title>";
        html += "<style>";
        html += "body { font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; padding: 20px; background: #f0f0f0; }";
        html += ".card { background: white; border-radius: 10px; padding: 20px; margin: 15px 0; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
        html += "h1 { color: #333; text-align: center; }";
        html += ".sensor { display: flex; justify-content: space-between; align-items: center; margin: 10px 0; padding: 15px; background: #f9f9f9; border-radius: 5px; }";
        html += ".sensor-label { font-weight: bold; color: #555; }";
        html += ".sensor-value { font-size: 24px; color: #007bff; }";
        html += ".unit { font-size: 16px; color: #666; margin-left: 5px; }";
        html += ".estado { text-align: center; padding: 20px; border-radius: 10px; font-size: 28px; font-weight: bold; margin: 15px 0; transition: all 0.3s; }";
        html += ".parado { background: #ffc107; color: #000; }";
        html += ".avanzando { background: #28a745; color: white; }";
        html += ".retrocediendo { background: #dc3545; color: white; }";
        html += "</style>";
        html += "<script>";
        html += "function actualizarDatos() {";
        html += "  fetch('/datos').then(r => r.json()).then(data => {";
        html += "    document.getElementById('distancia').innerHTML = data.distancia.toFixed(1) + ' <span class=\"unit\">cm</span>';";
        html += "    document.getElementById('temperatura').innerHTML = data.temperatura.toFixed(1) + ' <span class=\"unit\">°C</span>';";
        html += "    document.getElementById('luz').innerHTML = data.luz ? '☀️ Detectada' : '🌙 Oscuro';";
        html += "    var estadoDiv = document.getElementById('estado');";
        html += "    estadoDiv.className = 'card estado ' + data.estado.toLowerCase();";
        html += "    var icono = data.estado === 'AVANZANDO' ? '⬆️' : (data.estado === 'RETROCEDIENDO' ? '⬇️' : '⏸️');";
        html += "    estadoDiv.innerHTML = icono + ' ' + data.estado;";
        html += "  });";
        html += "}";
        html += "setInterval(actualizarDatos, 500);";
        html += "window.onload = actualizarDatos;";
        html += "</script>";
        html += "</head><body>";
        html += "<h1>🚗 Monitor Coche Robot</h1>";
        html += "<div class='card estado parado' id='estado'>⏸️ PARADO</div>";
        html += "<div class='card'>";
        html += "<div class='sensor'><span class='sensor-label'>📏 Distancia:</span><span class='sensor-value' id='distancia'>-- cm</span></div>";
        html += "<div class='sensor'><span class='sensor-label'>🌡️ Temperatura:</span><span class='sensor-value' id='temperatura'>-- °C</span></div>";
        html += "<div class='sensor'><span class='sensor-label'>💡 Luminosidad:</span><span class='sensor-value' id='luz'>--</span></div>";
        html += "</div>";
        html += "</body></html>";
        servidor->send(200, "text/html", html);
    });
    
    // Ruta para obtener datos en JSON
    servidor->on("/datos", [this]() {
        servidor->send(200, "application/json", obtenerDatosJSON());
    });
    
    servidor->begin();
    Serial.println("Servidor web iniciado en el puerto 80");
}

// Atender peticiones de clientes
void Coche::atenderClientes() {
    if (servidor) {
        servidor->handleClient();
    }
}

// Obtener datos de sensores en formato JSON
String Coche::obtenerDatosJSON() {
    // Actualizar lecturas
    ultimaDistancia = leerDistancia();
    ultimaTemperatura = leerTemperatura();
    ultimaLuz = leerLuz();
    
    String json = "{";
    json += "\"distancia\":" + String(ultimaDistancia, 2) + ",";
    json += "\"temperatura\":" + String(ultimaTemperatura, 2) + ",";
    json += "\"luz\":" + String(ultimaLuz) + ",";
    json += "\"estado\":\"" + estadoMovimiento + "\"";
    json += "}";
    return json;
}