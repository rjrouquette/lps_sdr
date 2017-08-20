#include <iostream>
#include <fftw3.h>
#include <cmath>
#include <iomanip>

#define SAMPLE_RATE (102400000) // 102.4 MHz = 12.8MHz * 8
#define SYMBOL_LENGTH (4096) // 40us symbol duration; 25kHz per bin
#define TAPER_LENGTH (256)
#define CHANNEL_OFFSET (640) // 640 - 16 MHz; 800 - 20Mhz
#define CHANNEL_SPACING (2) // half-populated for better channel isolation
#define CHANNEL_COUNT (80)
#define DAC_MAX (127)

float *window_taper;
void initWindowTaper();

fftwf_plan ifft;
fftwf_complex *ifft_in;
float *ifft_out;
void modulator(int *out, bool *bits);

int main() {
    initWindowTaper();
    std::cout << "Initialized Window Taper" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    for(int i = 0; i < TAPER_LENGTH;) {
        for(int j = 0; i < TAPER_LENGTH && j < 8; j++) {
            std::cout << "\t" << window_taper[i++];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    ifft_in = new fftwf_complex[(SYMBOL_LENGTH / 2) + 1];
    ifft_out = new float[SYMBOL_LENGTH];
    ifft = fftwf_plan_dft_c2r_1d(SYMBOL_LENGTH, ifft_in, ifft_out, FFTW_MEASURE);

    std::cout << "Initialized IFFT" << std::endl << std::endl;

    bool *bits = new bool[CHANNEL_COUNT];
    int *result = new int[SYMBOL_LENGTH];

    modulator(result, bits);
    std::cout << "Modulator Test:" << std::endl;
    for(int i = 0; i < SYMBOL_LENGTH;) {
        for(int j = 0; i < SYMBOL_LENGTH && j < 8; j++) {
            std::cout << "\t" << result[i++];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    return 0;
}

void modulator(int *out, bool *bits) {
    for(int i = 0; i < (SYMBOL_LENGTH / 2) + 1; i++) {
        ifft_in[i][0] = 0;
        ifft_in[i][1] = 0;
    }
    for(int i = 0; i < CHANNEL_COUNT; i++) {
        int j = (i * CHANNEL_SPACING) + CHANNEL_OFFSET;
        ifft_in[j][0] = bits[j] ? 1 : -1;
    }
    fftwf_execute(ifft);

    // apply window taper
    for(int i = 0; i < TAPER_LENGTH; i++) {
        ifft_out[i] *= window_taper[i];
        ifft_out[SYMBOL_LENGTH - i - 1] *= window_taper[i];
    }

    // normalize
    float max = 0;
    for(int i = 0; i < SYMBOL_LENGTH; i++) {
        if(ifft_out[i] < 0) {
            if(-ifft_out[i] > max) max = -ifft_out[i];
        }
        else {
            if(ifft_out[i] > max) max = ifft_out[i];
        }
    }

    for(int i = 0; i < SYMBOL_LENGTH; i++) {
        out[i] = (int) roundf((DAC_MAX * ifft_out[i]) / max);
    }
}

void initWindowTaper() {
    window_taper = new float[TAPER_LENGTH];
    const double length_inv = 1.0 / TAPER_LENGTH;
    for(int i = 0; i < TAPER_LENGTH; i++) {
        window_taper[TAPER_LENGTH - i - 1] = (float) (length_inv + (sin((M_PI * (i + 1)) / TAPER_LENGTH) - sin((M_PI * i) / TAPER_LENGTH)) / M_PI);
    }
    for(int i = 0; i < TAPER_LENGTH; i++) {
        window_taper[i] /= window_taper[TAPER_LENGTH-1];
    }
}
