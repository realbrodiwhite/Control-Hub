#include "hardware.h"

// Cache control registers
#define CACHE_CONTROL   ((volatile uint32_t*)0x3F002000)
#define CACHE_FLUSH     ((volatile uint32_t*)0x3F002040)

// DMA control block (must be 32-byte aligned)
typedef struct __attribute__((aligned(32))) {
    uint32_t ti;
    uint32_t source_ad;
    uint32_t dest_ad;
    uint32_t txfr_len;
    uint32_t stride;
    uint32_t nextconbk;
    uint32_t reserved[2];
} dma_control_block_t;

// Static DMA control block
static dma_control_block_t __attribute__((aligned(32))) dma_cb;

// Initialize hardware features
void hardware_init(void) {
    // Enable hardware features
    enable_neon();
    enable_gpu();
    enable_dma();
    
    // Setup system timer for microsecond precision
    setup_hardware_timer(1); // 1us resolution
}

// Enable NEON SIMD unit
void enable_neon(void) {
    uint32_t cpacr;
    __asm__ volatile (
        "mrc p15, 0, %0, c1, c0, 2\n"
        "orr %0, %0, #(3 << 20)\n" // Enable NEON/VFP
        "orr %0, %0, #(3 << 22)\n"
        "mcr p15, 0, %0, c1, c0, 2\n"
        "isb\n"
        : "=r" (cpacr)
        :
        : "memory"
    );
}

// Enable GPU acceleration
void enable_gpu(void) {
    // Initialize V3D hardware
    *((volatile uint32_t*)(V3D_BASE + 0x00)) = 1; // Enable V3D
}

// Enable DMA controller
void enable_dma(void) {
    // Reset DMA channel
    *DMA_CS = (1 << 31);
    while (*DMA_CS & (1 << 31));
    
    // Clear any error flags
    *DMA_CS = (1 << 2) | (1 << 3);
}

// Setup hardware timer
void setup_hardware_timer(uint32_t interval_us) {
    // Clear timer interrupt
    *TIMER_CS = 0xFF;
    
    // Set first compare value
    *TIMER_C0 = *TIMER_CLO + interval_us;
}

// Get system time in microseconds
uint64_t get_system_time(void) {
    uint32_t hi = *TIMER_CHI;
    uint32_t lo = *TIMER_CLO;
    if (*TIMER_CHI != hi) {
        // High word changed during read
        hi = *TIMER_CHI;
        lo = *TIMER_CLO;
    }
    return ((uint64_t)hi << 32) | lo;
}

// Precise microsecond delay
void delay_microseconds(uint32_t us) {
    uint32_t start = *TIMER_CLO;
    while (*TIMER_CLO - start < us);
}

// DMA memory copy
void dma_memcpy(void* dest, const void* src, size_t size) {
    // Setup DMA control block
    dma_cb.ti = (1 << 0)  |    // SRCREQ
                (1 << 4)  |    // INC
                (1 << 6)  |    // DEST_INC
                (1 << 8)  |    // WAIT_RESP
                (1 << 9);      // TDMODE
    
    dma_cb.source_ad = (uint32_t)src;
    dma_cb.dest_ad = (uint32_t)dest;
    dma_cb.txfr_len = size;
    dma_cb.stride = 0;
    dma_cb.nextconbk = 0;
    
    // Start DMA transfer
    *DMA_CS = 0;
    *DMA_CONBLK_AD = (uint32_t)&dma_cb;
    *DMA_CS = (1 << 0);  // Start
    
    // Wait for completion
    while (*DMA_CS & (1 << 0));
}

// NEON optimized memory copy
void neon_copy_block(void* dest, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    // Process 16 bytes at a time using NEON
    size_t blocks = size / 16;
    __asm__ volatile (
        "1:\n"
        "vld1.8 {d0-d1}, [%[src]]!\n"
        "vst1.8 {d0-d1}, [%[dest]]!\n"
        "subs %[count], %[count], #1\n"
        "bne 1b\n"
        : [dest] "+r" (d), [src] "+r" (s), [count] "+r" (blocks)
        :
        : "d0", "d1", "memory"
    );
    
    // Handle remaining bytes
    size_t remaining = size % 16;
    while (remaining--) {
        *d++ = *s++;
    }
}

// NEON optimized input processing
void neon_process_input(void* output, const void* input, size_t size) {
    uint8_t* out = (uint8_t*)output;
    const uint8_t* in = (const uint8_t*)input;
    
    size_t blocks = size / 16;
    __asm__ volatile (
        "1:\n"
        "vld1.8 {d0-d1}, [%[in]]!\n"
        "vqadd.u8 q0, q0, q0\n"     // Saturating add for smoothing
        "vst1.8 {d0-d1}, [%[out]]!\n"
        "subs %[count], %[count], #1\n"
        "bne 1b\n"
        : [out] "+r" (out), [in] "+r" (in), [count] "+r" (blocks)
        :
        : "q0", "memory"
    );
}

// GPU frame processing
void gpu_process_frame(void* output, const void* input, size_t width, size_t height) {
    // TODO: Implement V3D GPU acceleration for frame processing
    // This would involve setting up GPU command lists and shaders
    // For now, fall back to CPU processing
    dma_memcpy(output, input, width * height * 4);
}
