#include "StdAfx.h"
#include "SDLMouse.h"

#include <math.h>
#include <stdio.h>
#include <ILog.h>
#include <ISystem.h>
#include <IConsole.h>
#include "Input.h"
#include <IGame.h>

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

bool CSDLMouse::Init(ISystem *pSystem)
{
	m_pSystem = pSystem;
	m_pLog = pSystem->GetILog();
	m_pTimer = pSystem->GetITimer();

	// mouse accel
	i_mouse_accel = m_pSystem->GetIConsole()->CreateVariable("i_mouse_accel", "0.0", VF_DUMPTODISK,
															 "Set mouse acceleration, 0.0 means no acceleration.\n"
															 "Usage: i_mouse_accel [float number] (usually a small number, 0.1 is a good one)\n"
															 "Default is 0.0 (off)");

	// mouse accel cap
	i_mouse_accel_max = m_pSystem->GetIConsole()->CreateVariable("i_mouse_accel_max", "100.0", VF_DUMPTODISK,
																 "Set mouse max mouse delta when using acceleration.\n"
																 "Usage: i_mouse_accel_max [float number]\n"
																 "Default is 100.0");

	// mouse smooth
	i_mouse_smooth = m_pSystem->GetIConsole()->CreateVariable("i_mouse_smooth", "0.0", VF_DUMPTODISK,
															  "Set mouse smoothing value, also if 0 (disabled) there will be a simple average between the old and the actual input.\n"
															  "Usage: i_mouse_smooth [float number] (1.0 = very very smooth, 30 = almost istant)\n"
															  "Default is 0.0");

	// mouse mirror
	i_mouse_mirror = m_pSystem->GetIConsole()->CreateVariable("i_mouse_mirror", "0", VF_DUMPTODISK,
															  "Set mouse mirroring, if not 0 the mouse input will be mirrored.\n"
															  "Usage: i_mouse_smooth [0 or 1]\n"
															  "Default is 0");

	m_pLog->Log("Initializing mouse\n");

	m_fVScreenX = 400.0f;
	m_fVScreenY = 300.0f;

	m_fDblClickTime = 0.2f;

	m_fSensitivity = 0.2f;
	m_fSensitivityScale = 1;

	m_fLastRelease[0] = m_fLastRelease[1] = m_fLastRelease[2] = 0.0f;

	memset(m_Deltas, 0, sizeof(m_Deltas));
	// smooth
	memset(m_OldDeltas, 0, sizeof(m_OldDeltas));

	memset(m_DeltasInertia, 0, sizeof(float) * 2);
	memset(m_Events, 0, sizeof(m_Events));

	m_wheelChecked = false;

	SDL_SetRelativeMouseMode(SDL_TRUE);

	return (true);
}

///////////////////////////////////////////
void CSDLMouse::Update(bool bPrevFocus)
{
	SDL_Event event;
	std::vector<SDL_Event> events;

	memset(m_Deltas, 0, sizeof(m_Deltas));
	memcpy(m_oldEvents, m_Events, sizeof(m_Events));
	memset(m_Events, 0, sizeof(m_Events));

	float mouseDelta[6];
	memset(mouseDelta, 0, sizeof(mouseDelta));

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				m_Events[0] = 128;
			}
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				m_Events[1] = 128;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				m_Events[0] = 0;
			}
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				m_Events[0] = 0;
			}
		}
		else if (event.type == SDL_MOUSEWHEEL)
		{
			mouseDelta[2] = event.wheel.y;
		}
		else if (event.type == SDL_MOUSEMOTION)
		{
			mouseDelta[0] += float((int)event.motion.xrel);
			mouseDelta[1] += float((int)event.motion.yrel);
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

	m_Deltas[0] = mouseDelta[0];
	m_Deltas[1] = mouseDelta[1];
	m_Deltas[2] = mouseDelta[2];

	float mouseaccel = i_mouse_accel->GetFVal();

	if (mouseaccel > 0.0001f)
	{
		m_Deltas[0] = m_Deltas[0] * (float)fabs(m_Deltas[0] * mouseaccel /*m_fAccelerationScale*/);
		m_Deltas[1] = m_Deltas[1] * (float)fabs(m_Deltas[1] * mouseaccel /*m_fAccelerationScale*/);

		CapDeltas(i_mouse_accel_max->GetFVal());
	}
	SmoothDeltas(i_mouse_smooth->GetFVal());

	// mouse mirror? only in game, not while using the menu.
	if (i_mouse_mirror->GetIVal())
	{
		IGame *p_game = m_pSystem->GetIGame();

#ifndef _ISNOTFARCRY
		if (!p_game || !GetIXGame(p_game)->IsInMenu())
		{
			m_Deltas[0] = -m_Deltas[0];
			m_Deltas[1] = -m_Deltas[1];
		}
#endif
	}

	if (m_Deltas[0] != 0)
	{
		PostEvent(XKEY_MAXIS_X, SInputEvent::MOUSE_MOVE, m_Deltas[0]);
	}
	if (m_Deltas[1] != 0)
	{
		PostEvent(XKEY_MAXIS_Y, SInputEvent::MOUSE_MOVE, m_Deltas[1]);
	}
	if (m_Deltas[2] > 0)
	{
		PostEvent(XKEY_MWHEEL_UP, SInputEvent::MOUSE_MOVE, m_Deltas[2]);
	}
	if (m_Deltas[2] < 0)
	{
		PostEvent(XKEY_MWHEEL_DOWN, SInputEvent::MOUSE_MOVE, m_Deltas[2]);
	}

#define HANDLE_MOUSE_EVENT(XKEY) nkey = XKEY2IDX(XKEY); \
	if (m_Events[nkey] != m_oldEvents[nkey]) \
	{\
		if (m_Events[nkey] & 0x80)\
			PostEvent(XKEY, SInputEvent::KEY_PRESS);\
		else\
			PostEvent(XKEY, SInputEvent::KEY_RELEASE);\
	}

	int nkey;

	HANDLE_MOUSE_EVENT(XKEY_MOUSE1)
	HANDLE_MOUSE_EVENT(XKEY_MOUSE2)
	HANDLE_MOUSE_EVENT(XKEY_MOUSE3)
	HANDLE_MOUSE_EVENT(XKEY_MOUSE4)
	HANDLE_MOUSE_EVENT(XKEY_MOUSE5)
	HANDLE_MOUSE_EVENT(XKEY_MOUSE6)
	HANDLE_MOUSE_EVENT(XKEY_MOUSE7)
	HANDLE_MOUSE_EVENT(XKEY_MOUSE8)

	// GENERATE EVENTS FOR WHEEL AND AXES
	// mouse wheel
	if (m_Deltas[2] > 0.0f)
	{
		m_Events[XKEY2IDX(XKEY_MWHEEL_UP)] = 0x80;
	}
	else
	{
		m_Events[XKEY2IDX(XKEY_MWHEEL_UP)] = 0;
	}

	if (m_Deltas[2] < 0.0f)
	{
		m_Events[XKEY2IDX(XKEY_MWHEEL_DOWN)] = 0x80;
	}
	else
	{
		m_Events[XKEY2IDX(XKEY_MWHEEL_DOWN)] = 0;
	}

	HANDLE_MOUSE_EVENT(XKEY_MWHEEL_UP);
	HANDLE_MOUSE_EVENT(XKEY_MWHEEL_DOWN);

#undef HANDLE_MOUSE_EVENT

	// mouse axis
	if (fabs(m_Deltas[0]) > 1e-5f)
		m_Events[XKEY2IDX(XKEY_MAXIS_X)] = 0x80;
	if (fabs(m_Deltas[1]) > 1e-5f)
		m_Events[XKEY2IDX(XKEY_MAXIS_Y)] = 0x80;

	m_kInertia = 0; // NO INERTIA ANYMORE...
	// mouse inertia
	if (m_kInertia > 0)
	{
		float dt = m_pTimer->GetFrameTime();
		if (dt > 0.1f)
			dt = 0.1f;
		for (int i = 0; i < 2; i++)
			m_Deltas[i] = (m_DeltasInertia[i] += (m_Deltas[i] - m_DeltasInertia[i]) * m_kInertia * dt);
	}

	// Update the Virtual Screen position
	m_fVScreenX += GetDeltaX() * 4.0f;
	m_fVScreenY += GetDeltaY() * 4.0f;

	if (m_fVScreenX < 0)
		m_fVScreenX = 0;
	else if (m_fVScreenX > 800 - 1)
		m_fVScreenX = 800 - 1;

	if (m_fVScreenY < 0)
		m_fVScreenY = 0;
	else if (m_fVScreenY > 600 - 1)
		m_fVScreenY = 600 - 1;

#define HANDLE_DBL_CLICK(XKEY) nIdx = XKEY2IDX(XKEY);\
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))\
	{\
		if (!m_bDblClick[nIdx])\
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();\
		else\
			m_bDblClick[nIdx] = false;\
	}

	int nIdx;
	HANDLE_DBL_CLICK(XKEY_MOUSE1)
	HANDLE_DBL_CLICK(XKEY_MOUSE2)
	HANDLE_DBL_CLICK(XKEY_MOUSE3)
	HANDLE_DBL_CLICK(XKEY_MOUSE4)
	HANDLE_DBL_CLICK(XKEY_MOUSE5)
	HANDLE_DBL_CLICK(XKEY_MOUSE6)
	HANDLE_DBL_CLICK(XKEY_MOUSE7)
	HANDLE_DBL_CLICK(XKEY_MOUSE8)

#undef HANDLE_DBL_CLICK
}

void CSDLMouse::ClearKeyState()
{
	memset(m_fLastRelease, 0, sizeof(m_fLastRelease));
	memset(m_Events, 0, sizeof(m_Events));
	memset(m_oldEvents, 0, sizeof(m_oldEvents));
	memset(m_bDblClick, 0, sizeof(m_bDblClick));
}

int CSDLMouse::XKEY2IDX(int nKey)
{
	switch (nKey)
	{
	case XKEY_MOUSE1:
		return 0;
	case XKEY_MOUSE2:
		return 1;
	case XKEY_MOUSE3:
		return 2;
	case XKEY_MOUSE4:
		return 3;
	case XKEY_MOUSE5:
		return 4;
	case XKEY_MOUSE6:
		return 5;
	case XKEY_MOUSE7:
		return 6;
	case XKEY_MOUSE8:
		return 7;
	case XKEY_MWHEEL_UP:
		return 8;
	case XKEY_MWHEEL_DOWN:
		return 9;
	case XKEY_MAXIS_X:
		return 10;
	case XKEY_MAXIS_Y:
		return 11;
	};

	return -1;
}

bool CSDLMouse::MouseDown(int p_numButton)
{
	return ((m_Events[XKEY2IDX(p_numButton)] & 0x80) != 0);
}

bool CSDLMouse::MousePressed(int p_numButton)
{
	int nIdx = XKEY2IDX(p_numButton);
	return ((m_Events[nIdx] & 0x80) != 0) && ((m_oldEvents[nIdx] & 0x80) == 0);
}

bool CSDLMouse::MouseDblClick(int p_numButton)
{
	int nIdx = XKEY2IDX(p_numButton);
	if ((nIdx <= 2) && ((m_Events[nIdx] & 0x80) != 0) && ((m_oldEvents[nIdx] & 0x80) == 0))
	{
		if ((m_pTimer->GetAsyncCurTime() - m_fLastRelease[nIdx]) <= m_fDblClickTime)
		{
			m_fLastRelease[nIdx] = 0.0f;
			m_bDblClick[nIdx] = true;
			return true;
		}
	}
	return false;
}

bool CSDLMouse::MouseReleased(int p_numButton)
{
	int nIdx = XKEY2IDX(p_numButton);
	return ((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0);
}

bool CSDLMouse::GetOSKeyName(int nKey, wchar_t *szwKeyName, int iBufSize)
{
	wcsncpy(szwKeyName, L"Mouse STUB", iBufSize);
	return true;
}

void CSDLMouse::Shutdown()
{
	m_pLog->LogToFile("Mouse Shutdown\n");
}

bool CSDLMouse::SetExclusive(bool value, void *hwnd)
{
	return true;
}

void CSDLMouse::SetMouseWheelRotation(int value)
{
	m_Deltas[2] = (float)(value);
	m_wheelChecked = false;
}

void CSDLMouse::SetVScreenX(float fX)
{
	m_fVScreenX = fX;
}

void CSDLMouse::SetVScreenY(float fY)
{
	m_fVScreenY = fY;
}

float CSDLMouse::GetVScreenX()
{
	return m_fVScreenX + 0.5f;
}

float CSDLMouse::GetVScreenY()
{
	return m_fVScreenY + 0.5f;
}

void CSDLMouse::PostEvent(int key, SInputEvent::EType type, float value, unsigned int timestamp)
{
	// Post Input events.
	SInputEvent event;
	event.key = key;
	event.type = type;
	if (timestamp)
		event.timestamp = timestamp;
	else
		event.timestamp = GetTickCount();
	event.moidifiers = m_pInput->GetModifiers();
	event.keyname = m_pInput->GetKeyName(event.key, event.moidifiers);
	event.value = value;
	m_pInput->PostInputEvent(event);
}

#define GETLEN2D(v) (v[0] * v[0] + v[1] * v[1])

void CSDLMouse::SmoothDeltas(float accel, float decel)
{
	if (accel < 0.0001f)
	{
		// do nothing ,just like it was before.
		return;
	}
	else if (accel < 0.9999f) // mouse smooth, average the old and the actual delta by the delta ammount, less delta = more smooth speed.
	{
		float delta[2];

		delta[0] = m_Deltas[0] - m_OldDeltas[0];
		delta[1] = m_Deltas[1] - m_OldDeltas[1];

		float len = sqrt(GETLEN2D(delta));

		float amt = 1.0f - (crymin(10.0f, len) / 10.0f * crymin(accel, 0.9f));

		// m_pLog->Log("amt:%f\n",amt);

		m_Deltas[0] = m_OldDeltas[0] + delta[0] * amt;
		m_Deltas[1] = m_OldDeltas[1] + delta[1] * amt;
	}
	else if (accel < 1.0001f) // mouse smooth, just average the old and the actual delta.
	{
		m_Deltas[0] = (m_Deltas[0] + m_OldDeltas[0]) * 0.5f;
		m_Deltas[1] = (m_Deltas[1] + m_OldDeltas[1]) * 0.5f;
	}
	else // mouse smooth with acceleration
	{
		float dt = crymin(m_pTimer->GetFrameTime(), 0.1f);

		float delta[2];

		float amt = 0.0;

		// if the input want to stop use twice of the acceleration.
		if (GETLEN2D(m_Deltas) < 0.0001f)
			if (decel > 0.0001f) // there is a custom deceleration value? use it.
				amt = crymin(1, dt * decel);
			else // otherwise acceleration * 2 is the default.
				amt = crymin(1, dt * accel * 2.0f);
		else
			amt = crymin(1, dt * accel);

		delta[0] = m_Deltas[0] - m_OldDeltas[0];
		delta[1] = m_Deltas[1] - m_OldDeltas[1];

		m_Deltas[0] = m_OldDeltas[0] + delta[0] * amt;
		m_Deltas[1] = m_OldDeltas[1] + delta[1] * amt;
	}

	m_OldDeltas[0] = m_Deltas[0];
	m_OldDeltas[1] = m_Deltas[1];
}

void CSDLMouse::CapDeltas(float cap)
{
	float temp;

	temp = fabs(m_Deltas[0]) / cap;
	if (temp > 1.0f)
		m_Deltas[0] /= temp;

	temp = fabs(m_Deltas[1]) / cap;
	if (temp > 1.0f)
		m_Deltas[1] /= temp;
}
