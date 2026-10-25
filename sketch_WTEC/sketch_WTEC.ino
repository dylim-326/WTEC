#include <SPI.h>    // RFID는 SPI 통신을 하므로 SPI 라이브러리 선언
#include <MFRC522.h>    // MFRC522 RFID 리더기를 사용하기 위한 라이브러리 선언-->RFID 리더기와의 상호작용을 쉽게 만듦
#include <SoftwareSerial.h> // 블루투스를 통해 소프트웨어 시리얼 통신하기 위한 라이브러리 선언

const int pin1 = 2;   // 첫 번째 센서 핀을 아두이노 우노 2번에 연결   
(**const:그 대상을 변경하지 않는 "상수")
const int pin2 = 3;   // 두 번째 센서 핀을 아두이노 우노 3번에 연결
int count = 0;    // 강의실 안으로 들어온 인원수를 변수 count라 설정하고 초기값은 0으로 한다

#define RST_PIN 9   // RFID 모듈의 RST 핀을 9번에 연결
#define SS_PIN 10   // RFID 모듈의 SS 핀을 10번에 연결
MFRC522 mfrc522(SS_PIN, RST_PIN);   // RFID 모듈 초기화

#define BT_RXD 5  // 블루투스 모듈 RX 핀을 5번에 연결
#define BT_TXD 4  // 블루투스 모듈 TX 핀을 4번에 연결
String data;  // 강의실 호실을 입력받는 String 변수 data 선언
int option, chair;  // 탐색 여부와 잔여석을 나타내는 변수 각각 선언 
SoftwareSerial mySerial(BT_TXD, BT_RXD);  // 블루투스 시리얼과 통신하기 위해 변수 선언 RX: 4, TX: 5
// 블루투스와 아두이노의 RX와 TX 핀은 서로 연결해 주어야 수신과 통신이 됨 수신-수신/통신-통신 연결하면 수신통신 불가능

byte lastCardUID[4] = {0};    // 마지막으로 인식된 카드의 UID 저장하는 배열을 0으로 초기화, 새로운 카드가 인식될 때마다 해당 UID로 업데이트
int Class = 0;    // 0 = 수업X, 1 = 수업O

// 두 RFID 카드의 UID가 같은지 비교하는 함수 (이 여부를 판별해 수업의 시작 및 종료 상태 파악)
bool isEqualCardUID(byte cardUID1[], byte cardUID2[]){    // <함수 정의> 반환값으로 bool 사용, cardUID1과 cardUID2라는 두 개의 byte 배열을 매개변수로 받음
  for (byte i = 0; i < 4; i++){   // 반복문을 통해 UID 배열의 각 요소를 하나씩 비교 (대부분의 RFID 카드 UID는 4바이트로 이루어짐--0부터 3까지의 인덱스를 확인)
    if (cardUID1[i] != cardUID2[i]){    // 각 배열의 해당 인덱스에 위치한 요소를 비교
      return false;   // 하나의 바이트라도 다르다면 false 반환 후 함수 종료
    }
  }
  return true;    // 모든 바이트가 동일하게 일치한다면 true 반환
}
// 현재 카드 UID를 마지막으로 인식된 카드 UID로 업데이트 하는 함수
void updateLastCardUID(byte currentCardUID[]){    // updateLastCardUID는 함수의 이름, currentCardUID라는 byte 배열을 매개변수로 받음 (반환형이 void--어떤 값을 반환하지X)
  for (byte i = 0; i < 4; i++){   // currentCardUID 배열의 각 요소를 lastCardUID 배열에 복사
    lastCardUID[i] = currentCardUID[i];   // 현재 인식한 카드 UID(currentCardUID)를 lastCardUID로 업데이트
  }
}
void setup() {
  Serial.begin(9600);   // 컴퓨터와 아두이노 간에 시리얼 통신을 초기화 (통신 속도 9600)
  mySerial.begin(9600); // 블루투스와 아두이노 간에 시리얼 통신을 초기화 (통신 속도 9600)
  delay(1000); // 블루투스와 연결 성공시키기 위한 시간
  pinMode(pin1, INPUT);   // pinMode 함수를 사용해 특정 핀을 입력으로 씀
  pinMode(pin2, INPUT);   
  SPI.begin();    // SPI 통신 시작-->RFID 모듈과 같이 SPI 통신을 사용하는 장치들은 이 명령을 통해 통신 시작
  mfrc522.PCD_Init();   // MFRC522 라이브러리를 사용하여 RFID 리더기 모듈을 초기화 (RFID 모듈 사용 전 필수로 수행)
  mySerial.println("Let's start WTEC");
  
  delay(1500);
  mySerial.println("탐색을 원하면: 1 입력");
  delay(1500);
  mySerial.println("종료를 원하면: 2 입력");
}
void loop() {
  if (mySerial.available()) // 블루투스 초기 설정하기 (이름: BMEPRO, 비밀번호 : 000000)
  {
    Serial.write(mySerial.read());
  }
  if(Serial.available())
  {
    mySerial.write(Serial.read());
  }
  int val1 = digitalRead(pin1);   // pin1에서 digital 방식으로 읽어온 값을 변수 val1에 대입 
  int val2 = digitalRead(pin2);   // pin2에서 digital 방식으로 읽어온 값을 변수 val2에 대입 

  (적외선 센서는 물체가 상대적으로 가까워졌을 때 LOW(물체의 접근), 멀어졌을 때 HIGH(물체의 이탈))
// 첫 번째 센서를 시작으로 출입 감지하는 경우
  if(val1==LOW){
    delay(800);   // delay를 추가하여 여러 번 감지하는 것을 방지 
    if(digitalRead(pin2)==LOW){  // pin1이 감지되고 0.8초 후 pin2가 감지되는 경우
      count++;   // count에 1을 더한다(입실)
      Serial.print("1명이 입실했습니다. 강의실 인원수 : "); 
      Serial.println(count);                   
      delay(100);   //다음 사람 감지를 위한 delay 
    }
  }
  // [RFID 감지]
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){    
    // 새로운 카드 감지됐는지 확인--(새로운 카드 감지 여부 판단 함수 && 카드의 UID를 읽어오는 함수) 둘 다 true 일 때 이 조건문 실행
    byte currentCardUID[4];   // 현재 카드 UID를 저장할 배열 선언 (UID는 보통 4byte로 이루어짐)
    for (byte i = 0; i < mfrc522.uid.size; i++){    // uid.size는 현재 카드 UID 배열의 크기
      currentCardUID[i] = mfrc522.uid.uidByte[i];   // 리더기에 태그된 카드 UID 값이 mfrc522.uid.uidByte에 저장이 되는데 이걸 currentCardUID에 복사
    }
        if (isEqualCardUID(currentCardUID, lastCardUID)){   // 현재 카드와 이전 카드의 UID를 비교하는 함수 isEqualCardUID를 호출
      if (Class){
        // 이미 수업이 진행 중인 상태에서 같은 카드가 인식된 경우
        Class = 0;    // 수업 종료
        Serial.println("수업 종료" + String(Class) + count);
        delay(500);   // 0.5초 일시 정지 -- 시리얼 모니터에 표시된 내용 읽기 쉽게 하기 위함
      }
      else{
        // 수업이 진행되지 않고 있는 상태에서 같은 카드가 인식된 경우
        Class = 1;    // 수업 시작
        Serial.println("수업 시작" + String(Class) + count);
        delay(500);
      }
    }
    else{
      // 새로운 카드가 인식된 경우 (isEqualCardUID 함수 결과가 false인 것임!)
      Class = 1;    // 수업 시작
      Serial.println("수업 시작" + String(Class) + count);
      delay(500);  
    }
        //현재 카드 UID를 마지막으로 인식된 카드 UID로 업데이트
    updateLastCardUID(currentCardUID);
  }
    // [블루투스 화면 출력시키기]
  // 선택한 강의실의 수업 여부를 출력하기
  while (mySerial.available())  // 탐색 여부를 받는 while문 1: 계속 탐색 2: 종료 
  {
    switch( (mySerial.read()) ) // 블루투스 시리얼 수신 값(블루투스로 전송되는 값)을 읽는 함수 read() 
    {
      case '1': // 1이 입력될 경우 
        delay(1000);  // 화면에 출력을 제대로 하기 위한 지연 시간
        mySerial.println("강의실 입력 :");  // 탐색하고자 하는 강의실 호수를 물어보는 문장을 화면에 출력
        option = 1; // 변수 option의 값을 1로 변경하고 이 값을 if문에서 활용
        break;  // while문 탈출

      case '2': // 2가 입력될 경우 
        delay(1000); // 화면에 출력을 제대로 하기 위한 지연 시간
        mySerial.println("탐색 종료"); // 탐색 화면 종료 출력
        delay(1000);  // 화면에 출력을 제대로 하기 위한 지연 시간
        mySerial.println("Thanks for"); 
        delay(1000);  // 화면에 출력을 제대로 하기 위한 지연 시간
        mySerial.println("Using WTEC"); // 종료 문구를 화면에 출력
        break;  // while문 탈출
    }
 
    if (option==1) // option의 값이 1일 경우 (= 탐색을 하는 경우)
    {
      while (mySerial.available())
      {
        String data = mySerial.readStringUntil('\n'); // 개행이 들어오기 전까지의 값을 data 변수에 선언
        Serial.println(data); // 호실 확인을 위해서 Serial 화면에 출력시킴
        if (data == "306") // 306호가 입력되었을 경우
        {
          delay(1000);
          if (Class==0) // 306호에서 수업이 진행되고 있지 않을 경우
          {
            mySerial.println("현재 수업 없음"); // 수업 여부를 화면에 출력
            delay(1000);
            mySerial.println("현재 강의실에");
            mySerial.println(String(count) + "명 있음");  // 강의실 내 인원 수를 출력
            delay(1000);
            chair = 50 - count; // 수용 인원을 가정하고 남은 좌석을 구해 chair 변수에 저장
            delay(1000);
            mySerial.println("남은 좌석 " + String(chair) + "석");  // 남은 좌석 화면에 출력
            delay(1500);
            mySerial.println("탐색을 원하면: 1 입력");  // 탐색과 종료를 되묻기 
            delay(1500);
            mySerial.println("종료를 원하면: 2 입력");
          }
          else  // 해당 교실에서 수업을 진행 중일 경우
          {
            mySerial.println("현재 수업 있음"); // 수업이 진행 중이라는 문구 화면에 출력
            delay(1500);
            mySerial.println("탐색을 원하면: 1 입력");  // 탐색과 종료 되묻기
            delay(1500);
            mySerial.println("종료를 원하면: 2 입력");
          }
        }
        if (data == "602") // 602호일 경우에도 306호 코드와 동일한 형식으로 
        {
          delay(1000);
          if (Class==0)
          {
            mySerial.println("현재 수업 없음");
            delay(1000);
            mySerial.println("현재 강의실에");
            mySerial.println(String(count) + "명 있음");
            delay(1000);
            chair = 50 - count;
            delay(1000);
            mySerial.println("남은 좌석 " + String(chair) + "석");
            delay(1500);
            mySerial.println("탐색을 원하면: 1 입력");
            delay(1500);
            mySerial.println("종료를 원하면: 2 입력");
          }
          else
          {
            mySerial.println("현재 수업 있음");
            delay(1500);
            mySerial.println("탐색을 원하면: 1 입력");
            delay(1500);
            mySerial.println("종료를 원하면: 2 입력");
          }
        }
      }
    }
  }
}



