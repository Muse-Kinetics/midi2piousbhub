/**
 * @file midi_hid.cpp
 * @brief MIDI to USB HID interface implementation
 */

#include "midi_hid.h"
#include "MIDI_bytestream_parser.hpp"
#include "tusb.h"
#include "pico/stdlib.h"
#include <cstdio>

// Internal state
static hid_event_buffer_t hid_buffer;
static bool pending_key_release = false;
static uint32_t key_release_time_ms = 0;
static MidiBytestreamParser midi_parser(nullptr);

// Internal functions
static uint8_t map_midi_note_to_keycode(uint8_t midi_note);

void midi_hid_init(void)
{
    // Initialize HID buffer
    hid_buffer.head = 0;
    hid_buffer.tail = 0;
    hid_buffer.count = 0;
    
    // Initialize timer-based key release state
    pending_key_release = false;
    key_release_time_ms = 0;
    
    printf("MIDI HID module initialized\n");
}

void midi_hid_process_midi(const uint8_t* buffer, uint32_t bytes_read)
{
    printf("midi_hid_process_midi called, bytes_read=%lu\n", bytes_read);
    
    if (bytes_read > 0)
    {
        for (size_t i = 0; i < bytes_read; i++) 
        {
            if (midi_parser.parse(buffer[i]))
            {
                printf("Parsed MIDI message: ");
                for (size_t j = 0; j < 3; j++) {
                    printf("%d ", midi_parser.msg[j]);
                }
                printf("\n");

                uint8_t midi_note = midi_parser.msg[1];
                uint8_t keycode = map_midi_note_to_keycode(midi_note);
                
                if (keycode != 0) {
                    // Only process note-on messages (ignore note-off)
                    if ((midi_parser.msg[0] & 0xF0) == MIDI_NOTE_ON && midi_parser.msg[2] != 0) {
                        // Enqueue key press event
                        midi_hid_event_enqueue(KEY_PRESS, keycode, midi_note);
                    }
                }
            }
        }
    }
}

void midi_hid_task(void)
{
    static uint32_t start_ms = 0;
    
    // Rate limiting like in the working keyboard example
    uint32_t current_ms = to_ms_since_boot(get_absolute_time());
    if (current_ms - start_ms < HID_TASK_INTERVAL_MS) return;
    start_ms += HID_TASK_INTERVAL_MS;
    
    // Early return if HID not ready
    if (!tud_hid_ready()) {
        return;
    }
    
    // Check for pending key release timer
    if (pending_key_release && current_ms >= key_release_time_ms) {
        // Time to send all keys released
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        pending_key_release = false;
        printf("Released all keys (timer expired)\n");
    }
    
    // Process one event from the buffer
    hid_event_t event;
    if (midi_hid_event_dequeue(&event)) {
        printf("Dequeued event: action=%d, keycode=0x%02X, midi_note=%d\n", 
               event.action, event.keycode, event.midi_note);
               
        if (event.action == KEY_PRESS) {
            // Send key press report - create proper array like working keyboard example
            uint8_t keycodes[6] = { 0 };  // Array of 6 key codes, initialized to 0
            keycodes[0] = event.keycode;  // Set the first key code
            printf("Sending HID key press: keycode=0x%02X\n", event.keycode);
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycodes);  // Pass array, not pointer to single byte
            
            // Set timer for key release
            pending_key_release = true;
            key_release_time_ms = current_ms + KEY_RELEASE_TIMEOUT_MS;
        }
        // Note: We no longer process KEY_RELEASE events from MIDI
    }
}

bool midi_hid_event_enqueue(hid_action_t action, uint8_t keycode, uint8_t midi_note)
{
    // Check if buffer is full
    if (hid_buffer.count >= HID_EVENT_BUFFER_SIZE) {
        printf("HID event buffer full, dropping event\n");
        return false;
    }
    
    // Add event to buffer
    hid_buffer.events[hid_buffer.head] = {action, keycode, midi_note};
    hid_buffer.head = (hid_buffer.head + 1) % HID_EVENT_BUFFER_SIZE;
    hid_buffer.count++;
    
    printf("Enqueued HID event: action=%d, keycode=%d, note=%d (count=%zu)\n", 
          action, keycode, midi_note, hid_buffer.count);
    return true;
}

bool midi_hid_event_dequeue(hid_event_t* event)
{
    // Check if buffer is empty
    if (hid_buffer.count == 0) {
        return false;
    }
    
    // Get event from buffer
    *event = hid_buffer.events[hid_buffer.tail];
    hid_buffer.tail = (hid_buffer.tail + 1) % HID_EVENT_BUFFER_SIZE;
    hid_buffer.count--;
    
    return true;
}

size_t midi_hid_get_buffer_count(void)
{
    return hid_buffer.count;
}

/**
 * @brief Map MIDI note numbers to HID key codes
 * @param midi_note MIDI note number
 * @return HID key code, or 0 if note should be ignored
 */
static uint8_t map_midi_note_to_keycode(uint8_t midi_note)
{
    switch (midi_note) {
        case 38:
            return HID_KEY_DOWN_ARROW;
        case 36:
            return HID_KEY_UP_ARROW;
        case 47:
            return HID_KEY_ENTER;
        case 42:
            return HID_KEY_SPACE;
        // For debugging, also support the test mapping
        case 39:
            return HID_KEY_A;
        case 37:
            return HID_KEY_B;
        default:
            printf("Ignoring MIDI note: %d\n", midi_note);
            return 0;  // Ignore this note
    }
}
