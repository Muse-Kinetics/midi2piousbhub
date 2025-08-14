/**
 * @file midi_hid.h
 * @brief MIDI to USB HID interface for Pico
 *
 * This module handles conversion of MIDI note messages to USB HID events
 * with timer-based key release and FIFO buffering.
 */

#ifndef MIDI_HID_H
#define MIDI_HID_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Configuration constants
#define HID_EVENT_BUFFER_SIZE 100
#define HID_TASK_INTERVAL_MS 10
#define KEY_RELEASE_TIMEOUT_MS 500

// HID Report ID for keyboard
#define REPORT_ID_KEYBOARD 1

// HID Key codes
#define HID_KEY_A 0x04
#define HID_KEY_B 0x05
#define HID_KEY_SPACE 0x2C
#define HID_KEY_ENTER 0x28
#define HID_KEY_DOWN_ARROW 0x51
#define HID_KEY_UP_ARROW 0x52

// MIDI note constants
#define MIDI_NOTE_ON 0x90

// HID action types
typedef enum {
    KEY_PRESS,
    KEY_RELEASE
} hid_action_t;

// HID event structure
typedef struct {
    hid_action_t action;
    uint8_t keycode;     // HID keycode (like HID_KEY_A)
    uint8_t midi_note;   // Original MIDI note for debugging
} hid_event_t;

// Circular buffer for HID events
typedef struct {
    hid_event_t events[HID_EVENT_BUFFER_SIZE];
    size_t head;
    size_t tail;
    size_t count;
} hid_event_buffer_t;

// Public API functions

/**
 * @brief Initialize the MIDI HID module
 */
void midi_hid_init(void);

/**
 * @brief Process MIDI data and convert to HID events
 * @param buffer MIDI data buffer
 * @param bytes_read Number of bytes in buffer
 */
void midi_hid_process_midi(const uint8_t* buffer, uint32_t bytes_read);

/**
 * @brief Main HID task - should be called regularly from main loop
 * Handles event processing, HID reports, and timer-based key release
 */
void midi_hid_task(void);

/**
 * @brief Enqueue a HID event into the buffer
 * @param action Key action (press/release)
 * @param keycode HID key code
 * @param midi_note Original MIDI note for debugging
 * @return true if enqueued successfully, false if buffer full
 */
bool midi_hid_event_enqueue(hid_action_t action, uint8_t keycode, uint8_t midi_note);

/**
 * @brief Dequeue a HID event from the buffer
 * @param event Pointer to store the dequeued event
 * @return true if event dequeued, false if buffer empty
 */
bool midi_hid_event_dequeue(hid_event_t* event);

/**
 * @brief Get the current buffer count for debugging
 * @return Number of events in buffer
 */
size_t midi_hid_get_buffer_count(void);

#ifdef __cplusplus
}
#endif

#endif // MIDI_HID_H
