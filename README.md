# Audio Processing and Visualization Program

## **Program Overview**
Program processes an audio file by:
1. Applying frequency filtering based on user-specified weights for different frequency ranges.
2. Saves filtered audio to a new file.
3. Visualizes the audio data as an equalizer using SDL2 graphics.

### **Key Features**
- Utilizes the **FFTW3** library for Fast Fourier Transform (FFT) and inverse FFT.
- Processes audio in the frequency domain.
- Visualizes 32 equalizer bars, each representing a frequency range, with bar height corresponding to magnitude.

## **All Functions**

### **1. `save_filtered_output()`**
Saves filtered audio data to a new WAV file.
  - Handles stereo (2 channels) and mono (1 channel) data.
  - Interleaves data for stereo output.
  - Writes processed data to specified output file and ensures mem management.

### **2. `prompt_for_weights()`**
Prompts user to input weights for 8 frequency ranges.
  - Accepts weights for predefined frequency bands (example: 0-100 Hz, 100-300 Hz).
  - These weights modify magnitude of frequency ranges during FFT processing.

### **3. `apply_log_scaling()`**
Applies logarithmic scaling to magnitudes for better visualization.
  - Normalizes and scales magnitudes using a logarithmic function.

### **4. `visualize_equalizer()`**
Visualizes audio data as an equalizer using SDL2.
  - Performs FFT to calculate frequency band magnitudes.
  - Smooths data for stable visualization.
  - Renders 32 equalizer bars in SDL2, bar height scaled with `apply_log_scaling()`.

### **5. `apply_frequency_filter()`**
Applies user-defined frequency weights to audio data.
  - Converts audio to frequency domain using FFT.
  - Modifies frequency magnitudes using weights.
  - Converts data back to time domain using inverse FFT.
  - Normalizes output to prevent clipping.

### **6. `initialize_bar_to_range_map()`**
Maps 32 equalizer bars to 8 frequency ranges.
  - Assigns bars to ranges based on their corresponding frequency bands.
  - Stores this mapping in an array.

### **7. `main()`**
Drives program.
  1. Opens and reads an audio file.
  2. Prompts user for frequency weights.
  3. Filters audio using specified weights.
  4. Saves filtered output to a new file.
  5. Initializes SDL2 and visualizes audio in real-time.

## **Libraries**
- **FFTW3**: For efficient Fast Fourier Transform (FFT) calculations.
- **libsndfile**: For reading and writing WAV files.
- **SDL2**: For creating a graphical equalizer visualization.

## **Main Flow of Execution**
1. **Audio Loading**: Reads an input WAV file.
2. **User Input for Filtering**: Prompts for 8 frequency band weights.
3. **Filtering**: Applies frequency filter using FFT and inverse FFT.
4. **Saving Output**: Saves filtered audio to a new WAV file (e.g., `filtered_output.wav`).
5. **Visualization**:
   - Displays a real-time equalizer with 32 bars using SDL2.
   - Bars update dynamically to match audio playback.
6. **Main Loop**: Continuously updates equalizer visualization.

## **Issues**
- Frequency Spectrum works exactly as expected, all bars work properly and any type
of different weights apply can be seen visually. (Lowpass, highpass, bandpass, others)
- The output file writing had issues, several instances of a lowpass worked, once I 
was able to get a highpass out as well. There is a unique issue in which the output file
seems to be half the speed of the input. Despite spending many hours working on this, I
have not been able to figure what the issue is and why it is occuring. At the very least
I have a functioning frequency spectrum that uses predefined weights from the user and 
modifies a .wav file, then visualizes it using SDL rendering.
