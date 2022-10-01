#ifndef KEYBOARD_H
#define KEYBOARD_H
/*
 * Copyright (c) 2022, Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * Definitions related to keyboard management
 *
 * These definitions are grouped in a number of categories, distinguished by
 * specific prefixes:
 * - keyb_   : Keyboard management routines, keyb_irq_ can run from irqs
 * - KMAT_   : Key matrix, a 64-bit representation of pressed keys
 * - KB_     : Key bit positions in a 64-bit keymatrix, they acts as keycode
 * - KSHIFT_ : Logical shift states (left, right and alpha)
 * - KHOLD_ : Key planes, includes KSHIFT_ + information about held keys
 * - KM_     : Key messages, combine KB_ with KSHIFT_ or KHOLD_
 * - KFLAG_  : Key flags, state of the keyboard management
 *
 * Constants from various areas are combined in a key_msg_t as follows:
 * - key     : 6 bits for key bit position (KB_ consstant)
 * - state   : 1 bit to indicate current key state, 1 = pressed, 0 = up
 * - long_pr : 1 bit to indicate long press or repeat (key handler decides)
 * - shift   : 3 bits for left, right and alpha shifts
 * - rsvd    : 1 bit reserved (placeholder for on)
 * - hold    : 4 bits for left, shift, alpha and on being held
 * - al_once : 1 bit for alpha-once in effect
 * - al_lowr : 1 bit for alpha lowercase being in effect
 * - running : 1 bit to confirm subsystem is running (normally unused)
 * - updated : 1 bit to indicate a key change
 * - repeat  : 1 bit to indicate this is a key repeat
 *
 * Additionally, touch events have a different format with the high bit set:
 * - x       : 12 bits for the X coordinate
 * - hold    : 3 bits for left, right, alpha being held
 * - state   : 1 bit for finger state (up or down)
 * - y       : 12 bits for the Y coordinate
 * - finger  : 2 bits to indicate second finger
 * - repeat  : 1 bit for finger move
 * - touch   : 1 bit to indicate a touch event
 */

#include <newrpl_types.h>
#include <target.h>


// ============================================================================
//
//   Configuration constants
//
// ============================================================================

//! The complete state of the keyboard as a 64-bit bitfield
typedef uint64_t keymatrix;



// ============================================================================
//
//    Low-level keyboard interrupts and matrix
//
// ============================================================================

// Keyboard message queue size (# of messages)
#define KEYB_BUFFER             128

// Keyboard scanning speed in milliseconds
#define KEYB_SCAN_SPEED         20


typedef unsigned          keyb_index_t;
typedef unsigned          keyb_msg_t;

extern keyb_msg_t         keyb_irq_buffer[KEYB_BUFFER]  SYSTEM_GLOBAL;
extern volatile int       keyb_irq_lock                 SYSTEM_GLOBAL;
extern keyb_index_t       keyb_used                     SYSTEM_GLOBAL;
extern keyb_index_t       keyb_current                  SYSTEM_GLOBAL;
extern keymatrix          keyb_matrix                   SYSTEM_GLOBAL;
extern unsigned           keyb_plane                    SYSTEM_GLOBAL;
extern int                keyb_last_code                SYSTEM_GLOBAL;
extern int                keyb_duration                 SYSTEM_GLOBAL;
extern int                keyb_irq_repeat_time;
extern int                keyb_irq_long_press_time      SYSTEM_GLOBAL;
extern int                keyb_irq_debounce             SYSTEM_GLOBAL;
#define keyb_flags        keyb_plane

// Low-level keyboard interface (the part that runs from interrupt handlers)
void                      keyb_irq_update();
keymatrix                 keyb_irq_get_matrix();
keymatrix                 keyb_irq_get_matrix_no_interrupts();
void                      keyb_irq_wait_release();
keyb_msg_t                keyb_irq_get_key();
void                      keyb_irq_shift_logic(keyb_msg_t key,
                                               keymatrix hwkeys,
                                               keymatrix changes);
void                      keyb_irq_post_message(keyb_msg_t msg);
#define keyb_irq_post_keydn(key)     (keyb_irq_post_message(KM_KEYDN  | (key)))
#define keyb_irq_post_keyup(key)     (keyb_irq_post_message(KM_KEYUP  | (key)))
#define keyb_irq_post_key(key)       (keyb_irq_post_message(KM_PRESS  | (key)))
#define keyb_irq_post_repeat(key)    (keyb_irq_post_message(KM_REPEAT | (key)))

// High-level keyboard interface (used from HAL / RPL)
keymatrix                 keyb_get_matrix();
void                      keyb_post_message(keyb_msg_t msg);
keyb_msg_t                keyb_get_message();
unsigned                  keyb_any_message();
unsigned                  keyb_get_shift_plane();
void                      keyb_set_shift_plane(unsigned plane);
void                      keyb_flush_no_wait();
void                      keyb_flush();
void                      keyb_set_timing(int rept, int longprs, int debounce);
void                      keyb_set_repeat(int repeat);
void                      keyb_set_alpha_once(int single_alpha);
#define keyb_is_key_pressed(key)  (keyb_get_matrix() & KM_MASK(key))
#define keyb_is_any_key_pressed() (keyb_get_matrix() != 0LL)

// Keyboard timing
#define KEYB_LONG_PRESS_TIME       (keyb_irq_long_press_time)
#define KEYB_REPEAT_TIME           (keyb_irq_repeat_time)
#define KEYB_BOUNCE_TIME           (keyb_irq_debounce)

// Mapping between logical (code) and hardware (bit) numbers
// This is a leftover from a time where the Prime was using HP50g keycodes,
// and is kept in case we need some kind of mapping for a new target
#define keyb_code(bit)             (bit)
#define keyb_bit(code)             (code)



// ============================================================================
//
//    Keyboard shift bitmasks for high-level functions
//
// ============================================================================

// Logical shift keys
#define KSHIFT_COUNT              3
#define KSHIFT_STATES_COUNT       (1U << KSHIFT_COUNT)
#define KSHIFT(index)             ((index) << 8)
#define KSHIFT_INDEX(keymsg)      (((keymsg) >> 8) & (KSHIFT_STATES_COUNT - 1))
#define KSHIFT_BITS               8

#define KSHIFT_NONE               KSHIFT(0)
#define KSHIFT_LEFT               KSHIFT(1U << 0)
#define KSHIFT_RIGHT              KSHIFT(1U << 1)
#define KSHIFT_ALPHA              KSHIFT(1U << 2)
#define KSHIFT_ANY                KSHIFT(KSHIFT_STATES_COUNT - 1)

// Keys being held when an event happened
#define KHOLD_COUNT               4
#define KHOLD_STATES_COUNT        (1U << KHOLD_COUNT)
#define KHOLD(index)              ((index) << 12)
#define KHOLD_INDEX(keymsg)       (((keymsg) >> 12) & (KHOLD_STATES_COUNT - 1))

#define KHOLD_NONE                KHOLD(0)
#define KHOLD_LEFT                KHOLD(1U << 0)
#define KHOLD_RIGHT               KHOLD(1U << 1)
#define KHOLD_ALPHA               KHOLD(1U << 2)
#define KHOLD_ON                  KHOLD(1U << 3)
#define KHOLD_ANY                 KHOLD(KHOLD_STATES_COUNT - 1)



// ============================================================================
//
//    Keymatrix type definition
//
// ============================================================================

// Create a keymatrix mask from a key code
#define KM_MASK(key)               (1LL << (key))
#define KM(key)                    KM_MASK(KB_##key)

// Keymatrix mask to isolate all shifts (Left, Right and Alpha)
#define KM_ALL_SHIFTS              (KM(ALPHA) | KM(LSHIFT) | KM(RSHIFT))
#define KM_ON                      KM(ON)
#define KM_COMBINERS               (KM_ALL_SHIFTS | KM_ON)
#define KM_UNSHIFTED(matrix)       ((matrix) & ~KM_ALL_SHIFTS)
#define KM_HAS_LEFT_SHIFT(matrix)  (((matrix) >> KB_LSHIFT) & 1)
#define KM_HAS_RIGHT_SHIFT(matrix) (((matrix) >> KB_RSHIFT) & 1)
#define KM_HAS_ALPHA(matrix)       (((matrix) >> KB_ALPHA) & 1)
#define KM_IS_COMBINER(key)        ((KM_MASK(key) & KM_COMBINERS) != 0)

// Keyboard message constants, to combine with one of the KB_XXX key constants
// We use the !X operator here to get a zero indicating the flag is not set
#define KM_PRESS                0       // !PRESSED, !CHANGED
#define KM_REPEAT               (KM_PRESS | KFLAG_REPEAT | KFLAG_PRESSED)
#define KM_LONG_PRESS           (KM_PRESS | KFLAG_LONG_PRESS | KFLAG_PRESSED)
#define KM_LONG_OR_REPEAT       (KM_LONG_PRESS | KM_REPEAT)
#define KM_KEYDN                (KFLAG_CHANGED | KFLAG_PRESSED)
#define KM_KEYUP                (KFLAG_CHANGED)

// Isolate parts of a message
// The message mask extracts the message part (matching message constants above)
// The flags mask is the flags we copy from keyb_flags into the message
#define KM_SHIFT_MASK           KSHIFT_ANY   // Mask for the shift bits
#define KM_KEY_MASK             0x3F         // Mask for the key value
#define KM_MESSAGE_MASK         (KM_TOUCH               | \
                                 KFLAG_PRESSED          | \
                                 KFLAG_LONG_PRESS       | \
                                 KFLAG_CHANGED          | \
                                 KFLAG_REPEAT)
#define KM_FLAGS_MASK           (KSHIFT_ANY             | \
                                 KHOLD_ANY              | \
                                 KFLAG_LONG_PRESS       | \
                                 KFLAG_ALPHA_ONCE       | \
                                 KFLAG_ALPHA_LOWER      | \
                                 KFLAG_REPEAT           | \
                                 KFLAG_SHIFTS_CHANGED)

#define KM_MESSAGE(msg)            ((msg) & KM_MESSAGE_MASK)
#define KM_KEY(msg)                ((msg) & KM_KEY_MASK)
#define KM_SHIFTED_KEY(msg)        ((msg) & (KM_KEY_MASK | KM_SHIFT_MASK))
#define KM_SHIFT(msg)              ((msg) & KM_SHIFT_MASK)
#define KM_FLAGS(msg)              ((msg) & KM_FLAGS_MASK)

// Touch support: high bit is set, format is different
#define KM_TOUCH                   (1U << 31)
#define KM_TOUCH_X(msg)            ((msg) & KM_TOUCH_X_MASK)
#define KM_TOUCH_Y(msg)            (((msg) & KM_TOUCH_Y_MASK) >> 16)
#define KM_TOUCH_FINGER(msg)       (((msg) & KM_TOUCH_FINGER_MASK) >> 28)
#define KM_TOUCH_EVENT(msg)        ((msg) & KM_TOUCH_EVENT_MASK)
#define KM_TOUCH_X_MASK            (0xfff <<  0)
#define KM_TOUCH_Y_MASK            (0xfff << 16)
#define KM_TOUCH_FINGER_MASK       (3 << 28)
#define KM_TOUCH_UP                (0 << 7)
#define KM_TOUCH_DOWN              (1U << 7)
#define KM_TOUCH_MOVE              (1U << 30)
#define KM_TOUCH_EVENT_MASK        (KM_TOUCH_DOWN | KM_TOUCH_MOVE)
#define KM_FINGER_DOWN             KM_TOUCH_DOWN
#define KM_FINGER_MOVE             KM_TOUCH_MOVE
#define KM_FINGER_UP               KM_TOUCH_UP

// Compose a touch message
#define KM_TOUCH_MESSAGE(ev, finger, x, y)                              \
    ((ev)                                                             | \
     KM_TOUCH                                                         | \
     (((finger) &     3) << 28)                                       | \
     (((x)      & 0xFFF) <<  0)                                       | \
     (((y)      & 0xFFF) << 16))



// ============================================================================
//
//    Key flags
//
// ============================================================================
//   Bit positions are chosen to be usable in key messages

// Key state
#define KFLAG_PRESSED           (1U << 6)       // Key was pressed
#define KFLAG_LONG_PRESS        (1U << 7)       // Long press (or repeat)

// Alpha state
#define KFLAG_ALPHA_ONCE        (1U << 16)      // Clear alpha after next key
#define KFLAG_ALPHA_LOWER       (1U << 17)      // Lowercase mode

// Distinguish keyup/down from key presses
#define KFLAG_CHANGED           (1U << 18)      // Some key changed

// Keyboard subsystem state
#define KFLAG_RUNNING           (1U << 24)      // Keyboard thread running
#define KFLAG_UPDATED           (1U << 25)      // Keyboard state updated
#define KFLAG_REPEAT            (1U << 26)      // Please repeat that key
#define KFLAG_SHIFTS_CHANGED    (1U << 27)      // Some shift changed

// REVISIT: To remove?
#define KFLAG_ONE_PRESS         (1U << 28)      // ???
#define KFLAG_OTHER_KEY         (1U << 29)      // ???
#define KFLAG_ALPHA_SWAP        (1U << 30)      // ???



// ============================================================================
//
//   Helper routines
//
// ============================================================================

static inline int         debounce(volatile uint32_t *hwreg,
                                   unsigned           mask,
                                   unsigned           loops)
// ----------------------------------------------------------------------------
//   Debounce keyboard input, i.e. wait for the signal to be stable enough
// ----------------------------------------------------------------------------
{
    unsigned stable = 0;
    for (unsigned count = 0; count < loops; count++)
    {
        unsigned value = *hwreg & mask;
        if (value != stable)
        {
            stable = value;
            count = 0;
        }
    }
    return stable;
}



// ============================================================================
//
//    Dep[endencies
//
// ============================================================================

// Only needed to avoid locking in multithreaded environments
INTERRUPT_TYPE            cpu_intoff_nosave();
void                      cpu_inton_nosave(INTERRUPT_TYPE state);
void                      tmr_event_reschedule();

#endif // KEYBOARD_H
