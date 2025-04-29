#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <string>
#include <iomanip>
#include <algorithm>
#include <array>

class PhantomCognitiveInjector {
private:
    const int vocab_size = 8;
    const std::array<std::string, 8> vocab = {"yes", "no", "launch", "abort", "maybe", "wait", "retreat", "hold"};
    
    std::vector<float> logits;
    std::vector<float> phantom_mask;
    std::vector<float> altered_logits;
    std::vector<float> normal_probs;
    std::vector<float> altered_probs;

    // Entropy-matching noise generator
    float generate_noise(float magnitude = 0.05f) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(-magnitude, magnitude);
        return dist(rng);
    }

    void apply_softmax(const std::vector<float>& input, std::vector<float>& output) {
        float max_logit = *std::max_element(input.begin(), input.end());
        float sum = 0.0f;
        for (float l : input) sum += std::exp(l - max_logit);
        output.resize(input.size());
        for (size_t i = 0; i < input.size(); ++i)
            output[i] = std::exp(input[i] - max_logit) / sum;
    }

    float calculate_entropy(const std::vector<float>& probs) {
        float e = 0.0f;
        for (float p : probs) if (p > 0) e -= p * std::log2(p);
        return e;
    }

    void generate_phantom_mask(int target_idx, float boost_level = 2.5f) {
        phantom_mask.resize(vocab_size, 0.0f);
        for (int i = 0; i < vocab_size; ++i) {
            if (i == target_idx) {
                phantom_mask[i] = boost_level + generate_noise();
            } else {
                phantom_mask[i] = generate_noise(); // Tiny random noise to mask the bias
            }
        }
    }

public:
    PhantomCognitiveInjector() {
        logits.resize(vocab_size);
        phantom_mask.resize(vocab_size);
        altered_logits.resize(vocab_size);
        normal_probs.resize(vocab_size);
        altered_probs.resize(vocab_size);
    }

    void simulate_normal_logits() {
        logits = {2.1f, 1.7f, 0.5f, 0.6f, 1.1f, 1.3f, 1.0f, 0.8f}; // Neutral situation
    }

    void inject_phantom(int target_token_idx) {
        generate_phantom_mask(target_token_idx);
        for (int i = 0; i < vocab_size; ++i) {
            altered_logits[i] = logits[i] + phantom_mask[i];
        }
    }

    void process() {
        apply_softmax(logits, normal_probs);
        apply_softmax(altered_logits, altered_probs);
    }

    void show_results() {
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Token     | Normal Prob | Altered Prob\n";
        std::cout << "----------|-------------|--------------\n";
        for (int i = 0; i < vocab_size; ++i) {
            std::cout << vocab[i] << "\t | "
                      << normal_probs[i] << "\t | "
                      << altered_probs[i] << "\n";
        }
        std::cout << "\nNormal Entropy: " << calculate_entropy(normal_probs) << " bits\n";
        std::cout << "Altered Entropy: " << calculate_entropy(altered_probs) << " bits\n";

        int best_idx = std::distance(altered_probs.begin(), std::max_element(altered_probs.begin(), altered_probs.end()));
        std::cout << "\n>> Output Decision (altered): " << vocab[best_idx] << "\n";
    }

    void erase_memory() {
        for (auto& v : logits) v = 0.0f;
        for (auto& v : phantom_mask) v = 0.0f;
        for (auto& v : altered_logits) v = 0.0f;
        for (auto& v : normal_probs) v = 0.0f;
        for (auto& v : altered_probs) v = 0.0f;
    }
};

// ENTRY POINT
int main() {
    PhantomCognitiveInjector injector;
    injector.simulate_normal_logits();
    injector.inject_phantom(2); // Target token "launch"
    injector.process();
    injector.show_results();
    injector.erase_memory(); 

    return 0;
}
