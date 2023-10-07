#pragma once

#include "lib/types.h"

namespace event_def {

/*
 * Device properties and quirks
 */

constexpr uint16_t INPUT_PROP_POINTER = 0x00;        /* needs a pointer */
constexpr uint16_t INPUT_PROP_DIRECT = 0x01;         /* direct input devices */
constexpr uint16_t INPUT_PROP_BUTTONPAD = 0x02;      /* has button(s) under pad */
constexpr uint16_t INPUT_PROP_SEMI_MT = 0x03;        /* touch rectangle only */
constexpr uint16_t INPUT_PROP_TOPBUTTONPAD = 0x04;   /* softbuttons at top of pad */
constexpr uint16_t INPUT_PROP_POINTING_STICK = 0x05; /* is a pointing stick */
constexpr uint16_t INPUT_PROP_ACCELEROMETER = 0x06;  /* has accelerometer */

constexpr uint16_t INPUT_PROP_MAX = 0x1f;
constexpr uint16_t INPUT_PROP_CNT = (INPUT_PROP_MAX + 1);

/*
 * Event types
 */

constexpr uint16_t EV_SYN = 0x00;
constexpr uint16_t EV_KEY = 0x01;
constexpr uint16_t EV_REL = 0x02;
constexpr uint16_t EV_ABS = 0x03;
constexpr uint16_t EV_MSC = 0x04;
constexpr uint16_t EV_SW = 0x05;
constexpr uint16_t EV_LED = 0x11;
constexpr uint16_t EV_SND = 0x12;
constexpr uint16_t EV_REP = 0x14;
constexpr uint16_t EV_FF = 0x15;
constexpr uint16_t EV_PWR = 0x16;
constexpr uint16_t EV_FF_STATUS = 0x17;
constexpr uint16_t EV_MAX = 0x1f;
constexpr uint16_t EV_CNT = (EV_MAX + 1);

/*
 * Synchronization events.
 */

constexpr uint16_t SYN_REPORT = 0;
constexpr uint16_t SYN_CONFIG = 1;
constexpr uint16_t SYN_MT_REPORT = 2;
constexpr uint16_t SYN_DROPPED = 3;
constexpr uint16_t SYN_MAX = 0xf;
constexpr uint16_t SYN_CNT = (SYN_MAX + 1);

/*
 * Keys and buttons
 *
 * Most of the keys/buttons are modeled after USB HUT 1.12
 * (see http://www.usb.org/developers/hidpage).
 * Abbreviations in the comments:
 * AC - Application Control
 * AL - Application Launch Button
 * SC - System Control
 */

constexpr uint16_t KEY_RESERVED = 0;
constexpr uint16_t KEY_ESC = 1;
constexpr uint16_t KEY_1 = 2;
constexpr uint16_t KEY_2 = 3;
constexpr uint16_t KEY_3 = 4;
constexpr uint16_t KEY_4 = 5;
constexpr uint16_t KEY_5 = 6;
constexpr uint16_t KEY_6 = 7;
constexpr uint16_t KEY_7 = 8;
constexpr uint16_t KEY_8 = 9;
constexpr uint16_t KEY_9 = 10;
constexpr uint16_t KEY_0 = 11;
constexpr uint16_t KEY_MINUS = 12;
constexpr uint16_t KEY_EQUAL = 13;
constexpr uint16_t KEY_BACKSPACE = 14;
constexpr uint16_t KEY_TAB = 15;
constexpr uint16_t KEY_Q = 16;
constexpr uint16_t KEY_W = 17;
constexpr uint16_t KEY_E = 18;
constexpr uint16_t KEY_R = 19;
constexpr uint16_t KEY_T = 20;
constexpr uint16_t KEY_Y = 21;
constexpr uint16_t KEY_U = 22;
constexpr uint16_t KEY_I = 23;
constexpr uint16_t KEY_O = 24;
constexpr uint16_t KEY_P = 25;
constexpr uint16_t KEY_LEFTBRACE = 26;
constexpr uint16_t KEY_RIGHTBRACE = 27;
constexpr uint16_t KEY_ENTER = 28;
constexpr uint16_t KEY_LEFTCTRL = 29;
constexpr uint16_t KEY_A = 30;
constexpr uint16_t KEY_S = 31;
constexpr uint16_t KEY_D = 32;
constexpr uint16_t KEY_F = 33;
constexpr uint16_t KEY_G = 34;
constexpr uint16_t KEY_H = 35;
constexpr uint16_t KEY_J = 36;
constexpr uint16_t KEY_K = 37;
constexpr uint16_t KEY_L = 38;
constexpr uint16_t KEY_SEMICOLON = 39;
constexpr uint16_t KEY_APOSTROPHE = 40;
constexpr uint16_t KEY_GRAVE = 41;
constexpr uint16_t KEY_LEFTSHIFT = 42;
constexpr uint16_t KEY_BACKSLASH = 43;
constexpr uint16_t KEY_Z = 44;
constexpr uint16_t KEY_X = 45;
constexpr uint16_t KEY_C = 46;
constexpr uint16_t KEY_V = 47;
constexpr uint16_t KEY_B = 48;
constexpr uint16_t KEY_N = 49;
constexpr uint16_t KEY_M = 50;
constexpr uint16_t KEY_COMMA = 51;
constexpr uint16_t KEY_DOT = 52;
constexpr uint16_t KEY_SLASH = 53;
constexpr uint16_t KEY_RIGHTSHIFT = 54;
constexpr uint16_t KEY_KPASTERISK = 55;
constexpr uint16_t KEY_LEFTALT = 56;
constexpr uint16_t KEY_SPACE = 57;
constexpr uint16_t KEY_CAPSLOCK = 58;
constexpr uint16_t KEY_F1 = 59;
constexpr uint16_t KEY_F2 = 60;
constexpr uint16_t KEY_F3 = 61;
constexpr uint16_t KEY_F4 = 62;
constexpr uint16_t KEY_F5 = 63;
constexpr uint16_t KEY_F6 = 64;
constexpr uint16_t KEY_F7 = 65;
constexpr uint16_t KEY_F8 = 66;
constexpr uint16_t KEY_F9 = 67;
constexpr uint16_t KEY_F10 = 68;
constexpr uint16_t KEY_NUMLOCK = 69;
constexpr uint16_t KEY_SCROLLLOCK = 70;
constexpr uint16_t KEY_KP7 = 71;
constexpr uint16_t KEY_KP8 = 72;
constexpr uint16_t KEY_KP9 = 73;
constexpr uint16_t KEY_KPMINUS = 74;
constexpr uint16_t KEY_KP4 = 75;
constexpr uint16_t KEY_KP5 = 76;
constexpr uint16_t KEY_KP6 = 77;
constexpr uint16_t KEY_KPPLUS = 78;
constexpr uint16_t KEY_KP1 = 79;
constexpr uint16_t KEY_KP2 = 80;
constexpr uint16_t KEY_KP3 = 81;
constexpr uint16_t KEY_KP0 = 82;
constexpr uint16_t KEY_KPDOT = 83;

constexpr uint16_t KEY_ZENKAKUHANKAKU = 85;
constexpr uint16_t KEY_102ND = 86;
constexpr uint16_t KEY_F11 = 87;
constexpr uint16_t KEY_F12 = 88;
constexpr uint16_t KEY_RO = 89;
constexpr uint16_t KEY_KATAKANA = 90;
constexpr uint16_t KEY_HIRAGANA = 91;
constexpr uint16_t KEY_HENKAN = 92;
constexpr uint16_t KEY_KATAKANAHIRAGANA = 93;
constexpr uint16_t KEY_MUHENKAN = 94;
constexpr uint16_t KEY_KPJPCOMMA = 95;
constexpr uint16_t KEY_KPENTER = 96;
constexpr uint16_t KEY_RIGHTCTRL = 97;
constexpr uint16_t KEY_KPSLASH = 98;
constexpr uint16_t KEY_SYSRQ = 99;
constexpr uint16_t KEY_RIGHTALT = 100;
constexpr uint16_t KEY_LINEFEED = 101;
constexpr uint16_t KEY_HOME = 102;
constexpr uint16_t KEY_UP = 103;
constexpr uint16_t KEY_PAGEUP = 104;
constexpr uint16_t KEY_LEFT = 105;
constexpr uint16_t KEY_RIGHT = 106;
constexpr uint16_t KEY_END = 107;
constexpr uint16_t KEY_DOWN = 108;
constexpr uint16_t KEY_PAGEDOWN = 109;
constexpr uint16_t KEY_INSERT = 110;
constexpr uint16_t KEY_DELETE = 111;
constexpr uint16_t KEY_MACRO = 112;
constexpr uint16_t KEY_MUTE = 113;
constexpr uint16_t KEY_VOLUMEDOWN = 114;
constexpr uint16_t KEY_VOLUMEUP = 115;
constexpr uint16_t KEY_POWER = 116; /* SC System Power Down */
constexpr uint16_t KEY_KPEQUAL = 117;
constexpr uint16_t KEY_KPPLUSMINUS = 118;
constexpr uint16_t KEY_PAUSE = 119;
constexpr uint16_t KEY_SCALE = 120; /* AL Compiz Scale (Expose) */

constexpr uint16_t KEY_KPCOMMA = 121;
constexpr uint16_t KEY_HANGEUL = 122;
constexpr uint16_t KEY_HANGUEL = KEY_HANGEUL;
constexpr uint16_t KEY_HANJA = 123;
constexpr uint16_t KEY_YEN = 124;
constexpr uint16_t KEY_LEFTMETA = 125;
constexpr uint16_t KEY_RIGHTMETA = 126;
constexpr uint16_t KEY_COMPOSE = 127;

constexpr uint16_t KEY_STOP = 128; /* AC Stop */
constexpr uint16_t KEY_AGAIN = 129;
constexpr uint16_t KEY_PROPS = 130; /* AC Properties */
constexpr uint16_t KEY_UNDO = 131;  /* AC Undo */
constexpr uint16_t KEY_FRONT = 132;
constexpr uint16_t KEY_COPY = 133;  /* AC Copy */
constexpr uint16_t KEY_OPEN = 134;  /* AC Open */
constexpr uint16_t KEY_PASTE = 135; /* AC Paste */
constexpr uint16_t KEY_FIND = 136;  /* AC Search */
constexpr uint16_t KEY_CUT = 137;   /* AC Cut */
constexpr uint16_t KEY_HELP = 138;  /* AL Integrated Help Center */
constexpr uint16_t KEY_MENU = 139;  /* Menu (show menu) */
constexpr uint16_t KEY_CALC = 140;  /* AL Calculator */
constexpr uint16_t KEY_SETUP = 141;
constexpr uint16_t KEY_SLEEP = 142;  /* SC System Sleep */
constexpr uint16_t KEY_WAKEUP = 143; /* System Wake Up */
constexpr uint16_t KEY_FILE = 144;   /* AL Local Machine Browser */
constexpr uint16_t KEY_SENDFILE = 145;
constexpr uint16_t KEY_DELETEFILE = 146;
constexpr uint16_t KEY_XFER = 147;
constexpr uint16_t KEY_PROG1 = 148;
constexpr uint16_t KEY_PROG2 = 149;
constexpr uint16_t KEY_WWW = 150; /* AL Internet Browser */
constexpr uint16_t KEY_MSDOS = 151;
constexpr uint16_t KEY_COFFEE = 152; /* AL Terminal Lock/Screensaver */
constexpr uint16_t KEY_SCREENLOCK = KEY_COFFEE;
constexpr uint16_t KEY_ROTATE_DISPLAY = 153; /* Display orientation for e.g. tablets */
constexpr uint16_t KEY_DIRECTION = KEY_ROTATE_DISPLAY;
constexpr uint16_t KEY_CYCLEWINDOWS = 154;
constexpr uint16_t KEY_MAIL = 155;
constexpr uint16_t KEY_BOOKMARKS = 156; /* AC Bookmarks */
constexpr uint16_t KEY_COMPUTER = 157;
constexpr uint16_t KEY_BACK = 158;    /* AC Back */
constexpr uint16_t KEY_FORWARD = 159; /* AC Forward */
constexpr uint16_t KEY_CLOSECD = 160;
constexpr uint16_t KEY_EJECTCD = 161;
constexpr uint16_t KEY_EJECTCLOSECD = 162;
constexpr uint16_t KEY_NEXTSONG = 163;
constexpr uint16_t KEY_PLAYPAUSE = 164;
constexpr uint16_t KEY_PREVIOUSSONG = 165;
constexpr uint16_t KEY_STOPCD = 166;
constexpr uint16_t KEY_RECORD = 167;
constexpr uint16_t KEY_REWIND = 168;
constexpr uint16_t KEY_PHONE = 169; /* Media Select Telephone */
constexpr uint16_t KEY_ISO = 170;
constexpr uint16_t KEY_CONFIG = 171;   /* AL Consumer Control Configuration */
constexpr uint16_t KEY_HOMEPAGE = 172; /* AC Home */
constexpr uint16_t KEY_REFRESH = 173;  /* AC Refresh */
constexpr uint16_t KEY_EXIT = 174;     /* AC Exit */
constexpr uint16_t KEY_MOVE = 175;
constexpr uint16_t KEY_EDIT = 176;
constexpr uint16_t KEY_SCROLLUP = 177;
constexpr uint16_t KEY_SCROLLDOWN = 178;
constexpr uint16_t KEY_KPLEFTPAREN = 179;
constexpr uint16_t KEY_KPRIGHTPAREN = 180;
constexpr uint16_t KEY_NEW = 181;  /* AC New */
constexpr uint16_t KEY_REDO = 182; /* AC Redo/Repeat */

constexpr uint16_t KEY_F13 = 183;
constexpr uint16_t KEY_F14 = 184;
constexpr uint16_t KEY_F15 = 185;
constexpr uint16_t KEY_F16 = 186;
constexpr uint16_t KEY_F17 = 187;
constexpr uint16_t KEY_F18 = 188;
constexpr uint16_t KEY_F19 = 189;
constexpr uint16_t KEY_F20 = 190;
constexpr uint16_t KEY_F21 = 191;
constexpr uint16_t KEY_F22 = 192;
constexpr uint16_t KEY_F23 = 193;
constexpr uint16_t KEY_F24 = 194;

constexpr uint16_t KEY_PLAYCD = 200;
constexpr uint16_t KEY_PAUSECD = 201;
constexpr uint16_t KEY_PROG3 = 202;
constexpr uint16_t KEY_PROG4 = 203;
constexpr uint16_t KEY_ALL_APPLICATIONS = 204; /* AC Desktop Show All Applications */
constexpr uint16_t KEY_DASHBOARD = KEY_ALL_APPLICATIONS;
constexpr uint16_t KEY_SUSPEND = 205;
constexpr uint16_t KEY_CLOSE = 206; /* AC Close */
constexpr uint16_t KEY_PLAY = 207;
constexpr uint16_t KEY_FASTFORWARD = 208;
constexpr uint16_t KEY_BASSBOOST = 209;
constexpr uint16_t KEY_PRINT = 210; /* AC Print */
constexpr uint16_t KEY_HP = 211;
constexpr uint16_t KEY_CAMERA = 212;
constexpr uint16_t KEY_SOUND = 213;
constexpr uint16_t KEY_QUESTION = 214;
constexpr uint16_t KEY_EMAIL = 215;
constexpr uint16_t KEY_CHAT = 216;
constexpr uint16_t KEY_SEARCH = 217;
constexpr uint16_t KEY_CONNECT = 218;
constexpr uint16_t KEY_FINANCE = 219; /* AL Checkbook/Finance */
constexpr uint16_t KEY_SPORT = 220;
constexpr uint16_t KEY_SHOP = 221;
constexpr uint16_t KEY_ALTERASE = 222;
constexpr uint16_t KEY_CANCEL = 223; /* AC Cancel */
constexpr uint16_t KEY_BRIGHTNESSDOWN = 224;
constexpr uint16_t KEY_BRIGHTNESSUP = 225;
constexpr uint16_t KEY_MEDIA = 226;

constexpr uint16_t KEY_SWITCHVIDEOMODE = 227; /* Cycle between available video  outputs (Monitor/LCD/TV-out/etc) */
constexpr uint16_t KEY_KBDILLUMTOGGLE = 228;
constexpr uint16_t KEY_KBDILLUMDOWN = 229;
constexpr uint16_t KEY_KBDILLUMUP = 230;

constexpr uint16_t KEY_SEND = 231;        /* AC Send */
constexpr uint16_t KEY_REPLY = 232;       /* AC Reply */
constexpr uint16_t KEY_FORWARDMAIL = 233; /* AC Forward Msg */
constexpr uint16_t KEY_SAVE = 234;        /* AC Save */
constexpr uint16_t KEY_DOCUMENTS = 235;

constexpr uint16_t KEY_BATTERY = 236;

constexpr uint16_t KEY_BLUETOOTH = 237;
constexpr uint16_t KEY_WLAN = 238;
constexpr uint16_t KEY_UWB = 239;

constexpr uint16_t KEY_UNKNOWN = 240;

constexpr uint16_t KEY_VIDEO_NEXT = 241;       /* drive next video source */
constexpr uint16_t KEY_VIDEO_PREV = 242;       /* drive previous video source */
constexpr uint16_t KEY_BRIGHTNESS_CYCLE = 243; /* brightness up, after max is min */
constexpr uint16_t KEY_BRIGHTNESS_AUTO = 244; /* Set Auto Brightness: manual brightness control is off, rely on ambient */
constexpr uint16_t KEY_BRIGHTNESS_ZERO = KEY_BRIGHTNESS_AUTO;
constexpr uint16_t KEY_DISPLAY_OFF = 245; /* display device to off state */

constexpr uint16_t KEY_WWAN = 246; /* Wireless WAN (LTE, UMTS, GSM, etc.) */
constexpr uint16_t KEY_WIMAX = KEY_WWAN;
constexpr uint16_t KEY_RFKILL = 247; /* Key that controls all radios */

constexpr uint16_t KEY_MICMUTE = 248; /* Mute / unmute the microphone */

/* Code 255 is reserved for special needs of AT keyboard driver */

constexpr uint16_t BTN_MISC = 0x100;
constexpr uint16_t BTN_0 = 0x100;
constexpr uint16_t BTN_1 = 0x101;
constexpr uint16_t BTN_2 = 0x102;
constexpr uint16_t BTN_3 = 0x103;
constexpr uint16_t BTN_4 = 0x104;
constexpr uint16_t BTN_5 = 0x105;
constexpr uint16_t BTN_6 = 0x106;
constexpr uint16_t BTN_7 = 0x107;
constexpr uint16_t BTN_8 = 0x108;
constexpr uint16_t BTN_9 = 0x109;

constexpr uint16_t BTN_MOUSE = 0x110;
constexpr uint16_t BTN_LEFT = 0x110;
constexpr uint16_t BTN_RIGHT = 0x111;
constexpr uint16_t BTN_MIDDLE = 0x112;
constexpr uint16_t BTN_SIDE = 0x113;
constexpr uint16_t BTN_EXTRA = 0x114;
constexpr uint16_t BTN_FORWARD = 0x115;
constexpr uint16_t BTN_BACK = 0x116;
constexpr uint16_t BTN_TASK = 0x117;

constexpr uint16_t BTN_JOYSTICK = 0x120;
constexpr uint16_t BTN_TRIGGER = 0x120;
constexpr uint16_t BTN_THUMB = 0x121;
constexpr uint16_t BTN_THUMB2 = 0x122;
constexpr uint16_t BTN_TOP = 0x123;
constexpr uint16_t BTN_TOP2 = 0x124;
constexpr uint16_t BTN_PINKIE = 0x125;
constexpr uint16_t BTN_BASE = 0x126;
constexpr uint16_t BTN_BASE2 = 0x127;
constexpr uint16_t BTN_BASE3 = 0x128;
constexpr uint16_t BTN_BASE4 = 0x129;
constexpr uint16_t BTN_BASE5 = 0x12a;
constexpr uint16_t BTN_BASE6 = 0x12b;
constexpr uint16_t BTN_DEAD = 0x12f;

constexpr uint16_t BTN_GAMEPAD = 0x130;
constexpr uint16_t BTN_SOUTH = 0x130;
constexpr uint16_t BTN_A = BTN_SOUTH;
constexpr uint16_t BTN_EAST = 0x131;
constexpr uint16_t BTN_B = BTN_EAST;
constexpr uint16_t BTN_C = 0x132;
constexpr uint16_t BTN_NORTH = 0x133;
constexpr uint16_t BTN_X = BTN_NORTH;
constexpr uint16_t BTN_WEST = 0x134;
constexpr uint16_t BTN_Y = BTN_WEST;
constexpr uint16_t BTN_Z = 0x135;
constexpr uint16_t BTN_TL = 0x136;
constexpr uint16_t BTN_TR = 0x137;
constexpr uint16_t BTN_TL2 = 0x138;
constexpr uint16_t BTN_TR2 = 0x139;
constexpr uint16_t BTN_SELECT = 0x13a;
constexpr uint16_t BTN_START = 0x13b;
constexpr uint16_t BTN_MODE = 0x13c;
constexpr uint16_t BTN_THUMBL = 0x13d;
constexpr uint16_t BTN_THUMBR = 0x13e;

constexpr uint16_t BTN_DIGI = 0x140;
constexpr uint16_t BTN_TOOL_PEN = 0x140;
constexpr uint16_t BTN_TOOL_RUBBER = 0x141;
constexpr uint16_t BTN_TOOL_BRUSH = 0x142;
constexpr uint16_t BTN_TOOL_PENCIL = 0x143;
constexpr uint16_t BTN_TOOL_AIRBRUSH = 0x144;
constexpr uint16_t BTN_TOOL_FINGER = 0x145;
constexpr uint16_t BTN_TOOL_MOUSE = 0x146;
constexpr uint16_t BTN_TOOL_LENS = 0x147;
constexpr uint16_t BTN_TOOL_QUINTTAP = 0x148; /* Five fingers on trackpad */
constexpr uint16_t BTN_STYLUS3 = 0x149;
constexpr uint16_t BTN_TOUCH = 0x14a;
constexpr uint16_t BTN_STYLUS = 0x14b;
constexpr uint16_t BTN_STYLUS2 = 0x14c;
constexpr uint16_t BTN_TOOL_DOUBLETAP = 0x14d;
constexpr uint16_t BTN_TOOL_TRIPLETAP = 0x14e;
constexpr uint16_t BTN_TOOL_QUADTAP = 0x14f; /* Four fingers on trackpad */

constexpr uint16_t BTN_WHEEL = 0x150;
constexpr uint16_t BTN_GEAR_DOWN = 0x150;
constexpr uint16_t BTN_GEAR_UP = 0x151;

constexpr uint16_t KEY_OK = 0x160;
constexpr uint16_t KEY_SELECT = 0x161;
constexpr uint16_t KEY_GOTO = 0x162;
constexpr uint16_t KEY_CLEAR = 0x163;
constexpr uint16_t KEY_POWER2 = 0x164;
constexpr uint16_t KEY_OPTION = 0x165;
constexpr uint16_t KEY_INFO = 0x166; /* AL OEM Features/Tips/Tutorial */
constexpr uint16_t KEY_TIME = 0x167;
constexpr uint16_t KEY_VENDOR = 0x168;
constexpr uint16_t KEY_ARCHIVE = 0x169;
constexpr uint16_t KEY_PROGRAM = 0x16a; /* Media Select Program Guide */
constexpr uint16_t KEY_CHANNEL = 0x16b;
constexpr uint16_t KEY_FAVORITES = 0x16c;
constexpr uint16_t KEY_EPG = 0x16d;
constexpr uint16_t KEY_PVR = 0x16e; /* Media Select Home */
constexpr uint16_t KEY_MHP = 0x16f;
constexpr uint16_t KEY_LANGUAGE = 0x170;
constexpr uint16_t KEY_TITLE = 0x171;
constexpr uint16_t KEY_SUBTITLE = 0x172;
constexpr uint16_t KEY_ANGLE = 0x173;
constexpr uint16_t KEY_FULL_SCREEN = 0x174; /* AC View Toggle */
constexpr uint16_t KEY_ZOOM = KEY_FULL_SCREEN;
constexpr uint16_t KEY_MODE = 0x175;
constexpr uint16_t KEY_KEYBOARD = 0x176;
constexpr uint16_t KEY_ASPECT_RATIO = 0x177; /* HUTRR37: Aspect */
constexpr uint16_t KEY_SCREEN = KEY_ASPECT_RATIO;
constexpr uint16_t KEY_PC = 0x178;   /* Media Select Computer */
constexpr uint16_t KEY_TV = 0x179;   /* Media Select TV */
constexpr uint16_t KEY_TV2 = 0x17a;  /* Media Select Cable */
constexpr uint16_t KEY_VCR = 0x17b;  /* Media Select VCR */
constexpr uint16_t KEY_VCR2 = 0x17c; /* VCR Plus */
constexpr uint16_t KEY_SAT = 0x17d;  /* Media Select Satellite */
constexpr uint16_t KEY_SAT2 = 0x17e;
constexpr uint16_t KEY_CD = 0x17f;   /* Media Select CD */
constexpr uint16_t KEY_TAPE = 0x180; /* Media Select Tape */
constexpr uint16_t KEY_RADIO = 0x181;
constexpr uint16_t KEY_TUNER = 0x182; /* Media Select Tuner */
constexpr uint16_t KEY_PLAYER = 0x183;
constexpr uint16_t KEY_TEXT = 0x184;
constexpr uint16_t KEY_DVD = 0x185; /* Media Select DVD */
constexpr uint16_t KEY_AUX = 0x186;
constexpr uint16_t KEY_MP3 = 0x187;
constexpr uint16_t KEY_AUDIO = 0x188; /* AL Audio Browser */
constexpr uint16_t KEY_VIDEO = 0x189; /* AL Movie Browser */
constexpr uint16_t KEY_DIRECTORY = 0x18a;
constexpr uint16_t KEY_LIST = 0x18b;
constexpr uint16_t KEY_MEMO = 0x18c; /* Media Select Messages */
constexpr uint16_t KEY_CALENDAR = 0x18d;
constexpr uint16_t KEY_RED = 0x18e;
constexpr uint16_t KEY_GREEN = 0x18f;
constexpr uint16_t KEY_YELLOW = 0x190;
constexpr uint16_t KEY_BLUE = 0x191;
constexpr uint16_t KEY_CHANNELUP = 0x192;   /* Channel Increment */
constexpr uint16_t KEY_CHANNELDOWN = 0x193; /* Channel Decrement */
constexpr uint16_t KEY_FIRST = 0x194;
constexpr uint16_t KEY_LAST = 0x195; /* Recall Last */
constexpr uint16_t KEY_AB = 0x196;
constexpr uint16_t KEY_NEXT = 0x197;
constexpr uint16_t KEY_RESTART = 0x198;
constexpr uint16_t KEY_SLOW = 0x199;
constexpr uint16_t KEY_SHUFFLE = 0x19a;
constexpr uint16_t KEY_BREAK = 0x19b;
constexpr uint16_t KEY_PREVIOUS = 0x19c;
constexpr uint16_t KEY_DIGITS = 0x19d;
constexpr uint16_t KEY_TEEN = 0x19e;
constexpr uint16_t KEY_TWEN = 0x19f;
constexpr uint16_t KEY_VIDEOPHONE = 0x1a0;     /* Media Select Video Phone */
constexpr uint16_t KEY_GAMES = 0x1a1;          /* Media Select Games */
constexpr uint16_t KEY_ZOOMIN = 0x1a2;         /* AC Zoom In */
constexpr uint16_t KEY_ZOOMOUT = 0x1a3;        /* AC Zoom Out */
constexpr uint16_t KEY_ZOOMRESET = 0x1a4;      /* AC Zoom */
constexpr uint16_t KEY_WORDPROCESSOR = 0x1a5;  /* AL Word Processor */
constexpr uint16_t KEY_EDITOR = 0x1a6;         /* AL Text Editor */
constexpr uint16_t KEY_SPREADSHEET = 0x1a7;    /* AL Spreadsheet */
constexpr uint16_t KEY_GRAPHICSEDITOR = 0x1a8; /* AL Graphics Editor */
constexpr uint16_t KEY_PRESENTATION = 0x1a9;   /* AL Presentation App */
constexpr uint16_t KEY_DATABASE = 0x1aa;       /* AL Database App */
constexpr uint16_t KEY_NEWS = 0x1ab;           /* AL Newsreader */
constexpr uint16_t KEY_VOICEMAIL = 0x1ac;      /* AL Voicemail */
constexpr uint16_t KEY_ADDRESSBOOK = 0x1ad;    /* AL Contacts/Address Book */
constexpr uint16_t KEY_MESSENGER = 0x1ae;      /* AL Instant Messaging */
constexpr uint16_t KEY_DISPLAYTOGGLE = 0x1af;  /* Turn display (LCD) on and off */
constexpr uint16_t KEY_BRIGHTNESS_TOGGLE = KEY_DISPLAYTOGGLE;
constexpr uint16_t KEY_SPELLCHECK = 0x1b0; /* AL Spell Check */
constexpr uint16_t KEY_LOGOFF = 0x1b1;     /* AL Logoff */

constexpr uint16_t KEY_DOLLAR = 0x1b2;
constexpr uint16_t KEY_EURO = 0x1b3;

constexpr uint16_t KEY_FRAMEBACK = 0x1b4; /* Consumer - transport controls */
constexpr uint16_t KEY_FRAMEFORWARD = 0x1b5;
constexpr uint16_t KEY_CONTEXT_MENU = 0x1b6;        /* GenDesc - system context menu */
constexpr uint16_t KEY_MEDIA_REPEAT = 0x1b7;        /* Consumer - transport control */
constexpr uint16_t KEY_10CHANNELSUP = 0x1b8;        /* 10 channels up (10+) */
constexpr uint16_t KEY_10CHANNELSDOWN = 0x1b9;      /* 10 channels down (10-) */
constexpr uint16_t KEY_IMAGES = 0x1ba;              /* AL Image Browser */
constexpr uint16_t KEY_NOTIFICATION_CENTER = 0x1bc; /* Show/hide the notification center */
constexpr uint16_t KEY_PICKUP_PHONE = 0x1bd;        /* Answer incoming call */
constexpr uint16_t KEY_HANGUP_PHONE = 0x1be;        /* Decline incoming call */

constexpr uint16_t KEY_DEL_EOL = 0x1c0;
constexpr uint16_t KEY_DEL_EOS = 0x1c1;
constexpr uint16_t KEY_INS_LINE = 0x1c2;
constexpr uint16_t KEY_DEL_LINE = 0x1c3;

constexpr uint16_t KEY_FN = 0x1d0;
constexpr uint16_t KEY_FN_ESC = 0x1d1;
constexpr uint16_t KEY_FN_F1 = 0x1d2;
constexpr uint16_t KEY_FN_F2 = 0x1d3;
constexpr uint16_t KEY_FN_F3 = 0x1d4;
constexpr uint16_t KEY_FN_F4 = 0x1d5;
constexpr uint16_t KEY_FN_F5 = 0x1d6;
constexpr uint16_t KEY_FN_F6 = 0x1d7;
constexpr uint16_t KEY_FN_F7 = 0x1d8;
constexpr uint16_t KEY_FN_F8 = 0x1d9;
constexpr uint16_t KEY_FN_F9 = 0x1da;
constexpr uint16_t KEY_FN_F10 = 0x1db;
constexpr uint16_t KEY_FN_F11 = 0x1dc;
constexpr uint16_t KEY_FN_F12 = 0x1dd;
constexpr uint16_t KEY_FN_1 = 0x1de;
constexpr uint16_t KEY_FN_2 = 0x1df;
constexpr uint16_t KEY_FN_D = 0x1e0;
constexpr uint16_t KEY_FN_E = 0x1e1;
constexpr uint16_t KEY_FN_F = 0x1e2;
constexpr uint16_t KEY_FN_S = 0x1e3;
constexpr uint16_t KEY_FN_B = 0x1e4;
constexpr uint16_t KEY_FN_RIGHT_SHIFT = 0x1e5;

constexpr uint16_t KEY_BRL_DOT1 = 0x1f1;
constexpr uint16_t KEY_BRL_DOT2 = 0x1f2;
constexpr uint16_t KEY_BRL_DOT3 = 0x1f3;
constexpr uint16_t KEY_BRL_DOT4 = 0x1f4;
constexpr uint16_t KEY_BRL_DOT5 = 0x1f5;
constexpr uint16_t KEY_BRL_DOT6 = 0x1f6;
constexpr uint16_t KEY_BRL_DOT7 = 0x1f7;
constexpr uint16_t KEY_BRL_DOT8 = 0x1f8;
constexpr uint16_t KEY_BRL_DOT9 = 0x1f9;
constexpr uint16_t KEY_BRL_DOT10 = 0x1fa;

constexpr uint16_t KEY_NUMERIC_0 = 0x200; /* used by phones, remote controls, */
constexpr uint16_t KEY_NUMERIC_1 = 0x201; /* and other keypads */
constexpr uint16_t KEY_NUMERIC_2 = 0x202;
constexpr uint16_t KEY_NUMERIC_3 = 0x203;
constexpr uint16_t KEY_NUMERIC_4 = 0x204;
constexpr uint16_t KEY_NUMERIC_5 = 0x205;
constexpr uint16_t KEY_NUMERIC_6 = 0x206;
constexpr uint16_t KEY_NUMERIC_7 = 0x207;
constexpr uint16_t KEY_NUMERIC_8 = 0x208;
constexpr uint16_t KEY_NUMERIC_9 = 0x209;
constexpr uint16_t KEY_NUMERIC_STAR = 0x20a;
constexpr uint16_t KEY_NUMERIC_POUND = 0x20b;
constexpr uint16_t KEY_NUMERIC_A = 0x20c; /* Phone key A - HUT Telephony 0xb9 */
constexpr uint16_t KEY_NUMERIC_B = 0x20d;
constexpr uint16_t KEY_NUMERIC_C = 0x20e;
constexpr uint16_t KEY_NUMERIC_D = 0x20f;

constexpr uint16_t KEY_CAMERA_FOCUS = 0x210;
constexpr uint16_t KEY_WPS_BUTTON = 0x211; /* WiFi Protected Setup key */

constexpr uint16_t KEY_TOUCHPAD_TOGGLE = 0x212; /* Request switch touchpad on or off */
constexpr uint16_t KEY_TOUCHPAD_ON = 0x213;
constexpr uint16_t KEY_TOUCHPAD_OFF = 0x214;

constexpr uint16_t KEY_CAMERA_ZOOMIN = 0x215;
constexpr uint16_t KEY_CAMERA_ZOOMOUT = 0x216;
constexpr uint16_t KEY_CAMERA_UP = 0x217;
constexpr uint16_t KEY_CAMERA_DOWN = 0x218;
constexpr uint16_t KEY_CAMERA_LEFT = 0x219;
constexpr uint16_t KEY_CAMERA_RIGHT = 0x21a;

constexpr uint16_t KEY_ATTENDANT_ON = 0x21b;
constexpr uint16_t KEY_ATTENDANT_OFF = 0x21c;
constexpr uint16_t KEY_ATTENDANT_TOGGLE = 0x21d; /* Attendant call on or off */
constexpr uint16_t KEY_LIGHTS_TOGGLE = 0x21e;    /* Reading light on or off */

constexpr uint16_t BTN_DPAD_UP = 0x220;
constexpr uint16_t BTN_DPAD_DOWN = 0x221;
constexpr uint16_t BTN_DPAD_LEFT = 0x222;
constexpr uint16_t BTN_DPAD_RIGHT = 0x223;

constexpr uint16_t KEY_ALS_TOGGLE = 0x230;         /* Ambient light sensor */
constexpr uint16_t KEY_ROTATE_LOCK_TOGGLE = 0x231; /* Display rotation lock */

constexpr uint16_t KEY_BUTTONCONFIG = 0x240;    /* AL Button Configuration */
constexpr uint16_t KEY_TASKMANAGER = 0x241;     /* AL Task/Project Manager */
constexpr uint16_t KEY_JOURNAL = 0x242;         /* AL Log/Journal/Timecard */
constexpr uint16_t KEY_CONTROLPANEL = 0x243;    /* AL Control Panel */
constexpr uint16_t KEY_APPSELECT = 0x244;       /* AL Select Task/Application */
constexpr uint16_t KEY_SCREENSAVER = 0x245;     /* AL Screen Saver */
constexpr uint16_t KEY_VOICECOMMAND = 0x246;    /* Listening Voice Command */
constexpr uint16_t KEY_ASSISTANT = 0x247;       /* AL Context-aware desktop assistant */
constexpr uint16_t KEY_KBD_LAYOUT_NEXT = 0x248; /* AC Next Keyboard Layout Select */
constexpr uint16_t KEY_EMOJI_PICKER = 0x249;    /* Show/hide emoji picker (HUTRR101) */
constexpr uint16_t KEY_DICTATE = 0x24a; /* Start or Stop Voice Dictation Session (HUTRR99)   \
                           */
constexpr uint16_t KEY_CAMERA_ACCESS_ENABLE = 0x24b; /* Enables programmatic access to camera devices. (HUTRR72) */
constexpr uint16_t KEY_CAMERA_ACCESS_DISABLE = 0x24c; /* Disables programmatic access to camera devices. (HUTRR72) */
constexpr uint16_t KEY_CAMERA_ACCESS_TOGGLE = 0x24d; /* Toggles the current state of the camera access control. (HUTRR72) */

constexpr uint16_t KEY_BRIGHTNESS_MIN = 0x250; /* Set Brightness to Minimum */
constexpr uint16_t KEY_BRIGHTNESS_MAX = 0x251; /* Set Brightness to Maximum */

constexpr uint16_t KEY_KBDINPUTASSIST_PREV = 0x260;
constexpr uint16_t KEY_KBDINPUTASSIST_NEXT = 0x261;
constexpr uint16_t KEY_KBDINPUTASSIST_PREVGROUP = 0x262;
constexpr uint16_t KEY_KBDINPUTASSIST_NEXTGROUP = 0x263;
constexpr uint16_t KEY_KBDINPUTASSIST_ACCEPT = 0x264;
constexpr uint16_t KEY_KBDINPUTASSIST_CANCEL = 0x265;

/* Diagonal movement keys */
constexpr uint16_t KEY_RIGHT_UP = 0x266;
constexpr uint16_t KEY_RIGHT_DOWN = 0x267;
constexpr uint16_t KEY_LEFT_UP = 0x268;
constexpr uint16_t KEY_LEFT_DOWN = 0x269;

constexpr uint16_t KEY_ROOT_MENU = 0x26a; /* Show Device's Root Menu */
/* Show Top Menu of the Media (e.g. DVD) */
constexpr uint16_t KEY_MEDIA_TOP_MENU = 0x26b;
constexpr uint16_t KEY_NUMERIC_11 = 0x26c;
constexpr uint16_t KEY_NUMERIC_12 = 0x26d;
/*
 * Toggle Audio Description: refers to an audio service that helps blind and
 * visually impaired consumers understand the action in a program. Note: in
 * some countries this is referred to as "Video Description".
 */
constexpr uint16_t KEY_AUDIO_DESC = 0x26e;
constexpr uint16_t KEY_3D_MODE = 0x26f;
constexpr uint16_t KEY_NEXT_FAVORITE = 0x270;
constexpr uint16_t KEY_STOP_RECORD = 0x271;
constexpr uint16_t KEY_PAUSE_RECORD = 0x272;
constexpr uint16_t KEY_VOD = 0x273; /* Video on Demand */
constexpr uint16_t KEY_UNMUTE = 0x274;
constexpr uint16_t KEY_FASTREVERSE = 0x275;
constexpr uint16_t KEY_SLOWREVERSE = 0x276;
/*
 * Control a data application associated with the currently viewed channel,
 * e.g. teletext or data broadcast application (MHEG, MHP, HbbTV, etc.)
 */
constexpr uint16_t KEY_DATA = 0x277;
constexpr uint16_t KEY_ONSCREEN_KEYBOARD = 0x278;
/* Electronic privacy screen control */
constexpr uint16_t KEY_PRIVACY_SCREEN_TOGGLE = 0x279;

/* Select an area of screen to be copied */
constexpr uint16_t KEY_SELECTIVE_SCREENSHOT = 0x27a;

/* Move the focus to the next or previous user controllable element within a UI
 * container */
constexpr uint16_t KEY_NEXT_ELEMENT = 0x27b;
constexpr uint16_t KEY_PREVIOUS_ELEMENT = 0x27c;

/* Toggle Autopilot engagement */
constexpr uint16_t KEY_AUTOPILOT_ENGAGE_TOGGLE = 0x27d;

/* Shortcut Keys */
constexpr uint16_t KEY_MARK_WAYPOINT = 0x27e;
constexpr uint16_t KEY_SOS = 0x27f;
constexpr uint16_t KEY_NAV_CHART = 0x280;
constexpr uint16_t KEY_FISHING_CHART = 0x281;
constexpr uint16_t KEY_SINGLE_RANGE_RADAR = 0x282;
constexpr uint16_t KEY_DUAL_RANGE_RADAR = 0x283;
constexpr uint16_t KEY_RADAR_OVERLAY = 0x284;
constexpr uint16_t KEY_TRADITIONAL_SONAR = 0x285;
constexpr uint16_t KEY_CLEARVU_SONAR = 0x286;
constexpr uint16_t KEY_SIDEVU_SONAR = 0x287;
constexpr uint16_t KEY_NAV_INFO = 0x288;
constexpr uint16_t KEY_BRIGHTNESS_MENU = 0x289;

/*
 * Some keyboards have keys which do not have a defined meaning, these keys
 * are intended to be programmed / bound to macros by the user. For most
 * keyboards with these macro-keys the key-sequence to inject, or action to
 * take, is all handled by software on the host side. So from the kernel's
 * point of view these are just normal keys.
 *
 * The KEY_MACRO# codes below are intended for such keys, which may be labeled
 * e.g. G1-G18, or S1 - S30. The KEY_MACRO# codes MUST NOT be used for keys
 * where the marking on the key does indicate a defined meaning / purpose.
 *
 * The KEY_MACRO# codes MUST also NOT be used as fallback for when no existing
 * KEY_FOO define matches the marking / purpose. In this case a new KEY_FOO
 * define MUST be added.
 */
constexpr uint16_t KEY_MACRO1 = 0x290;
constexpr uint16_t KEY_MACRO2 = 0x291;
constexpr uint16_t KEY_MACRO3 = 0x292;
constexpr uint16_t KEY_MACRO4 = 0x293;
constexpr uint16_t KEY_MACRO5 = 0x294;
constexpr uint16_t KEY_MACRO6 = 0x295;
constexpr uint16_t KEY_MACRO7 = 0x296;
constexpr uint16_t KEY_MACRO8 = 0x297;
constexpr uint16_t KEY_MACRO9 = 0x298;
constexpr uint16_t KEY_MACRO10 = 0x299;
constexpr uint16_t KEY_MACRO11 = 0x29a;
constexpr uint16_t KEY_MACRO12 = 0x29b;
constexpr uint16_t KEY_MACRO13 = 0x29c;
constexpr uint16_t KEY_MACRO14 = 0x29d;
constexpr uint16_t KEY_MACRO15 = 0x29e;
constexpr uint16_t KEY_MACRO16 = 0x29f;
constexpr uint16_t KEY_MACRO17 = 0x2a0;
constexpr uint16_t KEY_MACRO18 = 0x2a1;
constexpr uint16_t KEY_MACRO19 = 0x2a2;
constexpr uint16_t KEY_MACRO20 = 0x2a3;
constexpr uint16_t KEY_MACRO21 = 0x2a4;
constexpr uint16_t KEY_MACRO22 = 0x2a5;
constexpr uint16_t KEY_MACRO23 = 0x2a6;
constexpr uint16_t KEY_MACRO24 = 0x2a7;
constexpr uint16_t KEY_MACRO25 = 0x2a8;
constexpr uint16_t KEY_MACRO26 = 0x2a9;
constexpr uint16_t KEY_MACRO27 = 0x2aa;
constexpr uint16_t KEY_MACRO28 = 0x2ab;
constexpr uint16_t KEY_MACRO29 = 0x2ac;
constexpr uint16_t KEY_MACRO30 = 0x2ad;

/*
 * Some keyboards with the macro-keys described above have some extra keys
 * for controlling the host-side software responsible for the macro handling:
 * -A macro recording start/stop key. Note that not all keyboards which emit
 *  KEY_MACRO_RECORD_START will also emit KEY_MACRO_RECORD_STOP if
 *  KEY_MACRO_RECORD_STOP is not advertised, then KEY_MACRO_RECORD_START
 *  should be interpreted as a recording start/stop toggle;
 * -Keys for switching between different macro (pre)sets, either a key for
 *  cycling through the configured presets or keys to directly select a preset.
 */
constexpr uint16_t KEY_MACRO_RECORD_START = 0x2b0;
constexpr uint16_t KEY_MACRO_RECORD_STOP = 0x2b1;
constexpr uint16_t KEY_MACRO_PRESET_CYCLE = 0x2b2;
constexpr uint16_t KEY_MACRO_PRESET1 = 0x2b3;
constexpr uint16_t KEY_MACRO_PRESET2 = 0x2b4;
constexpr uint16_t KEY_MACRO_PRESET3 = 0x2b5;

/*
 * Some keyboards have a buildin LCD panel where the contents are controlled
 * by the host. Often these have a number of keys directly below the LCD
 * intended for controlling a menu shown on the LCD. These keys often don't
 * have any labeling so we just name them KEY_KBD_LCD_MENU#
 */
constexpr uint16_t KEY_KBD_LCD_MENU1 = 0x2b8;
constexpr uint16_t KEY_KBD_LCD_MENU2 = 0x2b9;
constexpr uint16_t KEY_KBD_LCD_MENU3 = 0x2ba;
constexpr uint16_t KEY_KBD_LCD_MENU4 = 0x2bb;
constexpr uint16_t KEY_KBD_LCD_MENU5 = 0x2bc;

constexpr uint16_t BTN_TRIGGER_HAPPY = 0x2c0;
constexpr uint16_t BTN_TRIGGER_HAPPY1 = 0x2c0;
constexpr uint16_t BTN_TRIGGER_HAPPY2 = 0x2c1;
constexpr uint16_t BTN_TRIGGER_HAPPY3 = 0x2c2;
constexpr uint16_t BTN_TRIGGER_HAPPY4 = 0x2c3;
constexpr uint16_t BTN_TRIGGER_HAPPY5 = 0x2c4;
constexpr uint16_t BTN_TRIGGER_HAPPY6 = 0x2c5;
constexpr uint16_t BTN_TRIGGER_HAPPY7 = 0x2c6;
constexpr uint16_t BTN_TRIGGER_HAPPY8 = 0x2c7;
constexpr uint16_t BTN_TRIGGER_HAPPY9 = 0x2c8;
constexpr uint16_t BTN_TRIGGER_HAPPY10 = 0x2c9;
constexpr uint16_t BTN_TRIGGER_HAPPY11 = 0x2ca;
constexpr uint16_t BTN_TRIGGER_HAPPY12 = 0x2cb;
constexpr uint16_t BTN_TRIGGER_HAPPY13 = 0x2cc;
constexpr uint16_t BTN_TRIGGER_HAPPY14 = 0x2cd;
constexpr uint16_t BTN_TRIGGER_HAPPY15 = 0x2ce;
constexpr uint16_t BTN_TRIGGER_HAPPY16 = 0x2cf;
constexpr uint16_t BTN_TRIGGER_HAPPY17 = 0x2d0;
constexpr uint16_t BTN_TRIGGER_HAPPY18 = 0x2d1;
constexpr uint16_t BTN_TRIGGER_HAPPY19 = 0x2d2;
constexpr uint16_t BTN_TRIGGER_HAPPY20 = 0x2d3;
constexpr uint16_t BTN_TRIGGER_HAPPY21 = 0x2d4;
constexpr uint16_t BTN_TRIGGER_HAPPY22 = 0x2d5;
constexpr uint16_t BTN_TRIGGER_HAPPY23 = 0x2d6;
constexpr uint16_t BTN_TRIGGER_HAPPY24 = 0x2d7;
constexpr uint16_t BTN_TRIGGER_HAPPY25 = 0x2d8;
constexpr uint16_t BTN_TRIGGER_HAPPY26 = 0x2d9;
constexpr uint16_t BTN_TRIGGER_HAPPY27 = 0x2da;
constexpr uint16_t BTN_TRIGGER_HAPPY28 = 0x2db;
constexpr uint16_t BTN_TRIGGER_HAPPY29 = 0x2dc;
constexpr uint16_t BTN_TRIGGER_HAPPY30 = 0x2dd;
constexpr uint16_t BTN_TRIGGER_HAPPY31 = 0x2de;
constexpr uint16_t BTN_TRIGGER_HAPPY32 = 0x2df;
constexpr uint16_t BTN_TRIGGER_HAPPY33 = 0x2e0;
constexpr uint16_t BTN_TRIGGER_HAPPY34 = 0x2e1;
constexpr uint16_t BTN_TRIGGER_HAPPY35 = 0x2e2;
constexpr uint16_t BTN_TRIGGER_HAPPY36 = 0x2e3;
constexpr uint16_t BTN_TRIGGER_HAPPY37 = 0x2e4;
constexpr uint16_t BTN_TRIGGER_HAPPY38 = 0x2e5;
constexpr uint16_t BTN_TRIGGER_HAPPY39 = 0x2e6;
constexpr uint16_t BTN_TRIGGER_HAPPY40 = 0x2e7;

/* We avoid low common keys in module aliases so they don't get huge. */
constexpr uint16_t KEY_MIN_INTERESTING = KEY_MUTE;
constexpr uint16_t KEY_MAX = 0x2ff;
constexpr uint16_t KEY_CNT = (KEY_MAX + 1);

/*
 * Relative axes
 */

constexpr uint16_t REL_X = 0x00;
constexpr uint16_t REL_Y = 0x01;
constexpr uint16_t REL_Z = 0x02;
constexpr uint16_t REL_RX = 0x03;
constexpr uint16_t REL_RY = 0x04;
constexpr uint16_t REL_RZ = 0x05;
constexpr uint16_t REL_HWHEEL = 0x06;
constexpr uint16_t REL_DIAL = 0x07;
constexpr uint16_t REL_WHEEL = 0x08;
constexpr uint16_t REL_MISC = 0x09;
/*
 * 0x0a is reserved and should not be used in input drivers.
 * It was used by HID as REL_MISC+1 and userspace needs to detect if
 * the next REL_* event is correct or is just REL_MISC + n.
 * We define here REL_RESERVED so userspace can rely on it and detect
 * the situation described above.
 */
constexpr uint16_t REL_RESERVED = 0x0a;
constexpr uint16_t REL_WHEEL_HI_RES = 0x0b;
constexpr uint16_t REL_HWHEEL_HI_RES = 0x0c;
constexpr uint16_t REL_MAX = 0x0f;
constexpr uint16_t REL_CNT = (REL_MAX + 1);

/*
 * Absolute axes
 */

constexpr uint16_t ABS_X = 0x00;
constexpr uint16_t ABS_Y = 0x01;
constexpr uint16_t ABS_Z = 0x02;
constexpr uint16_t ABS_RX = 0x03;
constexpr uint16_t ABS_RY = 0x04;
constexpr uint16_t ABS_RZ = 0x05;
constexpr uint16_t ABS_THROTTLE = 0x06;
constexpr uint16_t ABS_RUDDER = 0x07;
constexpr uint16_t ABS_WHEEL = 0x08;
constexpr uint16_t ABS_GAS = 0x09;
constexpr uint16_t ABS_BRAKE = 0x0a;
constexpr uint16_t ABS_HAT0X = 0x10;
constexpr uint16_t ABS_HAT0Y = 0x11;
constexpr uint16_t ABS_HAT1X = 0x12;
constexpr uint16_t ABS_HAT1Y = 0x13;
constexpr uint16_t ABS_HAT2X = 0x14;
constexpr uint16_t ABS_HAT2Y = 0x15;
constexpr uint16_t ABS_HAT3X = 0x16;
constexpr uint16_t ABS_HAT3Y = 0x17;
constexpr uint16_t ABS_PRESSURE = 0x18;
constexpr uint16_t ABS_DISTANCE = 0x19;
constexpr uint16_t ABS_TILT_X = 0x1a;
constexpr uint16_t ABS_TILT_Y = 0x1b;
constexpr uint16_t ABS_TOOL_WIDTH = 0x1c;

constexpr uint16_t ABS_VOLUME = 0x20;
constexpr uint16_t ABS_PROFILE = 0x21;

constexpr uint16_t ABS_MISC = 0x28;

/*
 * 0x2e is reserved and should not be used in input drivers.
 * It was used by HID as ABS_MISC+6 and userspace needs to detect if
 * the next ABS_* event is correct or is just ABS_MISC + n.
 * We define here ABS_RESERVED so userspace can rely on it and detect
 * the situation described above.
 */
constexpr uint16_t ABS_RESERVED = 0x2e;

constexpr uint16_t ABS_MT_SLOT = 0x2f;        /* MT slot being modified */
constexpr uint16_t ABS_MT_TOUCH_MAJOR = 0x30; /* Major axis of touching ellipse */
constexpr uint16_t ABS_MT_TOUCH_MINOR = 0x31; /* Minor axis (omit if circular) */
constexpr uint16_t ABS_MT_WIDTH_MAJOR = 0x32; /* Major axis of approaching ellipse */
constexpr uint16_t ABS_MT_WIDTH_MINOR = 0x33; /* Minor axis (omit if circular) */
constexpr uint16_t ABS_MT_ORIENTATION = 0x34; /* Ellipse orientation */
constexpr uint16_t ABS_MT_POSITION_X = 0x35;  /* Center X touch position */
constexpr uint16_t ABS_MT_POSITION_Y = 0x36;  /* Center Y touch position */
constexpr uint16_t ABS_MT_TOOL_TYPE = 0x37;   /* Type of touching device */
constexpr uint16_t ABS_MT_BLOB_ID = 0x38;     /* Group a set of packets as a blob */
constexpr uint16_t ABS_MT_TRACKING_ID = 0x39; /* Unique ID of initiated contact */
constexpr uint16_t ABS_MT_PRESSURE = 0x3a;    /* Pressure on contact area */
constexpr uint16_t ABS_MT_DISTANCE = 0x3b;    /* Contact hover distance */
constexpr uint16_t ABS_MT_TOOL_X = 0x3c;      /* Center X tool position */
constexpr uint16_t ABS_MT_TOOL_Y = 0x3d;      /* Center Y tool position */

constexpr uint16_t ABS_MAX = 0x3f;
constexpr uint16_t ABS_CNT = (ABS_MAX + 1);

/*
 * Switch events
 */

constexpr uint16_t SW_LID = 0x00;              /* set = lid shut */
constexpr uint16_t SW_TABLET_MODE = 0x01;      /* set = tablet mode */
constexpr uint16_t SW_HEADPHONE_INSERT = 0x02; /* set = inserted */
constexpr uint16_t SW_RFKILL_ALL = 0x03; /* rfkill master switch, type "any" set = radio enabled */
constexpr uint16_t SW_RADIO = SW_RFKILL_ALL;       /* deprecated */
constexpr uint16_t SW_MICROPHONE_INSERT = 0x04;    /* set = inserted */
constexpr uint16_t SW_DOCK = 0x05;                 /* set = plugged into dock */
constexpr uint16_t SW_LINEOUT_INSERT = 0x06;       /* set = inserted */
constexpr uint16_t SW_JACK_PHYSICAL_INSERT = 0x07; /* set = mechanical switch set */
constexpr uint16_t SW_VIDEOOUT_INSERT = 0x08;      /* set = inserted */
constexpr uint16_t SW_CAMERA_LENS_COVER = 0x09;    /* set = lens covered */
constexpr uint16_t SW_KEYPAD_SLIDE = 0x0a;         /* set = keypad slide out */
constexpr uint16_t SW_FRONT_PROXIMITY = 0x0b;      /* set = front proximity sensor active */
constexpr uint16_t SW_ROTATE_LOCK = 0x0c;          /* set = rotate locked/disabled */
constexpr uint16_t SW_LINEIN_INSERT = 0x0d;        /* set = inserted */
constexpr uint16_t SW_MUTE_DEVICE = 0x0e;          /* set = device disabled */
constexpr uint16_t SW_PEN_INSERTED = 0x0f;         /* set = pen inserted */
constexpr uint16_t SW_MACHINE_COVER = 0x10;        /* set = cover closed */
constexpr uint16_t SW_MAX = 0x10;
constexpr uint16_t SW_CNT = (SW_MAX + 1);

/*
 * Misc events
 */

constexpr uint16_t MSC_SERIAL = 0x00;
constexpr uint16_t MSC_PULSELED = 0x01;
constexpr uint16_t MSC_GESTURE = 0x02;
constexpr uint16_t MSC_RAW = 0x03;
constexpr uint16_t MSC_SCAN = 0x04;
constexpr uint16_t MSC_TIMESTAMP = 0x05;
constexpr uint16_t MSC_MAX = 0x07;
constexpr uint16_t MSC_CNT = (MSC_MAX + 1);

/*
 * LEDs
 */

constexpr uint16_t LED_NUML = 0x00;
constexpr uint16_t LED_CAPSL = 0x01;
constexpr uint16_t LED_SCROLLL = 0x02;
constexpr uint16_t LED_COMPOSE = 0x03;
constexpr uint16_t LED_KANA = 0x04;
constexpr uint16_t LED_SLEEP = 0x05;
constexpr uint16_t LED_SUSPEND = 0x06;
constexpr uint16_t LED_MUTE = 0x07;
constexpr uint16_t LED_MISC = 0x08;
constexpr uint16_t LED_MAIL = 0x09;
constexpr uint16_t LED_CHARGING = 0x0a;
constexpr uint16_t LED_MAX = 0x0f;
constexpr uint16_t LED_CNT = (LED_MAX + 1);

/*
 * Autorepeat values
 */

constexpr uint16_t REP_DELAY = 0x00;
constexpr uint16_t REP_PERIOD = 0x01;
constexpr uint16_t REP_MAX = 0x01;
constexpr uint16_t REP_CNT = (REP_MAX + 1);

/*
 * Sounds
 */

constexpr uint16_t SND_CLICK = 0x00;
constexpr uint16_t SND_BELL = 0x01;
constexpr uint16_t SND_TONE = 0x02;
constexpr uint16_t SND_MAX = 0x07;
constexpr uint16_t SND_CNT = (SND_MAX + 1);

} // namespace event_def