#include "IoMetadada.h"

IoMetadada::IoMetadada() : server(nullptr), state({0, 0, false, false, 0, 0, false, false}) {}

void IoMetadada::begin(WebServer& webServer) {
  server = &webServer;
  Serial.println("Registrando rutas WoT (/wot/td, /wot/behavior, /wot/affordances)");

  server->on("/wot/td", HTTP_ANY, [this]() {
    server->send(200, "application/td+json", thingDescription());
  });
  server->on("/wot/td/", HTTP_ANY, [this]() {
    server->send(200, "application/td+json", thingDescription());
  });

  server->on("/wot/behavior", HTTP_ANY, [this]() {
    server->send(200, "application/json", behaviorDescription());
  });
  server->on("/wot/behavior/", HTTP_ANY, [this]() {
    server->send(200, "application/json", behaviorDescription());
  });

  server->on("/wot/affordances", HTTP_ANY, [this]() {
    server->send(200, "application/json", affordancesDescription());
  });
  server->on("/wot/affordances/", HTTP_ANY, [this]() {
    server->send(200, "application/json", affordancesDescription());
  });
}

void IoMetadada::registerApi(WebServer& webServer,
                             const std::function<String()>& buildStatus,
                             const std::function<void()>& lightOn,
                             const std::function<void()>& lightOff,
                             const std::function<void()>& lightAuto,
                             const std::function<void()>& fanOn,
                             const std::function<void()>& fanOff,
                             const std::function<void()>& fanAuto,
                             const std::function<void(int)>& setLightThreshold,
                             const std::function<void(int)>& setFanThreshold) {
  server = &webServer;

  server->on("/api/status", HTTP_GET, [this, buildStatus]() {
    String json = buildStatus ? buildStatus() : "{}";
    server->send(200, "application/json", json);
  });

  server->on("/api/light/on", HTTP_GET, [this, lightOn, buildStatus]() {
    if (lightOn) lightOn();
    server->send(200, "application/json", buildStatus ? buildStatus() : "{}");
  });
  server->on("/api/light/off", HTTP_GET, [this, lightOff, buildStatus]() {
    if (lightOff) lightOff();
    server->send(200, "application/json", buildStatus ? buildStatus() : "{}");
  });
  server->on("/api/light/auto", HTTP_GET, [this, lightAuto, buildStatus]() {
    if (lightAuto) lightAuto();
    server->send(200, "application/json", buildStatus ? buildStatus() : "{}");
  });
  server->on("/api/light/threshold", HTTP_GET, [this, setLightThreshold, buildStatus]() {
    if (setLightThreshold && server->hasArg("value")) {
      setLightThreshold(server->arg("value").toInt());
    }
    server->send(200, "application/json", buildStatus ? buildStatus() : "{}");
  });

  server->on("/api/fan/on", HTTP_GET, [this, fanOn, buildStatus]() {
    if (fanOn) fanOn();
    server->send(200, "application/json", buildStatus ? buildStatus() : "{}");
  });
  server->on("/api/fan/off", HTTP_GET, [this, fanOff, buildStatus]() {
    if (fanOff) fanOff();
    server->send(200, "application/json", buildStatus ? buildStatus() : "{}");
  });
  server->on("/api/fan/auto", HTTP_GET, [this, fanAuto, buildStatus]() {
    if (fanAuto) fanAuto();
    server->send(200, "application/json", buildStatus ? buildStatus() : "{}");
  });
  server->on("/api/fan/threshold", HTTP_GET, [this, setFanThreshold, buildStatus]() {
    if (setFanThreshold && server->hasArg("value")) {
      setFanThreshold(server->arg("value").toInt());
    }
    server->send(200, "application/json", buildStatus ? buildStatus() : "{}");
  });
}

void IoMetadada::updateState(const IoTServiceState& newState, const IPAddress& ip) {
  state = newState;
  deviceIp = ip;
}

String IoMetadada::baseUrl() const {
  String ipText = deviceIp.toString();
  if (ipText == "0.0.0.0" || ipText.length() == 0) {
    return "http://esp32.local";
  }
  return "http://" + ipText;
}

String IoMetadada::buildPropertyBlock(const String& id,
                                      const String& title,
                                      const String& description,
                                      const String& schema,
                                      const String& href) const {
  String block = "\"" + id + "\":{";
  block += "\"title\":\"" + title + "\",";
  block += "\"description\":\"" + description + "\",";
  block += "\"type\":\"object\",";
  block += "\"properties\":" + schema + ",";
  block += "\"forms\":[{\"href\":\"" + href + "\",\"op\":[\"readproperty\"],\"contentType\":\"application/json\"}]";
  block += "}";
  return block;
}

String IoMetadada::buildActionBlock(const String& id,
                                    const String& title,
                                    const String& description,
                                    const String& inputSchema,
                                    const String& forms) const {
  String block = "\"" + id + "\":{";
  block += "\"title\":\"" + title + "\",";
  block += "\"description\":\"" + description + "\",";
  block += "\"input\":" + inputSchema + ",";
  block += "\"forms\":" + forms;
  block += "}";
  return block;
}

String IoMetadada::thingDescription() const {
  String base = baseUrl();
  String td = "{";
  td += "\"@context\":[\"https://www.w3.org/2019/wot/td/v1\"],";
  td += "\"id\":\"urn:dev:ops:esp32:wemos-d1r32:smart-object\",";
  td += "\"title\":\"SmartRoomGateway\",";
  td += "\"description\":\"ESP32 exposes WoT metadata for light and temperature regulators\",";
  td += "\"base\":\"" + base + "\",";
  td += "\"securityDefinitions\":{\"nosec_sc\":{\"scheme\":\"nosec\"}},";
  td += "\"security\":[\"nosec_sc\"],";

  // --- Interaction affordances: properties ---
  td += "\"properties\":{";

  String lightSchema = "{"
                       "\"luminosity\":{\"type\":\"number\",\"unit\":\"volt\",\"readOnly\":true,\"minimum\":0,\"maximum\":4095},"
                       "\"mode\":{\"type\":\"string\",\"enum\":[\"AUTO\",\"MANUAL\"]},"
                       "\"isOn\":{\"type\":\"boolean\"},"
                       "\"threshold\":{\"type\":\"integer\",\"minimum\":0,\"maximum\":4095}"
                       "}";
  td += buildPropertyBlock("lightStatus",
                           "Light Status",
                           "Current LDR reading and bulb state",
                           lightSchema,
                           "/api/status");
  td += ",";

  String tempSchema = "{"
                      "\"temperature\":{\"type\":\"number\",\"unit\":\"celsius\",\"readOnly\":true},"
                      "\"mode\":{\"type\":\"string\",\"enum\":[\"AUTO\",\"MANUAL\"]},"
                      "\"isOn\":{\"type\":\"boolean\"},"
                      "\"threshold\":{\"type\":\"integer\"}"
                      "}";
  td += buildPropertyBlock("temperatureStatus",
                           "Temperature Status",
                           "Current ambient temperature and fan state",
                           tempSchema,
                           "/api/status");
  td += "},";

  // --- Interaction affordances: actions ---
  td += "\"actions\":{";
  String lightThresholdForms = "[{\"href\":\"/api/light/threshold\",\"op\":[\"invokeaction\"],\"contentType\":\"application/json\"}]";
  td += buildActionBlock("setLightThreshold",
                         "Set light threshold",
                         "Update the lux threshold that triggers the bulb",
                         "{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\",\"minimum\":0,\"maximum\":4095}}}",
                         lightThresholdForms);
  td += ",";
  String fanThresholdForms = "[{\"href\":\"/api/fan/threshold\",\"op\":[\"invokeaction\"],\"contentType\":\"application/json\"}]";
  td += buildActionBlock("setFanThreshold",
                         "Set fan threshold",
                         "Update the temperature threshold that triggers the fan",
                         "{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\",\"minimum\":0,\"maximum\":60}}}",
                         fanThresholdForms);
  td += ",";
  String switchLightForms = "[{\"href\":\"/api/light/on\",\"op\":[\"invokeaction\"],\"contentType\":\"application/json\"},{\"href\":\"/api/light/off\",\"op\":[\"invokeaction\"],\"contentType\":\"application/json\"},{\"href\":\"/api/light/auto\",\"op\":[\"invokeaction\"],\"contentType\":\"application/json\"}]";
  td += buildActionBlock("switchLight",
                         "Switch light",
                         "Turn the bulb on/off or return to automatic mode",
                         "{\"type\":\"object\",\"properties\":{\"mode\":{\"type\":\"string\",\"enum\":[\"on\",\"off\",\"auto\"]}}}",
                         switchLightForms);
  td += ",";
  String switchFanForms = "[{\"href\":\"/api/fan/on\",\"op\":[\"invokeaction\"],\"contentType\":\"application/json\"},{\"href\":\"/api/fan/off\",\"op\":[\"invokeaction\"],\"contentType\":\"application/json\"},{\"href\":\"/api/fan/auto\",\"op\":[\"invokeaction\"],\"contentType\":\"application/json\"}]";
  td += buildActionBlock("switchFan",
                         "Switch fan",
                         "Turn the fan on/off or return to automatic mode",
                         "{\"type\":\"object\",\"properties\":{\"mode\":{\"type\":\"string\",\"enum\":[\"on\",\"off\",\"auto\"]}}}",
                         switchFanForms);
  td += "},";

  // --- Interaction affordances: events (examples for behavior) ---
  td += "\"events\":{";
  td += "\"lightThresholdBreached\":{";
  td += "\"description\":\"Emitted when luminosity falls below threshold while in AUTO\",";
  td += "\"data\":{\"type\":\"object\",\"properties\":{\"luminosity\":{\"type\":\"number\"},\"threshold\":{\"type\":\"integer\"}}},";
  td += "\"forms\":[{\"href\":\"/api/status\",\"subprotocol\":\"longpoll\",\"contentType\":\"application/json\"}]";
  td += "},";
  td += "\"temperatureThresholdBreached\":{";
  td += "\"description\":\"Emitted when temperature rises above threshold while in AUTO\",";
  td += "\"data\":{\"type\":\"object\",\"properties\":{\"temperature\":{\"type\":\"number\"},\"threshold\":{\"type\":\"integer\"}}},";
  td += "\"forms\":[{\"href\":\"/api/status\",\"subprotocol\":\"longpoll\",\"contentType\":\"application/json\"}]";
  td += "}";
  td += "},";

  // --- Custom behavior summary ---
  td += "\"links\":[{\"rel\":\"behavior\",\"href\":\"/wot/behavior\",\"type\":\"application/json\"}],";
  td += "\"_behavior\":{\"light\":\"If luminosity is below threshold and mode is AUTO the bulb turns on, otherwise it is off.\",";
  td += "\"temperature\":\"If ambient temperature is above threshold and mode is AUTO the fan turns on, otherwise it is off.\"},";

  // Protocol binding examples in forms already, but keep extra reference
  td += "\"_protocolBinding\":\"HTTP forms bound to ESP32 WebServer endpoints for properties/actions/events\"";
  td += "}";
  return td;
}

String IoMetadada::behaviorDescription() const {
  String json = "{";
  json += "\"lightRegulator\":{";
  json += "\"goal\":\"Turn bulb on when daylight is below threshold\",";
  json += "\"automaticRule\":\"if luminosity < threshold and mode=AUTO => bulb ON else OFF\",";
  json += "\"manualOverride\":\"/api/light/on|off|auto\",";
  json += "\"currentThreshold\":" + String(state.lightThreshold) + ",";
  json += "\"lastLuminosity\":" + String(state.luminosity);
  json += "},";
  json += "\"temperatureRegulator\":{";
  json += "\"goal\":\"Turn fan on when temperature rises above threshold\",";
  json += "\"automaticRule\":\"if temperature > threshold and mode=AUTO => fan ON else OFF\",";
  json += "\"manualOverride\":\"/api/fan/on|off|auto\",";
  json += "\"currentThreshold\":" + String(state.fanThreshold) + ",";
  json += "\"lastTemperature\":" + String(state.temperature);
  json += "}";
  json += "}";
  return json;
}

String IoMetadada::affordancesDescription() const {
  String json = "{";
  json += "\"properties\":[\"/api/status\"],";
  json += "\"actions\":[\"/api/light/on\",\"/api/light/off\",\"/api/light/auto\",\"/api/fan/on\",\"/api/fan/off\",\"/api/fan/auto\",\"/api/light/threshold\",\"/api/fan/threshold\"],";
  json += "\"events\":[\"lightThresholdBreached\",\"temperatureThresholdBreached\"],";
  json += "\"dataSchemas\":{";
  json += "\"light\":{\"luminosity\":\"number\",\"threshold\":\"integer\"},";
  json += "\"temperature\":{\"temperature\":\"number\",\"threshold\":\"integer\"}";
  json += "},";
  json += "\"security\":{\"scheme\":\"nosec\"},";
  json += "\"protocolBinding\":\"HTTP GET endpoints exposed by WebServer\"";
  json += "}";
  return json;
}
