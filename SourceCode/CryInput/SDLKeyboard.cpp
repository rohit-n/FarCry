#include "StdAfx.h"
#include "Input.h"
#include <ILog.h>
#include <stdio.h>
#include "SDLKeyboard.h"

#include <ISystem.h>
#include <IConsole.h>
#ifdef _WIN32
#include <SDL.h>
#define STUBMSG(X) OutputDebugString(X)
#else
#include <SDL2/SDL.h>
#define STUBMSG(X) printf("%s", X)
#endif
#include "Input.h"

CSDLKeyboard::CSDLKeyboard()
{
	m_bExclusiveMode = false;
	m_pSystem = NULL;
	m_modifiers = 0;
}

CSDLKeyboard::~CSDLKeyboard()
{

}

bool CSDLKeyboard::Init(CInput* pInput, ISystem* pSystem)
{
	m_pSystem = pSystem;
	m_pLog = pSystem->GetILog();
	m_pInput = pInput;

	m_pLog->LogToFile("Initializing SDL Keyboard\n");

	memset(m_cTempKeys, 0, 256);
	memset(m_cKeysState, 0, sizeof(m_cKeysState));
	memset(m_cOldKeysState, 0, sizeof(m_cOldKeysState));

	return true;
}

void CSDLKeyboard::SetExclusive(bool value, void* hwnd)
{
	STUBMSG("SetExclusive STUB\n");
}

bool CSDLKeyboard::Acquire()
{
	STUBMSG("Acquire STUB\n");
	return true;
}

bool CSDLKeyboard::UnAcquire()
{
	STUBMSG("UnAcquire STUB\n");
	return true;
}

unsigned short CSDLKeyboard::SDL2XKEY(SDL_Keycode kc)
{
	switch (kc)
	{
	case SDLK_ESCAPE:        return XKEY_ESCAPE;
	case SDLK_1:             return XKEY_1;
	case SDLK_2:             return XKEY_2;
	case SDLK_3:             return XKEY_3;
	case SDLK_4:             return XKEY_4;
	case SDLK_5:             return XKEY_5;
	case SDLK_6:             return XKEY_6;
	case SDLK_7:             return XKEY_7;
	case SDLK_8:             return XKEY_8;
	case SDLK_9:             return XKEY_9;
	case SDLK_0:             return XKEY_0;
	case SDLK_MINUS:         return XKEY_MINUS;
	case SDLK_EQUALS:        return XKEY_EQUALS;
	case SDLK_BACKSPACE:          return XKEY_BACKSPACE;
	case SDLK_TAB:           return XKEY_TAB;
	case SDLK_q:             return XKEY_Q;
	case SDLK_w:             return XKEY_W;
	case SDLK_e:             return XKEY_E;
	case SDLK_r:             return XKEY_R;
	case SDLK_t:             return XKEY_T;
	case SDLK_y:             return XKEY_Y;
	case SDLK_u:             return XKEY_U;
	case SDLK_i:             return XKEY_I;
	case SDLK_o:             return XKEY_O;
	case SDLK_p:             return XKEY_P;
	case SDLK_LEFTBRACKET:      return XKEY_LBRACKET;
	case SDLK_RIGHTBRACKET:      return XKEY_RBRACKET;
	case SDLK_RETURN:        return XKEY_RETURN;
	case SDLK_LCTRL:      return XKEY_LCONTROL;
	case SDLK_a:             return XKEY_A;
	case SDLK_s:             return XKEY_S;
	case SDLK_d:             return XKEY_D;
	case SDLK_f:             return XKEY_F;
	case SDLK_g:             return XKEY_G;
	case SDLK_h:             return XKEY_H;
	case SDLK_j:             return XKEY_J;
	case SDLK_k:             return XKEY_K;
	case SDLK_l:             return XKEY_L;
	case SDLK_SEMICOLON:     return XKEY_SEMICOLON;
	case SDLK_QUOTE:    return XKEY_APOSTROPHE;
	case SDLK_BACKQUOTE:         return XKEY_TILDE;
	case SDLK_LSHIFT:        return XKEY_LSHIFT;
	case SDLK_BACKSLASH:     return XKEY_BACKSLASH;
	case SDLK_z:             return XKEY_Z;
	case SDLK_x:             return XKEY_X;
	case SDLK_c:             return XKEY_C;
	case SDLK_v:             return XKEY_V;
	case SDLK_b:             return XKEY_B;
	case SDLK_n:             return XKEY_N;
	case SDLK_m:             return XKEY_M;
	case SDLK_COMMA:         return XKEY_COMMA;
	case SDLK_PERIOD:        return XKEY_PERIOD;
	case SDLK_SLASH:         return XKEY_SLASH;
	case SDLK_RSHIFT:        return XKEY_RSHIFT;
	case SDLK_ASTERISK:      return XKEY_MULTIPLY;
	case SDLK_LALT:          return XKEY_LALT;
	case SDLK_SPACE:         return XKEY_SPACE;
	case SDLK_CAPSLOCK:      return XKEY_CAPSLOCK;
	case SDLK_F1:            return XKEY_F1;
	case SDLK_F2:            return XKEY_F2;
	case SDLK_F3:            return XKEY_F3;
	case SDLK_F4:            return XKEY_F4;
	case SDLK_F5:            return XKEY_F5;
	case SDLK_F6:            return XKEY_F6;
	case SDLK_F7:            return XKEY_F7;
	case SDLK_F8:            return XKEY_F8;
	case SDLK_F9:            return XKEY_F9;
	case SDLK_F10:           return XKEY_F10;
	case SDLK_NUMLOCKCLEAR:       return XKEY_NUMLOCK;
	case SDLK_SCROLLLOCK:        return XKEY_SCROLLLOCK;

	case SDLK_PLUS:           return XKEY_ADD;
	case SDLK_DECIMALSEPARATOR:       return XKEY_DECIMAL;
	case SDLK_F11:           return XKEY_F11;
	case SDLK_F12:           return XKEY_F12;
	case SDLK_F13:           return XKEY_F13;
	case SDLK_F14:           return XKEY_F14;
	case SDLK_F15:           return XKEY_F15;

	case SDLK_RCTRL:         return XKEY_RCONTROL;
	case SDLK_SYSREQ:        return XKEY_PRINT;
	case SDLK_RALT:          return XKEY_RALT;
	case SDLK_PAUSE:         return XKEY_PAUSE;
	case SDLK_HOME:          return XKEY_HOME;
	case SDLK_UP:            return XKEY_UP;
	case SDLK_PAGEUP:        return XKEY_PAGE_UP;
	case SDLK_LEFT:          return XKEY_LEFT;
	case SDLK_RIGHT:         return XKEY_RIGHT;
	case SDLK_END:           return XKEY_END;
	case SDLK_DOWN:          return XKEY_DOWN;
	case SDLK_PAGEDOWN:      return XKEY_PAGE_DOWN;
	case SDLK_INSERT:        return XKEY_INSERT;
	case SDLK_DELETE:        return XKEY_DELETE;
	}

	return XKEY_NULL;
}

//////////////////////////////////////////////////////////////////////////
unsigned char CSDLKeyboard::XKEY2ASCII(unsigned short nCode, int modifiers)
{
	char ret = '\0';
#define HANDLE_CASE(XKEY, C) case XKEY: \
	ret = C; \
	break;

	switch (nCode)
	{
		HANDLE_CASE(XKEY_Q, 'q')
		HANDLE_CASE(XKEY_W, 'w')
		HANDLE_CASE(XKEY_E, 'e')
		HANDLE_CASE(XKEY_R, 'r')
		HANDLE_CASE(XKEY_T, 't')
		HANDLE_CASE(XKEY_Y, 'y')
		HANDLE_CASE(XKEY_U, 'u')
		HANDLE_CASE(XKEY_I, 'i')
		HANDLE_CASE(XKEY_O, 'o')
		HANDLE_CASE(XKEY_P, 'p')
		HANDLE_CASE(XKEY_A, 'a')
		HANDLE_CASE(XKEY_S, 's')
		HANDLE_CASE(XKEY_D, 'd')
		HANDLE_CASE(XKEY_F, 'f')
		HANDLE_CASE(XKEY_G, 'g')
		HANDLE_CASE(XKEY_H, 'h')
		HANDLE_CASE(XKEY_J, 'j')
		HANDLE_CASE(XKEY_K, 'k')
		HANDLE_CASE(XKEY_L, 'l')
		HANDLE_CASE(XKEY_Z, 'z')
		HANDLE_CASE(XKEY_X, 'x')
		HANDLE_CASE(XKEY_C, 'c')
		HANDLE_CASE(XKEY_V, 'v')
		HANDLE_CASE(XKEY_B, 'b')
		HANDLE_CASE(XKEY_N, 'n')
		HANDLE_CASE(XKEY_M, 'm')
	default:
		ret = '\0';
	}
#undef HANDLE_CASE

	if ((modifiers & XKEY_MOD_CONTROL) && (modifiers & XKEY_MOD_ALT))
	{
		//STUB
		return 0;
	}
	else if ((modifiers & XKEY_MOD_CAPSLOCK) != 0)
	{
		//STUB
		return 0;
	}
	else if ((modifiers & XKEY_MOD_SHIFT) != 0)
	{
		ret += 32;
	}

	return ret;
}

bool CSDLKeyboard::GetOSKeyName(int nKey, wchar_t* szwKeyName, int iBufSize)
{
#define HANDLE_CASE(XKEY, NAME) case XKEY: \
	wcsncpy(szwKeyName, L##NAME, iBufSize); \
	break;

	switch (nKey)
	{
		HANDLE_CASE(XKEY_ESCAPE, "Escape")
		HANDLE_CASE(XKEY_1, "1")
		HANDLE_CASE(XKEY_2, "2")
		HANDLE_CASE(XKEY_3, "3")
		HANDLE_CASE(XKEY_4, "4")
		HANDLE_CASE(XKEY_5, "5")
		HANDLE_CASE(XKEY_6, "6")
		HANDLE_CASE(XKEY_7, "7")
		HANDLE_CASE(XKEY_8, "8")
		HANDLE_CASE(XKEY_9, "9")
		HANDLE_CASE(XKEY_0, "0")
		HANDLE_CASE(XKEY_MINUS, "-")
		HANDLE_CASE(XKEY_EQUALS, "=")
		HANDLE_CASE(XKEY_BACKSPACE, "Backspace")
		HANDLE_CASE(XKEY_TAB, "Tab")
		HANDLE_CASE(XKEY_Q, "Q")
		HANDLE_CASE(XKEY_W, "W")
		HANDLE_CASE(XKEY_E, "E")
		HANDLE_CASE(XKEY_R, "R")
		HANDLE_CASE(XKEY_T, "T")
		HANDLE_CASE(XKEY_Y, "Y")
		HANDLE_CASE(XKEY_U, "U")
		HANDLE_CASE(XKEY_I, "I")
		HANDLE_CASE(XKEY_O, "O")
		HANDLE_CASE(XKEY_P, "P")
		HANDLE_CASE(XKEY_LBRACKET, "[")
		HANDLE_CASE(XKEY_RBRACKET, "]")
		HANDLE_CASE(XKEY_RETURN, "Enter")
		HANDLE_CASE(XKEY_LCONTROL, "Left Ctrl")
		HANDLE_CASE(XKEY_A, "A")
		HANDLE_CASE(XKEY_S, "S")
		HANDLE_CASE(XKEY_D, "D")
		HANDLE_CASE(XKEY_F, "F")
		HANDLE_CASE(XKEY_G, "G")
		HANDLE_CASE(XKEY_H, "H")
		HANDLE_CASE(XKEY_J, "J")
		HANDLE_CASE(XKEY_K, "K")
		HANDLE_CASE(XKEY_L, "L")
		HANDLE_CASE(XKEY_SEMICOLON, ";")
		HANDLE_CASE(XKEY_APOSTROPHE, "'")
		HANDLE_CASE(XKEY_TILDE, "~")
		HANDLE_CASE(XKEY_LSHIFT, "Left Shift")
		HANDLE_CASE(XKEY_BACKSLASH, "\\")
		HANDLE_CASE(XKEY_Z, "Z")
		HANDLE_CASE(XKEY_X, "X")
		HANDLE_CASE(XKEY_C, "C")
		HANDLE_CASE(XKEY_V, "V")
		HANDLE_CASE(XKEY_B, "B")
		HANDLE_CASE(XKEY_N, "N")
		HANDLE_CASE(XKEY_M, "M")
		HANDLE_CASE(XKEY_COMMA, ",")
		HANDLE_CASE(XKEY_PERIOD, ".")
		HANDLE_CASE(XKEY_SLASH, "/")
		HANDLE_CASE(XKEY_RSHIFT, "Right Shift")
		HANDLE_CASE(XKEY_MULTIPLY, "*")
		HANDLE_CASE(XKEY_LALT, "Left Alt")
		HANDLE_CASE(XKEY_SPACE, "Space")
		HANDLE_CASE(XKEY_CAPSLOCK, "Capslock")
		HANDLE_CASE(XKEY_F1, "F1")
		HANDLE_CASE(XKEY_F2, "F2")
		HANDLE_CASE(XKEY_F3, "F3")
		HANDLE_CASE(XKEY_F4, "F4")
		HANDLE_CASE(XKEY_F5, "F5")
		HANDLE_CASE(XKEY_F6, "F6")
		HANDLE_CASE(XKEY_F7, "F7")
		HANDLE_CASE(XKEY_F8, "F8")
		HANDLE_CASE(XKEY_F9, "F9")
		HANDLE_CASE(XKEY_F10, "F10")
		HANDLE_CASE(XKEY_F11, "F11")
		HANDLE_CASE(XKEY_F12, "F12")
		HANDLE_CASE(XKEY_F13, "F13")
		HANDLE_CASE(XKEY_F14, "F14")
		HANDLE_CASE(XKEY_F15, "F15")
		HANDLE_CASE(XKEY_NUMLOCK, "Num Lock")
		HANDLE_CASE(XKEY_SCROLLLOCK, "Scroll Lock")
		HANDLE_CASE(XKEY_ADD, "+")
		HANDLE_CASE(XKEY_DECIMAL, "Decimal")
		HANDLE_CASE(XKEY_RCONTROL, "Right Ctrl")
		HANDLE_CASE(XKEY_PRINT, "Print")
		HANDLE_CASE(XKEY_RALT, "Right Alt")
		HANDLE_CASE(XKEY_PAUSE, "Pause")
		HANDLE_CASE(XKEY_HOME, "Home")
		HANDLE_CASE(XKEY_UP, "Up")
		HANDLE_CASE(XKEY_PAGE_UP, "Page Up")
		HANDLE_CASE(XKEY_LEFT, "Left")
		HANDLE_CASE(XKEY_RIGHT, "Right")
		HANDLE_CASE(XKEY_END, "End")
		HANDLE_CASE(XKEY_DOWN, "Down")
		HANDLE_CASE(XKEY_PAGE_DOWN, "Page Down")
		HANDLE_CASE(XKEY_INSERT, "Insert")
		HANDLE_CASE(XKEY_DELETE, "Delete")
	default:
		wcsncpy(szwKeyName, L"<ERROR>", iBufSize);
		break;
	}
	return true;
#undef HANDLE_CASE
}

//////////////////////////////////////////////////////////////////////////
void CSDLKeyboard::SetupKeyNames()
{
	STUBMSG("SetupKeyNames STUB\n");
}

//////////////////////////////////////////////////////////////////////////
void CSDLKeyboard::FeedVirtualKey(int nVirtualKey, long lParam, bool bDown)
{
	STUBMSG("FeedVirtualKey STUB\n");
}

//////////////////////////////////////////////////////////////////////////
void CSDLKeyboard::ProcessKey(unsigned short cKey, bool bPressed, unsigned char* cTempKeys)
{
	assert(cKey < 256);
	assert(cKey > 0);

	if (bPressed)
	{
		cTempKeys[cKey] |= 0x80;
	}
	else
	{
		cTempKeys[cKey] &= ~0x80;
	}

	if (bPressed)
	{
		if (cKey == XKEY_LSHIFT) m_modifiers |= XKEY_MOD_LSHIFT;
		if (cKey == XKEY_RSHIFT) m_modifiers |= XKEY_MOD_RSHIFT;
		if (cKey == XKEY_LCONTROL) m_modifiers |= XKEY_MOD_LCONTROL;
		if (cKey == XKEY_RCONTROL) m_modifiers |= XKEY_MOD_RCONTROL;
		if (cKey == XKEY_LALT) m_modifiers |= XKEY_MOD_LALT;
		if (cKey == XKEY_RALT) m_modifiers |= XKEY_MOD_RALT;

		if (cKey == XKEY_CAPSLOCK)
		{

		}
		if (cKey == XKEY_NUMLOCK)
		{

		}
		if (cKey == XKEY_SCROLLLOCK)
		{

		}
	}
	else if (m_cKeysState[cKey] & 0x80)
	{
		if (cKey == XKEY_LSHIFT) m_modifiers &= ~XKEY_MOD_LSHIFT;
		if (cKey == XKEY_RSHIFT) m_modifiers &= ~XKEY_MOD_RSHIFT;
		if (cKey == XKEY_LCONTROL) m_modifiers &= ~XKEY_MOD_LCONTROL;
		if (cKey == XKEY_RCONTROL) m_modifiers &= ~XKEY_MOD_RCONTROL;
		if (cKey == XKEY_LALT) m_modifiers &= ~XKEY_MOD_LALT;
		if (cKey == XKEY_RALT) m_modifiers &= ~XKEY_MOD_RALT;
	}
	else
	{
		return;
	}

	// Post Input events.
	SInputEvent event;
	event.key = cKey;
	if (bPressed)
		event.type = SInputEvent::KEY_PRESS;
	else
		event.type = SInputEvent::KEY_RELEASE;

	event.timestamp = 0;
	event.moidifiers = m_modifiers;
	event.keyname = m_pInput->GetKeyName(event.key, event.moidifiers);

	if ((event.key == XKEY_TAB) && (event.type == SInputEvent::KEY_PRESS) && (cTempKeys[XKEY_LALT] & 0x80))
	{
		m_pInput->PostInputEvent(event);

		event.type = SInputEvent::KEY_RELEASE;
		m_pInput->PostInputEvent(event);
		cTempKeys[XKEY_TAB] = 0;
		cTempKeys[XKEY_LALT] = 0;
	}
	else
	{
		m_pInput->PostInputEvent(event);
	}
}

void CSDLKeyboard::Update()
{
	SDL_Event event;
	unsigned short xkey;
	std::vector<SDL_Event> events;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_KEYDOWN)
		{
			xkey = SDL2XKEY(event.key.keysym.sym);
			if (xkey > 0)
			{
				ProcessKey(xkey, true, m_cTempKeys);
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			xkey = SDL2XKEY(event.key.keysym.sym);
			if (xkey > 0)
			{
				ProcessKey(xkey, false, m_cTempKeys);
			}
		}
		else
		{
			events.push_back(event);
		}
	}

	for (SDL_Event ev : events)
	{
		SDL_PushEvent(&ev);
	}
	events.clear();
	memcpy(m_cOldKeysState, m_cKeysState, sizeof(m_cOldKeysState));
	memcpy(m_cKeysState, m_cTempKeys, sizeof(m_cKeysState));
}

void CSDLKeyboard::ShutDown()
{
	m_pLog->LogToFile("Keyboard Shutdown\n");
	UnAcquire();
}

void CSDLKeyboard::SetKey(int p_key, int value)
{

}

void CSDLKeyboard::SetPrevKey(int p_key, int value)
{

}

bool CSDLKeyboard::KeyDown(int p_key)
{
	return ((m_cKeysState[p_key] & 0x80) != 0);
}

bool CSDLKeyboard::KeyPressed(int p_key)
{
	if (((m_cKeysState[p_key] & 0x80) != 0) && ((m_cOldKeysState[p_key] & 0x80) == 0))
		return true;
	else return false;
}

bool CSDLKeyboard::KeyReleased(int p_key)
{
	return ((m_cKeysState[p_key] & 0x80) == 0) && ((m_cOldKeysState[p_key] & 0x80) != 0);
}

void CSDLKeyboard::ClearKey(int p_key)
{
	if (p_key < 256 && p_key >= 0)
	{
		m_cOldKeysState[p_key] = m_cKeysState[p_key];
		m_cKeysState[p_key] = NULL;
	}
}

int CSDLKeyboard::GetKeyPressedCode()
{
	for (int k = 0; k < 256; k++)
	{
		int nXKey = k;

		if (nXKey == XKEY_NULL)
			continue;

		if (KeyPressed(nXKey))
			return nXKey;
	}
	return -1;
}

const char* CSDLKeyboard::GetKeyPressedName()
{
	int key = GetKeyPressedCode();
	if (key == -1)
		return (NULL);

	return m_pInput->GetKeyName(key);
}

int CSDLKeyboard::GetKeyDownCode()
{
	for (int k = 0; k < 256; k++)
		if (KeyDown(k))
			return k;
	return -1;
}

const char* CSDLKeyboard::GetKeyDownName()
{
	int key = GetKeyDownCode();
	if (key == -1)
		return (NULL);

	return m_pInput->GetKeyName(key);
}

void CSDLKeyboard::WaitForKey()
{
}

void CSDLKeyboard::ClearKeyState()
{
	memset(m_cKeysState, 0, sizeof(m_cKeysState));
	m_modifiers = 0;
}

unsigned char CSDLKeyboard::GetKeyState(int nKey)
{
	if (nKey >= 0 && nKey < 256)
		return m_cKeysState[nKey];
	return 0;
}