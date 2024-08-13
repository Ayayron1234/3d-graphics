#pragma once
#ifdef GRAPHICS_PCH
#include "pch.h"
#else
#endif // GRAPHICS_PCH

#define _SPEC_KEY 0x40000000

namespace graphics {

enum class Key : uint8_t;

enum class KeyMod : uint8_t;

bool isKeyDown(Key key);

bool isKeyUp(Key key);
    
bool keyPressed(Key key);

bool keyReleased(Key key);

bool isModPressed(KeyMod mod);

uint8_t getMods();

enum class Key : uint8_t {
    A = 4,
    B = 5,
    C = 6,
    D = 7,
    E = 8,
    F = 9,
    G = 10,
    H = 11,
    I = 12,
    J = 13,
    K = 14,
    L = 15,
    M = 16,
    N = 17,
    O = 18,
    P = 19,
    Q = 20,
    R = 21,
    S = 22,
    T = 23,
    U = 24,
    V = 25,
    W = 26,
    X = 27,
    Y = 28,
    Z = 29,
    
    NUM_0 = 30,
    NUM_1 = 31,
    NUM_2 = 32,
    NUM_3 = 33,
    NUM_4 = 34,
    NUM_5 = 35,
    NUM_6 = 36,
    NUM_7 = 37,
    NUM_8 = 38,
    NUM_9 = 39,

    RETURN     = 40,
    ESCAPE     = 41,
    BACKSPACE  = 42,
    TAB        = 43,
    SPACE      = 44,

    //EXCLAIM    = '!',
    //QUOTEDBL   = '"',
    //HASH       = '#',
    //PERCENT    = '%',
    //DOLLAR     = '$',
    //AMPERSAND  = '&',
    //QUOTE      = '\'',
    //LEFTPAREN  = '(',
    //RIGHTPAREN = ')',
    //ASTERISK   = '*',
    //PLUS       = '+',
    //COMMA      = ',',
    //MINUS      = '-',
    //PERIOD     = '.',
    //SLASH      = '/',

    //COLON     = ':',
    SEMICOLON = 51,
    //LESS      = '<',
    //EQUALS    = '=',
    //GREATER   = '>',
    //QUESTION  = '?',
    //AT        = '@',

    LEFTBRACKET    = 47,
    RIGHTBRACKET   = 48,
    //CARET          = '^',
    //UNDERSCORE     = '_',
    //BACKQUOTE      = '`',

    F1  = 58,
    F2  = 59,
    F3  = 60,
    F4  = 61,
    F5  = 62,
    F6  = 63,
    F7  = 64,
    F8  = 65,
    F9  = 66,
    F10 = 67,
    F11 = 68,
    F12 = 69,

    PRINTSCREEN = 70,
    SCROLLLOCK  = 71,
    PAUSE       = 72,
    INSERT      = 73,

    HOME        = 74,
    PAGEUP      = 75,
    DELETE      = 76,
    END         = 77,
    PAGEDOWN    = 78,
    RIGHT       = 79,
    LEFT        = 80,
    DOWN        = 81,
    UP          = 82,
};

enum class KeyMod : uint8_t {
    NONE = 0x0000,
    LSHIFT = 0x0001,
    RSHIFT = 0x0002,
    LCTRL = 0x0040,
    RCTRL = 0x0080,
    LALT = 0x0100,
    RALT = 0x0200,
    LGUI = 0x0400,
    RGUI = 0x0800,
    NUM = 0x1000,
    CAPS = 0x2000,
    MODE = 0x4000,
    SCROLL = 0x8000,

    CTRL = LCTRL | RCTRL,
    SHIFT = LSHIFT | RSHIFT,
    ALT = LALT | RALT,
};

}
