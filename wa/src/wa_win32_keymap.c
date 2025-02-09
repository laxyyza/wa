#include "wa_win32_keymap.h"

wa_key_t
wa_win32_key_to_wa_key(WPARAM wparam)
{
    switch (wparam)
    {
        case 'Q':
            return WA_KEY_Q;
        case 'W':
            return WA_KEY_W;
        case 'E':
            return WA_KEY_E;
        case 'R':
            return WA_KEY_R;
        case 'T':
            return WA_KEY_T;
        case 'Y':
            return WA_KEY_Y;
        case 'U':
            return WA_KEY_U;
        case 'I':
            return WA_KEY_I;
        case 'O':
            return WA_KEY_O;
        case 'P':
            return WA_KEY_P;
        case 'A':
            return WA_KEY_A;
        case 'S':
            return WA_KEY_S;
        case 'D':
            return WA_KEY_D;
        case 'F':
            return WA_KEY_F;
        case 'G':
            return WA_KEY_G;
        case 'H':
            return WA_KEY_H;
        case 'J':
            return WA_KEY_J;
        case 'K':
            return WA_KEY_K;
        case 'L':
            return WA_KEY_L;
        case 'Z':
            return WA_KEY_Z;
        case 'X':
            return WA_KEY_X;
        case 'C':
            return WA_KEY_C;
        case 'V':
            return WA_KEY_V;
        case 'B':
            return WA_KEY_B;
        case 'N':
            return WA_KEY_N;
        case 'M':
            return WA_KEY_M;
        case '0':
            return WA_KEY_0;
        case '1':
            return WA_KEY_1;
        case '2':
            return WA_KEY_2;
        case '3':
            return WA_KEY_3;
        case '4':
            return WA_KEY_4;
        case '5':
            return WA_KEY_5;
        case '6':
            return WA_KEY_6;
        case '7':
            return WA_KEY_7;
        case '8':
            return WA_KEY_8;
        case '9':
            return WA_KEY_9;
        case VK_ESCAPE:
            return WA_KEY_ESC;
        case VK_TAB:
            return WA_KEY_TAB;
        case VK_CAPITAL:
            return WA_KEY_CAPS;
        case VK_LSHIFT:
            return WA_KEY_LSHIFT;
        case VK_LCONTROL:
		case VK_CONTROL:
            return WA_KEY_LCTRL;
        case VK_LMENU:
            return WA_KEY_LALT;
        case VK_SPACE:
            return WA_KEY_SPACE;
        case VK_RSHIFT:
            return WA_KEY_RSHIFT;
        case VK_RETURN:
            return WA_KEY_ENTER;
        case VK_UP:
            return WA_KEY_UP;
        case VK_RIGHT:
            return WA_KEY_RIGHT;
        case VK_DOWN:
            return WA_KEY_DOWN;
        case VK_LEFT:
            return WA_KEY_LEFT;
		case VK_BACK:
			return WA_KEY_BACKSPACE;
		case VK_DELETE:
			return WA_KEY_DEL;
		case VK_HOME:
			return WA_KEY_HOME;
		case VK_END:
			return WA_KEY_END;
        default:
            return WA_KEY_NONE;
    }
}

