#define TBASE      100
#define nstep     1000
#define ROWS      4
#define COLS      4

const unsigned long DEBOUNCETIME = 50;
volatile unsigned long previousMillis = 0;
volatile unsigned short numberOfButtonInterrupts = 0;
volatile unsigned short lastState;
bool prevState;
unsigned long prec=0;
unsigned long step = 0;
// Arduino Uno source
int keyInterrupt = 2;
volatile short row = -1;
volatile short col = -1;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
//Arduino Mega
byte intRow[ROWS] = {A15, A14, A13, A12};
byte intCols[COLS] = {A11, A10, A9, A8};

void debounce(){
   // sezione critica
   // protegge previousMillis che, essendo a 16it, potrebbe essere danneggiata se interrotta da un interrupt
   // numberOfButtonInterrupts ï¿½ 8bit e non ï¿½ danneggiabile ne in lettura ne in scrittura
   noInterrupts();
   // il valore lastintTime potrï¿½ essere in seguito letto interrotto ma non danneggiato
   unsigned long lastintTime = previousMillis;
   //bool ls = lastState;   
   interrupts();
   
   if ((numberOfButtonInterrupts != 0) //flag interrupt! Rimbalzo o valore sicuro? 
        && (millis() - lastintTime > DEBOUNCETIME ) //se ï¿½ passato il transitorio
		&& prevState != lastState // elimina transizioni anonale LL o HH
		&& digitalRead(intRow[row]) == lastState
	  ) //coincide con il valore di un polling
    { 
    //Serial.print("HIT: "); Serial.println(numberOfButtonInterrupts);
    numberOfButtonInterrupts = 0; // reset del flag
  
    prevState = lastState;
    if(lastState == HIGH){ // fronte di salita
		//Serial.println(" in SALITA");
    }else{
		//Serial.println(" in DISCESA");
		    //Serial.print(row); Serial.print(" - "); Serial.println(col);
		    doBtnAction(row, col);
    }
    //col = -1;
    //row = -1;
  }
}

ISR (PCINT2_vect) // handle pin change interrupt for D8 to D13 here
{
  previousMillis = millis(); // tempo evento
  // default rilascio: fronte di salita su un tasto qualsiasi (0,0)
  row = 0; 
  //col = 0;  
  lastState = HIGH;
  // rivelatore di pressione: fronte di discesa (sul tasto effettivamente premuto)
  for(short i=0; (lastState != LOW) && (i < ROWS); i++){
	  lastState = digitalRead(intRow[i]);
	  if(lastState == LOW){
		  row = i;
		  col = -1;
		  for(short j=0; (col == -1) && (j < COLS); j++){
				digitalWrite(intCols[j], HIGH);
				if(digitalRead(intRow[row])==HIGH){ //genera interrupt che si memorizzano
					col = j;
				}
				digitalWrite(intCols[j], LOW);
		  }
	  }
  } 
  PCIFR |= (1 << PCIF2);  //cancella gli interrupt memorizzati
  numberOfButtonInterrupts++; // contatore rimbalzi e flag segnalazione per entrambi i fronti
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(intCols[0], OUTPUT);
  digitalWrite(intCols[0], LOW);
  pinMode(intCols[1], OUTPUT);
  digitalWrite(intCols[1], LOW);
  pinMode(intCols[2], OUTPUT);
  digitalWrite(intCols[2], LOW);
  pinMode(intCols[3], OUTPUT);
  digitalWrite(intCols[3], LOW);
  
  pinMode(intRow[0], INPUT_PULLUP);
  pinMode(intRow[1], INPUT_PULLUP);
  pinMode(intRow[2], INPUT_PULLUP);
  pinMode(intRow[3], INPUT_PULLUP);
  prevState = HIGH; // si parte da pulsante rilasciato

  Serial.println("GO"); 
  //Arduino Mega interrupts settings
  PCIFR |= (1 << PCIF2);   // clear any outstanding interrupts
  PCICR |= (1 << PCIE2);   // set change interrupt
 
  PCMSK2 |= (1 << PCINT20); //Set PCINT20 (Analog input A12) to trigger an interrupt on state change.
  PCMSK2 |= (1 << PCINT21); //Set PCINT21 (Analog input A13) to trigger an interrupt on state change.
  PCMSK2 |= (1 << PCINT22); //Set PCINT22 (Analog input A14) to trigger an interrupt on state change.
  PCMSK2 |= (1 << PCINT23); //Set PCINT23 (Analog input A15) to trigger an interrupt on state change.
}

void loop() {   
  debounce();
}

void doBtnAction(unsigned short row, unsigned short col){
  Serial.print("CHAR: ");
  Serial.println(hexaKeys[row][col]); 
}

