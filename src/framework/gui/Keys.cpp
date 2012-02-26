/*
 *  Copyright (c) 2009-2011, NVIDIA Corporation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of NVIDIA Corporation nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "gui/Keys.hpp"
#include "base/String.hpp"
#include "base/DLLImports.hpp"

#ifdef FW_QT
#include <QtCore/qnamespace.h>
#endif

using namespace FW;

//------------------------------------------------------------------------

String FW::unicodeToKey(S32 cp)
{
    FW_ASSERT(cp >= 0 && cp <= 0x10FFFF);
    return sprintf("U+%04X", cp);
}

//------------------------------------------------------------------------

S32 FW::keyToUnicode(const String& key)
{
    const char* ptr = key.getPtr();
    S32 cp = 0;

    if (*ptr != 'U' || *++ptr != '+')
        return 0;

    while (*++ptr)
    {
        cp <<= 4;

        if (*ptr >= '0' && *ptr <= '9')
            cp += *ptr - '0';
        else if (*ptr >= 'A' && *ptr <= 'F')
            cp += *ptr - ('A' - 10);
        else
            return 0;

        if (cp > 0x10FFFF)
            return 0;
    }

    return cp;
}

//------------------------------------------------------------------------

#ifdef _MSC_VER
String FW::vkeyToKey(U32 vkey)
{
    // Translate special keys.

    switch (vkey)
    {
    case VK_CANCEL:                 return FW_KEY_CANCEL;
    case VK_BACK:                   return FW_KEY_BACKSPACE;
    case VK_TAB:                    return FW_KEY_TAB;
    case VK_CLEAR:                  return FW_KEY_CLEAR;
    case VK_RETURN:                 return FW_KEY_ENTER;
    case VK_SHIFT:                  return FW_KEY_SHIFT;
    case VK_CONTROL:                return FW_KEY_CONTROL;
    case VK_MENU:                   return FW_KEY_ALT;
    case VK_PAUSE:                  return FW_KEY_PAUSE;
    case VK_CAPITAL:                return FW_KEY_CAPS_LOCK;
    case VK_KANA:                   return FW_KEY_KANA_MODE;
    case VK_JUNJA:                  return FW_KEY_JUNJA_MODE;
    case VK_FINAL:                  return FW_KEY_FINAL_MODE;
    case VK_HANJA:                  return FW_KEY_HANJA_MODE;
    case VK_ESCAPE:                 return FW_KEY_ESCAPE;
    case VK_CONVERT:                return FW_KEY_CONVERT;
    case VK_NONCONVERT:             return FW_KEY_NONCONVERT;
    case VK_ACCEPT:                 return FW_KEY_ACCEPT;
    case VK_MODECHANGE:             return FW_KEY_MODE_CHANGE;
    case VK_SPACE:                  return FW_KEY_SPACE;
    case VK_PRIOR:                  return FW_KEY_PAGE_UP;
    case VK_NEXT:                   return FW_KEY_PAGE_DOWN;
    case VK_END:                    return FW_KEY_END;
    case VK_HOME:                   return FW_KEY_HOME;
    case VK_LEFT:                   return FW_KEY_LEFT;
    case VK_UP:                     return FW_KEY_UP;
    case VK_RIGHT:                  return FW_KEY_RIGHT;
    case VK_DOWN:                   return FW_KEY_DOWN;
    case VK_SELECT:                 return FW_KEY_SELECT;
    case VK_PRINT:                  return FW_KEY_PRINT_SCREEN;
    case VK_EXECUTE:                return FW_KEY_EXECUTE;
    case VK_SNAPSHOT:               return FW_KEY_PRINT_SCREEN;
    case VK_INSERT:                 return FW_KEY_INSERT;
    case VK_DELETE:                 return FW_KEY_DELETE;
    case VK_HELP:                   return FW_KEY_HELP;
    case VK_LWIN:                   return FW_KEY_WIN;
    case VK_RWIN:                   return FW_KEY_WIN;
    case VK_APPS:                   return FW_KEY_APPS;
    case VK_SLEEP:                  break;
    case VK_NUMPAD0:                return FW_KEY_0;
    case VK_NUMPAD1:                return FW_KEY_1;
    case VK_NUMPAD2:                return FW_KEY_2;
    case VK_NUMPAD3:                return FW_KEY_3;
    case VK_NUMPAD4:                return FW_KEY_4;
    case VK_NUMPAD5:                return FW_KEY_5;
    case VK_NUMPAD6:                return FW_KEY_6;
    case VK_NUMPAD7:                return FW_KEY_7;
    case VK_NUMPAD8:                return FW_KEY_8;
    case VK_NUMPAD9:                return FW_KEY_9;
    case VK_MULTIPLY:               return FW_KEY_ASTERISK;
    case VK_ADD:                    return FW_KEY_PLUS;
    case VK_SEPARATOR:              return FW_KEY_COMMA;
    case VK_SUBTRACT:               return FW_KEY_MINUS;
    case VK_DECIMAL:                return FW_KEY_PERIOD;
    case VK_DIVIDE:                 return FW_KEY_SLASH;
    case VK_F1:                     return FW_KEY_F1;
    case VK_F2:                     return FW_KEY_F2;
    case VK_F3:                     return FW_KEY_F3;
    case VK_F4:                     return FW_KEY_F4;
    case VK_F5:                     return FW_KEY_F5;
    case VK_F6:                     return FW_KEY_F6;
    case VK_F7:                     return FW_KEY_F7;
    case VK_F8:                     return FW_KEY_F8;
    case VK_F9:                     return FW_KEY_F9;
    case VK_F10:                    return FW_KEY_F10;
    case VK_F11:                    return FW_KEY_F11;
    case VK_F12:                    return FW_KEY_F12;
    case VK_F13:                    return FW_KEY_F13;
    case VK_F14:                    return FW_KEY_F14;
    case VK_F15:                    return FW_KEY_F15;
    case VK_F16:                    return FW_KEY_F16;
    case VK_F17:                    return FW_KEY_F17;
    case VK_F18:                    return FW_KEY_F18;
    case VK_F19:                    return FW_KEY_F19;
    case VK_F20:                    return FW_KEY_F20;
    case VK_F21:                    return FW_KEY_F21;
    case VK_F22:                    return FW_KEY_F22;
    case VK_F23:                    return FW_KEY_F23;
    case VK_F24:                    return FW_KEY_F24;
    case VK_NUMLOCK:                return FW_KEY_NUM_LOCK;
    case VK_SCROLL:                 return FW_KEY_SCROLL;
    case VK_OEM_FJ_JISHO:           break;
    case VK_OEM_FJ_MASSHOU:         break;
    case VK_OEM_FJ_TOUROKU:         break;
    case VK_OEM_FJ_LOYA:            break;
    case VK_OEM_FJ_ROYA:            break;
    case VK_BROWSER_BACK:           return FW_KEY_BROWSER_BACK;
    case VK_BROWSER_FORWARD:        return FW_KEY_BROWSER_FORWARD;
    case VK_BROWSER_REFRESH:        return FW_KEY_BROWSER_REFRESH;
    case VK_BROWSER_STOP:           return FW_KEY_BROWSER_STOP;
    case VK_BROWSER_SEARCH:         return FW_KEY_BROWSER_SEARCH;
    case VK_BROWSER_FAVORITES:      return FW_KEY_BROWSER_FAVORITES;
    case VK_BROWSER_HOME:           return FW_KEY_BROWSER_HOME;
    case VK_VOLUME_MUTE:            return FW_KEY_VOLUME_MUTE;
    case VK_VOLUME_DOWN:            return FW_KEY_VOLUME_DOWN;
    case VK_VOLUME_UP:              return FW_KEY_VOLUME_UP;
    case VK_MEDIA_NEXT_TRACK:       return FW_KEY_MEDIA_NEXT_TRACK;
    case VK_MEDIA_PREV_TRACK:       return FW_KEY_MEDIA_PREVIOUS_TRACK;
    case VK_MEDIA_STOP:             return FW_KEY_MEDIA_STOP;
    case VK_MEDIA_PLAY_PAUSE:       return FW_KEY_MEDIA_PLAY_PAUSE;
    case VK_LAUNCH_MAIL:            return FW_KEY_LAUNCH_MAIL;
    case VK_LAUNCH_MEDIA_SELECT:    return FW_KEY_SELECT_MEDIA;
    case VK_LAUNCH_APP1:            return FW_KEY_LAUNCH_APPLICATION1;
    case VK_LAUNCH_APP2:            return FW_KEY_LAUNCH_APPLICATION2;
    case VK_OEM_1:                  break;
    case VK_OEM_PLUS:               break;
    case VK_OEM_COMMA:              break;
    case VK_OEM_MINUS:              break;
    case VK_OEM_PERIOD:             break;
    case VK_OEM_2:                  break;
    case VK_OEM_3:                  break;
    case VK_OEM_4:                  break;
    case VK_OEM_5:                  break;
    case VK_OEM_6:                  break;
    case VK_OEM_7:                  break;
    case VK_OEM_8:                  break;
    case VK_OEM_AX:                 break;
    case VK_OEM_102:                break;
    case VK_ICO_HELP:               break;
    case VK_ICO_00:                 break;
    case VK_PROCESSKEY:             return FW_KEY_PROCESS;
    case VK_ICO_CLEAR:              break;
    case VK_PACKET:                 break;
    case VK_OEM_RESET:              break;
    case VK_OEM_JUMP:               break;
    case VK_OEM_PA1:                break;
    case VK_OEM_PA2:                break;
    case VK_OEM_PA3:                break;
    case VK_OEM_WSCTRL:             break;
    case VK_OEM_CUSEL:              break;
    case VK_OEM_ATTN:               break;
    case VK_OEM_FINISH:             break;
    case VK_OEM_COPY:               break;
    case VK_OEM_AUTO:               break;
    case VK_OEM_ENLW:               break;
    case VK_OEM_BACKTAB:            break;
    case VK_ATTN:                   return FW_KEY_ATTN;
    case VK_CRSEL:                  return FW_KEY_CRSEL;
    case VK_EXSEL:                  return FW_KEY_EXSEL;
    case VK_EREOF:                  return FW_KEY_ERASE_EOF;
    case VK_PLAY:                   return FW_KEY_PLAY;
    case VK_ZOOM:                   return FW_KEY_ZOOM;
    case VK_NONAME:                 break;
    case VK_PA1:                    break;
    case VK_OEM_CLEAR:              return FW_KEY_CLEAR;
    default:                        break;
    }

    // Translate to Unicode.

    S32 cp = MapVirtualKeyW(vkey, 2);

    // Translate dead keys to combining marks.

    switch (cp)
    {
    case 0x80000060: cp = 0x0300; break; // FW_KEY_DEAD_GRAVE
    case 0x800000B4: cp = 0x0301; break; // FW_KEY_DEAD_ACUTE
    case 0x8000005E: cp = 0x0302; break; // FW_KEY_DEAD_CIRCUMFLEX
    case 0x8000007E: cp = 0x0303; break; // FW_KEY_DEAD_TILDE
    case 0x800000AF: cp = 0x0304; break; // FW_KEY_DEAD_MACRON
    case 0x800002D8: cp = 0x0306; break; // FW_KEY_DEAD_BREVE
    case 0x800002D9: cp = 0x0307; break; // FW_KEY_DEAD_ABOVE_DOT
    case 0x800000A8: cp = 0x0308; break; // FW_KEY_DEAD_DIAERESIS
    case 0x800002DA: cp = 0x030A; break; // FW_KEY_DEAD_ABOVE_RING
    case 0x800002DD: cp = 0x030B; break; // FW_KEY_DEAD_DOUBLE_ACUTE
    case 0x800002C7: cp = 0x030C; break; // FW_KEY_DEAD_CARON
    case 0x800000B8: cp = 0x0327; break; // FW_KEY_DEAD_CEDILLA
    case 0x800002DB: cp = 0x0328; break; // FW_KEY_DEAD_OGONEK
    case 0x8000037A: cp = 0x0345; break; // FW_KEY_DEAD_IOTA
    case 0x8000309B: cp = 0x3099; break; // FW_KEY_DEAD_VOICED
    case 0x8000309C: cp = 0x309A; break; // FW_KEY_DEAD_SEMIVOICED
    default: break;
    }

    // Convert to string.

    return (cp) ? unicodeToKey(cp) : "";
}
#endif

#ifdef FW_QT
String FW::qtkeyToKey(int key, QChar chr)
{
    switch (key) {
    case Qt::Key_Escape: return FW_KEY_ESCAPE;
    case Qt::Key_Tab: return FW_KEY_TAB;
    case Qt::Key_Backtab: break;
    case Qt::Key_Backspace: return FW_KEY_BACKSPACE;
    case Qt::Key_Return: break;
    case Qt::Key_Enter: return FW_KEY_ENTER;
    case Qt::Key_Insert: return FW_KEY_INSERT;
    case Qt::Key_Delete: return FW_KEY_DELETE;
    case Qt::Key_Pause: return FW_KEY_PAUSE;
    case Qt::Key_Print: return FW_KEY_PRINT_SCREEN;
    case Qt::Key_SysReq: break;
    case Qt::Key_Clear: return FW_KEY_CLEAR;
    case Qt::Key_Home: return FW_KEY_HOME;
    case Qt::Key_End: return FW_KEY_END;
    case Qt::Key_Left: return FW_KEY_LEFT;
    case Qt::Key_Up: return FW_KEY_UP;
    case Qt::Key_Right: return FW_KEY_RIGHT;
    case Qt::Key_Down: return FW_KEY_DOWN;
    case Qt::Key_PageUp: return FW_KEY_PAGE_UP;
    case Qt::Key_PageDown: return FW_KEY_PAGE_DOWN;
    case Qt::Key_Control: return FW_KEY_CONTROL;
    case Qt::Key_Meta: return FW_KEY_META;
    case Qt::Key_Alt: return FW_KEY_ALT;
    case Qt::Key_CapsLock: return FW_KEY_CAPS_LOCK;
    case Qt::Key_NumLock: return FW_KEY_NUM_LOCK;
    case Qt::Key_ScrollLock: break;
    case Qt::Key_F1: return FW_KEY_F1;
    case Qt::Key_F2: return FW_KEY_F2;
    case Qt::Key_F3: return FW_KEY_F3;
    case Qt::Key_F4: return FW_KEY_F4;
    case Qt::Key_F5: return FW_KEY_F5;
    case Qt::Key_F6: return FW_KEY_F6;
    case Qt::Key_F7: return FW_KEY_F7;
    case Qt::Key_F8: return FW_KEY_F8;
    case Qt::Key_F9: return FW_KEY_F9;
    case Qt::Key_F10: return FW_KEY_F10;
    case Qt::Key_F11: return FW_KEY_F11;
    case Qt::Key_F12: return FW_KEY_F12;
    case Qt::Key_F13: return FW_KEY_F13;
    case Qt::Key_F14: return FW_KEY_F14;
    case Qt::Key_F15: return FW_KEY_F15;
    case Qt::Key_F16: return FW_KEY_F16;
    case Qt::Key_F17: return FW_KEY_F17;
    case Qt::Key_F18: return FW_KEY_F18;
    case Qt::Key_F19: return FW_KEY_F19;
    case Qt::Key_F20: return FW_KEY_F20;
    case Qt::Key_F21: return FW_KEY_F21;
    case Qt::Key_F22: return FW_KEY_F22;
    case Qt::Key_F23: return FW_KEY_F23;
    case Qt::Key_F24: return FW_KEY_F24;
    case Qt::Key_F25: break;
    case Qt::Key_F26: break;
    case Qt::Key_F27: break;
    case Qt::Key_F28: break;
    case Qt::Key_F29: break;
    case Qt::Key_F30: break;
    case Qt::Key_F31: break;
    case Qt::Key_F32: break;
    case Qt::Key_F33: break;
    case Qt::Key_F34: break;
    case Qt::Key_F35: break;
    case Qt::Key_Super_L: break;
    case Qt::Key_Super_R: break;
    case Qt::Key_Menu: break;
    case Qt::Key_Hyper_L: break;
    case Qt::Key_Hyper_R: break;
    case Qt::Key_Help: return FW_KEY_HELP;
    case Qt::Key_Direction_L: break;
    case Qt::Key_Direction_R: break;
    case Qt::Key_Space: return FW_KEY_SPACE;
    case Qt::Key_Exclam: return FW_KEY_EXCLAMATION;
    case Qt::Key_QuoteDbl: return FW_KEY_DOUBLE_QUOTE;
    case Qt::Key_NumberSign: return FW_KEY_HASH;
    case Qt::Key_Dollar: return FW_KEY_DOLLAR;
    case Qt::Key_Percent: break;
    case Qt::Key_Ampersand: return FW_KEY_AMPERSAND;
    case Qt::Key_Apostrophe: return FW_KEY_SINGLE_QUOTE;
    case Qt::Key_ParenLeft: return FW_KEY_LEFT_PARENTHESIS;
    case Qt::Key_ParenRight: return FW_KEY_RIGHT_PARENTHESIS;
    case Qt::Key_Asterisk: return FW_KEY_ASTERISK;
    case Qt::Key_Plus: return FW_KEY_PLUS;
    case Qt::Key_Comma: return FW_KEY_COMMA;
    case Qt::Key_Minus: return FW_KEY_MINUS;
    case Qt::Key_Period: return FW_KEY_PERIOD;
    case Qt::Key_Slash: return FW_KEY_SLASH;
    case Qt::Key_0: return FW_KEY_0;
    case Qt::Key_1: return FW_KEY_1;
    case Qt::Key_2: return FW_KEY_2;
    case Qt::Key_3: return FW_KEY_3;
    case Qt::Key_4: return FW_KEY_4;
    case Qt::Key_5: return FW_KEY_5;
    case Qt::Key_6: return FW_KEY_6;
    case Qt::Key_7: return FW_KEY_7;
    case Qt::Key_8: return FW_KEY_8;
    case Qt::Key_9: return FW_KEY_9;
    case Qt::Key_Colon: return FW_KEY_COLON;
    case Qt::Key_Semicolon: return FW_KEY_SEMICOLON;
    case Qt::Key_Less: return FW_KEY_LESS_THAN;
    case Qt::Key_Equal: return FW_KEY_EQUALS;
    case Qt::Key_Greater: return FW_KEY_GREATER_THAN;
    case Qt::Key_Question: return FW_KEY_QUESTION;
    case Qt::Key_At: return FW_KEY_AT;
    case Qt::Key_A: return FW_KEY_A;
    case Qt::Key_B: return FW_KEY_B;
    case Qt::Key_C: return FW_KEY_C;
    case Qt::Key_D: return FW_KEY_D;
    case Qt::Key_E: return FW_KEY_E;
    case Qt::Key_F: return FW_KEY_F;
    case Qt::Key_G: return FW_KEY_G;
    case Qt::Key_H: return FW_KEY_H;
    case Qt::Key_I: return FW_KEY_I;
    case Qt::Key_J: return FW_KEY_J;
    case Qt::Key_K: return FW_KEY_K;
    case Qt::Key_L: return FW_KEY_L;
    case Qt::Key_M: return FW_KEY_M;
    case Qt::Key_N: return FW_KEY_N;
    case Qt::Key_O: return FW_KEY_O;
    case Qt::Key_P: return FW_KEY_P;
    case Qt::Key_Q: return FW_KEY_Q;
    case Qt::Key_R: return FW_KEY_R;
    case Qt::Key_S: return FW_KEY_S;
    case Qt::Key_T: return FW_KEY_T;
    case Qt::Key_U: return FW_KEY_U;
    case Qt::Key_V: return FW_KEY_V;
    case Qt::Key_W: return FW_KEY_W;
    case Qt::Key_X: return FW_KEY_X;
    case Qt::Key_Y: return FW_KEY_Y;
    case Qt::Key_Z: return FW_KEY_Z;
    case Qt::Key_BracketLeft: return FW_KEY_LEFT_SQUARE_BRACKET;
    case Qt::Key_Backslash: return FW_KEY_BACKSLASH;
    case Qt::Key_BracketRight: return FW_KEY_RIGHT_SQUARE_BRACKET;
    case Qt::Key_AsciiCircum: return FW_KEY_CIRCUMFLEX;
    case Qt::Key_Underscore: return FW_KEY_UNDERSCORE;
    case Qt::Key_QuoteLeft: return FW_KEY_BACK_QUOTE;
    case Qt::Key_BraceLeft: return FW_KEY_LEFT_CURLY_BRACKET;
    case Qt::Key_Bar: return FW_KEY_VERTICAL_BAR;
    case Qt::Key_BraceRight: return FW_KEY_RIGHT_CURLY_BRACKET;
    case Qt::Key_AsciiTilde: break;
    case Qt::Key_nobreakspace: break;
    case Qt::Key_exclamdown: return FW_KEY_INVERTED_EXCLAMATION;
    case Qt::Key_cent: break;
    case Qt::Key_sterling: break;
    case Qt::Key_currency: break;
    case Qt::Key_yen: break;
    case Qt::Key_brokenbar: break;
    case Qt::Key_section: break;
    case Qt::Key_diaeresis: break;
    case Qt::Key_copyright: break;
    case Qt::Key_ordfeminine: break;
    case Qt::Key_guillemotleft: break;
    case Qt::Key_notsign: break;
    case Qt::Key_hyphen: break;
    case Qt::Key_registered: break;
    case Qt::Key_macron: break;
    case Qt::Key_degree: break;
    case Qt::Key_plusminus: break;
    case Qt::Key_twosuperior: break;
    case Qt::Key_threesuperior: break;
    case Qt::Key_acute: break;
    case Qt::Key_mu: break;
    case Qt::Key_paragraph: break;
    case Qt::Key_periodcentered: break;
    case Qt::Key_cedilla: break;
    case Qt::Key_onesuperior: break;
    case Qt::Key_masculine: break;
    case Qt::Key_guillemotright: break;
    case Qt::Key_onequarter: break;
    case Qt::Key_onehalf: break;
    case Qt::Key_threequarters: break;
    case Qt::Key_questiondown: break;
    case Qt::Key_Agrave: break;
    case Qt::Key_Aacute: break;
    case Qt::Key_Acircumflex: break;
    case Qt::Key_Atilde: break;
    case Qt::Key_Adiaeresis: break;
    case Qt::Key_Aring: break;
    case Qt::Key_AE: break;
    case Qt::Key_Ccedilla: break;
    case Qt::Key_Egrave: break;
    case Qt::Key_Eacute: break;
    case Qt::Key_Ecircumflex: break;
    case Qt::Key_Ediaeresis: break;
    case Qt::Key_Igrave: break;
    case Qt::Key_Iacute: break;
    case Qt::Key_Icircumflex: break;
    case Qt::Key_Idiaeresis: break;
    case Qt::Key_ETH: break;
    case Qt::Key_Ntilde: break;
    case Qt::Key_Ograve: break;
    case Qt::Key_Oacute: break;
    case Qt::Key_Ocircumflex: break;
    case Qt::Key_Otilde: break;
    case Qt::Key_Odiaeresis: break;
    case Qt::Key_multiply: break;
    case Qt::Key_Ooblique: break;
    case Qt::Key_Ugrave: break;
    case Qt::Key_Uacute: break;
    case Qt::Key_Ucircumflex: break;
    case Qt::Key_Udiaeresis: break;
    case Qt::Key_Yacute: break;
    case Qt::Key_THORN: break;
    case Qt::Key_ssharp: break;
    case Qt::Key_division: break;
    case Qt::Key_ydiaeresis: break;
    case Qt::Key_AltGr: break;
    case Qt::Key_Multi_key: break;
    case Qt::Key_Codeinput: return FW_KEY_CODE_INPUT;
    case Qt::Key_SingleCandidate: break;
    case Qt::Key_MultipleCandidate: break;
    case Qt::Key_PreviousCandidate: return FW_KEY_PREVIOUS_CANDIDATE;
    case Qt::Key_Mode_switch: return FW_KEY_MODE_CHANGE;
    case Qt::Key_Kanji: return FW_KEY_KANJI_MODE;
    case Qt::Key_Muhenkan: break;
    case Qt::Key_Henkan: break;
    case Qt::Key_Romaji: return FW_KEY_JAPANESE_ROMAJI;
    case Qt::Key_Hiragana: return FW_KEY_HIRAGANA;
    case Qt::Key_Katakana: return FW_KEY_KATAKANA;
    case Qt::Key_Hiragana_Katakana: break;
    case Qt::Key_Zenkaku: break;
    case Qt::Key_Hankaku: break;
    case Qt::Key_Zenkaku_Hankaku: break;
    case Qt::Key_Touroku: break;
    case Qt::Key_Massyo: break;
    case Qt::Key_Kana_Lock: return FW_KEY_KANA_MODE;
    case Qt::Key_Kana_Shift: break;
    case Qt::Key_Eisu_Shift: break;
    case Qt::Key_Eisu_toggle: break;
    case Qt::Key_Hangul: return FW_KEY_HANGUL_MODE;
    case Qt::Key_Hangul_Start: break;
    case Qt::Key_Hangul_End: break;
    case Qt::Key_Hangul_Hanja: return FW_KEY_HANJA_MODE;
    case Qt::Key_Hangul_Jamo: break;
    case Qt::Key_Hangul_Romaja: break;
    case Qt::Key_Hangul_Jeonja: break;
    case Qt::Key_Hangul_Banja: break;
    case Qt::Key_Hangul_PreHanja: break;
    case Qt::Key_Hangul_PostHanja: break;
    case Qt::Key_Hangul_Special: break;
    case Qt::Key_Dead_Grave: return FW_KEY_DEAD_GRAVE;
    case Qt::Key_Dead_Acute: return FW_KEY_DEAD_ACUTE;
    case Qt::Key_Dead_Circumflex: return FW_KEY_DEAD_CIRCUMFLEX;
    case Qt::Key_Dead_Tilde: return FW_KEY_DEAD_TILDE;
    case Qt::Key_Dead_Macron: return FW_KEY_DEAD_MACRON;
    case Qt::Key_Dead_Breve: return FW_KEY_DEAD_BREVE;
    case Qt::Key_Dead_Abovedot: return FW_KEY_DEAD_ABOVE_DOT;
    case Qt::Key_Dead_Diaeresis: return FW_KEY_DEAD_DIAERESIS;
    case Qt::Key_Dead_Abovering: return FW_KEY_DEAD_ABOVE_RING;
    case Qt::Key_Dead_Doubleacute: return FW_KEY_DEAD_DOUBLE_ACUTE;
    case Qt::Key_Dead_Caron: return FW_KEY_DEAD_CARON;
    case Qt::Key_Dead_Cedilla: return FW_KEY_DEAD_CEDILLA;
    case Qt::Key_Dead_Ogonek: return FW_KEY_DEAD_OGONEK;
    case Qt::Key_Dead_Iota: return FW_KEY_DEAD_IOTA;
    case Qt::Key_Dead_Voiced_Sound: return FW_KEY_DEAD_VOICED;
    case Qt::Key_Dead_Semivoiced_Sound: return FW_KEY_DEAD_SEMIVOICED;
    case Qt::Key_Dead_Belowdot: break;
    case Qt::Key_Dead_Hook: break;
    case Qt::Key_Dead_Horn: break;
    case Qt::Key_Back: return FW_KEY_BROWSER_BACK;
    case Qt::Key_Forward: return FW_KEY_BROWSER_FORWARD;
    case Qt::Key_Stop: return FW_KEY_BROWSER_STOP;
    case Qt::Key_Refresh: return FW_KEY_BROWSER_REFRESH;
    case Qt::Key_VolumeDown: return FW_KEY_VOLUME_DOWN;
    case Qt::Key_VolumeMute: return FW_KEY_VOLUME_MUTE;
    case Qt::Key_VolumeUp: return FW_KEY_VOLUME_UP;
    case Qt::Key_BassBoost: break;
    case Qt::Key_BassUp: break;
    case Qt::Key_BassDown: break;
    case Qt::Key_TrebleUp: break;
    case Qt::Key_TrebleDown: break;
    case Qt::Key_MediaPlay: break;
    case Qt::Key_MediaStop: return FW_KEY_MEDIA_STOP;
    case Qt::Key_MediaPrevious: return FW_KEY_MEDIA_PREVIOUS_TRACK;
    case Qt::Key_MediaNext: return FW_KEY_MEDIA_NEXT_TRACK;
    case Qt::Key_MediaRecord: break;
    case Qt::Key_MediaPause: break;
    case Qt::Key_MediaTogglePlayPause: return FW_KEY_MEDIA_PLAY_PAUSE;
    case Qt::Key_HomePage: return FW_KEY_BROWSER_HOME;
    case Qt::Key_Favorites: return FW_KEY_BROWSER_FAVORITES;
    case Qt::Key_Search: return FW_KEY_BROWSER_SEARCH;
    case Qt::Key_Standby: break;
    case Qt::Key_OpenUrl: break;
    case Qt::Key_LaunchMail: return FW_KEY_LAUNCH_MAIL;
    case Qt::Key_LaunchMedia: break;
    case Qt::Key_Launch0: break;
    case Qt::Key_Launch1: return FW_KEY_LAUNCH_APPLICATION1;
    case Qt::Key_Launch2: return FW_KEY_LAUNCH_APPLICATION2;
    case Qt::Key_Launch3: break;
    case Qt::Key_Launch4: break;
    case Qt::Key_Launch5: break;
    case Qt::Key_Launch6: break;
    case Qt::Key_Launch7: break;
    case Qt::Key_Launch8: break;
    case Qt::Key_Launch9: break;
    case Qt::Key_LaunchA: break;
    case Qt::Key_LaunchB: break;
    case Qt::Key_LaunchC: break;
    case Qt::Key_LaunchD: break;
    case Qt::Key_LaunchE: break;
    case Qt::Key_LaunchF: break;
    case Qt::Key_MonBrightnessUp: break;
    case Qt::Key_MonBrightnessDown: break;
    case Qt::Key_KeyboardLightOnOff: break;
    case Qt::Key_KeyboardBrightnessUp: break;
    case Qt::Key_KeyboardBrightnessDown: break;
    case Qt::Key_PowerOff: break;
    case Qt::Key_WakeUp: break;
    case Qt::Key_Eject: break;
    case Qt::Key_ScreenSaver: break;
    case Qt::Key_WWW: break;
    case Qt::Key_Memo: break;
    case Qt::Key_LightBulb: break;
    case Qt::Key_Shop: break;
    case Qt::Key_History: break;
    case Qt::Key_AddFavorite: break;
    case Qt::Key_HotLinks: break;
    case Qt::Key_BrightnessAdjust: break;
    case Qt::Key_Finance: break;
    case Qt::Key_Community: break;
    case Qt::Key_AudioRewind: break;
    case Qt::Key_BackForward: break;
    case Qt::Key_ApplicationLeft: break;
    case Qt::Key_ApplicationRight: break;
    case Qt::Key_Book: break;
    case Qt::Key_CD: break;
    case Qt::Key_Calculator: break;
    case Qt::Key_ToDoList: break;
    case Qt::Key_ClearGrab: break;
    case Qt::Key_Close: break;
    case Qt::Key_Copy: return FW_KEY_COPY;
    case Qt::Key_Cut: return FW_KEY_CUT;
    case Qt::Key_Display: break;
    case Qt::Key_DOS: break;
    case Qt::Key_Documents: break;
    case Qt::Key_Excel: break;
    case Qt::Key_Explorer: break;
    case Qt::Key_Game: break;
    case Qt::Key_Go: break;
    case Qt::Key_iTouch: break;
    case Qt::Key_LogOff: break;
    case Qt::Key_Market: break;
    case Qt::Key_Meeting: break;
    case Qt::Key_MenuKB: break;
    case Qt::Key_MenuPB: break;
    case Qt::Key_MySites: break;
    case Qt::Key_News: break;
    case Qt::Key_OfficeHome: break;
    case Qt::Key_Option: break;
    case Qt::Key_Paste: return FW_KEY_PASTE;
    case Qt::Key_Phone: break;
    case Qt::Key_Calendar: break;
    case Qt::Key_Reply: break;
    case Qt::Key_Reload: break;
    case Qt::Key_RotateWindows: break;
    case Qt::Key_RotationPB: break;
    case Qt::Key_RotationKB: break;
    case Qt::Key_Save: break;
    case Qt::Key_Send: break;
    case Qt::Key_Spell: break;
    case Qt::Key_SplitScreen: break;
    case Qt::Key_Support: break;
    case Qt::Key_TaskPane: break;
    case Qt::Key_Terminal: break;
    case Qt::Key_Tools: break;
    case Qt::Key_Travel: break;
    case Qt::Key_Video: break;
    case Qt::Key_Word: break;
    case Qt::Key_Xfer: break;
    case Qt::Key_ZoomIn: break;
    case Qt::Key_ZoomOut: break;
    case Qt::Key_Away: break;
    case Qt::Key_Messenger: break;
    case Qt::Key_WebCam: break;
    case Qt::Key_MailForward: break;
    case Qt::Key_Pictures: break;
    case Qt::Key_Music: break;
    case Qt::Key_Battery: break;
    case Qt::Key_Bluetooth: break;
    case Qt::Key_WLAN: break;
    case Qt::Key_UWB: break;
    case Qt::Key_AudioForward: break;
    case Qt::Key_AudioRepeat: break;
    case Qt::Key_AudioRandomPlay: break;
    case Qt::Key_Subtitle: break;
    case Qt::Key_AudioCycleTrack: break;
    case Qt::Key_Time: break;
    case Qt::Key_Hibernate: break;
    case Qt::Key_View: break;
    case Qt::Key_TopMenu: break;
    case Qt::Key_PowerDown: break;
    case Qt::Key_Suspend: break;
    case Qt::Key_ContrastAdjust: break;
    case Qt::Key_LaunchG: break;
    case Qt::Key_LaunchH: break;
    case Qt::Key_MediaLast: break;
    case Qt::Key_Select: return FW_KEY_SELECT;
    case Qt::Key_Yes: break;
    case Qt::Key_No: break;
    case Qt::Key_Cancel: return FW_KEY_CANCEL;
    case Qt::Key_Printer: break;
    case Qt::Key_Execute: return FW_KEY_EXECUTE;
    case Qt::Key_Sleep: break;
    case Qt::Key_Play: return FW_KEY_PLAY;
    case Qt::Key_Zoom: return FW_KEY_ZOOM;
    case Qt::Key_Context1: break;
    case Qt::Key_Context2: break;
    case Qt::Key_Context3: break;
    case Qt::Key_Context4: break;
    case Qt::Key_Call: break;
    case Qt::Key_Hangup: break;
    case Qt::Key_Flip: break;
    case Qt::Key_ToggleCallHangup: break;
    case Qt::Key_VoiceDial: break;
    case Qt::Key_LastNumberRedial: break;
    case Qt::Key_Camera: break;
    case Qt::Key_CameraFocus: break;
    default: break;
    }

    // FW_KEY_ACCEPT
    // FW_KEY_AGAIN
    // FW_KEY_ALL_CANDIDATES
    // FW_KEY_ALPHANUMERIC
    // FW_KEY_ALT_GRAPH
    // FW_KEY_APPS
    // FW_KEY_ATTN
    // FW_KEY_COMPOSE
    // FW_KEY_CRSEL
    // FW_KEY_CONVERT
    // FW_KEY_ERASE_EOF
    // FW_KEY_EXSEL
    // FW_KEY_FINAL_MODE
    // FW_KEY_FIND
    // FW_KEY_FULL_WIDTH
    // FW_KEY_HALF_WIDTH
    // FW_KEY_JAPANESE_HIRAGANA
    // FW_KEY_JAPANESE_KATAKANA
    // FW_KEY_JUNJA_MODE
    // FW_KEY_NONCONVERT
    // FW_KEY_PROCESS
    // FW_KEY_PROPS
    // FW_KEY_ROMAN_CHARACTERS
    // FW_KEY_SCROLL
    // FW_KEY_SELECT_MEDIA
    // FW_KEY_SHIFT
    // FW_KEY_STOP
    // FW_KEY_UNDO
    // FW_KEY_WIN
    // FW_KEY_EURO


    return unicodeToKey(chr.unicode());
}
#endif

//------------------------------------------------------------------------
