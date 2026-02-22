#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <cstdlib>
using namespace std;

// Generate analog reference signal (sine wave with adjustable amplitude and length)
vector<double> generateAnalog(int N, double freq, double fs, double amplitude) {
    vector<double> signal(N);
    for(int i = 0; i < N; i++) {
        double t = i / fs;
        signal[i] = amplitude * sin(2 * M_PI * freq * t);
    }
    return signal;
}

// Sample the analog signal (digital samples)
vector<double> sampleSignal(const vector<double>& analog, int step) {
    vector<double> samples;
    for(int i = 0; i < analog.size(); i += step) {
        samples.push_back(analog[i]);
    }
    return samples;
}

// Zero-Order Hold reconstruction
vector<double> zeroOrderHold(const vector<double>& samples, int step, int totalLength) {
    vector<double> reconstructed(totalLength);
    int sampleIndex = 0;
    for(int i = 0; i < totalLength; i++) {
        if(i % step == 0 && sampleIndex < samples.size()) {
            reconstructed[i] = samples[sampleIndex];
            sampleIndex++;
        } else {
            reconstructed[i] = reconstructed[i-1]; // hold last value
        }
    }
    return reconstructed;
}

// Mean Squared Error
double computeMSE(const vector<double>& analog, const vector<double>& reconstructed) {
    int N = min(analog.size(), reconstructed.size());
    double mse = 0.0;
    for(int i = 0; i < N; i++) {
        mse += pow(analog[i] - reconstructed[i], 2);
    }
    return mse / N;
}

// Quantize digital samples based on bit depth
void quantize(vector<double>& digital, double amplitude, int bits) {
    double maxVal = amplitude;
    double minVal = -amplitude;
    int levels = pow(2, bits);
    for (auto &d : digital) {
        double norm = (d - minVal) / (maxVal - minVal);
        int q = round(norm * (levels - 1));
        d = minVal + (q * (maxVal - minVal)) / (levels - 1);
    }
}

// Export to CSV: Analog, Digital, Reconstructed
void exportCSV(const vector<double>& analog, const vector<double>& reconstructed,
               const vector<double>& digital, int step, const string& filename) {
    ofstream file(filename);
    file << "Analog,Digital,Reconstructed\n";
    int N = analog.size();
    int dIndex = 0;
    for(int i = 0; i < N; i++) {
        double dig = (i % step == 0 && dIndex < digital.size()) ? digital[dIndex++] : 0.0;
        file << analog[i] << "," << dig << "," << reconstructed[i] << "\n";
    }
    file.close();
}

int main(int argc, char* argv[]) {
    // Default values
    int N = 1000;        // number of samples (x-axis length)
    double freq = 1.0;   // Hz
    double fs = 100.0;   // sampling frequency
    double amplitude = 1.0; // adjustable amplitude
    int step = 10;       // sample every 10 points
    int bits = 8;        // default quantization bits

    // Parse arguments if provided
    if (argc > 1) N = atoi(argv[1]);       // length
    if (argc > 2) amplitude = atof(argv[2]); // amplitude
    if (argc > 3) bits = atoi(argv[3]);    // bits

    // Generate signals
    auto analog = generateAnalog(N, freq, fs, amplitude);
    auto digital = sampleSignal(analog, step);

    // Quantize digital samples
    quantize(digital, amplitude, bits);

    // Reconstruct
    auto reconstructed = zeroOrderHold(digital, step, N);

    // Compute error
    double mse = computeMSE(analog, reconstructed);

    cout << "MSE = " << mse << endl;

    // Save CSV in backend folder
    exportCSV(analog, reconstructed, digital, step, "output.csv");

    return 0;
}
