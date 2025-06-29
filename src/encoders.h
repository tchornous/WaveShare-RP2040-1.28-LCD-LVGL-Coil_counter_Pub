#ifndef H__ENCODERS
#define H__ENCODERS

#include "hardware/gpio.h"

// Pin definitions

#define ENCODER_CLK        19 
#define ENCODER_DT         18 
#define COUNT_ENCODER_DT   22
#define COUNT_ENCODER_CLK  21

typedef enum {
    ENCODER_MODE_OFF,
    ENCODER_MODE_TARGET,
    ENCODER_MODE_HAND
} encoder_mode_t;

typedef struct {
    int counter;            // Current value of the encoder
    uint8_t Fupdate;        // Flag to indicate if the encoder value has been updated
    uint8_t state;          // Current state of the encoder
    uint8_t old_State;      // Previous state of the encoder
    encoder_mode_t mode;    // Current mode of the encoder
} Encoder;

extern Encoder counter_encoder;
extern Encoder hand_encoder;

void encoder_irq_handler(void);
void count_encoder_irq_handler(void);
void encoders_irq_handler(uint gpio, uint32_t events);

extern void encoders_init(void);
extern float encoder_to_turns(int32_t count);

#endif // H__ENCODERS
