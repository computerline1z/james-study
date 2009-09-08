#
//----------------------------------------------------------------------------
// File: CaptureSound.cpp
//
// Desc: The CaptureSound sample shows how to use DirectSoundCapture to capture 
//       sound into a wave file 
//
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <commdlg.h>
#include <mmreg.h>
//#include <d3dx9.h>
#include <dxerr9.h>
#include <dsound.h>
#include "resource.h"
#include "Common\DSUtil.h"
#include "Common\DXUtil.h"
#define NUM_REC_NOTIFICATIONS  16
#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

class Record  
{
public:

	Record();
	virtual ~Record();

	LPDIRECTSOUNDCAPTURE       g_pDSCapture        ;
	LPDIRECTSOUNDCAPTUREBUFFER g_pDSBCapture       ;
	LPDIRECTSOUNDNOTIFY        g_pDSNotify         ;
	WAVEFORMATEX               g_wfxInput;
	DSBPOSITIONNOTIFY          g_aPosNotify[ NUM_REC_NOTIFICATIONS + 1 ];  
	HANDLE                     g_hNotificationEvent;
	DWORD                      g_dwCaptureBufferSize;
	DWORD                      g_dwNextCaptureOffset;
	DWORD                      g_dwNotifySize;
	CWaveFile*                 g_pWaveFile;

	HRESULT InitDirectSound();
	HRESULT FreeDirectSound();
	HRESULT Format(DWORD index);
	VOID	GetWaveFormat( DWORD index, WAVEFORMATEX* pwfx );
	VOID    SetRecordedFile(LPTSTR  filepath);
	HRESULT CreateCaptureBuffer( WAVEFORMATEX* pwfxInput);
	HRESULT InitNotifications();
	HRESULT StartRecord();
	HRESULT StopRecord();
	HRESULT RecordCapturedData();
};

