#ifndef SDL_MOUSE
#define SDL_MOUSE

#include <IInput.h>
struct ITimer;
struct ILog;

struct ICVar;

#define XMOUSE_MAX_MOUSE_EVENTS		12				// e.g. mouse buttons, axis, ..

class CSDLMouse : public IMouse
{
public:
	CSDLMouse()
	{
		m_exmode = false;
		m_kInertia = 0;
		m_fAccelerationScale = 0.5f;
		m_bAcceleration = false;
	}
	virtual ~CSDLMouse()
	{
	}

	class CInput* m_pInput;

	bool	Init(ISystem* pSystem);
	void	Shutdown();
	void	Update(bool bPrevFocus);
	void	ClearKeyState();

	bool	MouseDown(int p_numButton);
	bool	MousePressed(int p_numButton);
	bool	MouseDblClick(int p_numButton);
	bool	MouseReleased(int p_numButton);
	bool GetOSKeyName(int nKey, wchar_t* szwKeyName, int iBufSize);

	void  SetMouseWheelRotation(int value); // set wheel state from app message loop

	bool	SetExclusive(bool value, void* hwnd = 0);

	inline float	GetDeltaX() { return m_Deltas[0] * (m_fSensitivity * m_fSensitivityScale); }
	inline float	GetDeltaY() { return m_Deltas[1] * (m_fSensitivity * m_fSensitivityScale); }
	inline float	GetDeltaZ()
	{
		m_wheelChecked = true;
		return m_Deltas[2] * (m_fSensitivity * m_fSensitivityScale);
	}

	void SetSensitvity(float fSensitivity)
	{
		if (fSensitivity > 0)
			m_fSensitivity = fSensitivity / 100;
		else
			m_fSensitivity = 0.0f;
	}
	float GetSensitvity()
	{
		return m_fSensitivity * 100;
	}
	void SetSensitvityScale(float fSensScale)
	{
		m_fSensitivityScale = fSensScale;
	}
	float GetSensitvityScale()
	{
		return m_fSensitivityScale;
	}
	inline void SetInertia(float kinertia) { m_kInertia = kinertia; }

	void SetVScreenX(float fX);
	void SetVScreenY(float fY);
	float GetVScreenX();
	float GetVScreenY();
private:
	int XKEY2IDX(int nKey);

	void PostEvent(int key, SInputEvent::EType type, float value = 0, unsigned int timestamp = 0);

	//smooth movement & mouse accel
	void CapDeltas(float cap);
	void SmoothDeltas(float accel, float decel = 0.0f);

	ICVar* i_mouse_accel;
	ICVar* i_mouse_accel_max;
	ICVar* i_mouse_smooth;
	ICVar* i_mouse_mirror;

	float						m_fDblClickTime;

	float						m_Deltas[5];
	float						m_OldDeltas[2];

	float						m_DeltasInertia[2];
	float						m_kInertia;
	unsigned char		m_Events[XMOUSE_MAX_MOUSE_EVENTS];
	unsigned char		m_oldEvents[XMOUSE_MAX_MOUSE_EVENTS];
	float						m_fLastRelease[XMOUSE_MAX_MOUSE_EVENTS];
	bool						m_bDblClick[XMOUSE_MAX_MOUSE_EVENTS];


	bool						m_exmode;
	bool						m_wheelChecked;

	float						m_fVScreenX;
	float						m_fVScreenY;

	ILog* m_pLog;
	ITimer* m_pTimer;
	ISystem* m_pSystem;

	float						m_fSensitivity;
	float 					m_fSensitivityScale;
	float 					m_fAccelerationScale;
	bool						m_bAcceleration;
};

#endif