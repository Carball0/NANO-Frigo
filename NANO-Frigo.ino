/* ---Controlador y termostato para refrigedador Peltier 2xTEC1-12706---
 * Controlador y termostato para refrigerador Peltier, formado por dos módulos
 * de placa Peltier controlados por relés, sensores de temperatura digitales,
 * y LCD 20x04 para mostrar la información.
 * Ofrece un control total sobre el funcionamiento de la nevera: control manual 
 * de encendido y apagado, modo de máxima y mínima potencia (siempre 12V), corte 
 * de funcionamiento en caso de sobrecalentamiento de los disipadores y 
 * componentes externos, estadísticas de estado y de uso, etc.
 * Los módulos trabajan a dos voltajes:
 * Relé LOW=12V (5A por módulo aprox)
 * Relé HIGH=5V (1.5A por módulo aprox)
 * 
 */
#include <string.h>
#include <Keypad.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

//Teclado de 3 botones - config
const byte rows = 3, cols = 1;
char keys[rows][cols] = {
  {'1'}, {'2'}, {'3'}
};
byte rowPins[rows] = {5, 6, 7};   //Pines digitales para filas
byte colPins[cols] = {8};         //Pines digitales para columnas
char key;                         //Boton registrado por usuario

//Pantalla en A4 (SDA) y A5 (SCL)
//OJO: Relé LOW = 12V, HIGH = 5V
int peltier1 = 2, pelt1State = LOW;
int peltier2 = 3, pelt2State = LOW;
int pinDHT22 = 4;
int pinDHT11 = 9;

float temp, hum;          //Lecturas sensor DHT22
float tempExt;            //Lecturas sensor DHT11
unsigned long startTime;  //Medidor de tiempo de ejecución

//Configuración del usuario 
bool highPerf;          //Modo alto rendimiento (módulos a 12V, 130W~)
bool lowPerf;           //Modo bajo rendimiento (módulos a 5V, 30W~)
bool cutOut;            //Sobrecalentamiento detectado (todo sistema OFF)
int cutOutTemp;         //Temperatura para activar modo cutOut
int cOThreshold;        //Threshold para desactivar modo cutOut
int tempUser;           //Temp seleccionado por usuario
int minTUser, maxTUser; //Temp máxima y mínima para ajustar en termostato
//-------------------------

DHT intT(pinDHT22, DHT22);
DHT extT(pinDHT11, DHT11);

LiquidCrystal_I2C lcd(0x27, 20, 4);
Keypad keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

void setup() {
  lcd.backlight();
  lcd.init();
  lcd.setCursor(1, 0);
  lcd.print("--Nevera Peltier--");
  lcd.setCursor(0, 1);
  lcd.print("Inicio Termostato...");
  delay(500);
  lcd.setCursor(0, 2);
  lcd.print("Cargando config...");
  intT.begin();
  extT.begin();
  startTime = millis();
  pinMode(peltier1, OUTPUT);
  pinMode(peltier2, OUTPUT);

  //------Configuración del usuario (por defecto)------
  highPerf = false; //Modo alta potencia
  lowPerf = false;  //Modo baja potencia
  cutOutTemp = 65;  //Temperatura de corte de corriente
  cOThreshold = 12; //Temp para fin de cutOut (cutOutTemp-cOThreshold)
  cutOut = false;   //Cortar corriente a peltiers OFF por defecto
  tempUser = 12;    //Temperatura termostato por defecto
  minTUser = 4;     //Temperatura termostato mínima
  maxTUser = 28;    //Temperatura termostato máxima
  digitalWrite(peltier1, pelt1State); //Enciende peltiers
  digitalWrite(peltier2, pelt2State);
  //---------------------------------------------------
  delay(1000);
  lcd.clear();
  delay(100);
}

void loop() {
  updateStats();
  printStatsLCD();
  thermostat();
  
  char key = keypad.getKey();
  buttonOptions(key);
  delay(100);
}

void updateStats() {
  float auxTemp = intT.readTemperature();
  float auxHum = intT.readHumidity();
  if(!(isnan(auxTemp) && auxTemp < -2)) {
    temp = auxTemp;
  }
  if(!(isnan(auxHum) && auxHum < 0 && auxHum > 100)) {
    hum = auxHum;
  }
  tempExt = extT.readTemperature();
}

void printStatsLCD() {    //Imprime temperatura y humedad interior
  if(cutOut) {  //En caso de alta temperatura de componentes
    lcd.setCursor(4, 0);
    lcd.print("!OVERHEATING!");
    lcd.setCursor(0, 1);
    lcd.print("Nevera apagada");
    lcd.setCursor(0, 2);
    lcd.print("T_e: ");
    lcd.print(tempExt);
    lcd.print(" T_i: ");
    lcd.print(temp);
    lcd.setCursor(0, 3);
    lcd.print("T_e reanudacion: ");
    lcd.print(cutOutTemp-cOThreshold);
    delay(2000);
    return;
  }
  int i = 0;
  if(pelt1State == LOW) i++;
  if(pelt2State == LOW) i++;

  if(i==0) {
    lcd.setCursor(0, 0);
    lcd.print("-Nevera Peltier LO -");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("-Nevera Peltier ON -");
  }
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp);
  if(highPerf) {
    lcd.print(" -HiPerf-");
  } else if(lowPerf) {
    lcd.print(" -LoPerf-");
  } else {
    lcd.print(" Set: ");
    lcd.print(tempUser);
  }
  lcd.setCursor(0, 2);
  lcd.print("Humedad: ");
  lcd.print(hum);
  lcd.setCursor(0, 3);
  lcd.print("P 12V: ");
  lcd.print(i);
  lcd.print(" P 5V: ");
  lcd.print(2-i);
}

void buttonOptions(char keyPressed) { //Muestra menú de opciones
  String options[4] = {"Cambiar termostato", "Ajuste Manual", "Estadisticas", "Salir"};
  int value = 0;
  if(keyPressed == '1') { //Botón del círculo 
    while(1) {
      lcd.clear();
      key = NO_KEY;
      lcd.setCursor(0, 0);
      lcd.print("-Menu configuracion-");
      while(key==NO_KEY) {
        lcd.setCursor(0, 1);
        lcd.print(options[value]);
        key = keypad.getKey();
        delay(30);
      }
      if(key=='2' && value>0) value--;
      
      if(key=='3' && value<3) value++;

      if(key=='1') {
        bool done = false;
        switch(value) {
          case 0: //Opción Cambio Temperatura
          {
            lcd.clear();
            int oldT = tempUser, newT = tempUser;
            while(!done) {
              lcd.setCursor(0, 0);
              lcd.print("-Cambia Temperatura-");
              lcd.setCursor(0, 1);
              lcd.print("Actual: ");
              if(highPerf) {
                lcd.print("Lowest");
              } else {
                lcd.print(oldT);
              }
              lcd.setCursor(0, 2);
              lcd.print("Set: ");
              lcd.print(newT);
              if(newT < 10) lcd.print(" ");
              lcd.setCursor(0, 3);
              lcd.print("T: ");
              lcd.print(intT.readTemperature());
              lcd.print(" H: ");
              lcd.print(intT.readHumidity());
              while(1) {
                key = keypad.getKey();
                if(key=='1') {
                  tempUser = newT;
                  if(highPerf) highPerf=false;
                  done = true;
                  break;
                } else if (key=='2') {
                  if(newT > minTUser) newT--;
                  break;
                } else if (key=='3') {
                  if(newT < maxTUser) newT++;
                  break;
                }
              }
            }
            lcd.clear();
            return;
          }

          case 1:  //Opción Ajuste Manual
           {
            lcd.clear();
            lcd.setCursor(2, 0);
            lcd.print("-Ajuste Manual-");
            lcd.setCursor(0, 1);
            lcd.print("Max. Rendimiento <");
            lcd.setCursor(0, 2);
            lcd.print("Min. Rendimiento >");
            lcd.setCursor(0, 3);
            lcd.print("Modulos Manual   o");
            key = NO_KEY;
            while(key==NO_KEY) {
              key = keypad.getKey();
              delay(100);
            }
            if(key == '1') {  // ON/OFF Peltiers
              int peltierSelected = 1;
              lcd.clear();
              while(!done) {
                lcd.clear();
                lcd.setCursor(1, 0);
                lcd.print("-ON/OFF  Modulos-");
                lcd.setCursor(0, 1);
                lcd.print("Peltier 1: ");
                if(pelt1State==LOW) lcd.print("HI");
                if(pelt1State==HIGH) lcd.print("LO");
                lcd.setCursor(0, 2);
                lcd.print("Peltier 2: ");
                if(pelt2State==LOW) lcd.print("HI");
                if(pelt2State==HIGH) lcd.print("LO");
                lcd.setCursor(0, 3);
                lcd.print("Cambiar P");
                lcd.print(peltierSelected);
                key = NO_KEY;
                while(1) {
                  key = keypad.getKey();
                  if(key == '1') {
                    if(peltierSelected == 1) {
                      modulesOnOff(!pelt1State, pelt2State);
                    } else if(peltierSelected == 2) {
                      modulesOnOff(pelt1State, !pelt2State);
                    }
                    lcd.setCursor(0, 3);
                    lcd.print("Hecho, pulsa o para salir");
                    lcd.print(peltierSelected);
                    done = true;
                    break;
                  } else if ((key == '2') || (key == '3')) {
                    if(peltierSelected == 1) {
                      peltierSelected = 2;
                    } else if(peltierSelected == 2) {
                      peltierSelected = 1;
                    }
                    lcd.setCursor(0, 3);
                    lcd.print("Cambiar P");
                    lcd.print(peltierSelected);
                    break;
                  }
                }
              }
              lcd.clear();
              return;
            }
            if(key == '3') {  // Max. Rendimiento
              lcd.clear();
              lcd.setCursor(1, 0);
              lcd.print("-Max. Rendimiento-");
              lcd.setCursor(0, 1);
              lcd.print("Desactiva termostato");
              lcd.setCursor(0, 2);
              lcd.print("Peltier siempre 12V");
              delay(1500);
              lcd.clear();
              lcd.setCursor(1, 0);
              lcd.print("-Max. Rendimiento-");
              lcd.setCursor(0, 1);
              lcd.print("o para activar");
              lcd.setCursor(0, 2);
              lcd.print("< o > para desact.");
              key = NO_KEY;
              while(key == NO_KEY) {
                key = keypad.getKey();
                if(key == '1') {
                  lcd.clear();
                  lcd.setCursor(1, 0);
                  lcd.print("-Max. Rendimiento-");
                  lcd.setCursor(6, 1);
                  lcd.print("Activado");
                  pelt1State = LOW;
                  pelt2State = LOW;
                  digitalWrite(peltier1, pelt1State);
                  digitalWrite(peltier2, pelt2State);
                  delay(1000);
                  highPerf = true;
                  if(lowPerf) lowPerf = false;
                  return;
                } else if(key == '2' || key == '3') {
                  lcd.clear();
                  lcd.setCursor(1, 0);
                  lcd.print("-Max. Rendimiento-");
                  lcd.setCursor(5, 1);
                  lcd.print("Desactivado");
                  delay(1000);
                  highPerf = false;
                  return;
                }
              }
            }
            if(key == '2') {  // Min. Rendimiento
              lcd.clear();
              lcd.setCursor(1, 0);
              lcd.print("-Min. Rendimiento-");
              lcd.setCursor(0, 1);
              lcd.print("Desactiva termostato");
              lcd.setCursor(0, 2);
              lcd.print("Modulos bajo consumo");
              lcd.setCursor(0, 3);
              lcd.print("(Bateria, coche...)");
              delay(1500);
              lcd.clear();
              lcd.setCursor(1, 0);
              lcd.print("-Min. Rendimiento-");
              lcd.setCursor(0, 1);
              lcd.print("o para activar");
              lcd.setCursor(0, 2);
              lcd.print("< o > para desact.");
              key = NO_KEY;
              while(key == NO_KEY) {
                key = keypad.getKey();
                if(key == '1') {
                  lcd.clear();
                  lcd.setCursor(1, 0);
                  lcd.print("-Min. Rendimiento-");
                  lcd.setCursor(6, 1);
                  lcd.print("Activado");
                  pelt1State = HIGH;
                  pelt2State = HIGH;
                  digitalWrite(peltier1, pelt1State);
                  digitalWrite(peltier2, pelt2State);
                  delay(1000);
                  lowPerf = true;
                  if(highPerf) highPerf = false;
                  return;
                } else if(key == '2' || key == '3') {
                  lcd.clear();
                  lcd.setCursor(1, 0);
                  lcd.print("-Min. Rendimiento-");
                  lcd.setCursor(5, 1);
                  lcd.print("Desactivado");
                  delay(1000);
                  lowPerf = false;
                  return;
                }
              }
            }
            return;
          }
         
         case 2:    //Estadisticas
         {
            lcd.clear();
            key = NO_KEY;
            lcd.setCursor(3, 0);
            lcd.print("-Estadisticas-");
            while(key == NO_KEY){
              lcd.setCursor(0, 1);
              lcd.print("ON: ");
              unsigned long elapsed = millis() - startTime;
              lcd.print(elapsed/1000/60/60); lcd.print("h ");
              lcd.print((elapsed/1000/60)%60); lcd.print("m ");
              lcd.print((elapsed/1000)%60); lcd.print("s");
              lcd.setCursor(0, 2);
              lcd.print("T_e: ");
              lcd.print(tempExt);
              lcd.print(" T_i: ");
              lcd.print(temp);
              lcd.setCursor(0, 3);
              lcd.print("CutOut: ");
              lcd.print(cutOutTemp);
              updateStats();
              key = keypad.getKey();
              delay(250);
            }
            lcd.clear();
            return;
          }
          
          case 3: return;   //Salir
          
        lcd.clear();
        }
      }
    }
  }
}

void modulesOnOff(int peltier1Aux, int peltier2Aux) { //Apaga o enciende Peltiers basados en el estado que recibe
  if(peltier1Aux != LOW && peltier1Aux != HIGH || peltier2Aux != LOW && peltier2Aux != HIGH) {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("!! ERROR");
    lcd.setCursor(0, 1);
    lcd.print("INVALID VALUE");
    lcd.setCursor(0, 2);  
    lcd.print("PELTIER NOT SET");
    delay(3000);
    lcd.clear();
    return;
  }
  if(peltier1Aux != pelt1State) {
    pelt1State = peltier1Aux;
    digitalWrite(peltier1, pelt1State);
    delay(200);
  }
  if(peltier2Aux != pelt2State) {
    pelt2State = peltier2Aux;
    digitalWrite(peltier2, pelt2State);
    delay(200);
  }
}

void thermostat() {
  //Si se supera la temperatura externa umbral, apagamos módulos
  if(cutOut && (pelt2State == HIGH || pelt1State == HIGH)) {
    if(highPerf) highPerf = false;
    if(extT.readTemperature() <= cutOutTemp-cOThreshold) cutOut = false;
    return;
  } else if(extT.readTemperature() >= cutOutTemp) {
    lcd.clear();
    modulesOnOff(HIGH, HIGH);
    cutOut = true;
    return;
  }

  //Comportamiento Alto Rendimiento (Enfriamiento máximo)
  if(highPerf && pelt1State == LOW && pelt2State == LOW) {
    return;
  } else if(highPerf && (pelt1State == HIGH || pelt2State == HIGH)) {
    modulesOnOff(LOW, LOW);
    return;
  }

  //Comportamiento Bajo Rendimiento (Bajo consumo)
  if(lowPerf && pelt1State == HIGH && pelt2State == HIGH) {
    return;
  } else if(lowPerf && (pelt1State == LOW || pelt2State == LOW)) {
    modulesOnOff(HIGH, HIGH);
    return;
  }
  
  //Si la temperatura baja 1 grado por debajo de lo establecido cambiamos a 5V
  if(temp<=tempUser-1) {
    modulesOnOff(HIGH, HIGH);
    return;
  }
  
  //Si la temperatura sube por encima de lo establecido se vuelve a 12V
  if(temp>=tempUser+1 && (pelt1State == HIGH || pelt2State == HIGH)) {
    modulesOnOff(LOW, LOW);
  }
}
