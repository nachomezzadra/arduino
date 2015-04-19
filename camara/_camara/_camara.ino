// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

#include <Wire.h> // For some strange reasons, Wire.h must be included here. Needed by DS1307new
#include <DS1307new.h>

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


//------------------------------
int intervaloFotosEnMinutos = 15;
//------------------------------
int intervaloFotoEnSegundos = intervaloFotosEnMinutos * SEGUNDOS_POR_MINUTO;
int segundosPasadosDesdeUltimaFoto = 0;
boolean esMomentoDeSacarFoto = false;

void setup() {
        Serial.begin(9600);
  
        // ---------- Setup de los Timers. NO CAMBIAR -------------
	// initialize Timer1
	cli();			// disable global interrupts
	TCCR1A = 0;		// set entire TCCR1A register to 0
	TCCR1B = 0;		// same for TCCR1B

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
        // ------------ Fin Setup de los Timers -----------------
        
        // -------------Set up RTC -----------------------------
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
        // ------------- Fin Set up RTC ---------------------------



        // Set up de camara
	pinMode(LEDPIN, OUTPUT); // led de prueba  
}


///////////////////// LOOP PRINCIPAL ///////////////////////
void loop()
{
  if (esMomentoDeSacarFoto == true) {
    sacarFoto();
    // ahora seteamos en false ya que acabamos de sacar foto y no tenemos que sacar otra hasta el proximo intervalo
    esMomentoDeSacarFoto = false;
  }
    
}
/////////////////// FIN LOOP PRINCIPAL ////////////////////


/** 
  * Interrupcion invocada por cada segundo.
  * Lleva la cuenta de la cantidad de segundos que paso desde la ultima foto que se saco.  Si es momento de sacar una nueva foto, 
  * entonces levanta flag de sacar foto, permitiendo al loop principal sacar la foto.
  * Manter esta interrupcion lo mas 'rapida' posible dado que mientras se encuentra en esta interrupcion, ninguna otra interrupcion puede ser manejada.
**/
ISR(TIMER1_COMPA_vect) {
    segundosPasadosDesdeUltimaFoto++;
    if (segundosPasadosDesdeUltimaFoto == intervaloFotoEnSegundos) {
        Serial.print("Momento de sacar foto ");
        esMomentoDeSacarFoto = true;
        segundosPasadosDesdeUltimaFoto = 0;
    }  
}
//-------------------------------------------
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
