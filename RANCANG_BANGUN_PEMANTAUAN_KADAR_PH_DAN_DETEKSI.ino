// Library yang diperlukan

#include <FirebaseESP8266.h>
//#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Mendefinisikan pin sensor
const int phpin = A0;
const int trigPin = 2;
const int echoPin = 0;
const int pompaup = 14;
const int pompadown = 16;
const int relay1 = 15;
const int relay2 = 13;
const int relay3 = 12;

// Harus diisi
#define FIREBASE_HOST "monitoring-sistem-akuaponik-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "piEpMZkxnpaKfKHMC5cPYRTwlQo2EMk60uIRAoBG"
#define WIFI_SSID "Kos Bawah"
#define WIFI_PASSWORD "setiakawan6"

// mendeklarasikan objek data dari FirebaseESP8266
FirebaseData firebaseData;

int nilaianalogph;
float voltage;
float Po = 0;
float phstep;
float PH7 = 2.2;
float PH4 = 3.0;

long duration, distance;
int tinggi, tinggimaksimal = 14;

String Buttonutama, Buttonisi, Buttonbuang;
int buttonutama, buttonisi, buttonbuang;

void setup() 
{
  Serial.begin(9600);

  pinMode (phpin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode (pompaup, OUTPUT);
  pinMode (pompadown, OUTPUT);
  pinMode (relay1, OUTPUT);
  pinMode (relay2, OUTPUT);
  pinMode (relay3, OUTPUT);
  
  digitalWrite (pompaup, HIGH);
  digitalWrite (pompadown, HIGH);
  digitalWrite (relay1, HIGH);
  digitalWrite (relay2, HIGH);
  digitalWrite (relay3, HIGH);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Monitoring");
  lcd.setCursor(0, 1);
  lcd.print("Sistem Akuaponik");
  //delay (1000);

  // Koneksi ke Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
//Kontrol Pompa Utama
  if(Firebase.setInt(firebaseData, "/Hasil_Pembacaan/Pompa Utama", 0))
  {
    //Success
    //Serial.println("Relay siap");
  } else {
    //Failed?, get the error reason from fbdo
    Serial.print("Gagal");
    Serial.println(firebaseData.errorReason());
  }

  //Kontrol Isi Air
  if(Firebase.setInt(firebaseData, "/Hasil_Pembacaan/Isi Air", 0))
  {
    //Success
    //Serial.println("Relay siap");
  } else {
    //Failed?, get the error reason from fbdo
    Serial.print("Gagal");
    Serial.println(firebaseData.errorReason());
  }

  //Kontrol Buang Air
  if(Firebase.setInt(firebaseData, "/Hasil_Pembacaan/Buang Air", 0))
  {
    //Success
    //Serial.println("Relay siap");
  } else {
    //Failed?, get the error reason from fbdo
    Serial.print("Gagal");
    Serial.println(firebaseData.errorReason());
  }
}

void loop() 
{
  nilaianalogph = analogRead (phpin);
  Serial.print("Nilai ADC : ");
  Serial.println (nilaianalogph);
  voltage = (2.88 / 1024.0) * nilaianalogph;
  Serial.print("Tegangan PH : ");
  Serial.print (voltage, 3);
  Serial.print ("\t");

  //phstep = (PH4 - PH7) / 3;
  Po = 6.86+((2.68-voltage)/0.165);
  //7.00 + ((PH7 - voltage) / phstep); 
  Serial.print("Nilai PH : ");
  Serial.println(Po, 1);

  if ( Po < 6) {
    digitalWrite (pompaup, LOW);
    delay(1000);
    digitalWrite (pompadown, HIGH);
  }
  else if (Po >= 8) {
    digitalWrite (pompaup, HIGH);
    delay(1000);
    digitalWrite (pompadown, LOW);
  }
  else {
    digitalWrite (pompaup, HIGH);
    digitalWrite (pompadown, HIGH);
    Serial.println ("PH Normal");
  }

  //ULTRASONIK
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  tinggi = tinggimaksimal - distance;

  Serial.print("Jarak objek : ");
  Serial.print(distance);
  Serial.print ("cm");
  Serial.print ("\t");

  Serial.print("Ketinggian air : ");
  Serial.print(tinggi);
  Serial.print("cm");
  Serial.println();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PH : ");
  lcd.setCursor(5, 0);
  lcd.print(Po, 1);

  lcd.setCursor(0, 1);
  lcd.print("Tinggi Air");
  lcd.setCursor(11, 1);
  lcd.print(tinggi);
  lcd.setCursor(13, 1);
  lcd.print("cm");
  //delay (1000);

  // Memberikan status suhu dan kelembaban kepada firebase
  if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/PH", Po)) 
  {
    //Serial.println("PH terkirim");
  } else {
    Serial.println("PH tidak terkirim");
    Serial.println("Karena: " + firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/Hasil_Pembacaan/Ketinggian Air", tinggi)) 
  {
    //Serial.println("Ketinggian Air terkirim");
    //Serial.println();
  } else {
    Serial.println("Ketinggian Air tidak terkirim");
    Serial.println("Karena: " + firebaseData.errorReason());
  }

  

  //Kontrol Pompa Utama
  if(Firebase.getString(firebaseData, "/Hasil_Pembacaan/Pompa Utama"))
  {
    Buttonutama = firebaseData.stringData();
    buttonutama = Buttonutama.toInt();
    if (buttonutama == 1)
    {
      digitalWrite(relay1, LOW);
    } else if (buttonutama == 0) {
      digitalWrite(relay1, HIGH);
      Serial.println("Pompa Utama dimatikan");
    }
  }

  //Kontrol Isi Air
  if(Firebase.getString(firebaseData, "/Hasil_Pembacaan/Isi Air"))
  {
    Buttonisi = firebaseData.stringData();
    buttonisi = Buttonisi.toInt();
    if (buttonisi == 1)
    {
      digitalWrite(relay2, LOW);
      Serial.println("Sedang Mengisi Air Kolam");
    } else if (buttonisi == 0) {
      digitalWrite(relay2, HIGH);
    }
  }

     //Kontrol Buang Air
  if(Firebase.getString(firebaseData, "/Hasil_Pembacaan/Buang Air"))
  {
    Buttonbuang = firebaseData.stringData();
    buttonbuang = Buttonbuang.toInt();
    if (buttonbuang == 1)
    {
      digitalWrite(relay3, LOW);
      Serial.println("Sedang Membuang Air Kolam");
    } else if (buttonbuang == 0) {
      digitalWrite(relay3, HIGH);
    }
  }
}
