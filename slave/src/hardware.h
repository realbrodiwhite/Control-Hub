#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

// Raspberry Pi 3B Hardware Acceleration
#define NEON_AVAILABLE
#define GPU_AVAILABLE

// GPU Hardware Addresses
#define GPU_BASE        0x3F400000
#define V3D_BASE        (GPU_BASE + 0xC00000)

// NEON SIMD Types
typedef uint8_t uint8x8_t __attribute__ ((vector_size (8)));
typedef uint8_t uint8x16_t __attribute__ ((vector_size (16)));
typedef int16_t int16x8_t __attribute__ ((vector_size (16)));
typedef float float32x4_t __attribute__ ((vector_size (16)));

// DMA Controller
#define DMA_BASE        0x3F007000
#define DMA_CS          ((volatile uint32_t*)(DMA_BASE + 0x00))
#define DMA_CONBLK_AD   ((volatile uint32_t*)(DMA_BASE + 0x04))
#define DMA_TI          ((volatile uint32_t*)(DMA_BASE + 0x08))
#define DMA_SOURCE_AD   ((volatile uint32_t*)(DMA_BASE + 0x0C))
#define DMA_DEST_AD     ((volatile uint32_t*)(DMA_BASE + 0x10))
#define DMA_TXFR_LEN    ((volatile uint32_t*)(DMA_BASE + 0x14))
#define DMA_STRIDE      ((volatile uint32_t*)(DMA_BASE + 0x18))
#define DMA_NEXTCONBK   ((volatile uint32_t*)(DMA_BASE + 0x1C))
#define DMA_DEBUG       ((volatile uint32_t*)(DMA_BASE + 0x20))

// Hardware Timer
#define TIMER_BASE      0x3F003000
#define TIMER_CS        ((volatile uint32_t*)(TIMER_BASE + 0x00))
#define TIMER_CLO       ((volatile uint32_t*)(TIMER_BASE + 0x04))
#define TIMER_CHI       ((volatile uint32_t*)(TIMER_BASE + 0x08))
#define TIMER_C0        ((volatile uint32_t*)(TIMER_BASE + 0x0C))
#define TIMER_C1        ((volatile uint32_t*)(TIMER_BASE + 0x10))
#define TIMER_C2        ((volatile uint32_t*)(TIMER_BASE + 0x14))
#define TIMER_C3        ((volatile uint32_t*)(TIMER_BASE + 0x18))

// Function Prototypes
void hardware_init(void);
void enable_neon(void);
void enable_gpu(void);
void enable_dma(void);
void setup_hardware_timer(uint32_t interval_us);
uint64_t get_system_time(void);
void delay_microseconds(uint32_t us);

// DMA Functions
void dma_memcpy(void* dest, const void* src, size_t size);
void dma_memset(void* dest, uint8_t value, size_t size);

// NEON Optimized Functions
void neon_copy_block(void* dest, const void* src, size_t size);
void neon_process_input(void* output, const void* input, size_t size);
void neon_interpolate(void* output, const void* prev, const void* next, float factor, size_t size);

// GPU Acceleration
void gpu_init(void);
void gpu_process_frame(void* output, const void* input, size_t width, size_t height);
void gpu_buffer_flip(void);

#endif // HARDWARE_H
