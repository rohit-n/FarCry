#ifndef SDL_KEYBOARD
#define SDL_KEYBOARD

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <IInput.h>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

struct ILog;
struct ICVar;

#define		TOGGLE_CAPSLOCK			(1 << 0)
#define		TOGGLE_NUMLOCK			(1 << 1)
#define		TOGGLE_SCROLLLOCK		(1 << 2)

#define KEYFLAG_BUFFERSIZE 32
class CInput;

class CSDLKeyboard : 
public IKeyboard  
{
public:
	bool Init(CInput *,ISystem *pSystem);
	CSDLKeyboard();
	virtual ~CSDLKeyboard();
	void ShutDown();
  void SetKey(int p_key, int value);
  void SetPrevKey(int p_key, int value);
	bool KeyDown(int p_key);
	bool KeyPressed(int p_key);
	bool KeyReleased(int p_key);
	void ClearKey(int p_key);
	int GetKeyPressedCode();
	const char *GetKeyPressedName();
	int GetKeyDownCode();
	const char *GetKeyDownName();
	void SetExclusive(bool value,void *hwnd=0);
	void WaitForKey();
	void ClearKeyState();
	unsigned char GetKeyState(int nKey);
	void Update();
	int GetModifiers() const { return m_modifiers; };

public:	
	unsigned short SDL2XKEY(SDL_Keycode kc);
	unsigned char XKEY2ASCII(unsigned short nCode,int modifiers);
	bool GetOSKeyName(int nKey, wchar_t *szwKeyName, int iBufSize);
	void FeedVirtualKey(int nVirtualKey,long lParam,bool bDown);
	
protected:
	bool	Acquire();
	bool	UnAcquire();
	void	SetupKeyNames();
	void	ProcessKey(unsigned short nKey,bool bPressed,unsigned char *cTempKeys);
	unsigned char	m_cKeysState[256];
	unsigned char	m_cOldKeysState[256];	
	unsigned char m_cTempKeys[256];	
	CInput *m_pInput;
	bool m_bExclusiveMode;
	ISystem	*m_pSystem;
	ILog		*m_pLog;
	int m_modifiers;
};
#endif