const int ledPin = 9;           // PWM pin to output the sinusoidal wave
const int ttlPin = 2;           // TTL input pin

const int waveSamples = 256;    // Number of samples for a full wave
int wave[waveSamples];          // Array to hold sine wave values
volatile bool trigger = false;  // Flag to start the sequence
bool sequenceRunning = false;   // Flag to indicate if the sequence is running
unsigned long sequenceStartTime = 0; // Time when the sequence started

const int amplitude = 255;      // Amplitude of the sine wave (0 to 255, brightness)
const float phaseShift = 90.0;  // Phase shift in degrees (0 to 360), adjust as needed
const float stimulusDuration = 30.0; // Duration for each frequency in seconds
const float offDuration = 30.0; // Duration for each off period in seconds

const int frequencyList[] = {1, 2, 5, 10, 20, 30}; // List of frequencies
const int totalFrequencies = sizeof(frequencyList) / sizeof(frequencyList[0]); // Number of frequencies
int currentFrequencyIndex = 0;  // Index to track the current frequency
float frequency;                // Current frequency value
unsigned long delayTime;        // Delay time in microseconds

void setup() {
  Serial.begin(9600);           // Start serial communication for debugging
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }
  delay(1000);                  // Adding a delay to ensure Serial communication is ready

  pinMode(ledPin, OUTPUT);      // Set ledPin as an output
  pinMode(ttlPin, INPUT);       // Set ttlPin as an input
  attachInterrupt(digitalPinToInterrupt(ttlPin), triggerWave, RISING); // Attach an interrupt to ttlPin
}

void loop() {
  if (trigger && !sequenceRunning) {
    // Start the sequence if triggered and not already running
    sequenceRunning = true;
    trigger = false; // Reset the trigger
    sequenceStartTime = millis(); // Record the start time
    currentFrequencyIndex = 0; // Start from the first frequency
    Serial.println("Starting frequency sequence.");
  }

  if (sequenceRunning) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = (currentTime - sequenceStartTime) / 1000; // Convert to seconds

    // Calculate the total duration of the sequence
    unsigned long totalDuration = (stimulusDuration + offDuration) * totalFrequencies;

    if (elapsedTime >= totalDuration) {
      // Sequence complete
      Serial.println("Sequence complete.");
      analogWrite(ledPin, 0); // Turn off the LED after the sequence is complete
      sequenceRunning = false; // Reset the flag to allow the sequence to start again
      return;
    }

    // Determine the current frequency index
    int freqIndex = elapsedTime / (unsigned long)(stimulusDuration + offDuration);

    if (freqIndex >= totalFrequencies) {
      freqIndex = totalFrequencies - 1; // Ensure index does not exceed bounds
    }

    currentFrequencyIndex = freqIndex;
    frequency = frequencyList[currentFrequencyIndex]; // Update the frequency
    generateWaveform(); // Generate the sine wave values for the new frequency
    calculateDelayTime(); // Recalculate the delay time for the new frequency

    unsigned long phaseTime = elapsedTime % (unsigned long)(stimulusDuration + offDuration);

    if (phaseTime < stimulusDuration) { // During the stimulus period
      Serial.print("Playing Frequency: "); Serial.println(frequency);
      playWaveform(); // Play the waveform for the stimulus duration
    } else {
      // During the off period
      Serial.println("Frequency off period.");
      delay(offDuration * 1000); // Wait for off duration
    }
  }
}

void setFrequency() {
  frequency = frequencyList[currentFrequencyIndex]; // Set the current frequency from the list
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

void triggerWave() {
  trigger = true; // Set trigger flag to true when a rising edge is detected on ttlPin
}

void playWaveform() {
  unsigned long startTime = millis();
  unsigned long elapsedTime = 0;
  unsigned long lastMicros = micros();  // Record the initial micros() value
  
  while (elapsedTime < stimulusDuration * 1000) {  // Convert stimulus duration to milliseconds
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

