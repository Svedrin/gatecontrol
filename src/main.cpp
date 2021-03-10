#pragma GCC diagnostic push  // save compiler settings
#pragma GCC diagnostic error "-Wall"

// kate: hl c++
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// this file #defines WIFI_SSID and WIFI_PASSWORD
#include <WifiParams.h>

#include <monitoredsensor.h>
#include <statemachine.h>

#define PIN_MON        D0 // Low while all sensors are working
#define PIN_UP         D1 // Low when the gate is up
#define PIN_DN         D2 // Low when the gate is down
#define PIN_AUTOCLOSE  D3 // Low when user presses the autoclose button
#define PIN_ERRORLED   D4 // Blinks while connecting, off while running, static when error
#define PIN_TRIGGER    D5 // Relay that triggers the gate
#define PIN_LB_BLOCKED D6 // Light barrier says "I'm blocked"
#define PIN_LB_CLEAR   D7 // Light barrier says "I'm clear"
#define PIN_STATUSLED  D8 // Static when autoclose enabled, blinks when about to close

WiFiClient espClient;
PubSubClient client(espClient);
StateMachine state_machine;
MonitoredSensor light_barrier(PIN_LB_BLOCKED, PIN_LB_CLEAR, LOW, true);

unsigned long last_tick;
unsigned long last_msg;
char hard_pos[50];
char prev_hard_pos[50];

char mqtt_topic_state[50];
char mqtt_topic_commands[50];
char mqtt_topic_command_acks[50];
char mqtt_topic_autoclose[50];

#define MQTT_TOPIC(which_one, topic_suffix) strncpy( \
        which_one, \
        ("ctrl/" + String(ESP.getChipId(), HEX) + "/" + topic_suffix).c_str(), \
        sizeof(which_one) \
    )

void on_mqtt_message(char* topic, byte* payload, unsigned int length);

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        String clientId = "ESP8266Client-" + String(ESP.getChipId(), HEX);
        if (client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
            client.subscribe(mqtt_topic_commands);
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup(void)
{
    // Configure pins
    pinMode(PIN_MON,        INPUT);
    pinMode(PIN_UP,         INPUT);
    pinMode(PIN_DN,         INPUT);
    pinMode(PIN_AUTOCLOSE,  INPUT_PULLUP);
    pinMode(PIN_LB_BLOCKED, INPUT_PULLUP);
    pinMode(PIN_LB_CLEAR,   INPUT_PULLUP);

    pinMode(PIN_ERRORLED,  OUTPUT);
    pinMode(PIN_STATUSLED, OUTPUT);
    pinMode(PIN_TRIGGER,   OUTPUT);

    digitalWrite(PIN_ERRORLED,  LOW);
    digitalWrite(PIN_STATUSLED, LOW);
    digitalWrite(PIN_TRIGGER,   LOW);

    // Start Serial
    Serial.begin(115200);

    // Populate topic names
    MQTT_TOPIC(mqtt_topic_state,        "current_hard_position");
    MQTT_TOPIC(mqtt_topic_commands,     "set_hard_position");
    MQTT_TOPIC(mqtt_topic_command_acks, "close_ack");
    MQTT_TOPIC(mqtt_topic_autoclose,    "autoclose");

    Serial.print("State topic: ");
    Serial.println(mqtt_topic_state);

    // Connect to WiFi
    int error_led_state = LOW;
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.mode(WIFI_STA);
    Serial.println();
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(PIN_ERRORLED, error_led_state);
        error_led_state = (error_led_state == HIGH ? LOW : HIGH);
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    digitalWrite(PIN_ERRORLED,  HIGH);

    // Print the IP address
    Serial.println(WiFi.localIP());

    randomSeed(micros());

    client.setServer(MQTT_HOST, MQTT_PORT);
    client.setCallback(on_mqtt_message);

    last_tick = 0;
    last_msg  = 0;
}


void on_mqtt_message(char* topic, byte* payload, unsigned int length) {
    char buf[50];
    if( length > sizeof(buf) - 1 ){
        length = sizeof(buf) - 1;
    }
    memcpy(buf, payload, length);
    buf[length] = 0;

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] = ");
    Serial.print(buf);

    if( strcmp(topic, mqtt_topic_commands) == 0 ){
        if(strncmp((const char *)payload, "CLOSED", length) == 0){
            Serial.println("Received CLOSE message");
            if (state_machine.cmd_close(millis()) == COMMAND_ACCEPTED) {
                client.publish(mqtt_topic_command_acks, "waiting");
            } else {
                client.publish(mqtt_topic_command_acks, "reject");
            }
        }
        else if(strncmp((const char *)payload, "COMMIT", length) == 0){
            Serial.println("Received COMMIT message");
            if (state_machine.cmd_commit(millis()) == COMMAND_ACCEPTED) {
                client.publish(mqtt_topic_command_acks, "commit");
            } else {
                client.publish(mqtt_topic_command_acks, "abort");
            }
        }
    }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }

    client.loop();

    unsigned long now = millis();

    if(now > last_tick + 100){
        last_tick = now;

        esp_state_t esp_state;
        esp_state.sensor_gate_up   = (digitalRead(PIN_UP) == LOW ? SENSOR_ACTIVE : SENSOR_CLEAR);
        esp_state.sensor_gate_down = (digitalRead(PIN_DN) == LOW ? SENSOR_ACTIVE : SENSOR_CLEAR);
        esp_state.button_autoclose = (digitalRead(PIN_AUTOCLOSE) == LOW ? SENSOR_ACTIVE : SENSOR_CLEAR);
        esp_state.sensor_light_barrier = light_barrier.read(now);
        esp_state.millis = now;

        step_t step = state_machine.step(&esp_state);

        switch(step.current_state){
            case GATE_INIT:
                strcpy(hard_pos, "INIT");
                break;

            case GATE_OPEN:
            case GATE_CLOSE_AUTO:
            case GATE_CLOSE_PREPARE:
            case GATE_CLOSE_TRIGGERED:
                strcpy(hard_pos, "OPEN");
                break;

            case GATE_BLOCKED:
                strcpy(hard_pos, "BLOCKED");
                break;

            case GATE_OWAIT:
            case GATE_UNKNOWN:
                strcpy(hard_pos, "UNKNOWN");
                break;

            case GATE_CLOSED:
            case GATE_OPEN_TRIGGERED:
                strcpy(hard_pos, "CLOSED");
                break;

            case GATE_ERROR:
                strcpy(hard_pos, "ERROR");
                digitalWrite(PIN_ERRORLED, LOW);
                break;
        }

        if(now > last_msg + 1000 || strcmp(hard_pos, prev_hard_pos) != 0){
            last_msg = now;
            strcpy(prev_hard_pos, hard_pos);
            client.publish(mqtt_topic_state, hard_pos);

            Serial.print("Hard position = ");
            Serial.println(hard_pos);
        }

        // autoclose_state indicates when certain events have happened
        // and are only set for the one step where they happened
        if (step.autoclose_state == AUTOCLOSE_ENABLED) {
            client.publish(mqtt_topic_autoclose, "enabled");
        }
        else if (step.autoclose_state == AUTOCLOSE_PENDING) {
            client.publish(mqtt_topic_autoclose, "pending");
        }
        else if (step.autoclose_state == AUTOCLOSE_RESET) {
            client.publish(mqtt_topic_autoclose, "reset");
        }
        else if (step.autoclose_state == AUTOCLOSE_TRIGGERED) {
            client.publish(mqtt_topic_autoclose, "triggered");
        }

        // Drive the status LED.
        // When we received a CLOSE command, blink (by only lighting
        // up for the second half of each second)
        if (step.current_state == GATE_CLOSE_PREPARE) {
            digitalWrite(PIN_STATUSLED, (now % 1000 > 500 ? HIGH : LOW));
        }
        // When autoclose is enabled, light up statically
        else if (step.autoclose_state == AUTOCLOSE_ON) {
            digitalWrite(PIN_STATUSLED, HIGH);
        }
        // Otherwise, go dark
        else {
            digitalWrite(PIN_STATUSLED, LOW);
        }

        if (step.trigger) {
            Serial.println("trigger! pew pew");
            digitalWrite(PIN_TRIGGER, HIGH);
            delay(500);
            digitalWrite(PIN_TRIGGER, LOW);
        }
    }

    delay(1);
}

#pragma GCC diagnostic pop // Restore compiler settings
