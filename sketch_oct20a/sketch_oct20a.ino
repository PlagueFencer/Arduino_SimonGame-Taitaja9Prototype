const int buttonPIN_1 = A4;
const int buttonPIN_2 = A3;
// const int buttonPIN_3 = ;
// const int buttonPIN_4 = ;



 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("ESP32 starting");
  pinMode(buttonPIN_1, INPUT);
  pinMode(buttonPIN_2, INPUT);
}
 
void loop() {
  // put your main code here, to run repeatedly:
  int button1_status = digitalRead(buttonPIN_1);
  int button2_status = digitalRead(buttonPIN_2);
  if(button1_status == 1)
  {
    Serial.println("Button state is HIGH");
  }
  else
  {
    Serial.println("Button state is LOW");
  }
  if(button2_status == 1)
  {
    Serial.println("Button state is HIGH");
  }
  else
  {
    Serial.println("Button state is LOW");
  }
}
 
 