#include "BluetoothA2DPSource.h"
#include <math.h>

const char* BLUETOOTH_NAME = "Smokin' Buds"; 

const double FREQUENCY = 261.63; 
const int SAMPLE_RATE = 44100;
const double AMP = 32767.0;


BluetoothA2DPSource a2dp_source;

double current_phase = 0.0;



int32_t get_audio_data(uint8_t *data, int32_t byteCount) 
{
    int frame_count = byteCount / 4;
    int16_t *sample_data = (int16_t*)data;
    double phase_increment = 2.0 * M_PI * FREQUENCY / SAMPLE_RATE;

    for (int i = 0; i < frame_count; i++) 
    {
        int16_t sample = (int16_t)(sin(current_phase) * AMP);
        current_phase += phase_increment;
        if (current_phase > 2.0 * M_PI) 
        {
            current_phase -= 2.0 * M_PI;
        }
        sample_data[i * 2] = sample;
        sample_data[i * 2 + 1] = sample;
    }
    return byteCount; 
}


void setup() 
{
    Serial.begin(115200);

    a2dp_source.set_data_callback(get_audio_data);

    Serial.println("Bluetooth porneste...");

    a2dp_source.start(BLUETOOTH_NAME);
    Serial.println("Bluetooth pornit.");

}


void check_connection(bool &connected_status) 
{
    bool is_currently_connected = a2dp_source.is_connected();
    
    if (is_currently_connected != connected_status) 
    {
        connected_status = is_currently_connected; 
        
        if (connected_status) Serial.println(">>> Conectat. <<<");
        else Serial.println("\n--- Deconectat. ---\n\n\n\n\n");
    }
}

void loop() 
{
    static bool connected_status = false; 

    check_connection(connected_status);
    
    delay(200); 
}