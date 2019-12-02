#include "EasyCAT.h"
#include <SPI.h>
#include <Servo.h>

// EasyCATクラスのインスタンス
EasyCAT easyCAT;

unsigned long Millis = 0;
unsigned long PreviousMillis = 0;

// 実験用のサーボ
Servo servo;

void setup()
{
  // サーボはポート3に接続
  servo.attach(3);
  servo.write(90);
  
  Serial.begin(115200);
  Serial.print ("\nEasyCAT - Generic EtherCAT slave\n");

  // EasyCATの初期化
  if (easyCAT.Init() == true)
  {
    Serial.print ("initialized");
  }
  else
  {
    // 初期化に失敗した場合
    Serial.print ("initialization failed");
    
    pinMode(13, OUTPUT);
    while(1)
    {
      digitalWrite (13, LOW);
      delay(500);
      digitalWrite (13, HIGH);
      delay(500);
    }
  }
  PreviousMillis = millis();
}

void loop()
{
  // EasyCATのメインタスク
  easyCAT.MainTask();
  // ユーザーアプリケーション
  Application();
}

void Application ()
{
  // 10msecごとに実行
  Millis = millis();
  if (Millis - PreviousMillis >= 10)
  {
    PreviousMillis = Millis;
    
    // OUTバッファの0バイトめの値をサーボに出力
    servo.write(easyCAT.BufferOut.Byte[0]);
    
    // INバッファの0～1バイトめにA0ポートのアナログ入力値を格納
    int vol = analogRead(0);
    byte vol_h = (byte)(vol >> 8);
    byte vol_l = (byte)(vol & 0xFF);
    easyCAT.BufferIn.Byte[0] = vol_h;
    easyCAT.BufferIn.Byte[1] = vol_l;
  }
}
