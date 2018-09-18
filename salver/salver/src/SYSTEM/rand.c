/*
 * hal_rand.c
 *
 *  Created on: Jul 31, 2018
 *      Author: ecarx
 */


#include "rand.h"
static int32_t next_factor;

struct random_data
  {
    int32_t *fptr;                /* Front pointer.  */
    int32_t *rptr;                /* Rear pointer.  */
    int32_t *state;                /* Array of state values.  */
    int rand_type;                /* Type of random number generator.  */
    int rand_deg;                /* Degree of random number generator.  */
    int rand_sep;                /* Distance between front and rear.  */
    int32_t *end_ptr;                /* Pointer behind state table.  */
  };

/* Linear congruential.  */
#define        TYPE_0                0
#define        BREAK_0                8
#define        DEG_0                0
#define        SEP_0                0
/* x**7 + x**3 + 1.  */
#define        TYPE_1                1
#define        BREAK_1                32
#define        DEG_1                7
#define        SEP_1                3
/* x**15 + x + 1.  */
#define        TYPE_2                2
#define        BREAK_2                64
#define        DEG_2                15
#define        SEP_2                1
/* x**31 + x**3 + 1.  */
#define        TYPE_3                3
#define        BREAK_3                128
#define        DEG_3                31
#define        SEP_3                3
/* x**63 + x + 1.  */
#define        TYPE_4                4
#define        BREAK_4                256
#define        DEG_4                63
#define        SEP_4                1

/* Initially, everything is set up as if from:
 initstate(1, randtbl, 128);
 Note that this initialization takes advantage of the fact that srandom
 advances the front and rear pointers 10*rand_deg times, and hence the
 rear pointer which starts at 0 will also end up at zero; thus the zeroth
 element of the state information, which contains info about the current
 position of the rear pointer is just
 (MAX_TYPES * (rptr - state)) + TYPE_3 == TYPE_3.  */

static int32_t randtbl[DEG_3 + 1] =
        {
                TYPE_3,
                -1726662223, 379960547, 1735697613, 1040273694, 1313901226,
                1627687941, -179304937, -2073333483, 1780058412, -1989503057,
                -615974602, 344556628, 939512070, -1249116260, 1507946756,
                -812545463, 154635395, 1388815473, -1926676823, 525320961,
                -1009028674, 968117788, -123449607, 1284210865, 435012392,
                -2017506339, -911064859, -370259173, 1132637927, 1398500161,
                -205601318,
        };
static struct random_data unsafe_state =
        {
                /* FPTR and RPTR are two pointers into the state info, a front and a rear
                 pointer.  These two pointers are always rand_sep places apart, as they
                 cycle through the state information.  (Yes, this does mean we could get
                 away with just one pointer, but the code for random is more efficient
                 this way).  The pointers are left positioned as they would be from the call:
                 initstate(1, randtbl, 128);
                 (The position of the rear pointer, rptr, is really 0 (as explained above
                 in the initialization of randtbl) because the state table pointer is set
                 to point to randtbl[1] (as explained below).)  */
                .fptr = &randtbl[SEP_3 + 1],
                .rptr = &randtbl[1],
                /* The following things are the pointer to the state information table,
                 the type of the current generator, the degree of the current polynomial
                 being used, and the separation between the two pointers.
                 Note that for efficiency of random, we remember the first location of
                 the state information, not the zeroth.  Hence it is valid to access
                 state[-1], which is used to store the type of the R.N.G.
                 Also, we remember the last location, since this is more efficient than
                 indexing every time to find the address of the last element to see if
                 the front and rear pointers have wrapped.  */
                .state = &randtbl[1],
                .rand_type = TYPE_3,
                .rand_deg = DEG_3,
                .rand_sep = SEP_3,
                .end_ptr = &randtbl[sizeof(randtbl) / sizeof(randtbl[0])]
        };
static int __random_r(struct random_data *buf, int32_t *result)
{
    int32_t *state;
    if (buf == NULL || result == NULL)
    {
        return -1;
    }
    state = buf->state;
    if (buf->rand_type == TYPE_0)
    {
        int32_t val = ((state[0] * 1103515245U) + 12345U) & 0x7fffffff;
        state[0] = val;
        *result = val;
    }
    else
    {
        int32_t *fptr = buf->fptr;
        int32_t *rptr = buf->rptr;
        int32_t *end_ptr = buf->end_ptr;
        uint32_t val;
        val = *fptr += (uint32_t) *rptr;
        /* Chucking least random bit.  */
        *result = val >> 1;
        ++fptr;
        if (fptr >= end_ptr)
        {
            fptr = state;
            ++rptr;
        }
        else
        {
            ++rptr;
            if (rptr >= end_ptr)
                rptr = state;
        }
        buf->fptr = fptr;
        buf->rptr = rptr;
    }
    return 0;
}

int rand(void)
{
    int32_t retval;
    (void) __random_r (&unsafe_state, &retval);
    return (retval ^ next_factor);
}

void srand(int32_t factor)
{
    next_factor = factor;
}
