#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "arm_math.h"
#include <stdio.h>
#include <math.h>

#define SAMPLE_RATE 10000       // Sampling rate in Hz, nyquist = 5000 Hz
#define FFT_SIZE 1024            //n of points in fft
#define NOISETHRESHOLD 350.0f   // minimum magnitude to consider valid signal

// Moving average smoothing
#define AVG_SIZE 3
float freq_history[AVG_SIZE] = {0};
int idx = 0;

float32_t adc_buffer[FFT_SIZE]; // store raw adcreadings
float32_t fft_output[FFT_SIZE]; // store fft output (complex numbers)
float32_t mag[FFT_SIZE/2];      // store magnitude of fft output

// Band-pass filter (IIR)
#define BPF_ORDER 1
float32_t bpf_state[BPF_ORDER*4];
arm_biquad_casd_df1_inst_f32 S;

// band-pass 
float32_t bpf_coeffs[5] = {
    0.067455, 0.134911, 0.067455, -1.142980, 0.412802,
};

void adc_init_hb100() {
    adc_init();
    adc_gpio_init(26);          // GPIO26 = ADC0
    adc_select_input(0);
}

void sample_signal() {
    for (int i = 0; i < FFT_SIZE; i++) {
        adc_buffer[i] = (float32_t)adc_read();  // Read ADC value
        sleep_us(1000000 / SAMPLE_RATE);        // Wait for next sample
    }
}

void preprocess() {
    float32_t mean;
    arm_mean_f32(adc_buffer, FFT_SIZE, &mean);                                  // calculates the average voltage in the buffer
    arm_offset_f32(adc_buffer, -mean, adc_buffer, FFT_SIZE);                    // remove DC component

    for (int i = 0; i < FFT_SIZE; i++) {
        float32_t w = 0.5f - 0.5f * arm_cos_f32(2 * PI * i / (FFT_SIZE - 1));   // Hanning window
        adc_buffer[i] *= w;
    }

    //arm_biquad_cascade_df1_init_f32(&S, BPF_ORDER, bpf_coeffs, bpf_state);
    //arm_biquad_cascade_df1_f32(&S, adc_buffer, adc_buffer, FFT_SIZE);
}

// ---------- Smooth Frequency ----------
float smooth_freq(float f) {
    freq_history[idx] = f;
    idx = (idx + 1) % AVG_SIZE;

    float sum = 0;
    int count = 0;
    for (int i = 0; i < AVG_SIZE; i++) {
        if (freq_history[i] > 0) {
            sum += freq_history[i];
            count++;
        }
    }
    return (count > 0) ? sum / count : 0.0f;
}

float get_frequency() {
    arm_rfft_fast_instance_f32 fft;                     // FFT instance
    arm_rfft_fast_init_f32(&fft, FFT_SIZE);             // Initialize FFT
    arm_rfft_fast_f32(&fft, adc_buffer, fft_output, 0); // Perform FFT

    arm_cmplx_mag_f32(fft_output, mag, FFT_SIZE/2);     // Calculate magnitude

    uint32_t index;
    float32_t max;
    arm_max_f32(mag + 1, FFT_SIZE/2 - 1, &max, &index); // Find peak frequency bin, skip DC component
    printf("maxfound: %.3f \n", max);

    index += 1;                                         // Adjust index due to skipping DC component

    if (max>NOISETHRESHOLD){
        float freq = (float)index * SAMPLE_RATE / FFT_SIZE;
        return smooth_freq(freq);  // Apply smoothing
    }else{
        return smooth_freq(0.0f);  // Smooth zeros too
    }

}

float freq_to_speed(float f) {
    return f / 19.49f;                                  // Convert frequency to speed in m/s
}

int main() {
    stdio_init_all(); 
    adc_init_hb100(); 

    while (1) {
        sample_signal();
        preprocess();
        float f = get_frequency();
        float v = freq_to_speed(f);

        if (v > 0.01f) {  // Only print if valid
            printf("Freq: %.1f Hz  Speed: %.2f m/s\n", f, v);
        } else {
            printf("No movement\n");
        }

        sleep_ms(50);                                   // Small delay before next measurement
    }
}
