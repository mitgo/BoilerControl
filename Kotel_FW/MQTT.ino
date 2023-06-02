//#include <WiFiClient.h>
#include <PubSubClient.h>           // Для MQTT

void onPowerCommand(bool state, HAHVAC* sender);
void onModeCommand(HAHVAC::Mode mode, HAHVAC* sender);

void MqttInit() {
  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));
  device.setName(MODULE_NAME);
  device.setManufacturer("Dmitriy Scherban");
  device.setModel("BoilerCtrl");
  device.setSoftwareVersion(MODULE_VERSION);

  //Assign Callbacks for HVAC  hvac.onTargetTemperatureCommand(onGetTargetTemp);
  hvac.onPowerCommand(onPowerCommand);
  hvac.onModeCommand(onModeCommand);
  hvac.setName("InsideClimate");
  hvac.setMinTemp(16);
  hvac.setMaxTemp(26);
  hvac.setTempStep(0.5);  

  HApodacha.setUnitOfMeasurement("°C");
  HApodacha.setDeviceClass("temperature");
  HApodacha.setIcon("mdi:thermometer");
  HApodacha.setName("Boiler: Podacha");

  HAobratka.setUnitOfMeasurement("°C");
  HAobratka.setDeviceClass("temperature");
  HAobratka.setIcon("mdi:thermometer");
  HAobratka.setName("Boiler: Obratka");

  HAinside.setUnitOfMeasurement("°C");
  HAinside.setDeviceClass("temperature");
  HAinside.setIcon("mdi:thermometer");
  HAinside.setName("Inside: Temp");

  HAoutside.setUnitOfMeasurement("°C");
  HAoutside.setDeviceClass("temperature");
  HAoutside.setIcon("mdi:thermometer");
  HAoutside.setName("Outside: Temp");

  HACirc.setUnitOfMeasurement("St");
  HACirc.setIcon("mdi:pump");
  HACirc.setName("Boiler: Circ");
  
  HATen.setUnitOfMeasurement("St");
  HATen.setIcon("mdi:air-filter");
  HATen.setName("Boiler: TEN");

  HATime.setUnitOfMeasurement("Sec");
  HATime.setIcon("mdi:timelapse");
  HATime.setName("Boiler: TimeHeatNight");

  HATimeM.setUnitOfMeasurement("Sec");
  HATimeM.setIcon("mdi:timelapse");
  HATimeM.setName("Boiler: TimeHeatMorning");

  HAEnergyNight.setUnitOfMeasurement("kWh");
  HAEnergyNight.setIcon("mdi:flash");
  HAEnergyNight.setName("Boiler: NihgtPower");

  HAEnergyMorning.setUnitOfMeasurement("kWh");
  HAEnergyMorning.setIcon("mdi:flash");
  HAEnergyMorning.setName("Boiler: MorningPower");
  
  IPAddress ip;
  ip.fromString(String(cfg_mqtt_server));
  bool a = mqtt.begin(ip, cfg_mqttport, cfg_userMqtt, cfg_passMqtt);
  if (a) dspLogOut("MQTT запущен");
  else dspLogOut("Проблемы с MQTT");
}
