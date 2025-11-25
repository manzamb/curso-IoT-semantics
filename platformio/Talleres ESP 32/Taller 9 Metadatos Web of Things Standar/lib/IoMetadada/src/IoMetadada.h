#pragma once
#include <Arduino.h>
#include <functional>
#include <WebServer.h>
#include <WiFi.h>

struct IoTServiceState {
  float luminosity;
  float temperature;
  bool lightOn;
  bool fanOn;
  int lightThreshold;
  int fanThreshold;
  bool lightManual;
  bool fanManual;
};

class IoMetadada {
 public:
  IoMetadada();
  void begin(WebServer& webServer);
  void registerApi(WebServer& webServer,
                   const std::function<String()>& buildStatus,
                   const std::function<void()>& lightOn,
                   const std::function<void()>& lightOff,
                   const std::function<void()>& lightAuto,
                   const std::function<void()>& fanOn,
                   const std::function<void()>& fanOff,
                   const std::function<void()>& fanAuto,
                   const std::function<void(int)>& setLightThreshold,
                   const std::function<void(int)>& setFanThreshold);
  void updateState(const IoTServiceState& newState, const IPAddress& ip);
  String thingDescription() const;
  String behaviorDescription() const;
  String affordancesDescription() const;

 private:
  String baseUrl() const;
  String buildPropertyBlock(const String& id,
                            const String& title,
                            const String& description,
                            const String& schema,
                            const String& href) const;
  String buildActionBlock(const String& id,
                          const String& title,
                          const String& description,
                          const String& inputSchema,
                          const String& forms) const;

  WebServer* server;
  IoTServiceState state;
  IPAddress deviceIp;
};
