/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP8266
 *
 * Copyright (c) 2023 mobizt
 *
 */

/** This example will show how to authenticate as a user with Email and password.
 *
 * You need to enable Email/Password provider.
 * In Firebase console, select Authentication, select Sign-in method tab,
 * under the Sign-in providers list, enable Email/Password provider.
 *
 * From this example, the user will be granted to access the specific location that matches
 * the user uid.
 *
 * This example will modify the database rules to set up the security rule which need to
 * guard the unauthorized access with the user Email.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "ASUS"
#define WIFI_PASSWORD "zxcvbnml"
#define led_pin_1 D1  // Menggunakan pin D1 untuk LED 1
#define led_pin_2 D2  // Menggunakan pin D2 untuk LED 2
// #define dht_type DHT11
// DHT dht(dht_pin, dht_type);
unsigned long previousMillis = 0;  // Variabel untuk menyimpan waktu terakhir

const long interval = 200; 
/** 2. Define the API key
 *
 * The API key (required) can be obtained since you created the project and set up
 * the Authentication in Firebase console. Then you will get the API key from
 * Firebase project Web API key in Project settings, on General tab should show the
 * Web API Key.
 *
 * You may need to enable the Identity provider at https://console.cloud.google.com/customer-identity/providers
 * Select your project, click at ENABLE IDENTITY PLATFORM button.
 * The API key also available by click at the link APPLICATION SETUP DETAILS.
 *
 */
#define API_KEY "AIzaSyDYr5lxd1rMmxP2kdoyhrki7BWC43aG3DU"
/* 3. Define the user Email and password that already registerd or added in your project */
#define USER_EMAIL "iniemail@gmail.com"
#define USER_PASSWORD "inipassword"
float t = 0.0;
float h = 0.0;
float f = 0.0;

/* 4. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "utees-42f78-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/** 5. Define the database secret (optional)
 *
 * This database secret needed only for this example to modify the database rules
 *
 * If you edit the database rules yourself, this is not required.
 */
#define DATABASE_SECRET "DATABASE_SECRET"

/* 6. Define the Firebase Data object */
FirebaseData fbdo;

/* 7. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 8. Define the FirebaseConfig data for config data */
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

String base_path = "/UTS/";

void setup() {

  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  multi.addAP(WIFI_SSID, WIFI_PASSWORD);
  multi.run();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    if (millis() - ms > 10000)
      break;
#endif
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL */
  config.database_url = DATABASE_URL;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 /, 1024 / Tx buffer size in bytes from 512 - 16384 */);

  fbdo.setResponseSize(4096);


  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);

  /** Now modify the database rules (if not yet modified)
     *
     * The user, <user uid> in this case will be granted to read and write
     * at the certain location i.e. "/UsersData/<user uid>".
     *
     * If you database rules has been modified, please comment this code out.
     *
     * The character $ is to make a wildcard variable (can be any name) represents any node key
     * which located at some level in the rule structure and use as reference variable
     * in .read, .write and .validate rules
     *
     * For this case $userId represents any <user uid> node that places under UsersData node i.e.
     * /UsersData/<user uid> which <user uid> is user UID.
     *
     * Please check your the database rules to see the changes after run the below code.
     */
  String var = "$userId";
  String val = "($userId === auth.uid && auth.token.premium_account === true && auth.token.admin === true)";
  Firebase.setReadWriteRules(fbdo, base_path, var, val, val, DATABASE_SECRET);

  /** path for user data is now "/UsersData/<user uid>"
     * The user UID can be taken from auth.token.uid
     *
     * The refresh token can be accessed from Firebase.getRefreshToken().
     */
}

void loop() {
  // Firebase.ready() should be called repeatedly to handle authentication tasks.
    unsigned long currentMillis = millis();

  // Periksa apakah sudah mencapai interval yang diinginkan (2 detik)
  if (currentMillis - previousMillis >= interval) {
    // Simpan waktu terakhir
    previousMillis = currentMillis;

    // Lakukan sesuatu di sini setiap 2 detik
    // Misalnya, Anda dapat menyalakan atau mematikan LED, atau melakukan tugas lainnya.
    readFirebase();

    // Contoh: Menyalakan LED
  // sensorUpdate();
  }
  // if (Firebase.getString(fbdo, base_path+"led")) {
  //   Serial.println(fbdo.stringData());
  //   if (fbdo.stringData() == "1") {
  //     digitalWrite(led_pin, HIGH);
  //   } else if (fbdo.stringData() == "0") {
  //     digitalWrite(led_pin, LOW);
  //   }
  // }
  // delay(100);
}
// void sensorUpdate() {
//   t = dht.readTemperature();
//   f = dht.readTemperature(true);
//   h = dht.readHumidity();
//   if (!isnan(h) && !isnan(t) && !isnan(f)) {
//     Serial.print(F("Humidity: "));
//     Serial.print(h);
//     Serial.print(F("% Temperature: "));
//     Serial.print(t);
//     Serial.print(F("°C ,"));
//     Serial.print(f);
//     Serial.println(F("°F "));

//     if (Firebase.setFloat(fbdo, base_path + "suhu", t)) {
//       Serial.println("PASSED (Suhu)");
//     } else {
//       Serial.println("FAILED (Suhu)");
//       Serial.println("REASON: " + fbdo.errorReason());
//       ESP.restart();
//     }

//     if (Firebase.setFloat(fbdo, base_path + "kelembapan", h)) {
//       Serial.println("PASSED (Kelembapan)");
//     } else {
//       Serial.println("FAILED (Kelembapan)");
//       Serial.println("REASON: " + fbdo.errorReason());
//       ESP.restart();
//     }

//     Serial.println("PATH (Suhu): " + fbdo.dataPath());
//     Serial.println("TYPE (Suhu): " + fbdo.dataType());
//     Serial.println("ETag (Suhu): " + fbdo.ETag());

//     Serial.println("PATH (Kelembapan): " + fbdo.dataPath());
//     Serial.println("TYPE (Kelembapan): " + fbdo.dataType());
//     Serial.println("ETag (Kelembapan): " + fbdo.ETag());

//     Serial.println("------------------------------------");
//   } else {
//     Serial.println(F("Failed to read from DHT sensor!"));
//     return;
//   }
// }
void readFirebase() {
  // Baca status pasien 1 dari Firebase
  if (Firebase.getInt(fbdo, base_path + "Pasien 1/Status")) {
    int status1 = fbdo.intData();

    if (status1 == 1) {
      // Jika status pasien 1 adalah 1, maka nyalakan LED pada pin 5
      digitalWrite(led_pin, HIGH);
    } else {
      // Jika status pasien 1 bukan 1, maka matikan LED pada pin 5
      digitalWrite(led_pin, LOW);
    }
  }

  // Baca status pasien 2 dari Firebase
  if (Firebase.getInt(fbdo, base_path + "Pasien 2/Status")) {
    int status2 = fbdo.intData();

    if (status2 == 1) {
      // Jika status pasien 2 adalah 1, maka nyalakan LED pada pin 4
      digitalWrite(dht_pin, HIGH);
    } else {
      // Jika status pasien 2 bukan 1, maka matikan LED pada pin 4
      digitalWrite(dht_pin, LOW);
    }
  }
}