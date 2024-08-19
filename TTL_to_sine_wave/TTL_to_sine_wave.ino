const int ledPin = 9;           // PWM pin to output the sinusoidal wave
const int ttlPin = 2;           // TTL input pin

const int waveSamples = 256;    // Number of samples for a full wave
int wave[waveSamples];          // Array to hold sine wave values
volatile bool trigger = false;

const int amplitude = 255;      // Amplitude of the sine wave (0 to 255, brightness)
const float phaseShift = 90.0;   // Phase shift in degrees (0 to 360), adjust as needed
const float duration = 3.0;    // Desired duration in seconds

float frequency = 10;          // Frequency of the sine wave in Hz
unsigned long delayTime;        // Delay time in microseconds
int cycles;                     // Number of cycles to output

void setup() {
  Serial.begin(9600);           // Start serial communication for debugging
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }
  delay(1000);                  // Adding a delay to ensure Serial communication is ready

  pinMode(ledPin, OUTPUT);      // Set ledPin as an output
  pinMode(ttlPin, INPUT);       // Set ttlPin as an input
  attachInterrupt(digitalPinToInterrupt(ttlPin), triggerWave, RISING); // Attach an interrupt to ttlPin that calls triggerWave() on a rising edge

  generateWaveform();           // Generate the sine wave values
  calculateDelayTime();         // Calculate the delay time based on the desired frequency
  calculateCycles();            // Calculate the number of cycles based on the desired duration

  // Debugging output
  Serial.print("Frequency: "); Serial.println(frequency);
  Serial.print("Delay Time (us): "); Serial.println(delayTime);
  Serial.print("Cycles: "); Serial.println(cycles);
}

void loop() {
  if (trigger) {
    playWaveform();             // If trigger is true (a TTL pulse has been detected), play the sine wave
    trigger = false;            // Reset trigger to false
  }
}

void generateWaveform() {
  float phaseShiftRad = phaseShift * PI / 180.0;  // Convert phase shift to radians
  
  for (int i = 0; i < waveSamples; i++) {
    float angle = (2 * PI * i / waveSamples);  // Normalized angle from 0 to 2*PI
    wave[i] = int((sin(angle - phaseShiftRad) + 1) * (amplitude / 2.0));  // Apply phase shift
  }
}

void calculateDelayTime() {
  float period = 1.0 / frequency;                    // Calculate period in seconds
  unsigned long totalMicroseconds = period * 1e6;    // Convert period to microseconds
  delayTime = totalMicroseconds / waveSamples;       // Calculate delay time for each sample

  // Debugging output
  Serial.print("Period (s): "); Serial.println(period);
  Serial.print("Total Microseconds: "); Serial.println(totalMicroseconds);
  Serial.print("Calculated Delay Time (us): "); Serial.println(delayTime);
}

void calculateCycles() {
  cycles = duration * frequency;                   // Calculate the number of cycles for the desired duration
  
  // Debugging output
  Serial.print("Duration (s): "); Serial.println(duration);
  Serial.print("Calculated Cycles: "); Serial.println(cycles);
}

void triggerWave() {
  trigger = true;               // Set trigger flag to true when a rising edge is detected on ttlPin
}

void playWaveform() {
  unsigned long startTime = millis();
  unsigned long elapsedTime = 0;
  unsigned long lastMicros = micros();  // Record the initial micros() value
  
  while (elapsedTime < duration * 1000) {  // Convert duration to milliseconds
    for (int i = 0; i < waveSamples; i++) {  // Loop through each sample in the wave
      analogWrite(ledPin, wave[i]);    // Output each sample of the sine wave to ledPin using PWM
      // Wait until the correct interval has passed using micros()
      while (micros() - lastMicros < delayTime) {
        // Wait until the delay time has elapsed
      }
      lastMicros = micros();  // Update the lastMicros for the next iteration
    }
    elapsedTime = millis() - startTime;  // Calculate elapsed time
  }
  analogWrite(ledPin, 0);  // Turn off the LED after the waveform is complete
}
