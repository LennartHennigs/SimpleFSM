/////////////////////////////////////////////////////////////////

#include "SimpleFSM.h"
#include "Button2.h"    // https://github.com/LennartHennigs/Button2

#if defined(ARDUINO_ARCH_ESP32)
    #include <WiFi.h>
    #include <WiFiClient.h>
    #include <WebServer.h>
#elif defined(ARDUINO_ARCH_ESP8266)
    #include <ESP8266WiFi.h>
    #include <WiFiClient.h>
    #include <ESP8266WebServer.h>
#else 
    #error "you need an ESP32 or ESP8266"
#endif

/////////////////////////////////////////////////////////////////

#define BUTTON_PIN  39

#define WIFI_SSID    "Zeltlager"
#define WIFI_PASSWD  "HeerSZ7!D32015"

/////////////////////////////////////////////////////////////////

SimpleFSM fsm;
Button2 btn;

#if defined(ARDUINO_ARCH_ESP32)
  WebServer server(80);
#else
  ESP8266WebServer server(80);
#endif

/////////////////////////////////////////////////////////////////

bool connectToWiFi(char* ssid, char* passwd, int timeout_after =-1) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwd);
  // Wait for connection
  long now = millis();
  while ((WiFi.status() != WL_CONNECTED) && ((timeout_after > -1) && (millis() < (now + timeout_after)))) {
    delay(500);
  }
  return (WiFi.status() == WL_CONNECTED);
}

/////////////////////////////////////////////////////////////////

void on_red() {
  Serial.println("\nState: RED");
}
 
void on_green() {
  Serial.println("\nState: GREEN");
}

void ongoing() {
  Serial.print(".");
}

void on_button_press() {
  Serial.println("\nBUTTON!");
}

/////////////////////////////////////////////////////////////////

State s[] = {
  State("red light", on_red, ongoing),
  State("green light", on_green, ongoing),
  State("button pressed", on_button_press, ongoing)
};

enum triggers {
  button_was_pressed = 1  
};

Transition transitions[] = {
  Transition(&s[0], &s[2], button_was_pressed)
};

TimedTransition timedTransitions[] = {
  TimedTransition(&s[0], &s[1], 6000),
  TimedTransition(&s[1], &s[0], 4000),
  TimedTransition(&s[2], &s[1], 1000)
};

int num_transitions = sizeof(transitions) / sizeof(Transition);
int num_timed = sizeof(timedTransitions) / sizeof(TimedTransition);

/////////////////////////////////////////////////////////////////

void button_handler(Button2 btn) {
  fsm.trigger(button_was_pressed);
}

/////////////////////////////////////////////////////////////////

void showGraph() {
  String message = "";
  message += F("<html>\n");
  message += F("<head>\n");
  message += F("<meta http-equiv='refresh' content='1'>\n");
  message += F("<title>GraphVizArt</title>\n");
  message += F("</head>\n");
  message += F("<body>\n");
  message += F("<a href='http://gravizo.com/'><img src='https://g.gravizo.com/svg?");
  message += fsm.getDotDefinition();
  message += F("'/></a>");
  message += F("</body>\n");
  message += F("</html>\n");
  server.send ( 200, F("text/html"), message);
}

/////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(300);
    }
  Serial.println();
  Serial.println();
  Serial.println("SimpleFSM - Mixed Transition (Traffic light with button for green)\n");
    
  fsm.add(timedTransitions, num_timed);
  fsm.add(transitions, num_transitions);
  
  fsm.setInitialState(&s[0]);

  btn.begin(BUTTON_PIN);
  btn.setTapHandler(button_handler); 

  if (!connectToWiFi(WIFI_SSID, WIFI_PASSWD, 10000)) {
    Serial.println(F("Could not connect to WiFi."));
  } else {
    Serial.print(F("Open http://"));
    Serial.print(WiFi.localIP());
    Serial.print(F(" in your browser.\n"));

    server.on( "/", showGraph);
    server.onNotFound([](){
      server.send(404, F("text/plain"), F("File Not Found\n"));
    });
    server.begin();
  }
  server.handleClient();
}

/////////////////////////////////////////////////////////////////

void loop() {
  fsm.run();
  btn.loop();
  server.handleClient();
}

/////////////////////////////////////////////////////////////////