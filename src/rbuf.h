
#ifndef RBUF_INCLUDE_H_
#define RBUF_INCLUDE_H_

#include <stdint.h>
#include <errno.h>

#ifndef ARRAY_LEN
#define ARRAY_LEN(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))
#endif

#ifndef __COMPILER_BARRIER
#include <avr/cpufunc.h> 
// ensure these writes are last in routine
#define __COMPILER_BARRIER() _MemoryBarrier() 
#endif

#define __CTASSERT_POWER_OF_TWO(V) \
  _Static_assert((V) && !((V) & ((V) - 1)), \
               "Value is not a power of two")

typedef uint8_t __uatomic_t;

struct rbuf_s {
    uint16_t data[128];
    /// write index - only modified by "producer"/writer/enqueue:er
    __uatomic_t wrpos;
    /// read index - only modified by "consumer"/reader/deequeue:er
    __uatomic_t rdpos;
};

static inline uint8_t rbuf_len(const struct rbuf_s *rb)
{
    __CTASSERT_POWER_OF_TWO(ARRAY_LEN(rb->data));

    return (rb->wrpos - rb->rdpos) & (ARRAY_LEN(rb->data) - 1);
}

static inline uint8_t rbuf_isempty(const struct rbuf_s *rb)
{
    return rb->wrpos == rb->rdpos;
}

static inline uint8_t rbuf_enqueue(struct rbuf_s *rb, uint16_t value)
{
    __uatomic_t next = (rb->wrpos + 1) & (ARRAY_LEN(rb->data) - 1);
    if (next == rb->wrpos) {
        return (uint8_t) ENOBUFS;
    }

    rb->data[next] = value;
    __COMPILER_BARRIER();
    rb->wrpos = next;

    return 0;
}

static inline uint8_t rbuf_dequeue(struct rbuf_s *rb, uint16_t *value) 
{
    if (rbuf_isempty(rb)) {
        return (uint8_t) EAGAIN;
    }

    *value = rb->data[rb->rdpos];
    __uatomic_t next = (rb->rdpos + 1) & (ARRAY_LEN(rb->data) - 1);
    __COMPILER_BARRIER();
    rb->rdpos = next;

    return 1;
}

#endif
