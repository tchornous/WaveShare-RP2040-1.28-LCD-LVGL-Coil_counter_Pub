#include "encoders.h"
#include "main.h"

const float COUNTS_PER_TURN = 200.0f; // 100 pulses * 2 fronts

Encoder counter_encoder;
Encoder hand_encoder;

// Initialize encoder
void encoder_init(Encoder* enc, uint CLK_PIN, uint DT_PIN)
{
    _gpio_init(CLK_PIN);
    _gpio_init(DT_PIN);
    gpio_set_dir(CLK_PIN, GPIO_IN);
    gpio_set_dir(DT_PIN, GPIO_IN);
    enc->old_State = gpio_get(CLK_PIN);
    enc->Fupdate = 1;
    enc->counter = 0;
    enc->state   = 0;
}

// Initialize encoders
void encoders_init(void)
{
    encoder_init(&counter_encoder, COUNT_ENCODER_CLK, COUNT_ENCODER_DT);
    encoder_init(&hand_encoder,          ENCODER_CLK,       ENCODER_DT);
    // Set initial values for encoders
    hand_encoder.counter = START_TARGET_VALUE;
    hand_encoder.mode = ENCODER_MODE_OFF;


    // irq init
    gpio_set_irq_enabled_with_callback( COUNT_ENCODER_CLK,      \
                                        GPIO_IRQ_EDGE_RISE |    \
                                        GPIO_IRQ_EDGE_FALL,     \
                                        true,                   \
                                        encoders_irq_handler);

    gpio_set_irq_enabled              ( ENCODER_CLK,            \
                                        GPIO_IRQ_EDGE_RISE |    \
                                        GPIO_IRQ_EDGE_FALL,     \
                                        true);
}

float encoder_to_turns(int32_t count) {
    return count / COUNTS_PER_TURN;
}

// handler selector
void encoders_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == COUNT_ENCODER_CLK)
    {
        count_encoder_irq_handler();
    }
    else if (gpio == ENCODER_CLK)
    {
        encoder_irq_handler();
    }
}

// hand enc irq handler
void encoder_irq_handler()
{
    if(!gpio_get(ENCODER_CLK)){return;}

    // only front fronts
    if(gpio_get(ENCODER_CLK) && hand_encoder.mode == ENCODER_MODE_TARGET)
    {  
        if(gpio_get(ENCODER_DT))
        {
            hand_encoder.counter++;
        }
        else
        {
            hand_encoder.counter--;
        }
        hand_encoder.Fupdate = 1;
    }

    // hand mode
    else if(gpio_get(ENCODER_CLK) && hand_encoder.mode == ENCODER_MODE_HAND)
    {  
        if(gpio_get(ENCODER_DT))
        {
            counter_encoder.counter = counter_encoder.counter + 20;
        }
        else
        {
            counter_encoder.counter = counter_encoder.counter - 20;
        }
        counter_encoder.Fupdate = 1;
    } 

}

// count enc irq handler
void count_encoder_irq_handler()
{
    // front and rear fronts
    counter_encoder.state = gpio_get(COUNT_ENCODER_CLK);
    if(counter_encoder.state != counter_encoder.old_State)
    {  
        if(gpio_get(COUNT_ENCODER_DT) == counter_encoder.state)
        {
            counter_encoder.counter++;
        }
        else
        {
            counter_encoder.counter--;
        }
        counter_encoder.Fupdate = 1;
    }
    counter_encoder.old_State = counter_encoder.state;
}