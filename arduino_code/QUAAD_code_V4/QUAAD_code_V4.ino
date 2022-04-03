//// VARIABLES ////
// Pin addressing
const int pin_CLOCK = 9;
const int pin_RESET = 8;
const int pin_CLK_DIV_IN[4]  = {A0, A1, A6, A3}; //Analog pins
const int pin_CLK_DIV_OUT[4] = {3, 2, 1, 0}; //Digital pins
const int pin_PTRN[4] = {A4, A5, A2, A7};
const int pin_A[4] = {4, 6, 10, 12};
const int pin_B[4] = {5, 7, 11, 13};


// Reset input
bool wasReset = false;

// Clock input
int last_clock_state = 0;
int curr_clock_state = 0;
bool went_up   = false;
bool went_down = false;
int clockCount = 0;

// Clock divider
int divisions[] = {32, 16, 12, 8, 7, 5, 4, 3, 2, 1};
int mapped_div_ind = 1;
int clock_div_read = 0;
int division[4] = {1, 1, 1, 1};
int clock_div_mid[4] = {2000, 2000, 2000, 2000};

// Pattern reader
int pattern_read = 0;
int ptrn_size = 30;
int ptrn_offset = 309;
int ptrn_mid[4] = {2000, 2000, 2000, 2000};
int pattern[4] = {1, 1, 1, 1};
// Sequencer
int seq_step = 1;

void setup() {
analogReference(DEFAULT);
pinMode(pin_CLOCK,INPUT);
for (int i = 0; i <= 3; i++){
pinMode(pin_A[i],OUTPUT); pinMode(pin_B[i],OUTPUT); pinMode(pin_CLK_DIV_OUT[i],OUTPUT);
//pinMode(pin_PTRN[i],INPUT); pinMode(pin_CLK_DIV_IN[i],INPUT);
clock_div_read = analogRead(pin_CLK_DIV_IN[i]);
    mapped_div_ind   = constrain(map(clock_div_read,0,922,0,9),0,9);
    division[i]      = divisions[mapped_div_ind];
    clock_div_mid[i] = 51+(1024/10)*mapped_div_ind;
}
}

void loop() {
// Reset input
if (digitalRead(pin_RESET) == HIGH){
  if (wasReset == false){
    clockCount = 0;
    wasReset = true;}}
else {wasReset = false;}
  
// Clock input
curr_clock_state = digitalRead(pin_CLOCK);
if      (curr_clock_state == 1 & last_clock_state == 0){went_up   = true;
                                                        if(clockCount<3360){clockCount++;} //3360 can be divided by the numbers in divisions[]
                                                        else               {clockCount=1;}}
else if (curr_clock_state == 0 & last_clock_state == 1){went_down = true;}
else                                                   {went_up = false; went_down = false;}
last_clock_state = curr_clock_state;


// Clock Divider, Sequencer
if (went_up){
  for (int i = 0; i <= 3; i++) { 
    
    if ((clockCount+division[i]-1) % division[i] == 0){
    digitalWrite(pin_CLK_DIV_OUT[i],HIGH);
      seq_step = (((clockCount-1)/division[i])%4)+1;
      patternDriver(pattern[i],seq_step,pin_A[i],pin_B[i]);
    }
  }
}

if (went_down){
  for (int i = 0; i <= 3; i++){
    digitalWrite(pin_CLK_DIV_OUT[i],LOW);
        // Pattern reader
    pattern_read = analogRead(pin_PTRN[i])+ptrn_size/2;
    
    if (abs(pattern_read-ptrn_mid[i])>20){
      // -5:0 V
    if      (pattern_read >= 129 & pattern_read < 309) {pattern[i] = map(pattern_read,129,279,1,6); ptrn_mid[i] = ptrn_offset -6*ptrn_size + ptrn_size/2 + (pattern[i]-1)*ptrn_size;}
     // 0:5 V
    else if (pattern_read >= 309 & pattern_read < 489) {pattern[i] = map(pattern_read,309,459,1,6); ptrn_mid[i] = ptrn_offset +0*ptrn_size + ptrn_size/2 + (pattern[i]-1)*ptrn_size;}
     // 5:10 V
    else if (pattern_read >= 489 & pattern_read < 669) {pattern[i] = map(pattern_read,489,639,1,6); ptrn_mid[i] = ptrn_offset +6*ptrn_size + ptrn_size/2 + (pattern[i]-1)*ptrn_size;}
     // 10:15V
    else if (pattern_read >= 669 & pattern_read < 849) {pattern[i] = map(pattern_read,669,819,1,6); ptrn_mid[i] = ptrn_offset+12*ptrn_size + ptrn_size/2 + (pattern[i]-1)*ptrn_size;}
                                          }
       // Clock divider reader
    clock_div_read = analogRead(pin_CLK_DIV_IN[i]);
    if (abs(clock_div_read-clock_div_mid[i])>54){
    mapped_div_ind   = constrain(map(clock_div_read,0,922,0,9),0,9);
    division[i]      = divisions[mapped_div_ind];
    clock_div_mid[i] = 51+(1024/10)*mapped_div_ind;}
    
    }}
  
}

void patternDriver(int pattern, int seq_step, int pinA, int pinB){
    int patterns[] = {1, 2, 3, 4,
                      1, 2, 4, 3,
                      2, 4, 1, 3,
                      3, 4, 2, 1,
                      4, 2, 3, 1,
                      4, 3, 2, 1};
    int step_states[] = {LOW,  LOW,   // step 1 - B, A
                         LOW,  HIGH,  // step 2 - B, A
                         HIGH, LOW,   // step 3 - B, A
                         HIGH, HIGH}; // step 4 - B, A
                         
  digitalWrite(pinA, step_states[2*(patterns[4*(pattern-1)+seq_step-1]-1)+1]);   
  digitalWrite(pinB, step_states[2*(patterns[4*(pattern-1)+seq_step-1]-1)  ]);
}
