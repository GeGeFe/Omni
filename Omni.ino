// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include <AFMotor.h>
#include <SoftwareSerial.h>

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

struct MotorControl {
  bool activo;              // Indica si el motor está activo
  unsigned long tiempoFin;  // Almacena el tiempo en que debe detenerse
};

MotorControl motores[4];  // Array para 4 motores

int luztrasera = 13;
int luzdelantera = A1;

int mibocina = A0;
boolean tonoActivo = false;       // Indica si el tono está activo
unsigned long tiempoTonoFin = 0;  // Almacena el tiempo en que debe detenerse el tono

SoftwareSerial BT1(0, 1);  // RX | TX

char comando = 'S';
char prevComando = 'A';
int velocidad = 255;
unsigned long duracion_maxima = 2000;
unsigned long timer0 = 2000;  //Stores the time (in millis since execution started)
unsigned long timer1 = 0;     //Stores the time when the last command was received from the phone

// Función para dividir la cadena
int split(String str, char delimiter, String* arr, int maxElements) {
  int index = 0;
  int start = 0;
  while (index < maxElements && (start = str.indexOf(delimiter)) != -1) {
    arr[index++] = str.substring(0, start);
    str = str.substring(start + 1);
  }
  if (index < maxElements) {
    arr[index++] = str;  // Agrega el último elemento
  }
  return index;  // Devuelve el número de elementos
}

void setup() {
  //  Serial.begin(9600);           // set up Serial library at 9600 bps
  pinMode(mibocina, OUTPUT);      // Bocina
  pinMode(luzdelantera, OUTPUT);  // Luz delantera
  pinMode(luztrasera, OUTPUT);    // Luz trasera

  // turn on motor
  motor1.setSpeed(velocidad);  // Rueda
  motor2.setSpeed(velocidad);  // Rueda
  motor3.setSpeed(velocidad);  // Cuello
  motor4.setSpeed(velocidad);  // Pinza

  //** Testeo **
  motor1.run(FORWARD);
  delay(1000);
  motor1.run(RELEASE);

  motor2.run(FORWARD);
  delay(1000);
  motor2.run(RELEASE);

  motor3.run(FORWARD);
  delay(1000);
  motor3.run(RELEASE);

  motor4.run(FORWARD);
  delay(1000);
  motor4.run(RELEASE);

  tone(mibocina, 1000);
  delay(500);
  noTone(mibocina);

  digitalWrite(luztrasera, HIGH);
  delay(500);
  digitalWrite(luztrasera, LOW);

  //** Fin testeo **

  BT1.begin(9600);
}

void loop() {
  if (BT1.available()) {  // Usar si control por bluetooth.
    //  if(Serial.available()>0) { // Usar si control por puerto serie.
    timer1 = millis();
    prevComando = comando;
    String input = BT1.readStringUntil('\n');  // Lee hasta el salto de línea
    comando = input.charAt(0);
    String params[10]; // Array para almacenar los parámetros
    int count = split(input, ' ', params, 10); // Divide la cadena por espacios
    //    comando = BT1.read();
    //    Serial.write(comando);
    //  comando = Serial.read(); // Usar si control por puerto serie.
    if (comando != prevComando) {
      switch (comando) {
        // Código para bocina.
        case 'V':
          tonoActivo = true;
          tone(mibocina, 1000);
          break;
        case 'v':
          tonoActivo = false;
          break;
        // Código para motores.
        case 'a':
          motor1.run(FORWARD);
          motores[0].activo = true;
          motores[0].tiempoFin = millis() + params[1].toInt(); // Duración en parámetro
          break;
        case 'A':
          motor1.run(BACKWARD);
          motores[0].activo = true;
          motores[0].tiempoFin = millis() + params[1].toInt(); // Duración en parámetro
          break;
        case 'b':
          motor2.run(FORWARD);
          motores[1].activo = true;
          motores[1].tiempoFin = millis() + params[1].toInt(); // Duración en parámetro
          break;
        case 'B':
          motor2.run(BACKWARD);
          motores[1].activo = true;
          motores[1].tiempoFin = millis() + params[1].toInt(); // Duración en parámetro
          break;
        case 'c':
          motor3.run(FORWARD);
          motores[2].activo = true;
          motores[2].tiempoFin = millis() + params[1].toInt(); // Duración en parámetro
          break;
        case 'C':
          motor3.run(BACKWARD);
          motores[2].activo = true;
          motores[2].tiempoFin = millis() + params[1].toInt(); // Duración en parámetro
          break;
        case 'S':  // Solo detiene ruedas
          motor3.run(RELEASE);
          motores[2].activo = false;
          motores[2].tiempoFin = 0;
          motor2.run(RELEASE);
          motores[1].activo = false;
          motores[1].tiempoFin = 0;
          motor1.run(RELEASE);
          motores[0].activo = false;
          motores[0].tiempoFin = 0;
          break;
        case 'D':  // Detiene todos los motores
          motor4.run(RELEASE);
          motores[3].activo = false;
          motores[3].tiempoFin = 0;
          motor3.run(RELEASE);
          motores[2].activo = false;
          motores[2].tiempoFin = 0;
          motor2.run(RELEASE);
          motores[1].activo = false;
          motores[1].tiempoFin = 0;
          motor1.run(RELEASE);
          motores[0].activo = false;
          motores[0].tiempoFin = 0;
          break;
        // Código para luces.
        case 'W':
          digitalWrite(luzdelantera, HIGH);
          break;
        case 'w':
          digitalWrite(luzdelantera, LOW);
          break;
        case 'U':
          digitalWrite(luztrasera, HIGH);
          break;
        case 'u':
          digitalWrite(luztrasera, LOW);
          break;
        case '1':
          motor2.run(FORWARD);
          motor3.run(BACKWARD);
          break;
        case 'T':
/*          // Extraer los parámetros
          int espacio1 = input.indexOf(' ', 2);
          if (espacio1 != -1) {
            int frecuencia = input.substring(2, espacio1).toInt();
            int duracion = input.substring(espacio1 + 1).toInt();
*/
            // Generar el tono
            tone(mibocina, params[1].toInt(), params[2].toInt());
            tonoActivo = true;
            tiempoTonoFin = millis() + params[1].toInt();  // Calcular el tiempo en que debe detenerse
          break;  // Salir del case
          comando = "";
      }
    }
  }

  if (tonoActivo) {
    if (millis() >= tiempoTonoFin) {
      noTone(mibocina);    // Detener el tono
      tonoActivo = false;  // Marcar que el tono ya no está activo
    }
  } else {
    noTone(mibocina);
  }

  for (int i = 0; i < 4; i++) {
    if (motores[i].activo && millis() >= motores[i].tiempoFin) {
      // Detener el motor
      switch (i) {
        case 0: motor1.run(RELEASE); break;
        case 1: motor2.run(RELEASE); break;
        case 2: motor3.run(RELEASE); break;
        case 3: motor4.run(RELEASE); break;
      }
      motores[i].activo = false;  // Marcar como inactivo
    }
  }
}