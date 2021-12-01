#ifndef GPIO_H_
#define GPIO_H_

/** These assume portpin macros to be defined and used and with the following
 *  format (ex.)
 *  `#define GPIO_MY_PIN  B, 1` for port B and pin 1. no spaces!
 **/

#define GPIO_PORT(portpin) ___GPIO_PORT(portpin)
#define ___GPIO_PORT(port, pin) (PIN##port)

#define GPIO_MASK(portpin) ___GPIO_MASK(portpin)
#define ___GPIO_MASK(port, pin) (1 << PIN##port##pin) // same as (1 << pin)?

/// @param out. if zero input else output
#define GPIO_DIR(portpin, out) ___GPIO_DIR(portpin, out)
#define ___GPIO_DIR(port, pin, out)                                            \
    if (out) {                                                                 \
        DDR##port |= (1 << (pin));                                             \
    } else {                                                                   \
        DDR##port &= ~(1 << (pin));                                            \
    }

#define GPIO_OUTPUT(portpin, state) ___GPIO_OUTPUT(portpin, state)
#define ___GPIO_OUTPUT(port, pin, state) do {                                  \
    DDR##port |= (1 << (pin));                                                 \
    if (state)                                                                 \
        PORT##port |= (1 << (pin));                                            \
} while(0)

#define GPIO_INPUT(portpin, pullup) ___GPIO_INPUT(portpin, pullup)
#define ___GPIO_INPUT(port, pin, pullup) do {                                  \
    DDR##port &= ~(1 << (pin));                                                \
    if (pullup)                                                                \
        PORT##port |= (1 << (pin));                                            \
    else                                                                       \
        PORT##port &= ~(1 << (pin));                                           \
} while(0)

    // PB0, PB1 and PB2 are now inputs with pull-up enabled
/// @param val non-zero to set pin high
#define GPIO_WRITE(portpin, val) ___GPIO_WRITE(portpin, val)
#define ___GPIO_WRITE(port, pin, val)                                          \
    if (val) {                                                                 \
        PORT##port |= (1 << (pin));                                            \
    } else {                                                                   \
        PORT##port &= ~(1 << (pin));                                           \
    }

/// @return non-zero if pin is high
#define GPIO_READ(portpin) ___GPIO_READ(portpin)
#define ___GPIO_READ(port, pin) (PIN##port & (1 << (pin)))

#endif
