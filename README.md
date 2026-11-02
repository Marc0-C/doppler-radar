# doppler-radar
A homemade Doppler speed radar. A Raspberry Pi Pico samples the IF output of an HB100 microwave motion sensor, runs an FFT using CMSIS-DSP, and converts the dominant Doppler frequency into the speed of a moving target.

<img width="714" height="1106" alt="image" src="https://github.com/user-attachments/assets/17793c6e-c855-4c7e-8050-9bbabcc23189" />

<img width="701" height="1104" alt="image (1)" src="https://github.com/user-attachments/assets/e59bfe18-ec7b-4adf-a064-c6cb31c17888" />

<img width="1536" height="2048" alt="WhatsApp Image 2026-11-02 at 14 59 15" src="https://github.com/user-attachments/assets/86c6d295-9346-4a52-b21f-3565b9744cb2" />

The HB100 transmits a continuous 10.525 GHz signal. A moving object reflects it back with a frequency shift proportional to its radial speed:

f_doppler = 2 * v * f_tx / c   ->   f_doppler ≈ 19.49 Hz per 1 m/s

The signal chain in firmware:

Sample: 1024 samples from the ADC at 10 kHz (about 102 ms per window).
Remove DC: subtract the mean of the buffer.
Window: apply a Hann window to reduce spectral leakage.
FFT: real FFT (arm_rfft_fast_f32), then bin magnitudes.
Peak detect: find the strongest bin, skipping DC.
Threshold: ignore the result if the peak is below NOISETHRESHOLD.
Smooth: moving average over the last 3 results.
Convert: speed = frequency / 19.49.
Parameter	Value
Sample rate	10 kHz (Nyquist 5 kHz)
FFT size	1024 points
Frequency resolution	≈ 9.77 Hz per bin
Speed resolution	≈ 0.5 m/s per bin
Max measurable speed	≈ 256 m/s (limited by Nyquist)
Update rate	≈ 150 ms per reading

blink.c           # Firmware source
CMakeLists.txt    # Build configuration
blink.uf2         # Prebuilt firmware, ready to flash

maxfound: 812.430
Freq: 195.3 Hz  Speed: 10.02 m/s
