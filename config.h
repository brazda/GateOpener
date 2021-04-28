/*   
 * User-configurable Parameters 
*/

// Wifi Parameters

#define WIFI_SSID "mywifi"
#define WIFI_PASSWORD "password"

// Static IP Parameters

#define STATIC_IP true
#define IP 10,10,221,46
#define GATEWAY 10,10,221,254
#define SUBNET 255,255,255,0

// MQTT Parameters

#define MQTT_BROKER "10.10.221.20"
#define MQTT_CLIENTID "gate"
#define MQTT_USERNAME "mqtt"
#define MQTT_PASSWORD "pa55word"

// Relay Parameters

#define ACTIVE_HIGH_RELAY true

// Door 1 Parameters

#define GATE1_ALIAS "Door 1"
#define MQTT_GATE1_ACTION_TOPIC "garage/gate/1/action"
#define MQTT_GATE1_STATUS_TOPIC "garage/gate/1/status"
#define GATE1_OPEN_PIN D7
#define GATE1_CLOSE_PIN D7
#define GATE1_STATUS_PIN D6
int motor1pin1 = D2;
int motor1pin2 = D3;
int RPWM_Output = D4; // Arduino PWM output pin 5; connect to IBT-2 pin 1 (RPWM)



#define GATE1_STATUS_SWITCH_LOGIC "NO"

// Door 2 Parameters

#define GATE2_ENABLED false
#define GATE2_ALIAS "Door 2"
#define MQTT_GATE2_ACTION_TOPIC "garage/gate/2/action"
#define MQTT_GATE2_STATUS_TOPIC "garage/gate/2/status"
#define GATE2_OPEN_PIN D1
#define GATE2_CLOSE_PIN D1
#define GATE2_STATUS_PIN D1
int motor2pin1 = D7;
int motor2pin2 = D8;
int LPWM_Output = D5; // Arduino PWM output pin 6; connect to IBT-2 pin 2 (LPWM)
#define GATE2_STATUS_SWITCH_LOGIC "NO"
