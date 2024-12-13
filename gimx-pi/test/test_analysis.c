#include "test_analysis.h"
#include "../src/optimize.h"

// Circular buffer for trend analysis
#define HISTORY_SIZE 100
static struct {
    float latency_history[HISTORY_SIZE];
    float temp_history[HISTORY_SIZE];
    float power_history[HISTORY_SIZE];
    float cpu_history[HISTORY_SIZE];
    uint32_t history_pos;
    uint32_t history_count;
    analysis_result_t current_analysis;
} analysis_data = {0};

// Statistical functions
static float calculate_average(const float* data, uint32_t count) {
    if (count == 0) return 0.0f;
    float sum = 0.0f;
    for (uint32_t i = 0; i < count; i++) {
        sum += data[i];
    }
    return sum / count;
}

static float calculate_std_dev(const float* data, uint32_t count, float avg) {
    if (count < 2) return 0.0f;
    float sum_sq = 0.0f;
    for (uint32_t i = 0; i < count; i++) {
        float diff = data[i] - avg;
        sum_sq += diff * diff;
    }
    return __builtin_sqrtf(sum_sq / (count - 1));
}

static float calculate_percentile(float* data, uint32_t count, float percentile) {
    if (count == 0) return 0.0f;
    
    // Simple bubble sort for small datasets
    for (uint32_t i = 0; i < count - 1; i++) {
        for (uint32_t j = 0; j < count - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                float temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
    
    float index = (count - 1) * percentile;
    uint32_t lower = (uint32_t)index;
    float fraction = index - lower;
    
    if (lower + 1 < count) {
        return data[lower] + (data[lower + 1] - data[lower]) * fraction;
    }
    return data[lower];
}

static performance_trend_t analyze_trend(const float* data, uint32_t count) {
    if (count < 10) return TREND_UNKNOWN;
    
    float early_avg = 0.0f;
    float late_avg = 0.0f;
    uint32_t split = count / 2;
    
    // Compare first and second half averages
    for (uint32_t i = 0; i < split; i++) {
        early_avg += data[i];
        late_avg += data[count - 1 - i];
    }
    early_avg /= split;
    late_avg /= split;
    
    float change = (late_avg - early_avg) / early_avg;
    
    if (change > TREND_THRESHOLD) return TREND_DEGRADING;
    if (change < -TREND_THRESHOLD) return TREND_IMPROVING;
    return TREND_STABLE;
}

// Initialize analysis module
void analysis_init(void) {
    analysis_data.history_pos = 0;
    analysis_data.history_count = 0;
    __builtin_memset(&analysis_data.current_analysis, 0, sizeof(analysis_result_t));
}

// Process new test results
void analysis_process_results(const test_result_t* results) {
    // Update history
    analysis_data.latency_history[analysis_data.history_pos] = results->latency_us;
    analysis_data.temp_history[analysis_data.history_pos] = results->max_temp;
    analysis_data.power_history[analysis_data.history_pos] = results->power_draw_mA;
    analysis_data.cpu_history[analysis_data.history_pos] = results->cpu_usage;
    
    analysis_data.history_pos = (analysis_data.history_pos + 1) % HISTORY_SIZE;
    if (analysis_data.history_count < HISTORY_SIZE) {
        analysis_data.history_count++;
    }
    
    // Update latency statistics
    analysis_data.current_analysis.latency.avg = 
        calculate_average(analysis_data.latency_history, analysis_data.history_count);
    analysis_data.current_analysis.latency.std_dev = 
        calculate_std_dev(analysis_data.latency_history, analysis_data.history_count,
                         analysis_data.current_analysis.latency.avg);
    analysis_data.current_analysis.latency.trend = 
        analyze_trend(analysis_data.latency_history, analysis_data.history_count);
    
    // Update thermal statistics
    analysis_data.current_analysis.temperature.avg = 
        calculate_average(analysis_data.temp_history, analysis_data.history_count);
    analysis_data.current_analysis.temperature.std_dev = 
        calculate_std_dev(analysis_data.temp_history, analysis_data.history_count,
                         analysis_data.current_analysis.temperature.avg);
    analysis_data.current_analysis.temperature.trend = 
        analyze_trend(analysis_data.temp_history, analysis_data.history_count);
    
    // Detect anomalies
    uint32_t anomalies = 0;
    
    // Latency anomalies
    if (results->latency_us > analysis_data.current_analysis.latency.avg + 
        analysis_data.current_analysis.latency.std_dev * ANOMALY_THRESHOLD) {
        anomalies++;
    }
    
    // Temperature anomalies
    if (results->max_temp > analysis_data.current_analysis.temperature.avg +
        analysis_data.current_analysis.temperature.std_dev * ANOMALY_THRESHOLD) {
        anomalies++;
    }
    
    analysis_data.current_analysis.anomalies_detected = anomalies;
    
    // Update optimization suggestions
    uint32_t opt_flags = 0;
    
    if (analysis_data.current_analysis.latency.trend == TREND_DEGRADING) {
        opt_flags |= OPT_SUGGEST_NEON | OPT_SUGGEST_CACHE;
    }
    
    if (analysis_data.current_analysis.temperature.trend == TREND_DEGRADING) {
        opt_flags |= OPT_SUGGEST_COOLING | OPT_SUGGEST_FREQ;
    }
    
    if (results->cpu_usage > 80.0f) {
        opt_flags |= OPT_SUGGEST_DMA | OPT_SUGGEST_MEMORY;
    }
    
    analysis_data.current_analysis.optimization_flags = opt_flags;
}

// Get current analysis results
void analysis_get_stats(analysis_result_t* analysis) {
    if (analysis) {
        *analysis = analysis_data.current_analysis;
    }
}

// Calculate reliability score
float analysis_get_reliability_score(void) {
    float score = 100.0f;
    
    // Reduce score based on anomalies
    score -= analysis_data.current_analysis.anomalies_detected * 5.0f;
    
    // Reduce score based on trends
    if (analysis_data.current_analysis.latency.trend == TREND_DEGRADING) score -= 10.0f;
    if (analysis_data.current_analysis.temperature.trend == TREND_DEGRADING) score -= 10.0f;
    
    // Ensure score stays in valid range
    if (score < 0.0f) score = 0.0f;
    if (score > 100.0f) score = 100.0f;
    
    return score;
}

// Generate optimization recommendations
void analysis_generate_recommendations(char* buffer, uint32_t size) {
    if (!buffer || size == 0) return;
    
    uint32_t pos = 0;
    uint32_t flags = analysis_data.current_analysis.optimization_flags;
    
    // Helper macro for safe string append
    #define APPEND_STR(str) \
        do { \
            const char* s = str; \
            while (*s && pos < size - 1) buffer[pos++] = *s++; \
        } while(0)
    
    if (flags & OPT_SUGGEST_NEON) {
        APPEND_STR("- Enable NEON acceleration\n");
    }
    if (flags & OPT_SUGGEST_DMA) {
        APPEND_STR("- Use DMA for data transfers\n");
    }
    if (flags & OPT_SUGGEST_CACHE) {
        APPEND_STR("- Optimize cache usage\n");
    }
    if (flags & OPT_SUGGEST_FREQ) {
        APPEND_STR("- Adjust CPU frequency\n");
    }
    if (flags & OPT_SUGGEST_COOLING) {
        APPEND_STR("- Improve cooling solution\n");
    }
    
    buffer[pos] = '\0';
}
