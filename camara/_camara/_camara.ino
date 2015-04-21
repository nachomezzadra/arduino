// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

#include <Wire.h> // For some strange reasons, Wire.h must be included here. Needed by DS1307new
#include <DS1307new.h>

#include <IRremote.h>

#define LEDPIN 13
#define SEGUNDOS_POR_MINUTO 60
const String ESPACIO = "   ";

// RTC vars
uint16_t startAddr = 0x0000;            // Start address to store in the NV-RAM
uint16_t lastAddr;                      // new address for storing in NV-RAM
uint16_t TimeIsSet = 0xaa55;            // Helper that time must not set again

// para la camara de fotos
#define PRENDER 7
#define FOTO 6

const int LIBERAR_PRENDER = 250;  // Setear en 500 ms
const int DELAY_ENCENDIDO = 500; // Tiempo entre el ENCENDIDO y la FOTO
const int DURACION_PULSO = 250;  // Setear en 500 ms

//IR
const int RECV_PIN = 10;
IRrecv irrecv(RECV_PIN);
decode_results results;
unsigned long ultimoValorRecibidoIr = 0x000000;
boolean recibioSenialIr = false;


//------------------------------
int intervaloFotosEnMinutos = 15;
//------------------------------
int intervaloFotoEnSegundos = intervaloFotosEnMinutos * SEGUNDOS_POR_MINUTO;
volatile int segundosPasadosDesdeUltimaFoto = 0;
volatile boolean esMomentoDeSacarFoto = false;


///////////////////////// SET UP ////////////////////////////
void setup() {
    Serial.begin(9600);
    
    setupTimers();       
    
    setupIr();
    
    setupTinyRtc();
    
    pinMode(LEDPIN, OUTPUT); // led de prueba  
}
/////////////////////// FIN SET UP /////////////////////////

///////////////////// LOOP PRINCIPAL ///////////////////////
void loop()
{
  if (recibioSenialIr) {
    Serial.print("Senial IR recibida: ");
    Serial.println(ultimoValorRecibidoIr, HEX);
    procesarSenialIr();
    recibioSenialIr = false;
  }
  
  if (esMomentoDeSacarFoto) {
    sacarFoto();
    // ahora seteamos en false ya que acabamos de sacar foto y no tenemos que sacar otra hasta el proximo intervalo
    esMomentoDeSacarFoto = false;
  }

}
/////////////////// FIN LOOP PRINCIPAL ////////////////////

//-------------------------------------------
/** 
  * Interrupcion invocada por cada segundo.
  * Lleva la cuenta de la cantidad de segundos que pasaron desde la ultima foto que se saco.  Si es momento de sacar una nueva foto, 
  * entonces levanta flag de sacar foto, permitiendo al loop principal sacar la foto.
  * Mantener esta interrupcion lo mas 'rapida' posible dado que mientras se encuentra en esta interrupcion, ninguna otra interrupcion puede ser manejada.
**/
ISR(TIMER1_COMPA_vect) {
    segundosPasadosDesdeUltimaFoto++;
    if (debeSacarFoto(segundosPasadosDesdeUltimaFoto)) {
        Serial.print("Momento de sacar foto ");
        esMomentoDeSacarFoto = true;
        segundosPasadosDesdeUltimaFoto = 0;
    }  
}
//-------------------------------------------
void CHECK_IR(){
  while (irrecv.decode(&results)) {
    recibioSenialIr = true;
    // si el valor no es 'REPEAT'
    if (results.value != 0xFFFFFFFF) {
        imprimirValorIR();
        ultimoValorRecibidoIr = results.value;
    }
    irrecv.resume();
  }
}
//-------------------------------------------
//-------------------------------------------
void imprimirValorIR() {
  
  switch (results.decode_type) {
    case SONY: Serial.print("SONY: "); break;
    case NEC: Serial.print("NEC: "); break;
    case RC5: Serial.print("RC5: "); break;
    case RC6: Serial.print("RC6: "); break;
    case UNKNOWN: Serial.print("UNKNOWN: "); break;
  }
  
  switch (results.value) {
    case 0x849: Serial.println("Remoto SONY CD Player (Boton REPEAT)"); break;
    case 0x4017: Serial.println("Remoto SONY CD Player (Boton FADER)");    break;
    case 0x4294967295: Serial.println("Remoto SONY CD Player (Boton FADER)");    break;
    case 0x1233: Serial.println("Remoto SONY CD Player (Boton PLAY)");   break;
    case 0xFFFFFFFF: Serial.println("REPEAT"); break;  

    default: 
      Serial.println("Otro boton");
  }
}
//-------------------------------------------
/* 
* Determina si debe sacar la foto en base a la cantidad de segundos pasados desde la ultima foto y 
* si se encuentra dentro del intervalo para sacar foto.  De esta manera nos aseguramos que saque 
* fotos siempre a la misma hora y no solamente cada XX segundos desde que inicia el Arduino.
* Si el modulo da 0 queire decir que esta en los minutos exactos, por ende si en ese caso los 
* segunods son menores al intervalo en segundos, quiere decir que acaba de iniciar el Arduino y 
* debemos sacar una foto.
*/
boolean debeSacarFoto(int segundosPasadosDesdeUltimaFoto) {
    int minutosEnHora = RTC.minute;
    int modulo = minutosEnHora % intervaloFotosEnMinutos;
    return (((segundosPasadosDesdeUltimaFoto == intervaloFotoEnSegundos) && (modulo == 0)) || ((modulo == 0) && (segundosPasadosDesdeUltimaFoto < intervaloFotoEnSegundos)));
}
//-------------------------------------------
void sacarFoto() {
//   digitalWrite(LEDPIN, !digitalRead(LEDPIN)); 
  imprimirTiempo();
  
  prenderCamara();
  digitalWrite (FOTO , LOW);
  delay (DURACION_PULSO);
  digitalWrite (FOTO , HIGH);   
  ponerEnStandByCamara();
  
  Serial.println("Foto tomada");
}
//-------------------------------------------
void prenderCamara() {
    digitalWrite (PRENDER , LOW);  // Prende la camara
    delay (DELAY_ENCENDIDO); 
}
//-------------------------------------------
void ponerEnStandByCamara() {
    delay (LIBERAR_PRENDER);
    digitalWrite (PRENDER , HIGH); // Apaga la camara
} 
//-------------------------------------------
void imprimirTiempo() {
    RTC.getTime();
    
    String tiempo = ESPACIO;
    tiempo = tiempo + RTC.hour, DEC;
    tiempo = tiempo + ESPACIO;
    tiempo = tiempo + RTC.minute, DEC;
    tiempo = tiempo + ESPACIO;
    tiempo = tiempo + RTC.second, DEC;
   
    Serial.println(tiempo);
}
//-------------------------------------------
void procesarSenialIr() {
   if (results.decode_type == NEC) {
      Serial.print("NEC: ");
    } else if (results.decode_type == SONY) {
      Serial.print("SONY: ");
    } else if (results.decode_type == RC5) {
      Serial.print("RC5: ");
    } else if (results.decode_type == RC6) {
      Serial.print("RC6: ");
    } else if (results.decode_type == UNKNOWN) {
      Serial.print("UNKNOWN: ");
    }
    Serial.println(results.value);
    irrecv.resume(); // Receive the next value

  
    if (results.value == 849) { //Remoto SONY CD Player (Boton REPEAT)
      Serial.println("REMOTO PRENDER_ON");  
          digitalWrite (PRENDER , LOW); // Prende la camara 
    }
  
    if (results.value == 4017) { //Remoto SONY CD Player (Boton FADER)
      Serial.println("REMOTO PRENDER_OFF");  
          digitalWrite (PRENDER , HIGH); // Apaga la camara 
    }
  
  
    if (results.value == 4294967295) { //Remoto SONY CD Player (Boton FADER)
      Serial.println("REMOTO PRENDER_OFF");  
          digitalWrite (PRENDER , HIGH); // Apaga la camara 
    }
  
    if (results.value == 1233) { //Remoto SONY CD Player (Boton PLAY)
    Serial.println("Sacar Foto Con REMOTO");  
    digitalWrite (FOTO , LOW); // Sacar FOTO 
    delay (DURACION_PULSO);  
    digitalWrite (FOTO, HIGH); // Libera Sacar FOTO
    delay(1000);
    } else {
      digitalWrite (FOTO, HIGH); // Libera Sacar FOTO
   }
 
}
//-------------------------------------------
// FUNCIONES DE SET UP
//-------------------------------------------

/* 
* Setea timers para interrumpir cada un segundo.  La funcion que se llamara por cada interrupcion sera ISR(TIMER1_COMPA_vect).
*/
void setupTimers() {
  // initialize Timer1
  cli();      // disable global interrupts
  TCCR1A = 0;   // set entire TCCR1A register to 0
  TCCR1B = 0;   // same for TCCR1B

  // set compare match register to desired timer count:
  OCR1A = 15624;
  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler:
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  // enable global interrupts:
  sei();  
}  
//-------------------------------------------
// -------------Set up RTC ------------------
void setupTinyRtc() {
        /*
         PLEASE NOTICE: WE HAVE MADE AN ADDRESS SHIFT FOR THE NV-RAM!!!
         NV-RAM ADDRESS 0x08 HAS TO ADDRESSED WITH ADDRESS 0x00=0
         TO AVOID OVERWRITING THE CLOCK REGISTERS IN CASE OF
         ERRORS IN YOUR CODE. SO THE LAST ADDRESS IS 0x38=56!
         */
         RTC.setRAM(0, (uint8_t *)&startAddr, sizeof(uint16_t));// Store startAddr in NV-RAM address 0x08 
       
        /*
         Uncomment the next 2 lines if you want to SET the clock
         Comment them out if the clock is set.
         DON'T ASK ME WHY: YOU MUST UPLOAD THE CODE TWICE TO LET HIM WORK
         AFTER SETTING THE CLOCK ONCE.
         */
         //TimeIsSet = 0xffff;
         //RTC.setRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));  
       
        /*
        Control the clock.
         Clock will only be set if NV-RAM Address does not contain 0xaa.
         DS1307 should have a battery backup.
       
         */
        RTC.getRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
        if (TimeIsSet != 0xaa55)
        {
          RTC.stopClock();
       
          //LAS DOS LINEAS DE ABAJO SETEAN EL RELOJ
            // RTC.fillByYMD(2015,04,19);
            // RTC.fillByHMS(00,13,00);
       
       
          RTC.setTime();
          TimeIsSet = 0xaa55;
          RTC.setRAM(54, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
          RTC.startClock();
        }
       
        {
          RTC.getTime();
        }
       
        /*
         Control Register for SQW pin which can be used as an interrupt.
         */
        RTC.ctrl = 0x00;                      // 0x00=disable SQW pin, 0x10=1Hz,
        // 0x11=4096Hz, 0x12=8192Hz, 0x13=32768Hz
        RTC.setCTRL();
       
        Serial.println("Tiempo Actual");
       
        uint8_t MESZ;          
}
//-------------------------------------------
void setupIr() {
       attachInterrupt(0,CHECK_IR,CHANGE);
       irrecv.enableIRIn(); // Start the receiver
}
//-------------------------------------------
