  #include <PWMServo.h>
  #include <EEPROM.h>

  /* union to address individual bytes of 32bit int */
  typedef union
  {
    uint32_t uint;
    uint8_t ubyte[4];
  } eep_val;
  
  uint32_t sig = 0x01345678;
  bool calibrated = true;
  uint32_t pwm_neut = 0, pwm_min =0, pwm_max =0;

  uint8_t  pwm_byte;
  uint8_t  eep_ptr = 0;
  char kbd_char = 0;
  
  PWMServo ServoFront; 
  int posServoFront = 0;
  uint32_t pwm_val = 0;
  int ServoFront_neutral;
  int led = 13; //LED light
  int PWM_input = 23;


uint32_t read_afew( uint32_t howmany) {
  uint32_t val = 0;
  uint32_t i;

  for (i = 0; i < howmany; i++) {
      val=val+pulseIn(PWM_input,HIGH,21000);  
  }
  return (val/howmany);
}

void eep_write(uint32_t uint) {

    eep_val eep;

    eep.uint = uint;

 //   Serial.printf("Write %u to eeprom @ addr %u: %u %u %u %u\n", (uint32_t) eep.uint, eep_ptr, (uint8_t) eep.ubyte[3], (uint8_t) eep.ubyte[2], (uint8_t) eep.ubyte[1], (uint8_t) eep.ubyte[0]);
 
    EEPROM.write(eep_ptr++, eep.ubyte[3]);
    EEPROM.write(eep_ptr++, eep.ubyte[2]);
    EEPROM.write(eep_ptr++, eep.ubyte[1]);
    EEPROM.write(eep_ptr++, eep.ubyte[0]);
}

uint32_t eep_read() {

    eep_val eep;
    
//    Serial.printf("Reading from eeprom @ addr %u ", eep_ptr);
    eep.ubyte[3] = EEPROM.read(eep_ptr++);
    eep.ubyte[2] = EEPROM.read(eep_ptr++);
    eep.ubyte[1] = EEPROM.read(eep_ptr++);
    eep.ubyte[0] = EEPROM.read(eep_ptr++);
    
//    Serial.printf("read %u: %u %u %u %u\n", (uint32_t) eep.uint, eep_ptr, (uint8_t) eep.ubyte[3], (uint8_t) eep.ubyte[2], (uint8_t) eep.ubyte[1], (uint8_t) eep.ubyte[0]);
    return eep.uint;
}

  
void setup() {
  // put your setup code here, to run once:
  ServoFront.attach(5,800,2000); // pin 5 for front servo
  posServoFront = 100;
  ServoFront.write(posServoFront);
    
  pinMode(led, OUTPUT); 
  pinMode(PWM_input,INPUT);
  uint32_t start_time = millis();
  while (!Serial && ((millis() - start_time) < 5000)) ;
  
  Serial.begin(115200); // it's always 12M Baud
  if (eep_read() != sig) {
        Serial.printf("Calibration data cannot be found!");
        calibrated = false;
  }
  digitalWrite(led, HIGH); 
}

void reverse_dir() {

  
}


void menu() {
  Serial.println("");
  Serial.println("================= Welcome to service menu ===================");
  Serial.println("press \"C\" for calibration");
  Serial.println("press \"R\" for reversing servo direction");
  Serial.println("press \"Q\" for exit the menu");
  while (true) {
    while (!Serial.available()) continue;
    kbd_char = Serial.read();
//    Serial.printf("serial read: %c\n", kbd_char);
    if  ((kbd_char == 'c') || (kbd_char == 'C')) {
        calibrated = false;
        continue;
    }
    
    if  ((kbd_char == 'r') || (kbd_char == 'R')) {
        if (calibrated) 
          reverse_dir();
        else 
          Serial.println("Need to be calibrated first!");
        continue;
    }
    
    if  ((kbd_char == 'q') || (kbd_char == 'Q')) {
        return;
    }
  }
}

void front_test() {

  posServoFront = 60;ServoFront.write(posServoFront);Serial.printf("Servo position: %u\n",posServoFront);delay(2000); 
  posServoFront = 100;ServoFront.write(posServoFront);Serial.printf("Servo position: %u\n",posServoFront);delay(2000); 
  posServoFront = 140;ServoFront.write(posServoFront);Serial.printf("Servo position: %u\n",posServoFront);delay(2000); 
  posServoFront = 100;ServoFront.write(posServoFront);Serial.printf("Servo position: %u\n",posServoFront);delay(2000); 
  
}

bool front_steer() {
  int degree = 0;
  
  pwm_val = read_afew(1);
  // calibrarion is considered not good if the value is more than 6.25% outside of calibration range
  if ((pwm_val > (pwm_max + pwm_max/17)) || (pwm_val < (pwm_min - pwm_min/17))) {
    Serial.println("PWM value is outside of calibration range. Please re-calibrate!");
    return false;
  }  
  if (pwm_val > pwm_neut) {
    degree = (pwm_max - pwm_neut)/8; 
    posServoFront = (pwm_val - pwm_neut) / degree;
    posServoFront = posServoFront*5 + 100; 
  } else {
    degree = (pwm_neut - pwm_min)/8;
    posServoFront = (pwm_neut - pwm_val) / degree;
    posServoFront = 100 - posServoFront*5;
  }
  Serial.printf("PWM value: %u, front steering angle: %u\n", pwm_val, posServoFront);
  ServoFront.write(posServoFront);
  return true;

  
  /*
  if (pwm_val > pwm_neut) {
    degree = (pwm_max - pwm_neut)/40; 
    posServoFront = (pwm_val - pwm_neut) / degree;
    if (posServoFront%5<2.5){
      posServoFront-=posServoFront%5;
    } else {
      posServoFront+=(5 - (posServoFront%5));
    }
    posServoFront = posServoFront + 100; 
  } else {
    degree = (pwm_neut - pwm_min)/40;
    posServoFront = (pwm_neut - pwm_val) / degree;
    if (posServoFront%5<2.5){
      posServoFront-=posServoFront%5;
    } else {
      posServoFront+=(5 - (posServoFront%5));
    }
    posServoFront = 100 - posServoFront;
  }
  Serial.printf("PWM value: %u, front steering angle: %u\n", pwm_val, posServoFront);
  ServoFront.write(posServoFront);
  return true;
*/  
}

void loop() {

  Serial.print("Press \"M\" key to drop into the service menu");

  for (uint8_t i = 0; i < 11; i++) {
    while (Serial.available()) {
      kbd_char = Serial.read();
//      Serial.printf("serial read: %c\n", kbd_char);
      if  ((kbd_char == 'm') || (kbd_char == 'M')) {
        menu();
      }
    }
    Serial.printf(" %u", 10 -i);
    delay(1000);
  }
  
  Serial.println("");
  Serial.println("Continuing with normall boot");
  // put your main code here, to run repeatedly:
  if (calibrated == true) {

    pwm_neut = eep_read();
    pwm_max = eep_read();
    pwm_min = eep_read();
    
    Serial.printf("Neutal position: %u\n", pwm_neut);
    Serial.printf("Rightmost position: %u\n", pwm_max);
    Serial.printf("Leftmost position: %u\n", pwm_min);
    
  } else { 
    Serial.println("***** Please calibrate the PWM input *****");
start_cal:
    Serial.println("1. Turn ON remote controller and move control to neutral position then wait until further instructions...");
//    Serial.println("2. Move control a few times to both maxinum left and maximum right positions without any specific order.");
    delay(2000);
    
    pwm_neut = read_afew(2000);

    if (pwm_neut == 0) { // no PWM was detected 
      goto start_cal;
    }
    

    Serial.printf("PWM Neutal position: %u\n", pwm_neut);
    Serial.println("2. Now move control to rightmost position and hold it there until further instructions...");
    delay(1000);
    pwm_max = read_afew(2000);
    Serial.printf("PWM Rightmost position: %u\n", pwm_max);


    Serial.println("2. Now move control to leftmost position and hold it there until further instructions...");
    delay(1000);
    pwm_min = read_afew(2000);
    Serial.printf("PWM Leftmost position: %u\n", pwm_min);
 
    // write calibration values to eeprom
    eep_ptr = 0;
    eep_write(sig);
    eep_write(pwm_neut);
    eep_write(pwm_max);
    eep_write(pwm_min);

  }
 //   while (true) continue;
ServoFront_neutral = ServoFront.read();
Serial.printf("Servo neutral position: %u\n", ServoFront_neutral);
delay(2000);



/*
  for (posServoFront = ServoFront_neutral +5; posServoFront < ServoFront_neutral + 45; posServoFront += 5 )
//for(posServoFront = 10; posServoFront < 190; posServoFront += 10)  // goes from 10 degrees to 170 degrees 
  {                                  // in steps of 1 degree 
    ServoFront.write(posServoFront);              // tell servo to go to position in variable 'pos' 
    Serial.println(posServoFront);
    delay(1000);                       // waits 15ms for the servo to reach the position 
  } 
  for (posServoFront = ServoFront_neutral + 45; posServoFront > ServoFront_neutral - 45; posServoFront -= 5 )
  //for(posServoFront = 170; posServoFront>=10; posServoFront-=10)     // goes from 180 degrees to 0 degrees 
  {                                
    ServoFront.write(posServoFront);              // tell servo to go to position in variable 'pos' 
    Serial.println(posServoFront);
    delay(1000);                       // waits 15ms for the servo to reach the position 
  } 
  ServoFront.write(ServoFront_neutral);
  Serial.println(ServoFront_neutral);
*/
while (front_steer()) continue;
// we should not get here...
while (true) continue;
}
