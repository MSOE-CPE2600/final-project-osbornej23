/*********************************************************************
* @file equalizer.c
* @brief program reads an audio file, applies a frequency filter 
* to it based on user input, saves the filtered audio, 
* and visualizes it as an equalizer using SDL2
*
* Course: CPE2600
* Section: 111
* Assignment: Lab 13
* Date: 12/13/2024 (Last Updated)
* Name: Jadyn Osborne
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fftw3.h>
#include <sndfile.h>
#include <SDL2/SDL.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024
#define NUM_BARS 32
#define NUM_RANGES 8
#define SMOOTHING_FACTOR .1
#define MIN_BAR_HEIGHT 5.0

float *audio_data;
size_t audio_data_size;
float frequency_weights[NUM_RANGES];
int bar_to_range_map[NUM_BARS];

SDL_Renderer *renderer = NULL;

/**
 * @brief saves filtered audio data to a new file
 * @param output_filename - name of file to save filtered data
 * @param filtered_data - filtered audio data
 * @param num_samples - number of samples to write
 * @param sfinfo - audio file info (channels, sample rate, etc)
 */
void save_filtered_output(const char *output_filename, float *filtered_data, size_t num_samples, SF_INFO sfinfo) {
    // get number of channels from input audio file
    int num_channels = sfinfo.channels;
    
    // allocate memory for interleaved data if stereo
    float *interleaved_data = NULL;
    if (num_channels == 2) {
        interleaved_data = (float *)malloc(num_samples * 2 * sizeof(float));  // stereo data
        if (!interleaved_data) {
            printf("failed to allocate memory for interleaved data\n");
            return;
        }
        
        // combine stereo data into interleaved format
        for (size_t i = 0, j = 0; i < num_samples; i++) {
            interleaved_data[j++] = filtered_data[i];  // left channel
            interleaved_data[j++] = filtered_data[i];  // right channel (same for simplicity)
        }
    } else {
        interleaved_data = filtered_data;  // no interleaving for mono
    }

    // open output file for writing
    SNDFILE *outfile = sf_open(output_filename, SFM_WRITE, &sfinfo);
    if (!outfile) {
        printf("failed to open output file %s\n", output_filename);
        return;
    }

    // write filtered data to output file
    if (sf_write_float(outfile, interleaved_data, num_samples) != num_samples) {
        printf("failed to write all samples to output file\n");
    } else {
        printf("filtered output saved to %s\n", output_filename);
    }

    // free interleaved memory if allocated
    if (num_channels == 2) {
        free(interleaved_data);
    }

    sf_close(outfile);
}

/**
 * @brief prompts user for input weights for frequency ranges
 */
void prompt_for_weights() {
    // display frequency ranges to user
    printf("frequency ranges for weights\n");
    printf("1: 0-100 hz\n");
    printf("2: 100-300 hz\n");
    printf("3: 300-600 hz\n");
    printf("4: 600-1200 hz\n");
    printf("5: 1200-2400 hz\n");
    printf("6: 2400-4800 hz\n");
    printf("7: 4800-9600 hz\n");
    printf("8: 9600-22050 hz\n");

    printf("enter weights for 8 frequency ranges\n");
    // get weight values for each frequency range from user
    for (int i = 0; i < NUM_RANGES; i++) {
        scanf("%f", &frequency_weights[i]);
    }
}

/**
 * @brief applies log scaling to a given magnitude
 * @param magnitude - input magnitude to scale
 * @param max_magnitude - maximum magnitude for scaling
 * @return log-scaled magnitude
 */
float apply_log_scaling(float magnitude, float max_magnitude) {
    if (magnitude <= 0) return 0;  // avoid log of zero or negative
    float normalized = magnitude / max_magnitude;  // normalize magnitude
    return log10(1 + 9 * normalized);  // log scaling with normalization
}

/**
 * @brief visualizes equalizer bars based on audio data
 * @param audio_data - audio data to visualize
 * @param num_samples - number of samples in audio data
 */
void visualize_equalizer(float *audio_data, size_t num_samples) {
    // create FFT output buffer
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * (num_samples / 2));
    fftwf_plan p = fftwf_plan_dft_r2c_1d(num_samples, audio_data, out, FFTW_ESTIMATE);

    // execute FFT
    fftwf_execute(p);

    // clear screen before drawing new equalizer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // array to store magnitudes for each bar
    float magnitudes[NUM_BARS] = {0};
    float max_magnitude = 0;

    // calculate magnitude for each frequency band (bar)
    for (int i = 0; i < NUM_BARS; i++) {
        float start_bin = pow(2, (float)i * log2(num_samples / 2.0) / NUM_BARS);
        float end_bin = pow(2, (float)(i + 1) * log2(num_samples / 2.0) / NUM_BARS);
        if (start_bin >= end_bin) continue;

        float bin_sum = 0;
        // accumulate sum of bins within range for current bar
        for (size_t j = (size_t)start_bin; j < (size_t)end_bin; j++) {
            bin_sum += sqrt(out[j][0] * out[j][0] + out[j][1] * out[j][1]);
        }

        // store magnitude for this bar
        magnitudes[i] = bin_sum / ((size_t)end_bin - (size_t)start_bin);
        if (magnitudes[i] > max_magnitude) max_magnitude = magnitudes[i];
    }

    // apply frequency weights to bars
    for (int i = 0; i < NUM_BARS; i++) {
        int range_index = bar_to_range_map[i];
        magnitudes[i] *= frequency_weights[range_index];
    }

    // apply smoothing to equalizer bar heights
    static float smoothed_heights[NUM_BARS] = {0};

    for (int i = 0; i < NUM_BARS; i++) {
        float scaled_height = apply_log_scaling(magnitudes[i], max_magnitude) * 600;

        // apply smoothing factor and ensure minimum height visibility
        smoothed_heights[i] = SMOOTHING_FACTOR * scaled_height + (1 - SMOOTHING_FACTOR) * smoothed_heights[i];
        smoothed_heights[i] = fmax(smoothed_heights[i], MIN_BAR_HEIGHT);

        // calculate position for this bar
        int bar_x = i * (800 / NUM_BARS);
        int bar_width = 800 / NUM_BARS;

        // determine color for bar
        int r = (int)(128 + 127 * ((float)i / NUM_BARS));
        int g = 0;
        int b = (int)(255 - 127 * ((float)i / NUM_BARS));
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);

        // create and draw bar
        SDL_Rect bar = { bar_x, 600 - (int)smoothed_heights[i], bar_width, (int)smoothed_heights[i] };
        SDL_RenderFillRect(renderer, &bar);
    }

    // free FFT resources
    fftwf_destroy_plan(p);
    fftwf_free(out);

    // present the rendered result
    SDL_RenderPresent(renderer);
}

/**
 * @brief applies frequency filter to input audio data and stores result
 * @param input_data - input audio data
 * @param output_data - filtered output data
 * @param num_samples - number of samples to process
 */
void apply_frequency_filter(float *input_data, float *output_data, size_t num_samples) {
    // allocate memory for frequency domain representation
    fftwf_complex *freq_domain = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * (num_samples / 2));
    fftwf_plan forward_plan = fftwf_plan_dft_r2c_1d(num_samples, input_data, freq_domain, FFTW_ESTIMATE);

    // perform FFT on input data
    fftwf_execute(forward_plan);

    // apply frequency weights to frequency domain data
    for (int i = 0; i < NUM_BARS; i++) {
        float start_bin = pow(2, (float)i * log2(num_samples / 2.0) / NUM_BARS);
        float end_bin = pow(2, (float)(i + 1) * log2(num_samples / 2.0) / NUM_BARS);
        if (start_bin >= end_bin) continue;

        int range_index = bar_to_range_map[i];
        float weight = frequency_weights[range_index];

        // modify real and imaginary parts of frequency bins
        for (size_t j = (size_t)start_bin; j < (size_t)end_bin; j++) {
            freq_domain[j][0] *= weight; // real part
            freq_domain[j][1] *= weight; // imaginary part
        }
    }

    // perform inverse FFT to get filtered time domain data
    fftwf_plan inverse_plan = fftwf_plan_dft_c2r_1d(num_samples, freq_domain, output_data, FFTW_ESTIMATE);
    fftwf_execute(inverse_plan);

    // normalize filtered data (avoid audio becoming too loud)
    for (size_t i = 0; i < num_samples; i++) {
        output_data[i] /= num_samples;  // ensure normalization
    }

    // free FFT resources
    fftwf_destroy_plan(forward_plan);
    fftwf_destroy_plan(inverse_plan);
    fftwf_free(freq_domain);
}


/**
 * @brief initializes mapping of bars to frequency ranges
 */
void initialize_bar_to_range_map() {
    // assign bars to frequency ranges
    int ranges_per_bar = NUM_BARS / NUM_RANGES;
    for (int i = 0; i < NUM_BARS; i++) {
        bar_to_range_map[i] = i / ranges_per_bar;
    }
}

/**
 * @brief main function for processing audio file and visualizing results
 * @param argc - number of command line arguments
 * @param argv - command line arguments (file name of input audio)
 * @return status code indicating success or failure
 */
int main(int argc, char *argv[]) {
    // check if valid file name is provided
    if (argc != 2) {
        printf("usage: %s <file_name.wav>\n", argv[0]);
        return -1;
    }

    // open input audio file
    SNDFILE *infile;
    SF_INFO sfinfo;
    infile = sf_open(argv[1], SFM_READ, &sfinfo);
    if (!infile) {
        printf("failed to open audio file\n");
        return -1;
    }

    // read audio data into buffer
    size_t num_samples = sfinfo.frames * sfinfo.channels;
    audio_data = (float *)malloc(num_samples * sizeof(float));
    audio_data_size = num_samples;
    sf_read_float(infile, audio_data, num_samples);
    sf_close(infile);

    // initialize mapping of bars to frequency ranges
    initialize_bar_to_range_map();
    // prompt user for frequency weights
    prompt_for_weights();

    // allocate memory for filtered audio data
    float *filtered_data = (float *)malloc(num_samples * sizeof(float));
    if (!filtered_data) {
        printf("failed to allocate memory for filtered data\n");
        free(audio_data);
        return -1;
    }

    // apply frequency filtering
    apply_frequency_filter(audio_data, filtered_data, num_samples);

    // save filtered data to output file
    const char *output_filename = "filtered_output2.wav";
    save_filtered_output(output_filename, filtered_data, num_samples, sfinfo);

    // initialize SDL for visualizer
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("failed to initialize SDL: %s\n", SDL_GetError());
        free(audio_data);
        free(filtered_data);
        return -1;
    }

    // create SDL window for visualizer
    SDL_Window *window = SDL_CreateWindow("Equalizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("failed to create SDL window: %s\n", SDL_GetError());
        free(audio_data);
        free(filtered_data);
        return -1;
    }

    // create SDL renderer for drawing
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("failed to create SDL renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        free(audio_data);
        free(filtered_data);
        return -1;
    }

    // main loop for visualizing audio and handling SDL events
    SDL_Event e;
    int index = 0;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            break;
        }

        // visualize equalizer with current audio data
        visualize_equalizer(audio_data + index, BUFFER_SIZE);
        index += BUFFER_SIZE;
        if (index >= audio_data_size) index = 0;

        // delay to sync with audio playback rate
        SDL_Delay(1000 / (SAMPLE_RATE / BUFFER_SIZE));
    }

    // cleanup SDL resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // free dynamically allocated memory
    free(audio_data);
    free(filtered_data);

    return 0;
}