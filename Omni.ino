#include <AFMotor.h>
#include <SoftwareSerial.h>

struct MotorControl {
  bool activo = false;          // Indica si el motor está activo
  unsigned long tiempoFin = 0;  // Almacena el tiempo en que debe detenerse
  AF_DCMotor* motor;
};

const int totalmotores = 4;
MotorControl motores[totalmotores];  // Array para 4 motores

const int luztrasera = 13;
const int luzdelantera = A1;
const int luztesteo = luztrasera;
const int mibocina = A0;

boolean tonoActivo = false;       // Indica si el tono está activo
unsigned long tiempoTonoFin = 0;  // Almacena el tiempo en que debe detenerse el tono

boolean testeado = false;  // Indica si ya se realizó el testeo inicial

SoftwareSerial BT1(0, 1);  // RX | TX

char comando = 'S';

// Función para dividir la cadena
int split(String str, char delimiter, String* arr, int maxElements) {
  int index = 0;
  int start = 0;
  while (index < maxElements && (start = str.indexOf(delimiter)) != -1) {
    arr[index++] = str.substring(0, start);
    str = str.substring(start + 1);
  };
  if (index < maxElements) {
    arr[index++] = str;  // Agrega el último elemento
  };
  return index;  // Devuelve el número de elementos
};

void MoverMotor(int id, unsigned long duracion, int direccion, int velocidad) {
  motores[id].motor->setSpeed(velocidad);
  if (direccion == 0) {
    motores[id].motor->run(FORWARD);
  } else {
    motores[id].motor->run(BACKWARD);
  };
  motores[id].activo = true;
  motores[id].tiempoFin = millis() + duracion;
}

void DetenerMotor(int id) {
  motores[id].motor->run(RELEASE);
  motores[id].activo = false;
  motores[id].tiempoFin = 0;
}

void pulso() {
  digitalWrite(luztesteo, HIGH);
  delay(20);
  digitalWrite(luztesteo, LOW);
}

boolean Testeo() {
  for (int i = 0; i < totalmotores; i++) {
    pulso();
    MoverMotor(i, 1000, 0, 255);
    delay(1000);
    DetenerMotor(i);

    pulso();
    MoverMotor(i, 1000, 1, 255);
    delay(1000);
    DetenerMotor(i);
  };

  pulso();
  tone(mibocina, 1000);
  delay(1000);
  noTone(mibocina);

  pulso();
  digitalWrite(luztrasera, HIGH);
  delay(1000);
  digitalWrite(luztrasera, LOW);

  pulso();
  digitalWrite(luzdelantera, HIGH);
  delay(1000);
  digitalWrite(luzdelantera, LOW);
  return true;
}

void setup() {

  for (int i = 0; i < totalmotores; i++) {
    motores[i].motor = new AF_DCMotor(i + 1);
  };

  pinMode(mibocina, OUTPUT);      // Bocina
  pinMode(luzdelantera, OUTPUT);  // Luz delantera
  pinMode(luztrasera, OUTPUT);    // Luz trasera

  BT1.begin(9600);
  Serial.begin(9600);
};

void loop() {
  if (!testeado) {
    testeado = Testeo();
  };

  if (BT1.available()) {  // Usar si control por bluetooth.
    //  if(Serial.available()>0) { // Usar si control por puerto serie.
    String entradabluetooth = BT1.readStringUntil('\n');  // Lee hasta el salto de línea
    Serial.println(entradabluetooth);
    //    comando = entradabluetooth.charAt(0);
    String params[10];                                     // Array para almacenar los parámetros
    int count = split(entradabluetooth, ' ', params, 10);  // Divide la cadena por espacios
    comando = params[0].charAt(0);                         // Ver de agregar una pila de comandos para que no se pierdan o se ejecuten superpuestos
    switch (comando) {
      case 'T':
        testeado = false;
        break;
      // ** Código para motores. **
      case 'M':
        MoverMotor(params[1].toInt(), params[2].toInt(), params[3].toInt(), params[4].toInt());
        break;
      case '1':
        MoverMotor(0, params[1].toInt(), params[2].toInt(), params[3].toInt());
        if (params[2].toInt() == 0) {
          MoverMotor(2, params[1].toInt(), 1, params[3].toInt());
        } else {
          MoverMotor(2, params[1].toInt(), 0, params[3].toInt());
        }
        break;
      case '2':
        MoverMotor(1, params[1].toInt(), params[2].toInt(), params[3].toInt());
        if (params[2].toInt() == 0) {
          MoverMotor(0, params[1].toInt(), 1, params[3].toInt());
        } else {
          MoverMotor(0, params[1].toInt(), 0, params[3].toInt());
        }
        break;
      case '3':
        MoverMotor(0, params[1].toInt(), params[2].toInt(), params[3].toInt());
        if (params[2].toInt() == 0) {
          MoverMotor(1, params[1].toInt(), 1, params[3].toInt());
        } else {
          MoverMotor(1, params[1].toInt(), 0, params[3].toInt());
        }
        break;
      case 'S':  // Solo detiene ruedas
        DetenerMotor(0);
        DetenerMotor(1);
        DetenerMotor(2);
        break;
      case 'D':  // Detiene todos los motores
        for (int i = 0; i < totalmotores; i++) {
          DetenerMotor(i);
        }
        break;
      // ** Código para luces. **
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
      // ** Codigo para buzzer. **
      case 'F':
        tone(mibocina, params[1].toInt());
        tonoActivo = true;
        tiempoTonoFin = millis() + params[2].toInt();  // Calcular el tiempo en que debe detenerse
        break;
      case 'H':  // Respuesta a conección por Bluetooth
        tone(mibocina, 1000, 250);
        delay(250);
        tone(mibocina, 500, 250);
        delay(250);
        BT1.println("H");
        break;
    };
    comando = "";
  };

  if (tonoActivo && (millis() >= tiempoTonoFin)) {
    noTone(mibocina);    // Detener el tono
    tonoActivo = false;  // Marcar que el tono ya no está activo
      BT1.println("f");
  };

  for (int i = 0; i < totalmotores; i++) {
    if (motores[i].activo && (millis() >= motores[i].tiempoFin)) {  // Detener el motor
      DetenerMotor(i);
      BT1.print("m");
      BT1.println(i);
    };
  };
};
