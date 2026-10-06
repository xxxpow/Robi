#include "WifiServerModule.h"

WifiServerModule::WifiServerModule()
  : _server(WEB_SERVER_PORT),
    _trackCallback(nullptr) {}

void WifiServerModule::init() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID_NAME, WIFI_PASSWORD_KEY);

  IPAddress IP = WiFi.softAPIP();

  Serial.println();
  Serial.println("==============================");
  Serial.println(" WIFI ROBI SOFT-AP");
  Serial.print(" SSID: ");
  Serial.println(WIFI_SSID_NAME);
  Serial.print(" IP:   ");
  Serial.println(IP);
  Serial.println("==============================");

  _server.on("/", [this]() {
    handleRoot();
  });

  _server.on("/pista", [this]() {
    handlePista();
  });

  _server.begin();
  Serial.println("[WIFI] Servidor HTTP iniciado.");
}

void WifiServerModule::setTrackCallback(TrackRequestCallback cb) {
  _trackCallback = cb;
}

void WifiServerModule::handleClient() {
  _server.handleClient();
}

void WifiServerModule::handleRoot() {
  String pagina =
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Robi</title>"
    "</head>"
    "<body>"
    "<h1>ROBI</h1>"
    "<p>ESP32 conectado correctamente.</p>"
    "<p>IP: 192.168.4.1</p>"
    "</body>"
    "</html>";

  _server.send(200, "text/html", pagina);
}

void WifiServerModule::handlePista() {
  if (!_server.hasArg("p")) {
    _server.send(400, "text/plain", "Falta parametro p");
    return;
  }

  int pregunta = _server.arg("p").toInt();
  Serial.print("[HTTP] Pregunta recibida desde cliente: ");
  Serial.println(pregunta);

  if (_trackCallback != nullptr) {
    _trackCallback(pregunta);
  }

  _server.send(200, "text/plain", "OK");
}
