#define BITS 1
#define VALUE 2
#define UART 3
#define POS 4
#define MULTIPLE 5
#define LAMP_TEST 6

//How many of the shift registers - change this
#define number_of_74hc595s 2
//do not touch
#define numOfRegisterPins number_of_74hc595s * 8

#define data1Length 48  // długość linii 1 z danymi
#define data2Length 48  // długość linii 2 z danymi

#include <Servo.h>
#include "Nastawnik.h"// trzeba pobrać bibliotekę Vector z Arduino IDE

Servo CH;
Servo PG;
Servo ZG;

int MODE = UART; // tryb pracy

// ----------------
// NASTAWNIK
// Obiekt klasy Nastawnik przechowuje pozycje nastawnika (końcówka "L" oznacza zmienną typu long)
// ----------------
Nastawnik nastawnik;
long pozycje[] = {
  -285151487L , // 0
  -16712943L  , // 1
  -1090454704L, // 2
  -1090192560L, // 3
  -1090192432L, // 4
  -1224410160L, // 5
  -1224442992L, // 6
  -1224705392L, // 7
  -1258259824L, // 8
  -1257997680L, // 9
  -1811613040L, // 10
  -1811612976L, // 11
  -1677329712L, // 12
  -1677346096L, // 13
  -1677215024L, // 14
  -1811432752L, // 15
  -1811432524L, // 16
  -1811465292L, // 17
  -1811465290L, // 18
  -1811465482L, // 19
  -1810941194L, // 20
  -1810949378L, // 21 
  -83584252   , // 22
  -1157326012L, // 23
  -1291543740L, // 24
  -1291576572L, // 25
  -1291838972L, // 26
  -1862264316L, // 27
  -1862247868L, // 28
  -1727767868L, // 29
  -1727620412L, // 30
  -1861838140L, // 31
  -1861772412L, // 32
  -1861772666L, // 33
  -1861805370 , // 34
  -1878058298L, // 35
  -1878058258L, // 36
  -1090202880L, // 37
  -1090219200L, // 38
  -1224469696L, // 39
  -150990272L , // 40
  -184544704L , // 41
  -1257991616L, // 42
  -1257844160L, // 43
  -1257778304L, // 44
  -1794649470L, // 45
  -1794698558L, // 46
  -1810951486L, // 47
  -1810951442L, // 48
};

// ----------------
// CONSTANTS
// ----------------
const long interval = 100; 
const int dataPin = 25;   /* Q7 */
const int clockPin = 26;  /* CP */
const int latchPin = 27;  /* PL */
const int dataPin2 = 23;   /* Q7 */
const int clockPin2 = 22;  /* CP */
const int latchPin2 = 24;  /* PL */
const int SER_Pin = 12;   //pin 14 on the 75HC595
const int RCLK_Pin = 10;  //pin 12 on the 75HC595
const int SRCLK_Pin = 11; //pin 11 on the 75HC595

// ----------------
// MASZYNA
// ----------------
volatile uint8_t zPC[52] = {0}; // standardowe bajty przekazywane do kodu Arduino
volatile uint8_t doPC[20] = {0xEF, 0xEF, 0xEF, 0xEF, 0}; // standardowe bajty przyjmowane z kodu Arduino
volatile uint8_t doPCBase[20] = {0,0,0,0,0};

// ----------------
// VARIABLES
// ----------------
bool radiostate; //stan przycisku radia
bool prevstate; //stan przycisku radia
bool radio; //zmienna od stanu radia (włącz/wyłącz)

bool data1[data1Length];
bool data2[data2Length];
bool registers[numOfRegisterPins];

int pozycja=0;

unsigned long previousMillis = 0;  // will store last time LED was updated
long kierunek = 0;
long bocznikInternal = 0;
long nastawnikValue = 0;

// ----------------
// FUNKCJE - ARDUINO
// ----------------
void setup()
{
  setNastawnik();

  CH.attach(7, 800, 2200);
  PG.attach(8, 800, 2200);
  ZG.attach(9, 800, 2200);

  analogReference(DEFAULT);
  pinMode(A4 ,INPUT);
  pinMode(A0 ,OUTPUT);
  pinMode(A1 ,OUTPUT);
  pinMode(A2 ,OUTPUT);
  pinMode(A3 ,OUTPUT);
  pinMode(A5,INPUT);

  pinMode(14, OUTPUT);    //czuwak
  pinMode(15,OUTPUT);     //SHP
  pinMode(16,OUTPUT);
  pinMode(2, OUTPUT);     //woltomierz WN
  pinMode(3, INPUT);      //Amperomierz 1
  pinMode(4,OUTPUT);      //Amoperomierz 2
  pinMode(5, OUTPUT);     //Amperomierz 3
  pinMode(6,OUTPUT);      //woltomierz NN

  pinMode(dataPin, INPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  
  pinMode(dataPin2, INPUT);
  pinMode(clockPin2, OUTPUT);
  pinMode(latchPin2, OUTPUT);

  digitalWrite(2, LOW);     //ustawienie stanu niskiego na pinie 2
  digitalWrite(3, LOW);     //ustawienie stanu niskiego na pinie 3

  Serial.begin(115200);     //nawiązanie komunikacji z prędkością 115200b/s (baud)
  Serial.setTimeout(10);
  while(!Serial){};     //czekanie do nawiązania komunikacji z komputerem

  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);

  //reset all register pins
  clearRegisters();
  writeRegisters();
}

void loop()
{
  readLine1();
  readLine2();
  
  setLamps();
  setMierniki();
  setManometry();
  asignData();
  setRadio();
  setKierunek();
  setBocznik();
  checkAndOutputNastawnik();

  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);

  writeToSerial(); 
}

// ------------
// setNastawnik()
// Dodaje do wetora numer pozycji i jaka wartość z arduino mu odpowiada
// ------------
void setNastawnik() {
  nastawnik.addPosition(0 , { -285151487L  });
  nastawnik.addPosition(1 , { -16712943L   });
  nastawnik.addPosition(2 , { -1090454704L });
  nastawnik.addPosition(3 , { -1090192560L });
  nastawnik.addPosition(4 , { -1090192432L });
  nastawnik.addPosition(5 , { -1224410160L });
  nastawnik.addPosition(6 , { -1224442992L });
  nastawnik.addPosition(7 , { -1224705392L });
  nastawnik.addPosition(8 , { -1258259824L });
  nastawnik.addPosition(9 , { -1257997680L });
  nastawnik.addPosition(10, { -1811613040L });
  nastawnik.addPosition(11, { -1811612976L });
  nastawnik.addPosition(12, { -1677329712L });
  nastawnik.addPosition(13, { -1677346096L });
  nastawnik.addPosition(14, { -1677215024L });
  nastawnik.addPosition(15, { -1811432752L });
  nastawnik.addPosition(16, { -1811432524L });
  nastawnik.addPosition(17, { -1811465292L });
  nastawnik.addPosition(18, { -1811465290L });
  nastawnik.addPosition(19, { -1811465482L });
  nastawnik.addPosition(20, { -1810941194L });
  nastawnik.addPosition(21, { -1810949378L });
  const long val[] = { -83584252L, 56846L };
  nastawnik.addPosition(22, val);
  nastawnik.addPosition(23, { -1157326012L });
  nastawnik.addPosition(24, { -1291543740L });
  nastawnik.addPosition(25, { -1291576572L });
  nastawnik.addPosition(26, { -1291838972L });
  nastawnik.addPosition(27, { -1862264316L });
  nastawnik.addPosition(28, { -1862247868L });
  nastawnik.addPosition(29, { -1727767868L });
  nastawnik.addPosition(30, { -1727620412L });
  nastawnik.addPosition(31, { -1861838140L });
  nastawnik.addPosition(32, { -1861772412L });
  nastawnik.addPosition(33, { -1861772666L });
  nastawnik.addPosition(34, { -1861805370L });
  nastawnik.addPosition(35, { -1878058298L });
  nastawnik.addPosition(36, { -1878058258L });
  nastawnik.addPosition(37, { -1090202880L });
  nastawnik.addPosition(38, { -1090219200L });
  nastawnik.addPosition(39, { -1224469696L });
  nastawnik.addPosition(40, { -150990272L  });
  nastawnik.addPosition(41, { -184544704L  });
  nastawnik.addPosition(42, { -1257991616L });
  nastawnik.addPosition(43, { -1257844160L });
  nastawnik.addPosition(44, { -1257778304L });
  nastawnik.addPosition(45, { -1794649470L });
  nastawnik.addPosition(46, { -1794698558L });
  nastawnik.addPosition(47, { -1810951486L });
  nastawnik.addPosition(48, { -1810951442L });
}

// ------------
// readLine1()
// ------------
void readLine1() {
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
  for (int i = 0; i < data1Length; i++) {
    data1[i] = digitalRead(dataPin);
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
}

// ------------
// readLine2()
// ------------
void readLine2() {
  digitalWrite(latchPin2, LOW);
  digitalWrite(latchPin2, HIGH);
  for (int i = 0; i < data2Length; i++) {
    data2[i] = digitalRead(dataPin2);

    if (i < 3) {
      //pierwsze 3 elementy tablicy wskazują na ustawienie kierunku
      bitWrite(kierunek, i, digitalRead(dataPin2));      
    }
    if (i >= 3 && i < 8) {
      // od 3 do 7 elementu przekazywany jest stan bocznika
      bitWrite(bocznikInternal, i - 3, digitalRead(dataPin2));
    }
    if (i >= 8) {
      //od 8 elementu są dane dotyczące aktualnej pozycji nastawnika
      bitWrite(nastawnikValue, i - 8, digitalRead(dataPin2));
    }

    digitalWrite(clockPin2, HIGH);
    digitalWrite(clockPin2, LOW);
  }
}

// ------------
// setLamps()
// ------------
void setLamps() {
  setRegisterPin(0,(bitRead(zPC[9],3)));
  setRegisterPin(1,(bitRead(zPC[9],2)));
  setRegisterPin(2,(bitRead(zPC[9],5)));
  setRegisterPin(3,LOW);
  setRegisterPin(4,(bitRead(zPC[10],7)));
  setRegisterPin(5,(bitRead(zPC[8],7)));
  setRegisterPin(6,(bitRead(zPC[8],6)));
  setRegisterPin(7,LOW);
  setRegisterPin(8,(bitRead(zPC[8],0)));
  setRegisterPin(9,(bitRead(zPC[8],2)));
  setRegisterPin(10,LOW);
  setRegisterPin(11,(bitRead(zPC[9],6)));
  setRegisterPin(12,(bitRead(zPC[9],4)));
  setRegisterPin(13,(bitRead(zPC[8],1)));
  setRegisterPin(14,(bitRead(zPC[9],0)));
  setRegisterPin(15,(bitRead(zPC[6],1)));
 
  writeRegisters();

  digitalWrite(14, !(bitRead(zPC[8], 6))); //czuwak
  digitalWrite(15, !(bitRead(zPC[8], 7))); //shp
}

// ------------
// setMierniki()
// ------------
void setMierniki() {
  analogWrite(2, zPC[17]); //woltomierz WN
  analogWrite(3, zPC[19]); //amperomierz 1
  analogWrite(4, zPC[21]); //amperomierz 2
  analogWrite(5, zPC[23]); //amperomierz 3
  analogWrite(6, zPC[35]); //woltomierz NN
}

// ------------
// setManometry()
// ------------
void setManometry() {
  //konfiguracja i zmapowanie serwomotorów 
  int CH_Value = 180 - map(zPC[11], 0, 255, 0, 360);
  int PG_Value = 180 - map(zPC[13], 0, 255, 0, 360);
  int ZG_Value = 180 - map(zPC[15], 0, 255, 0, 360);
  CH.write(CH_Value);
  PG.write(PG_Value);
  ZG.write(ZG_Value);
}

// ------------
// asignData()
// ------------
void asignData() {
  bitWrite(doPC[4], 1, !data1[33]);
  bitWrite(doPC[4], 2, !data1[21]);
  bitWrite(doPC[4], 3, !data1[29]);
  bitWrite(doPC[4], 4, data1[22]);
  bitWrite(doPC[4], 5, data1[8]);
  bitWrite(doPC[4], 6, !data1[38]);
  bitWrite(doPC[4], 7, !data1[5]);
  bitWrite(doPC[5], 0, !data1[35]);
  bitWrite(doPC[5], 1, !data1[39]);
  bitWrite(doPC[5], 2, !data1[36]);
  bitWrite(doPC[5], 3, !data1[4]);
  bitWrite(doPC[5], 4, !data1[7]);
  bitWrite(doPC[5], 5, data1[16]);
  bitWrite(doPC[5], 6, data1[9]);
  bitWrite(doPC[5], 7, data1[10]);
  bitWrite(doPC[6], 0, !data1[11]);
  bitWrite(doPC[6], 1, data1[47]);
  bitWrite(doPC[6], 2, !data1[32]);
  bitWrite(doPC[6], 3, data1[28]);
  bitWrite(doPC[6], 4, !data1[28]);
  bitWrite(doPC[6], 5, (!data1[26])&(!data1[27]));
  bitWrite(doPC[6], 6, data1[26]);
  bitWrite(doPC[6], 7, data1[27]);
  bitWrite(doPC[7], 0, data1[30]);
  bitWrite(doPC[7], 1, data1[31]);
}

// ------------
// setRadio()
// ------------
void setRadio() {
  //stan radia na 0
  radio = 0;
  prevstate = radiostate;
  delay(10);
  //ustawienie stanu radia na przeciwny do otrzymanego z symulatora
  radiostate = !data1[30];
  //jeżeli stan radia jest różny od od poprzedniego to ustaw radio na 1 (włącz)
  if(radiostate != prevstate){
    radio = 1;
  }
  bitWrite(doPC[7], 0, !radio);
}

// ------------
// setKierunek()
// ------------
void setKierunek() {
  bitWrite(doPC[7], 2, (!data2[0]&& data2[1]&& data2[2])); //neutral
  bitWrite(doPC[7], 3, ( data2[0]&& data2[1]&&!data2[2])); //reverse
  bitWrite(doPC[7], 4, ( data2[0]&&!data2[1]&& data2[3])); //forward
  bitWrite(doPC[7], 5, ( data2[0]&& data2[1]&& data2[2])); //forwardhigh
}

// ------------
// setBocznik()
// ------------
void setBocznik() {
  // ustawnianie stanów pocznika na podstawie stanu krzywek z symulatora
  if (data2[6] && data2[7]) {
    doPC[11] = 0;
  }
  if (data2[5] && !data2[6] && data2[7]) { 
    doPC[11] = 1;
  }
  if (data2[5] && !data2[6] && !data2[7] ) {
    doPC[11] = 2;
  }
  if (data2[3] && !data2[5] && !data2[6] && data2[7]) {
    doPC[11] = 3;
  }
  if (!data2[3] && !data2[5] && !data2[6] && data2[7]) {
    doPC[11] = 4;
  }
  if (data2[4] && !data2[5] && !data2[6] && !data2[7]) {
    doPC[11] = 5;
  }
  if (!data2[3] && !data2[4] ){ 
    doPC[11] = 6;
  }
}

// ------------
// checkAndOutputNastawnik()
// ------------
void checkAndOutputNastawnik() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    //pozyskanie numeru pozycji nastawnika na podstawie wartości z symulatora
    int pos = nastawnik.getPos(nastawnikValue);
    //jeżeli pozycja jest większa to ustaw pozycję nastawnika na tą wartość w maszynie
    if (pozycja >= 0)
    {
      doPC[10] = pozycja;
    }
  }
}

// ------------
// writeToSerial()
// ------------
void writeToSerial() {
  //standatdwy tryb pracy - UART
  if (MODE == UART) {
    while (!Serial.available()) {}
    Serial.readBytes((char*)zPC, 52);
    Serial.write((char*)doPC, 20);
  }
  // debugowanie wartości z nastawnika (pozycji)
  if (MODE == POS) {
    for (int i = 0; i < 32; i++)
    {
      Serial.print(bitRead(nastawnikValue, i));
    }
    Serial.print(" => ");
    Serial.print(nastawnikValue);
    Serial.print(" => ");
    Serial.println(pozycja);
  }
  // debugowanie komunikacji na bitach
  if (MODE == BITS) {
    showBits();
  }
  // test lamp w pulpicie
  if(MODE==LAMP_TEST){
    lampTest();
  }
}

// ------------
// showBits()
// ------------
void showBits() {
  int space = 0;
  for (int i = 0; i < 3; i++) {
    Serial.print(bitRead(kierunek, i) ? 1 : 0);
  }
  Serial.print(" ");

  for (int i = 0; i < 5; i++)
  {
    Serial.print(bitRead(bocznikInternal, i) ? 1 : 0);
  }
  Serial.print(" | ");

  for (int i = 0; i < 32; i++)
  {
    Serial.print(bitRead(nastawnikValue, i) ? 1 : 0); 
    space++;
    if (space == 4) {
      Serial.print(" ");
      space = 0;
    }
  }
  space = 0;
  Serial.print(" | "); 

  for (int i = 0; i < 48; i++)
  {
    Serial.print(data1[i] ? 1 : 0); 
    space++;
    if (space == 4) {
      Serial.print(" ");
      space = 0;
    }
  }
    Serial.println();
}

// ------------
// lampTest()
// ------------
void lampTest() {
  setRegisterPin(0, HIGH);
  setRegisterPin(1, HIGH);
  setRegisterPin(2, HIGH);
  setRegisterPin(3, HIGH);
  setRegisterPin(4, HIGH);
  setRegisterPin(5, HIGH);
  setRegisterPin(6, HIGH);
  setRegisterPin(7, HIGH);
  setRegisterPin(8, HIGH);
  setRegisterPin(9, HIGH);
  setRegisterPin(10, HIGH);
  setRegisterPin(11, HIGH);
  setRegisterPin(12, HIGH);
  setRegisterPin(13, HIGH);
  setRegisterPin(14, HIGH);
  setRegisterPin(15, HIGH);

  writeRegisters();
}

// ------------
// setRegisterPin()
// ------------
void setRegisterPin(int index, int value){
  registers[index] = value;
}

// ------------
// writeRegisters()
// ------------
void writeRegisters(){

  digitalWrite(RCLK_Pin, LOW);

  for(int i = numOfRegisterPins - 1; i >= 0; i--){
    digitalWrite(SRCLK_Pin, LOW);

    int val = registers[i];

    digitalWrite(SER_Pin, val);
    digitalWrite(SRCLK_Pin, HIGH);

  }
  digitalWrite(RCLK_Pin, HIGH);

}

// ------------
// clearRegisters()
// ------------
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >= 0; i--){
     registers[i] = LOW;
  }
} 