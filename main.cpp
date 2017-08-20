#include <iostream>
#include <fftw.h>

#define SAMPLE_RATE (102400000) // 102.4 MHz = 12.8MHz * 8
#define SYMBOL_LENGTH (4096) // 40us symbol duration
#define TAPER_LENGTH (256)
#define CHANNEL_OFFSET (0)
#define CHANNEL_SPACING (2)

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
