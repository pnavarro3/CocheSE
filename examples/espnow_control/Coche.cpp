#include "Coche.h"

// Constructor
Coche::Coche(int m1A, int m1B, int m2A, int m2B, 
             int trig, int echo, int temp, int light, int luces) {
    motor1A = m1A;
    motor1B = m1B;
    motor2A = m2A;
    motor2B = m2B;
    trigPin = trig;
    echoPin = echo;
    tempPin = temp;
    lightPin = light;
    pinLuces = luces;
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
    esMaestro = true;  // Por defecto empieza como maestro
    ultimaVelocidadIzq = 0;
    ultimaVelocidadDer = 0;
    espnowInicializado = false;
    modoAutomatico = true;
    lucesAutomaticas = true;  // Luces automáticas activadas por defecto
    estadoLuces = false;  // Luces apagadas inicialmente
    memset(macRemota, 0, 6);
    
    // Inicializar variables de sensores compartidos
    temperaturaRemota = 0;
    luminosidadRemota = 0;
    tieneSensoresLocales = (temp >= 0 && light >= 0);  // Si los pines son válidos
    datosRemotosValidos = false;
    ultimosDatosRemotos = 0;
    
    // Inicializar variables de log
    mensajesEnviados = 0;
    mensajesRecibidos = 0;
    mensajesFallidos = 0;
    esperandoACK = false;
    ultimoEnvio = 0;
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
    
    // Configurar pin de luces si está definido
    if (pinLuces >= 0) {
        pinMode(pinLuces, OUTPUT);
        digitalWrite(pinLuces, LOW);  // Luces apagadas inicialmente
    }
    
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
    // Solo controlar distancia si es maestro Y modo automático está activado
    if (!esMaestro || !modoAutomatico) {
        return;
    }
    
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
    // Guardar velocidades para ESP-NOW
    ultimaVelocidadIzq = velocidadIzq;
    ultimaVelocidadDer = velocidadDer;
    
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
    
    // Ruta raíz - página HTML con controles de modo y luces
    servidor->on("/", [this]() {
        String html = "<!DOCTYPE html><html><head>";
        html += "<meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<title>Control Coche Robot</title>";
        html += "<style>";
        html += "body { font-family: Arial, sans-serif; max-width: 600px; margin: 20px auto; padding: 20px; background: #f0f0f0; }";
        html += ".card { background: white; border-radius: 10px; padding: 20px; margin: 15px 0; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
        html += "h1 { color: #333; text-align: center; margin-bottom: 5px; }";
        html += ".sensor { display: flex; justify-content: space-between; align-items: center; margin: 10px 0; padding: 15px; background: #f9f9f9; border-radius: 5px; }";
        html += ".sensor-label { font-weight: bold; color: #555; }";
        html += ".sensor-value { font-size: 24px; color: #007bff; }";
        html += ".unit { font-size: 16px; color: #666; margin-left: 5px; }";
        html += ".estado { text-align: center; padding: 20px; border-radius: 10px; font-size: 28px; font-weight: bold; margin: 15px 0; transition: all 0.3s; }";
        html += ".parado { background: #ffc107; color: #000; }";
        html += ".avanzando { background: #28a745; color: white; }";
        html += ".retrocediendo { background: #dc3545; color: white; }";
        html += ".modo { text-align: center; padding: 15px; border-radius: 10px; font-size: 20px; font-weight: bold; margin: 10px 0; }";
        html += ".maestro { background: #007bff; color: white; }";
        html += ".esclavo { background: #6c757d; color: white; }";
        html += ".luces { text-align: center; padding: 15px; border-radius: 10px; font-size: 18px; font-weight: bold; margin: 10px 0; transition: all 0.3s; }";
        html += ".luces-on { background: #ffd700; color: #000; }";
        html += ".luces-off { background: #333; color: #999; }";
        html += ".btn { display: block; width: 100%; padding: 15px; margin: 10px 0; font-size: 18px; font-weight: bold; border: none; border-radius: 5px; cursor: pointer; transition: all 0.3s; }";
        html += ".btn-maestro { background: #007bff; color: white; }";
        html += ".btn-maestro:hover { background: #0056b3; }";
        html += ".btn-esclavo { background: #6c757d; color: white; }";
        html += ".btn-esclavo:hover { background: #545b62; }";
        html += ".btn-auto { background: #28a745; color: white; }";
        html += ".btn-auto:hover { background: #218838; }";
        html += ".btn-manual { background: #ffc107; color: black; }";
        html += ".btn-manual:hover { background: #e0a800; }";
        html += ".btn-luces { background: #ffd700; color: black; }";
        html += ".btn-luces:hover { background: #ffed4e; }";
        html += ".modo-control { display: flex; gap: 10px; }";
        html += ".modo-control .btn { flex: 1; }";
        html += ".sensores-activos { font-size: 14px; color: #666; margin: 5px 0; }";
        html += ".sensor-activo { display: inline-block; margin: 5px 8px; padding: 5px 10px; background: #28a745; color: white; border-radius: 15px; font-size: 12px; }";
        html += ".sensor-inactivo { display: inline-block; margin: 5px 8px; padding: 5px 10px; background: #ccc; color: #666; border-radius: 15px; font-size: 12px; }";
        html += ".estadistica { display: inline-block; margin: 5px; padding: 8px 12px; background: #007bff; color: white; border-radius: 5px; font-size: 14px; font-weight: bold; }";
        html += "</style>";
        html += "<script>";
        html += "function actualizarDatos() {";
        html += "  fetch('/datos').then(r => r.json()).then(data => {";
        html += "    document.getElementById('distancia').innerHTML = data.distancia.toFixed(1) + ' <span class=\"unit\">cm</span>';";
        html += "    var origenTexto = '';";
        html += "    if (data.origenDatos === 'LOCAL') { origenTexto = ' 📡'; }";
        html += "    else if (data.origenDatos === 'REMOTO') { origenTexto = ' 📶'; }";
        html += "    else { origenTexto = ' ❌'; }";
        html += "    document.getElementById('temperatura').innerHTML = data.temperatura.toFixed(1) + ' <span class=\"unit\">°C</span>' + origenTexto;";
        html += "    document.getElementById('luz').innerHTML = (data.luz ? '☀️ Detectada' : '🌙 Oscuro') + origenTexto;";
        html += "    var estadoDiv = document.getElementById('estado');";
        html += "    estadoDiv.className = 'card estado ' + data.estado.toLowerCase();";
        html += "    var icono = data.estado === 'AVANZANDO' ? '⬆️' : (data.estado === 'RETROCEDIENDO' ? '⬇️' : '⏸️');";
        html += "    estadoDiv.innerHTML = icono + ' ' + data.estado;";
        html += "    var modoDiv = document.getElementById('modo');";
        html += "    modoDiv.className = 'card modo ' + (data.modo === 'MAESTRO' ? 'maestro' : 'esclavo');";
        html += "    modoDiv.innerHTML = (data.modo === 'MAESTRO' ? '👑 ' : '🤖 ') + data.modo;";
        html += "    var autoDiv = document.getElementById('modoAuto');";
        html += "    autoDiv.innerHTML = (data.automatico ? '🤖 AUTOMÁTICO' : '🎮 MANUAL');";
        html += "    if (data.lucesDisponibles) {";
        html += "      document.getElementById('seccionLuces').style.display = 'block';";
        html += "      var lucesDiv = document.getElementById('estadoLuces');";
        html += "      lucesDiv.className = 'card luces ' + (data.lucesEncendidas ? 'luces-on' : 'luces-off');";
        html += "      lucesDiv.innerHTML = (data.lucesEncendidas ? '💡 LUCES ENCENDIDAS' : '🌑 LUCES APAGADAS');";
        html += "      document.getElementById('btnLucesAuto').innerHTML = (data.lucesAutomaticas ? '🤖 LUCES AUTO' : '🎮 LUCES MANUAL');";
        html += "    } else {";
        html += "      document.getElementById('seccionLuces').style.display = 'none';";
        html += "    }";
        html += "    var sensoresHTML = '';";
        html += "    sensoresHTML += data.sensorUltrasonico ? '<span class=\"sensor-activo\">📏 HC-SR04</span>' : '<span class=\"sensor-inactivo\">📏 HC-SR04</span>';";
        html += "    sensoresHTML += data.sensorTemperatura ? '<span class=\"sensor-activo\">🌡️ LM35</span>' : '<span class=\"sensor-inactivo\">🌡️ LM35</span>';";
        html += "    sensoresHTML += data.sensorLuminosidad ? '<span class=\"sensor-activo\">💡 LM393</span>' : '<span class=\"sensor-inactivo\">💡 LM393</span>';";
        html += "    sensoresHTML += data.sensorLuces ? '<span class=\"sensor-activo\">💡 LED</span>' : '<span class=\"sensor-inactivo\">💡 LED</span>';";
        html += "    document.getElementById('sensoresActivos').innerHTML = sensoresHTML;";
        html += "  });";
        html += "}";
        html += "function cambiarModo(maestro) {";
        html += "  fetch('/modo?maestro=' + (maestro ? '1' : '0')).then(() => setTimeout(actualizarDatos, 500));";
        html += "}";
        html += "function toggleAutomatico() {";
        html += "  fetch('/automatico').then(() => setTimeout(actualizarDatos, 500));";
        html += "}";
        html += "function toggleLuces() {";
        html += "  fetch('/luces/toggle').then(() => setTimeout(actualizarDatos, 200));";
        html += "}";
        html += "function toggleLucesAuto() {";
        html += "  fetch('/luces/auto').then(() => setTimeout(actualizarDatos, 200));";
        html += "}";
        html += "setInterval(actualizarDatos, 500);";
        html += "window.onload = actualizarDatos;";
        html += "</script>";
        html += "</head><body>";
        html += "<h1>🚗 Control Coche Robot</h1>";
        html += "<div class='card modo maestro' id='modo'>👑 MAESTRO</div>";
        html += "<div class='card'>";
        html += "<div class='modo-control'>";
        html += "<button class='btn btn-maestro' onclick='cambiarModo(true)'>👑 Maestro</button>";
        html += "<button class='btn btn-esclavo' onclick='cambiarModo(false)'>🤖 Esclavo</button>";
        html += "</div>";
        html += "<button class='btn btn-auto' id='modoAuto' onclick='toggleAutomatico()'>🤖 AUTOMÁTICO</button>";
        html += "</div>";
        html += "<div id='seccionLuces' style='display:none;'>";
        html += "<div class='card luces luces-off' id='estadoLuces'>🌑 LUCES APAGADAS</div>";
        html += "<div class='card'>";
        html += "<button class='btn btn-luces' onclick='toggleLuces()'>💡 ON/OFF Luces</button>";
        html += "<button class='btn btn-auto' id='btnLucesAuto' onclick='toggleLucesAuto()'>🤖 LUCES AUTO</button>";
        html += "</div>";
        html += "</div>";
        html += "<div class='card estado parado' id='estado'>⏸️ PARADO</div>";
        html += "<div class='card'>";
        html += "<div class='sensor'><span class='sensor-label'>📏 Distancia:</span><span class='sensor-value' id='distancia'>-- cm</span></div>";
        html += "<div class='sensor'><span class='sensor-label'>🌡️ Temperatura:</span><span class='sensor-value' id='temperatura'>-- °C</span></div>";
        html += "<div class='sensor'><span class='sensor-label'>💡 Luminosidad:</span><span class='sensor-value' id='luz'>--</span></div>";
        html += "</div>";
        html += "<div class='card'>";
        html += "<h3 style='margin-top:0; color:#333;'>🔧 Sensores Activos</h3>";
        html += "<div class='sensores-activos' id='sensoresActivos'>Cargando...</div>";
        html += "</div>";
        html += "</body></html>";
        servidor->send(200, "text/html", html);
    });
    
    // Ruta para obtener datos en JSON
    servidor->on("/datos", [this]() {
        servidor->send(200, "application/json", obtenerDatosJSON());
    });
    
    // Ruta para cambiar modo maestro/esclavo
    servidor->on("/modo", [this]() {
        if (servidor->hasArg("maestro")) {
            bool nuevoModo = (servidor->arg("maestro") == "1");
            cambiarModo(nuevoModo);
            servidor->send(200, "text/plain", "Modo cambiado");
        } else {
            servidor->send(400, "text/plain", "Parámetro incorrecto");
        }
    });
    
    // Ruta para toggle modo automático/manual
    servidor->on("/automatico", [this]() {
        modoAutomatico = !modoAutomatico;
        if (!modoAutomatico) {
            detener();  // Detener al cambiar a manual
        }
        servidor->send(200, "text/plain", modoAutomatico ? "Automático" : "Manual");
    });
    
    // Ruta para toggle luces ON/OFF
    servidor->on("/luces/toggle", [this]() {
        toggleLuces();
        servidor->send(200, "text/plain", estadoLuces ? "Luces encendidas" : "Luces apagadas");
    });
    
    // Ruta para toggle modo automático de luces
    servidor->on("/luces/auto", [this]() {
        lucesAutomaticas = !lucesAutomaticas;
        if (!lucesAutomaticas) {
            apagarLuces();  // Apagar al desactivar automático
        }
        servidor->send(200, "text/plain", lucesAutomaticas ? "Luces automáticas" : "Luces manuales");
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
    // Actualizar lecturas usando los métodos que manejan sensores compartidos
    ultimaDistancia = leerDistancia();
    float tempActual = obtenerTemperaturaActual();
    int luzActual = obtenerLuminosidadActual();
    String origenDatos = obtenerOrigenDatos();
    
    String json = "{";
    json += "\"distancia\":" + String(ultimaDistancia, 2) + ",";
    json += "\"temperatura\":" + String(tempActual, 2) + ",";
    json += "\"luz\":" + String(luzActual) + ",";
    json += "\"estado\":\"" + estadoMovimiento + "\",";
    json += "\"modo\":\"" + obtenerModoTexto() + "\",";
    json += "\"automatico\":" + String(modoAutomatico ? "true" : "false") + ",";
    json += "\"lucesDisponibles\":" + String(pinLuces >= 0 ? "true" : "false") + ",";
    json += "\"lucesEncendidas\":" + String(estadoLuces ? "true" : "false") + ",";
    json += "\"lucesAutomaticas\":" + String(lucesAutomaticas ? "true" : "false") + ",";
    json += "\"tieneSensores\":" + String(tieneSensoresLocales ? "true" : "false") + ",";
    json += "\"origenDatos\":\"" + origenDatos + "\",";
    
    // Información de sensores activos
    json += "\"sensorUltrasonico\":" + String(trigPin >= 0 && echoPin >= 0 ? "true" : "false") + ",";
    json += "\"sensorTemperatura\":" + String(tempPin >= 0 ? "true" : "false") + ",";
    json += "\"sensorLuminosidad\":" + String(lightPin >= 0 ? "true" : "false") + ",";
    json += "\"sensorLuces\":" + String(pinLuces >= 0 ? "true" : "false") + ",";
    
    // Estadísticas ESP-NOW
    json += "\"mensajesEnviados\":" + String(mensajesEnviados) + ",";
    json += "\"mensajesRecibidos\":" + String(mensajesRecibidos) + ",";
    json += "\"mensajesFallidos\":" + String(mensajesFallidos) + ",";
    json += "\"tasaExito\":" + String(obtenerTasaExito(), 1);
    
    json += "}";
    return json;
}

// ========== FUNCIONES ESP-NOW ==========

// Variable global para instancia del coche (necesaria para callbacks)
static Coche* instanciaCocheGlobal = nullptr;

// Callback cuando se envía un mensaje ESP-NOW (datos)
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    if (instanciaCocheGlobal != nullptr) {
        bool exitoso = (sendStatus == 0);
        instanciaCocheGlobal->registrarACK(exitoso);
        
        if (exitoso) {
            // Serial.println("✓ ACK: Mensaje entregado");
        } else {
            Serial.println("✗ FALLO: Mensaje no entregado");
        }
    }
}

// Callback cuando se recibe un mensaje ESP-NOW
void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, uint8_t len) {
    if (instanciaCocheGlobal != nullptr) {
        // Verificar tipo de mensaje según tamaño
        if (len == sizeof(struct_mensaje)) {
            struct_mensaje mensaje;
            memcpy(&mensaje, incomingData, sizeof(mensaje));
            instanciaCocheGlobal->procesarComandoRecibido(&mensaje);
        } else if (len == sizeof(struct_control)) {
            struct_control control;
            memcpy(&control, incomingData, sizeof(control));
            instanciaCocheGlobal->procesarControlRecibido(&control);
        } else if (len == sizeof(struct_respuesta)) {
            struct_respuesta respuesta;
            memcpy(&respuesta, incomingData, sizeof(respuesta));
            instanciaCocheGlobal->procesarRespuestaSensores(&respuesta);
        }
    }
}

// Inicializar ESP-NOW en modo dual (puede ser maestro o esclavo)
void Coche::inicializarESPNowDual(uint8_t macOtroCoche[6], bool empezarComoMaestro) {
    instanciaCocheGlobal = this;
    esMaestro = empezarComoMaestro;
    memcpy(macRemota, macOtroCoche, 6);
    
    // Configurar WiFi en modo estación
    WiFi.mode(WIFI_STA);
    
    // Inicializar ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("Error inicializando ESP-NOW");
        espnowInicializado = false;
        return;
    }
    
    espnowInicializado = true;
    Serial.println("ESP-NOW inicializado en modo DUAL");
    Serial.print("Mi MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Modo inicial: ");
    Serial.println(esMaestro ? "MAESTRO" : "ESCLAVO");
    
    // Configurar rol inicial
    esp_now_set_self_role(esMaestro ? ESP_NOW_ROLE_COMBO : ESP_NOW_ROLE_COMBO);
    
    // Registrar callbacks
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    
    // Agregar peer (otro coche)
    esp_now_add_peer(macRemota, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    
    Serial.print("MAC del otro coche: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", macRemota[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
}

// Cambiar modo maestro/esclavo dinámicamente
void Coche::cambiarModo(bool nuevoModoMaestro) {
    if (!espnowInicializado) return;
    
    String modoAnterior = esMaestro ? "MAESTRO" : "ESCLAVO";
    esMaestro = nuevoModoMaestro;
    String modoNuevo = esMaestro ? "MAESTRO" : "ESCLAVO";
    
    Serial.print("Cambiando a modo: ");
    Serial.println(modoNuevo);
    
    // Registrar cambio de modo en el log
    agregarLog("MODO", modoAnterior + " → " + modoNuevo);
    
    // Si cambio a esclavo, detener motores
    if (!esMaestro) {
        detener();
        estadoMovimiento = "PARADO";
    }
    
    // Notificar al otro coche que cambie al modo contrario
    enviarCambioModo(nuevoModoMaestro);
}

// Enviar comando de cambio de modo al otro coche
void Coche::enviarCambioModo(bool yoSoyMaestro) {
    if (!espnowInicializado) return;
    
    struct_control control;
    strcpy(control.tipoComando, "CAMBIAR_MODO");
    strcpy(control.nuevoModo, yoSoyMaestro ? "ESCLAVO" : "MAESTRO");
    
    esp_now_send(macRemota, (uint8_t*)&control, sizeof(control));
    
    Serial.println("Comando de cambio de modo enviado");
}

// Procesar comando de control recibido
void Coche::procesarControlRecibido(struct_control* datos) {
    if (strcmp(datos->tipoComando, "CAMBIAR_MODO") == 0) {
        bool nuevoModo = (strcmp(datos->nuevoModo, "MAESTRO") == 0);
        
        Serial.print("Recibido cambio de modo a: ");
        Serial.println(nuevoModo ? "MAESTRO" : "ESCLAVO");
        
        String modoAnterior = esMaestro ? "MAESTRO" : "ESCLAVO";
        esMaestro = nuevoModo;
        String modoNuevo = esMaestro ? "MAESTRO" : "ESCLAVO";
        
        // Registrar cambio en el log
        agregarLog("MODO", "Remoto: " + modoAnterior + " → " + modoNuevo);
        
        // Si cambio a esclavo, detener
        if (!esMaestro) {
            detener();
            estadoMovimiento = "PARADO";
        }
    }
}

// Enviar comando ESP-NOW (solo maestro)
void Coche::enviarComandoESPNow() {
    if (!esMaestro || !espnowInicializado) return;
    
    // Control de flujo: No enviar si está esperando ACK o no ha pasado 100ms
    if (!puedeEnviar()) return;
    
    struct_mensaje mensaje;
    mensaje.velocidadIzq = ultimaVelocidadIzq;
    mensaje.velocidadDer = ultimaVelocidadDer;
    strcpy(mensaje.comando, estadoMovimiento.c_str());
    
    // Añadir datos de sensores si tenemos sensores locales
    mensaje.tieneSensores = tieneSensoresLocales;
    if (tieneSensoresLocales) {
        mensaje.temperatura = leerTemperatura();
        mensaje.luminosidad = leerLuz();
    } else {
        // Si no tenemos sensores, enviar valores inválidos
        mensaje.temperatura = -999;
        mensaje.luminosidad = -1;
    }
    
    // Marcar que estamos esperando ACK
    esperandoACK = true;
    ultimoEnvio = millis();
    
    // Enviar mensaje
    esp_now_send(macRemota, (uint8_t*)&mensaje, sizeof(mensaje));
    
    // Registrar en el log
    mensajesEnviados++;
    String detalle = estadoMovimiento + " V:" + String(ultimaVelocidadIzq) + "," + String(ultimaVelocidadDer);
    if (tieneSensoresLocales) {
        detalle += " T:" + String(mensaje.temperatura, 1) + " L:" + String(mensaje.luminosidad);
    }
    agregarLog("ENVIO", detalle);
}

// Procesar comando recibido (solo esclavo)
void Coche::procesarComandoRecibido(struct_mensaje* datos) {
    if (esMaestro) return; // Solo el esclavo procesa comandos de movimiento
    
    // Aplicar las velocidades recibidas directamente
    moverMotores(datos->velocidadIzq, datos->velocidadDer);
    
    // Actualizar estado
    estadoMovimiento = String(datos->comando);
    
    // Almacenar datos de sensores recibidos si el otro coche tiene sensores
    if (datos->tieneSensores) {
        temperaturaRemota = datos->temperatura;
        luminosidadRemota = datos->luminosidad;
        datosRemotosValidos = true;
        ultimosDatosRemotos = millis();
    }
    
    // Registrar en el log
    mensajesRecibidos++;
    String detalle = String(datos->comando) + " V:" + String(datos->velocidadIzq) + "," + String(datos->velocidadDer);
    if (datos->tieneSensores) {
        detalle += " T:" + String(datos->temperatura, 1) + " L:" + String(datos->luminosidad);
    }
    agregarLog("RECEP", detalle);
    
    // COMUNICACIÓN BIDIRECCIONAL: El esclavo responde con sus sensores
    enviarRespuestaSensores();
}

// Obtener modo actual
bool Coche::obtenerModo() {
    return esMaestro;
}

// Obtener modo como texto
String Coche::obtenerModoTexto() {
    return esMaestro ? "MAESTRO" : "ESCLAVO";
}

// Configurar modo automático/manual
void Coche::setModoAutomatico(bool automatico) {
    modoAutomatico = automatico;
    if (!automatico) {
        detener();
    }
}

// Obtener modo automático
bool Coche::obtenerModoAutomatico() {
    return modoAutomatico;
}

// Obtener dirección MAC
uint8_t* Coche::obtenerMAC() {
    static uint8_t mac[6];
    WiFi.macAddress(mac);
    return mac;
}

// ========== FUNCIONES DE CONTROL DE LUCES ==========

// Control automático de luces según sensor de luminosidad
void Coche::controlarLucesAutomaticas() {
    if (!lucesAutomaticas || pinLuces < 0) return;
    
    // Obtener lectura de luminosidad (local o remota)
    int lecturaLuz = obtenerLuminosidadActual();
    
    // Si no hay datos válidos, no hacer nada
    if (lecturaLuz < 0) return;
    
    // Si está oscuro (0) y las luces están apagadas, encenderlas
    if (lecturaLuz == 0 && !estadoLuces) {
        encenderLuces();
    }
    // Si hay luz (1) y las luces están encendidas, apagarlas
    else if (lecturaLuz == 1 && estadoLuces) {
        apagarLuces();
    }
}

// Encender luces
void Coche::encenderLuces() {
    if (pinLuces >= 0) {
        digitalWrite(pinLuces, HIGH);
        estadoLuces = true;
    }
}

// Apagar luces
void Coche::apagarLuces() {
    if (pinLuces >= 0) {
        digitalWrite(pinLuces, LOW);
        estadoLuces = false;
    }
}

// Alternar estado de luces
void Coche::toggleLuces() {
    if (estadoLuces) {
        apagarLuces();
    } else {
        encenderLuces();
    }
}

// Configurar modo automático de luces
void Coche::setLucesAutomaticas(bool automatico) {
    lucesAutomaticas = automatico;
    if (!automatico) {
        // Al desactivar automático, apagar luces
        apagarLuces();
    }
}

// Obtener estado de luces
bool Coche::obtenerEstadoLuces() {
    return estadoLuces;
}

// Obtener modo automático de luces
bool Coche::obtenerLucesAutomaticas() {
    return lucesAutomaticas;
}

// ========== FUNCIONES DE GESTIÓN DE SENSORES COMPARTIDOS ==========

// Verificar si este coche tiene sensores físicos conectados
bool Coche::tieneSensores() {
    return tieneSensoresLocales;
}

// Obtener temperatura actual (local si está disponible, sino remota)
float Coche::obtenerTemperaturaActual() {
    if (tieneSensoresLocales) {
        return leerTemperatura();
    } else if (datosRemotosValidos) {
        // Verificar que los datos remotos no sean muy antiguos (>5 segundos)
        if (millis() - ultimosDatosRemotos < 5000) {
            return temperaturaRemota;
        }
    }
    return -999; // Valor inválido
}

// Obtener luminosidad actual (local si está disponible, sino remota)
int Coche::obtenerLuminosidadActual() {
    if (tieneSensoresLocales) {
        return leerLuz();
    } else if (datosRemotosValidos) {
        // Verificar que los datos remotos no sean muy antiguos (>5 segundos)
        if (millis() - ultimosDatosRemotos < 5000) {
            return luminosidadRemota;
        }
    }
    return -1; // Valor inválido
}

// Obtener origen de datos de sensores
String Coche::obtenerOrigenDatos() {
    if (tieneSensoresLocales) {
        return "LOCAL";
    } else if (datosRemotosValidos && (millis() - ultimosDatosRemotos < 5000)) {
        return "REMOTO";
    }
    return "SIN_DATOS";
}

// ========== FUNCIONES DE LOG ==========

// Agregar entrada al log (solo Serial para depuración)
void Coche::agregarLog(String tipo, String detalle) {
    unsigned long ahora = millis();
    float timestamp = (float)ahora / 1000.0;
    Serial.print(timestamp, 3);
    Serial.print("s ");
    Serial.print(tipo);
    Serial.print(": ");
    Serial.println(detalle);
}

// Obtener contador de mensajes enviados
unsigned long Coche::obtenerMensajesEnviados() {
    return mensajesEnviados;
}

// Obtener contador de mensajes recibidos
unsigned long Coche::obtenerMensajesRecibidos() {
    return mensajesRecibidos;
}

// Obtener contador de mensajes fallidos
unsigned long Coche::obtenerMensajesFallidos() {
    return mensajesFallidos;
}

// Obtener tasa de éxito de mensajes (%)
float Coche::obtenerTasaExito() {
    unsigned long total = mensajesEnviados + mensajesFallidos;
    if (total == 0) return 100.0;
    return (float)mensajesEnviados / total * 100.0;
}

// Verificar si puede enviar mensaje (ACK recibido + throttling 100ms)
bool Coche::puedeEnviar() {
    // No puede enviar si está esperando ACK
    if (esperandoACK) return false;
    
    // No puede enviar si no han pasado 100ms desde el último envío (throttling)
    if (millis() - ultimoEnvio < 100) return false;
    
    return true;
}

// Registrar ACK de envío
void Coche::registrarACK(bool exitoso) {
    esperandoACK = false;  // Liberar bloqueo
    
    if (exitoso) {
        // ACK exitoso, no hacer nada adicional
    } else {
        mensajesFallidos++;
        agregarLog("ERROR", "Envío fallido");
    }
}

// Esclavo envía respuesta con sus datos de sensores
void Coche::enviarRespuestaSensores() {
    if (esMaestro || !espnowInicializado) return;  // Solo el esclavo envía respuestas
    if (!tieneSensoresLocales) return;  // Solo si tiene sensores
    
    // Control de flujo: No enviar si está esperando ACK o no ha pasado 100ms
    if (!puedeEnviar()) return;
    
    struct_respuesta respuesta;
    respuesta.temperatura = leerTemperatura();
    respuesta.luminosidad = leerLuz();
    respuesta.tieneSensores = true;
    strcpy(respuesta.origen, "ESCLAVO");
    
    // Marcar que estamos esperando ACK
    esperandoACK = true;
    ultimoEnvio = millis();
    
    esp_now_send(macRemota, (uint8_t*)&respuesta, sizeof(respuesta));
    
    // Registrar envío de sensores
    String detalle = "T:" + String(respuesta.temperatura, 1) + " L:" + String(respuesta.luminosidad);
    agregarLog("RESP", detalle);
}

// Maestro procesa respuesta de sensores del esclavo
void Coche::procesarRespuestaSensores(struct_respuesta* datos) {
    if (!esMaestro) return;  // Solo el maestro procesa respuestas
    
    if (datos->tieneSensores) {
        temperaturaRemota = datos->temperatura;
        luminosidadRemota = datos->luminosidad;
        datosRemotosValidos = true;
        ultimosDatosRemotos = millis();
        
        // Registrar recepción
        String detalle = String(datos->origen) + " T:" + String(datos->temperatura, 1) + " L:" + String(datos->luminosidad);
        agregarLog("RESP_RX", detalle);
    }
}