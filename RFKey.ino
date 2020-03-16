#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#include <EEPROM.h>

#define PN532_SCK  (10)
#define PN532_MOSI (13)
#define PN532_SS   (11)
#define PN532_MISO (12)

const int buzzer = 8;
const int RELAY = 7;

const int ENGINE_POWER_OFF = 0;
const int ENGINE_POWER_ON = 1;
const int ENGINE_POWER_LOCK = 2;



const int eeprom_size = 500;
char eeprom_buffer[eeprom_size];

int KEY_STATE = 0;

String cardData[] = {"","","",""};


Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);


void setup() {

  Serial.begin(9600);

  
  pinMode(buzzer, OUTPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  nfcSetup();
  getCardData();
  beep(2,300);
  
}

void resetData(){
    for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void loop() {
  readNfc();
}



//NFC

void getCardData(){
   read_string_from_eeprom(eeprom_buffer);
   cardData[0] = getValue(eeprom_buffer,'~',0);
   cardData[1] = getValue(eeprom_buffer,'~',1);
   cardData[2] = getValue(eeprom_buffer,'~',2);
   cardData[3] = getValue(eeprom_buffer,'~',3);
}


void readNfc(){
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  if (success) {
    Serial.println("Found a card!");
    Serial.print("UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    
    Serial.print("UID Value: ");
    

    String nfcCard = "";
    
    for (uint8_t i=0; i < uidLength; i++){
      nfcCard += "";
      nfcCard += uid[i];
    }

    Serial.println(nfcCard);
    
    boolean isUser = false;
    
    for (uint8_t i=0; i < 4 ; i++){
        Serial.print("Data: ");
        Serial.println(cardData[i] );
        Serial.print("NFC: ");
        Serial.println(nfcCard);
        if(cardData[i] == nfcCard){
            isUser = true;
        }
    }
    
    if(isUser){
      if(KEY_STATE == ENGINE_POWER_OFF){
        KEY_STATE = ENGINE_POWER_ON;
        beep(2,200);
        digitalWrite(RELAY, HIGH);
      }else if (KEY_STATE == ENGINE_POWER_ON){
        KEY_STATE = ENGINE_POWER_OFF;
        beep(3,200);
        digitalWrite(RELAY, LOW);
      }
    }else{
      

        beep(1,800);
        if(KEY_STATE == ENGINE_POWER_ON){
          KEY_STATE = ENGINE_POWER_OFF;
          digitalWrite(RELAY, LOW);
        }
       
    }
    
    Serial.println(nfcCard);
    Serial.println("");
    isUser = false;
    
    delay(1000);
  }
  else
  {
    Serial.println("Timed out waiting for a card");
  }

  
}

void save_string_to_eeprom(char *stringIn){
  for(int i = 0; i < strlen(stringIn); i++){
      EEPROM.write(i, stringIn[i]);
  }
}


void read_string_from_eeprom(char* theBuffer){
  for(int i = 0; i < eeprom_size - 1; i++){
    theBuffer[i] = EEPROM.read(i);
  }
}


String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void nfcSetup(){
  Serial.println("nfcSetup");
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    digitalWrite(RELAY, HIGH);
    beep(1,3000);
    while (1); // halt
  }
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
}

void beep(int isLong,int timeLong){
  for (uint8_t i=0; i < isLong; i++) {
    tone(buzzer, 1000); 
    delay(timeLong); 
    noTone(buzzer);
    delay(250); 
  }
}





