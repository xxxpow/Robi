#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Config.h"

typedef void (*TrackRequestCallback)(int question);

class WifiServerModule {
public:
  WifiServerModule();

  void init();
  void handleClient();
  void setTrackCallback(TrackRequestCallback cb);

private:
  WebServer _server;
  TrackRequestCallback _trackCallback;

  void handleRoot();
  void handlePista();
};
