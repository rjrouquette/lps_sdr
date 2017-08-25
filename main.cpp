#include <iostream>
#include <fftw3.h>
#include <cmath>
#include <iomanip>
#include <fstream>

#define SAMPLE_RATE (102400000) // 102.4 MHz = 12.8MHz * 8
#define SYMBOL_LENGTH (4096) // 40us symbol duration; 25kHz per bin
#define TAPER_LENGTH (256)
#define CHANNEL_OFFSET (641) // 640 - 16 MHz; 800 - 20Mhz
#define CHANNEL_SPACING (2) // half-populated for better channel isolation
#define CHANNEL_COUNT (80)
#define DAC_MAX (511)
#define DAC_OFF (512)
#define DAC_COMP (1023)
#define DAC_SCALE (2.0e-3 / 1024)
#define DAC_SLEW (1e-9)

std::ofstream dacOut[2];
int dacPrev;
void dacInit();
void dacOutput(double time, int value);

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

    for(int i = 0; i < CHANNEL_COUNT; i++) {
        bits[i] = false;
    }

    modulator(result, bits);
    std::cout << "Modulator Test" << std::endl;

    dacInit();

    double time = 1e-6;
    for(int i = 0; i < SYMBOL_LENGTH; i++) {
        dacOutput(time, result[i]);
        time += 1.0 / SAMPLE_RATE;
    }
    dacOutput(time, 0);

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
    max -= (max / DAC_MAX) / 2.0;

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

void dacInit() {
    dacOut[0].open("/tmp/dacOutputPos.pwl");
    dacOut[0] << std::scientific << std::setprecision(6);
    dacOut[0] << "0\t" << DAC_OFF * DAC_SCALE << std::endl;

    dacOut[1].open("/tmp/dacOutputNeg.pwl");
    dacOut[1] << std::scientific << std::setprecision(6);
    dacOut[0] << "0\t" << (DAC_COMP - DAC_OFF) * DAC_SCALE << std::endl;

    dacPrev = DAC_OFF;
}

void dacOutput(double time, int value) {
    value += DAC_OFF;

    dacOut[0] << (time - DAC_SLEW) << "\t" << dacPrev * DAC_SCALE << std::endl;
    dacOut[1] << (time - DAC_SLEW) << "\t" << (DAC_COMP - dacPrev) * DAC_SCALE << std::endl;

    dacOut[0] << (time) << "\t" << value * DAC_SCALE << std::endl;
    dacOut[1] << (time) << "\t" << (DAC_COMP - value) * DAC_SCALE << std::endl;

    dacPrev = value;
}
