#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Stepper.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "CIT_Alunos"
#define WIFI_PASSWORD "alunos@2024"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDsYXU0A88zPBVuc_P4KOPnS3clGQu8hI0"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://mic11-31a25-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const int PINO_LED = 32; 
const int PINO_POT = 36; 

int temperatura = 0;
int batimentos = 0; 
bool motor = false;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 2000;
bool signupOK = false;

//Motor de passos
// change this to the number of steps on your motor
#define STEPS 2050

// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to
Stepper stepper(STEPS, 26, 25, 33, 32);


// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}


void setup() {
  Serial.begin(115200);
  pinMode(PINO_LED, OUTPUT); 
  pinMode(PINO_POT, INPUT); 
  initWiFi();

  //FireBase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("SignUP OK!");
    signupOK = true;
  } else{
     Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // set the speed of the motor to 30 RPMs
  stepper.setSpeed(5);
}

void loop() {
  int potem = analogRead(PINO_POT);
  temperatura = map(potem, 0, 4095, 0, 100);
  batimentos = random(60,120);

  if(temperatura >= 40){
    Serial.println("Perigo!!!");
    digitalWrite(PINO_LED, HIGH);
    stepper.step(STEPS);
    motor = true;
  } else{
    digitalWrite(PINO_LED, LOW);
    motor = false;
  }

  Serial.print("\nTemperatura: ");
  Serial.println(temperatura);

  Serial.print("\nBatimentos: ");
  Serial.println(batimentos);

  //Firebase CRUD
  if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 500 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
  }

 
  //Enviar dados para database
  if(Firebase.RTDB.setInt(&fbdo, "Quarto/Temperatura", temperatura)){
    Serial.println(); Serial.print(temperatura);
    Serial.print(" - successfully saved to: "+ fbdo.dataPath());
    Serial.println(" (" + fbdo.dataType() + ") ");
  } else {
    Serial.println(" FAILED: " + fbdo.errorReason());
  }

  if(Firebase.RTDB.setInt(&fbdo, "Quarto/Batimentos", batimentos)){
    Serial.println(); Serial.print(batimentos);
    Serial.print(" - successfully saved to: "+ fbdo.dataPath());
    Serial.println(" (" + fbdo.dataType() + ") ");
  } else {
    Serial.println(" FAILED: " + fbdo.errorReason());
  }

  //Pegar dados do banco
  String nome ="";
  if (Firebase.RTDB.getString(&fbdo, "Quarto/Nome")){
    nome =fbdo.stringData();
    Serial.println("DATA: " + nome);
  } 

  if (Firebase.RTDB.getString(&fbdo, "Quarto/Motor")){
    motor =fbdo.boolData();
    Serial.println("Motor: " + motor);
  }

  delay(1000);
}