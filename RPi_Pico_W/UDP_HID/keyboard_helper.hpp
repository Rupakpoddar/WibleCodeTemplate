// Helper file for UDP_HID.ino

#include "Keyboard.h"

const uint8_t switchTable[75] = {
  0x00,                   // NOTHING
  KEY_ESC,                // ESC
  KEY_F1,                 // F1
  KEY_F2,                 // F2
  KEY_F3,                 // F3
  KEY_F4,                 // F4
  KEY_F5,                 // F5
  KEY_F6,                 // F6
  KEY_F7,                 // F7
  KEY_F8,                 // F8
  KEY_F9,                 // F9
  KEY_F10,                // F10
  KEY_F11,                // F11
  KEY_F12,                // F12
  KEY_DELETE,             // DELETE
  '`',                    // `
  '1',                    // 1
  '2',                    // 2
  '3',                    // 3
  '4',                    // 4
  '5',                    // 5
  '6',                    // 6
  '7',                    // 7
  '8',                    // 8
  '9',                    // 9
  '0',                    // 0
  '-',                    // -
  '=',                    // =
  KEY_BACKSPACE,          // BACKSPACE
  KEY_TAB,                // TAB
  'q',                    // q
  'w',                    // w
  'e',                    // e
  'r',                    // r
  't',                    // t
  'y',                    // y
  'u',                    // u
  'i',                    // i
  'o',                    // o
  'p',                    // p
  '[',                    // [
  ']',                    // ]
  '\\',                   // BACKSLASH
  KEY_CAPS_LOCK,          // CAPS
  'a',                    // a
  's',                    // s
  'd',                    // d
  'f',                    // f
  'g',                    // g
  'h',                    // h
  'j',                    // j
  'k',                    // k
  'l',                    // l
  ';',                    // ;
  '\'',                   // '
  KEY_RETURN,             // ENTER
  KEY_LEFT_SHIFT,         // SHIFT
  'z',                    // z
  'x',                    // x
  'c',                    // c
  'v',                    // v
  'b',                    // b
  'n',                    // n
  'm',                    // m
  ',',                    // COMMA
  '.',                    // .
  '/',                    // /
  KEY_UP_ARROW,           // ARROW_UP
  KEY_LEFT_CTRL,          // CTRL
  KEY_LEFT_ALT,           // ALT
  KEY_LEFT_GUI,           // WIN
  ' ',                    // SPACE
  KEY_LEFT_ARROW,         // ARROW_LEFT
  KEY_DOWN_ARROW,         // ARROW_DOWN
  KEY_RIGHT_ARROW,        // ARROW_RIGHT
};

const uint16_t consumerTable[6] = {
  KEY_SCAN_PREVIOUS,      // MEDIA_PREVIOUS
  KEY_PLAY_PAUSE,         // MEDIA_PLAY_PAUSE
  KEY_SCAN_NEXT,          // MEDIA_NEXT
  KEY_MUTE,               // MEDIA_VOLUME_MUTE
  KEY_VOLUME_DECREMENT,   // MEDIA_VOLUME_DOWN
  KEY_VOLUME_INCREMENT,   // MEDIA_VOLUME_UP
};
