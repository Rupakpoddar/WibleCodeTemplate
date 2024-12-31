#include "HID.h"
#include "Keyboard.h"

const uint8_t switchTable[75] = {
  0x00,                // NOTHING
  KEY_ESC,             // ESC
  KEY_F1,              // F1
  KEY_F2,              // F2
  KEY_F3,              // F3
  KEY_F4,              // F4
  KEY_F5,              // F5
  KEY_F6,              // F6
  KEY_F7,              // F7
  KEY_F8,              // F8
  KEY_F9,              // F9
  KEY_F10,             // F10
  KEY_F11,             // F11
  KEY_F12,             // F12
  KEY_DELETE,          // DELETE
  '`',                 // `
  '1',                 // 1
  '2',                 // 2
  '3',                 // 3
  '4',                 // 4
  '5',                 // 5
  '6',                 // 6
  '7',                 // 7
  '8',                 // 8
  '9',                 // 9
  '0',                 // 0
  '-',                 // -
  '=',                 // =
  KEY_BACKSPACE,       // BACKSPACE
  KEY_TAB,             // TAB
  'q',                 // q
  'w',                 // w
  'e',                 // e
  'r',                 // r
  't',                 // t
  'y',                 // y
  'u',                 // u
  'i',                 // i
  'o',                 // o
  'p',                 // p
  '[',                 // [
  ']',                 // ]
  '\\',                // BACKSLASH
  KEY_CAPS_LOCK,       // CAPS
  'a',                 // a
  's',                 // s
  'd',                 // d
  'f',                 // f
  'g',                 // g
  'h',                 // h
  'j',                 // j
  'k',                 // k
  'l',                 // l
  ';',                 // ;
  '\'',                // '
  KEY_RETURN,          // ENTER
  KEY_LEFT_SHIFT,      // SHIFT
  'z',                 // z
  'x',                 // x
  'c',                 // c
  'v',                 // v
  'b',                 // b
  'n',                 // n
  'm',                 // m
  ',',                 // COMMA
  '.',                 // .
  '/',                 // /
  KEY_UP_ARROW,        // ARROW_UP
  KEY_LEFT_CTRL,       // CTRL
  KEY_LEFT_ALT,        // ALT
  KEY_LEFT_GUI,        // WIN
  ' ',                 // SPACE
  KEY_LEFT_ARROW,      // ARROW_LEFT
  KEY_DOWN_ARROW,      // ARROW_DOWN
  KEY_RIGHT_ARROW,     // ARROW_RIGHT
};

#if defined(_USING_HID)

const uint16_t consumerTable[6] = {
  0x00B6,               // MEDIA_PREVIOUS
  0x00CD,               // MEDIA_PLAY_PAUSE
  0x00B5,               // MEDIA_NEXT
  0x00E2,               // MEDIA_VOLUME_MUTE
  0x00EA,               // MEDIA_VOLUME_DOWN
  0x00E9,               // MEDIA_VOLUME_UP
};

class ConsumerKeyboard {
private:
    void sendReport(uint16_t k) {
        HID().SendReport(3, &k, sizeof(k));
    }

public:
    ConsumerKeyboard() {
        static const uint8_t hidConsumerReportDescriptor[] PROGMEM = {
            0x05, 0x0C,                    // USAGE_PAGE (Consumer Devices)
            0x09, 0x01,                    // USAGE (Consumer Control)
            0xA1, 0x01,                    // COLLECTION (Application)
            0x85, 0x03,                    // REPORT_ID (3)
            0x15, 0x00,                    // LOGICAL_MINIMUM (0)
            0x26, 0xFF, 0x03,              // LOGICAL_MAXIMUM (1023)
            0x19, 0x00,                    // USAGE_MINIMUM (0)
            0x2A, 0xFF, 0x03,              // USAGE_MAXIMUM (1023)
            0x75, 0x10,                    // REPORT_SIZE (16)
            0x95, 0x01,                    // REPORT_COUNT (1)
            0x81, 0x00,                    // INPUT (Data, Array Type, Absolute Value Input)
            0xC0                           // END_COLLECTION
        };

        static HIDSubDescriptor node(hidConsumerReportDescriptor, sizeof(hidConsumerReportDescriptor));
        HID().AppendDescriptor(&node);
    }

    void press(uint16_t k) {
        sendReport(k);
    }

    void release() {
        sendReport(0);
    }
};

#endif
