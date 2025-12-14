#include "Coche.h"

// Variable global para callbacks ESP-NOW
static Coche* instanciaCocheGlobal = nullptr;

// Constructor
Coche::Coche(int m1A, int m1B, int m2A, int m2B, 
             int trig, int echo, int light, int luces) {
    motor1A = m1A; motor1B = m1B; motor2A = m2A; motor2B = m2B;
    trigPin = trig; echoPin = echo; lightPin = light; pinLuces = luces;
    distanciaMin = 18.0; distanciaMax = 22.0; kp = 8.0;
    servidor = nullptr;
    ultimaDistancia = 0; ultimaLuz = 0;
    strcpy(estadoMovimiento, "PARADO"); ultimaLecturaDistancia = 0;
    luminosidadRecibida = 0; distanciaRecibida = 0;
    datosRecibidos = false;
    esMaestro = false; ultimaVelocidadIzq = 0; ultimaVelocidadDer = 0;
    nuevosDatosESPNow = false;
    velocidadPreviaIzq = 0; velocidadPreviaDer = 0;
    espnowInicializado = false; lucesAutomaticas = true; estadoLuces = false;
    ultimoEnvio = 0;
    memset(macRemota, 0, 6);
}

// Inicialización de pines
void Coche::inicializar() {
    pinMode(motor1A, OUTPUT); pinMode(motor1B, OUTPUT);
    pinMode(motor2A, OUTPUT); pinMode(motor2B, OUTPUT);
    if (trigPin >= 0) pinMode(trigPin, OUTPUT);
    if (echoPin >= 0) pinMode(echoPin, INPUT);
    if (lightPin >= 0) pinMode(lightPin, INPUT);
    if (pinLuces >= 0) { pinMode(pinLuces, OUTPUT); digitalWrite(pinLuces, LOW); }
    detenerMotores();
}

// Leer distancia
float Coche::leerDistancia() {
    if (trigPin < 0 || echoPin < 0) return 0;
    unsigned long ahora = millis();
    if (ahora - ultimaLecturaDistancia > 200) {
        ultimaDistancia = leerDistanciaFiable();
        ultimaLecturaDistancia = ahora;
    }
    return ultimaDistancia;
}

float Coche::leerDistanciaFiable() {
    const int NUM_LECTURAS = 5;
    float suma = 0; int lecturasValidas = 0;
    for (int i = 0; i < NUM_LECTURAS; i++) {
        digitalWrite(trigPin, LOW); delayMicroseconds(2);
        digitalWrite(trigPin, HIGH); delayMicroseconds(10);
        digitalWrite(trigPin, LOW);
        long duracion = pulseIn(echoPin, HIGH, 30000);
        float distancia = duracion * 0.034 / 2.0;
        if (distancia > 2 && distancia < 400) {
            suma += distancia; lecturasValidas++;
        }
        if (i < NUM_LECTURAS - 1) delay(10);
    }
    if (lecturasValidas == 0) return (ultimaDistancia > 0) ? ultimaDistancia : 400;
    return suma / lecturasValidas;
}

// Leer luz
int Coche::leerLuz() {
    if (lightPin < 0) return 0;
    return digitalRead(lightPin);
}

// Control proporcional de distancia (solo maestro)
void Coche::controlarDistancia() {
    if (!esMaestro) return; // Solo el maestro controla
    
    float distanciaActual = leerDistancia();
    
    // Zona muerta: entre 15-20cm → PARADO
    if (distanciaActual >= distanciaMin && distanciaActual <= distanciaMax) {
        detenerMotores();
        strcpy(estadoMovimiento, "PARADO");
        return;
    }
    
    int velocidad = 0;
    
    // RETROCEDER: distancia < 15cm
    // A 15cm → 80 PWM, A 5cm → 180 PWM
    if (distanciaActual < distanciaMin) {
        strcpy(estadoMovimiento, "RETROCEDIENDO");
        
        if (distanciaActual <= 5.0) {
            velocidad = 180;
        } else if (distanciaActual >= 15.0) {
            velocidad = 80;
        } else {
            // Interpolación lineal: v = 80 + (15 - dist) * (180 - 80) / (15 - 5)
            velocidad = 80 + (int)((15.0 - distanciaActual) * 100.0 / 10.0);
        }
    }
    // AVANZAR: distancia > 20cm
    // A 20cm → 100 PWM, A 40cm → 255 PWM
    else {
        strcpy(estadoMovimiento, "AVANZANDO");
        
        if (distanciaActual >= 40.0) {
            velocidad = -255;
        } else if (distanciaActual <= 20.0) {
            velocidad = -100;
        } else {
            // Interpolación lineal: v = 100 + (dist - 20) * (255 - 100) / (40 - 20)
            velocidad = -(100 + (int)((distanciaActual - 20.0) * 155.0 / 20.0));
        }
    }
    
    moverMotores(velocidad, velocidad);
}

// Mover motores
void Coche::moverMotores(int velocidadIzq, int velocidadDer) {
    // BOOST: Impulso inicial si el motor estaba parado
    bool boostIzq = (ultimaVelocidadIzq == 0 && velocidadIzq != 0);
    bool boostDer = (ultimaVelocidadDer == 0 && velocidadDer != 0);
    
    // Motor izquierdo con boost
    if (velocidadIzq >= 0) {
        int velocidadReal = boostIzq ? 210 : velocidadIzq;
        analogWrite(motor1A, velocidadReal); 
        analogWrite(motor1B, 0);
    } else {
        int velocidadReal = boostIzq ? 210 : -velocidadIzq;
        analogWrite(motor1A, 0); 
        analogWrite(motor1B, velocidadReal);
    }
    
    // Motor derecho con boost
    if (velocidadDer >= 0) {
        int velocidadReal = boostDer ? 210 : velocidadDer;
        analogWrite(motor2A, velocidadReal); 
        analogWrite(motor2B, 0);
    } else {
        int velocidadReal = boostDer ? 210 : -velocidadDer;
        analogWrite(motor2A, 0); 
        analogWrite(motor2B, velocidadReal);
    }
    
    // Si aplicamos boost, esperar y luego aplicar velocidad real
    if (boostIzq || boostDer) {
        delay(100);
        
        if (velocidadIzq >= 0) {
            analogWrite(motor1A, velocidadIzq); 
            analogWrite(motor1B, 0);
        } else {
            analogWrite(motor1A, 0); 
            analogWrite(motor1B, -velocidadIzq);
        }
        
        if (velocidadDer >= 0) {
            analogWrite(motor2A, velocidadDer); 
            analogWrite(motor2B, 0);
        } else {
            analogWrite(motor2A, 0); 
            analogWrite(motor2B, -velocidadDer);
        }
    }
    
    // Guardar velocidades actuales para la próxima vez
    ultimaVelocidadIzq = velocidadIzq;
    ultimaVelocidadDer = velocidadDer;
}

// Detener motores
void Coche::detenerMotores() {
    analogWrite(motor1A, 0); analogWrite(motor1B, 0);
    analogWrite(motor2A, 0); analogWrite(motor2B, 0);
    ultimaVelocidadIzq = 0;
    ultimaVelocidadDer = 0;
}

void Coche::detener() {
    detenerMotores();
}

// Configuración
void Coche::setRangoDistancia(float minDist, float maxDist) {
    distanciaMin = minDist; distanciaMax = maxDist;
}

void Coche::setConstanteProporcional(float kp_value) {
    kp = kp_value;
}

// Inicializar WiFi
void Coche::inicializarWiFi(const char* ssid, const char* password) {
    Serial.print("Conectando a WiFi");
    WiFi.begin(ssid, password);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
        delay(500); Serial.print("."); intentos++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi conectado!");
        Serial.print("Dirección IP: "); Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nNo se pudo conectar a WiFi");
    }
}

// Servidor web
void Coche::inicializarServidorWeb() {
    servidor = new ESP8266WebServer(80);
    
    servidor->on("/", [this]() {
        String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<title>Monitor Coche " + String(esMaestro ? "MAESTRO" : "ESCLAVO") + "</title>";
        html += "<style>";
        html += "body{font-family:Arial;max-width:600px;margin:50px auto;padding:20px;background:#f0f0f0}";
        html += ".card{background:white;border-radius:10px;padding:20px;margin:15px 0;box-shadow:0 2px 5px rgba(0,0,0,0.1)}";
        html += "h1{color:#333;text-align:center}";
        html += ".sensor{display:flex;justify-content:space-between;align-items:center;margin:10px 0;padding:15px;background:#f9f9f9;border-radius:5px}";
        html += ".sensor-label{font-weight:bold;color:#555}";
        html += ".sensor-value{font-size:24px;color:#007bff}";
        html += ".unit{font-size:16px;color:#666;margin-left:5px}";
        html += ".estado{text-align:center;padding:20px;border-radius:10px;font-size:28px;font-weight:bold;margin:15px 0;transition:all 0.3s}";
        html += ".parado{background:#ffc107;color:#000}";
        html += ".avanzando{background:#28a745;color:white}";
        html += ".retrocediendo{background:#dc3545;color:white}";
        html += ".badge{display:inline-block;padding:5px 10px;border-radius:5px;font-size:14px;font-weight:bold}";
        html += ".maestro{background:#007bff;color:white}";
        html += ".esclavo{background:#6c757d;color:white}";
        html += "</style>";
        html += "<script>";
        html += "function actualizar(){fetch('/datos').then(r=>r.json()).then(data=>{";
        html += "document.getElementById('distancia').innerHTML=data.distancia.toFixed(1)+' <span class=\"unit\">cm</span>';";
        html += "document.getElementById('luz').innerHTML=data.luz?'☀️ Detectada':'🌙 Oscuro';";
        html += "var estadoDiv=document.getElementById('estado');";
        html += "estadoDiv.className='card estado '+data.estado.toLowerCase();";
        html += "var icono=data.estado==='AVANZANDO'?'⬆️':(data.estado==='RETROCEDIENDO'?'⬇️':'⏸️');";
        html += "estadoDiv.innerHTML=icono+' '+data.estado;";
        html += "})}";
        html += "setInterval(actualizar,500);window.onload=actualizar;";
        html += "</script></head><body>";
        html += "<h1>🚗 Coche <span class='badge " + String(esMaestro ? "maestro'>MAESTRO" : "esclavo'>ESCLAVO") + "</span></h1>";
        html += "<div class='card estado parado' id='estado'>⏸️ PARADO</div>";
        html += "<div class='card'>";
        html += "<div class='sensor'><span class='sensor-label'>📏 Distancia:</span><span class='sensor-value' id='distancia'>-- cm</span></div>";
        html += "<div class='sensor'><span class='sensor-label'>💡 Luminosidad:</span><span class='sensor-value' id='luz'>--</span></div>";
        html += "</div></body></html>";
        servidor->send(200, "text/html", html);
    });
    
    servidor->on("/datos", [this]() {
        servidor->send(200, "application/json", obtenerDatosJSON());
    });
    
    servidor->begin();
}

void Coche::atenderClientes() {
    if (servidor) servidor->handleClient();
}

String Coche::obtenerDatosJSON() {
    float dist; int luz;
    
    if (esMaestro) {
        dist = leerDistancia();
        luz = leerLuz();
    } else {
        dist = distanciaRecibida;
        luz = luminosidadRecibida;
    }
    
    String json = "{";
    json += "\"distancia\":" + String(dist, 2) + ",";
    json += "\"luz\":" + String(luz) + ",";
    json += "\"estado\":\"" + String(estadoMovimiento) + "\"";
    json += "}";
    return json;
}

// ========== ESP-NOW ==========

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    // Callback de envío
}

void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
    if (instanciaCocheGlobal != nullptr && len > 0) {
        char mensaje[200];
        memcpy(mensaje, incomingData, len);
        mensaje[len] = '\0';
        instanciaCocheGlobal->procesarMensajeRecibido(mensaje);
    }
}

// Inicializar como MAESTRO
void Coche::inicializarESPNowMaestro(uint8_t macEsclavo[6]) {
    instanciaCocheGlobal = this;
    esMaestro = true;
    memcpy(macRemota, macEsclavo, 6);
    
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != 0) return;
    
    espnowInicializado = true;
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_add_peer(macRemota, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

// Inicializar como ESCLAVO
void Coche::inicializarESPNowEsclavo(uint8_t macMaestro[6]) {
    instanciaCocheGlobal = this;
    esMaestro = false;
    memcpy(macRemota, macMaestro, 6);
    
    // Ya está en modo STA por WiFi
    if (esp_now_init() != 0) {
        Serial.println("Error al inicializar ESP-NOW");
        return;
    }
    
    espnowInicializado = true;
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_add_peer(macRemota, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);
}

// Enviar comando (no usado en esclavo)
void Coche::enviarComandoESPNow() {
    // El esclavo no envía comandos
}

// Procesar mensaje recibido (solo esclavo)
void Coche::procesarMensajeRecibido(const char* mensaje) {
    if (esMaestro) return;
    
    int luz;
    float dist;
    
    if (sscanf(mensaje, "luz=%d,dist=%f", &luz, &dist) == 2) {
        luminosidadRecibida = luz;
        distanciaRecibida = dist;
        datosRecibidos = true;
        nuevosDatosESPNow = true;
    }
}

// Ejecutar comando recibido (llamar desde loop)
void Coche::ejecutarComandoRecibido() {
    if (!nuevosDatosESPNow || esMaestro) return;
    
    nuevosDatosESPNow = false;
    
    float distanciaActual = distanciaRecibida;
    
    // Zona muerta: entre 15-20cm → PARADO
    if (distanciaActual >= distanciaMin && distanciaActual <= distanciaMax) {
        detenerMotores();
        strcpy(estadoMovimiento, "PARADO");
        return;
    }
    
    int velocidad = 0;
    
    // RETROCEDER: distancia < 15cm (80-180 PWM)
    if (distanciaActual < distanciaMin) {
        strcpy(estadoMovimiento, "RETROCEDIENDO");
        
        if (distanciaActual <= 5.0) {
            velocidad = 180;
        } else if (distanciaActual >= 15.0) {
            velocidad = 80;
        } else {
            velocidad = 80 + (int)((15.0 - distanciaActual) * 100.0 / 10.0);
        }
    }
    // AVANZAR: distancia > 20cm (100-255 PWM)
    else {
        strcpy(estadoMovimiento, "AVANZANDO");
        
        if (distanciaActual >= 40.0) {
            velocidad = -255;
        } else if (distanciaActual <= 20.0) {
            velocidad = -100;
        } else {
            velocidad = -(100 + (int)((distanciaActual - 20.0) * 155.0 / 20.0));
        }
    }
    
    moverMotores(velocidad, velocidad);
}

// Control de luces
void Coche::controlarLucesAutomaticas() {
    if (!lucesAutomaticas || pinLuces < 0) return;
    
    int lecturaLuz;
    if (esMaestro) {
        lecturaLuz = leerLuz();
    } else {
        if (!datosRecibidos) return;
        lecturaLuz = luminosidadRecibida;
    }
    
    if (lecturaLuz == 0 && !estadoLuces) encenderLuces();
    else if (lecturaLuz == 1 && estadoLuces) apagarLuces();
}

void Coche::encenderLuces() {
    if (pinLuces >= 0) { digitalWrite(pinLuces, HIGH); estadoLuces = true; }
}

void Coche::apagarLuces() {
    if (pinLuces >= 0) { digitalWrite(pinLuces, LOW); estadoLuces = false; }
}

bool Coche::obtenerEstadoLuces() {
    return estadoLuces;
}
