#include "BluetoothA2DPSource.h"
#include <math.h>

#define NOTES_COUNT 8

#define PITCH_POT 35      
#define WAVEFORM_POT 34   
#define LFO_SPEED_POT 33  
#define REVERB_POT 32     

const int NOTE_PINS[NOTES_COUNT] = {4, 16, 17, 22, 18, 19, 21, 23};
const float NOTE_FREQS[NOTES_COUNT] = {261.63f, 293.66f, 329.63f, 349.23f, 392.00f, 440.00f, 493.88f, 523.25f};

const int SAMPLE_RATE = 44100;
const float AMP = 2000.0f; 
const int WAVETABLE_SIZE = 512;
const int WAVE_TYPES = 3; 

#define DELAY_SIZE 10000 
int16_t delay_buffer[DELAY_SIZE]; 
int delay_head = 0; 
float reverb_amount = 0.0f; 

const float ATTACK = 0.001f;   
const float RELEASE = 0.0001f;  
const float LFO_DEPTH = 0.45f; 

int16_t wavetables[WAVE_TYPES][WAVETABLE_SIZE]; 
int current_wave_index = 0;

float filtered_sample = 0.0f;  
float lfo_phase = 0.0f;
float lfo_speed = 0.0f; 

float active_freqs[NOTES_COUNT]; 
float note_phases[NOTES_COUNT];  
float note_volumes[NOTES_COUNT]; 
bool note_pressed[NOTES_COUNT]; 

BluetoothA2DPSource a2dp_source;
const char* BLUETOOTH_NAME = "Smokin' Buds"; 

void create_wavetables(); 
void check_inputs();
int32_t get_audio_data(uint8_t *data, int32_t byteCount);
void check_connection(bool &connected_status);
void connect_bluetooth();

void setup() 
{
    Serial.begin(115200);

    for(int i = 0; i < NOTES_COUNT; i++) 
    {
        active_freqs[i] = 0.0f;
        note_phases[i] = 0.0f;
        note_volumes[i] = 0.0f;
        note_pressed[i] = false;
    }

    memset(delay_buffer, 0, sizeof(delay_buffer));
    create_wavetables();

    for (int i = 0; i < NOTES_COUNT; i++) pinMode(NOTE_PINS[i], INPUT_PULLUP);
    
    analogReadResolution(12);
    connect_bluetooth();
}

void loop() 
{
    static bool connected_status = false; 
    check_connection(connected_status);
    if (connected_status) check_inputs();
    delay(50); 
}

void create_wavetables() 
{
    for (int i = 0; i < WAVETABLE_SIZE; i++) 
    {
        double pos = (double)i / WAVETABLE_SIZE; 
        wavetables[0][i] = (int16_t)(sin(2.0 * M_PI * pos) * AMP); 
        wavetables[1][i] = (int16_t)(((2.0 * pos) - 1.0) * AMP);   
        if (i < WAVETABLE_SIZE / 2) wavetables[2][i] = (int16_t)AMP; 
        else wavetables[2][i] = (int16_t)-AMP;
    }
}

void check_inputs()
{
   int pitch = analogRead(PITCH_POT);
   float octave_factor = 1.0f;
   if (pitch < 1300) octave_factor = 0.5f;
   else if (pitch > 2800) octave_factor = 2.0f;

   int waveform = analogRead(WAVEFORM_POT); 
   if (waveform < 1300) current_wave_index = 0;      
   else if (waveform > 2800) current_wave_index = 2; 
   else current_wave_index = 1;                      
   
   int speed = analogRead(LFO_SPEED_POT);
   float speedNorm = (float)speed / 4095.0f;
   if (speedNorm < 0.05f) lfo_speed = 0.0f; 
   else lfo_speed = 0.5f + (15.0f * speedNorm * speedNorm);

   int reverb = analogRead(REVERB_POT);
   reverb_amount = ((float)reverb / 4095.0f) * 0.6f;

   for (int i = 0; i < NOTES_COUNT; i++)
   {
        if (digitalRead(NOTE_PINS[i]) == LOW) 
        {
            note_pressed[i] = true;
            active_freqs[i] = NOTE_FREQS[i] * octave_factor;
        } 
        else note_pressed[i] = false;
   }
}

int32_t get_audio_data(uint8_t *data, int32_t byteCount) 
{
    int frame_count = byteCount / 4;
    int16_t *sample_data = (int16_t*)data;
    const float PHASE_FACTOR = (float)WAVETABLE_SIZE / SAMPLE_RATE;

    for (int i = 0; i < frame_count; i++) 
    {
        float mixed_sample = 0.0f;

        float current_lfo = 0.0f;
        float cutoff = 0.95f; 

        if (lfo_speed > 0.1f) 
        { 
            current_lfo = sin(lfo_phase);
            lfo_phase += (2.0f * M_PI * lfo_speed) / SAMPLE_RATE;
            if (lfo_phase > 2.0f * M_PI) lfo_phase -= 2.0f * M_PI;
            cutoff = 0.15f + (LFO_DEPTH * (current_lfo + 1.0f)); 
        } 

        if (cutoff > 1.0f) cutoff = 1.0f;
        if (cutoff < 0.1f) cutoff = 0.1f;

        for (int n = 0; n < NOTES_COUNT; n++) 
        {
            if (note_pressed[n]) 
            {
                if (note_volumes[n] < 1.0f) 
                {
                    note_volumes[n] += ATTACK;
                    if (note_volumes[n] > 1.0f) note_volumes[n] = 1.0f;
                }
            } 
            else 
            {
                if (note_volumes[n] > 0.0f) 
                {
                    note_volumes[n] -= RELEASE;
                    if (note_volumes[n] < 0.0f) note_volumes[n] = 0.0f;
                }
            }

            if (note_volumes[n] > 0.001f) 
            {
                if (active_freqs[n] == 0.0f) active_freqs[n] = NOTE_FREQS[n];
                float step = active_freqs[n] * PHASE_FACTOR;
                int index = (int)note_phases[n];
                int16_t raw_wave = wavetables[current_wave_index][index];
                mixed_sample += (float)raw_wave * note_volumes[n];
                note_phases[n] += step;
                if (note_phases[n] >= WAVETABLE_SIZE) note_phases[n] -= WAVETABLE_SIZE;
            }
            else note_phases[n] = 0;
        }

        filtered_sample += cutoff * (mixed_sample - filtered_sample);
        
        int16_t old_echo = delay_buffer[delay_head];
        float reverbed_signal = filtered_sample + ((float)old_echo * reverb_amount);
        
        delay_buffer[delay_head] = (int16_t)reverbed_signal;
        delay_head++;
        if (delay_head >= DELAY_SIZE) delay_head = 0;
        
        int32_t final_output = (int32_t)reverbed_signal;

        if (final_output > 32767) final_output = 32767;
        else if (final_output < -32767) final_output = -32767;

        sample_data[i * 2] = (int16_t)final_output;     
        sample_data[i * 2 + 1] = (int16_t)final_output; 
    }
    return byteCount;
}

void connect_bluetooth()
{
    a2dp_source.set_data_callback(get_audio_data);
    Serial.println("Turning ON Bluetooth...");
    a2dp_source.start(BLUETOOTH_NAME);
    Serial.println("Bluetooth ON.");
}

void check_connection(bool &connected_status) 
{
    bool is_connected = a2dp_source.is_connected();
    if (is_connected != connected_status) 
    {
        connected_status = is_connected; 
        if (connected_status) Serial.println(">>> Connected. <<<");
        else Serial.println("--- Disconnected. ---");
    }
}