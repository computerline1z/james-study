#include "stdafx.h"
#include "record.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Record::Record()
{
	g_pDSCapture         = NULL;
	g_pDSBCapture        = NULL;
	g_pDSNotify          = NULL;
	
}

Record::~Record()
{

}
//-----------------------------------------------------------------------------
// Name: InitDirectSound()
// Desc: Initilizes DirectSound
//-----------------------------------------------------------------------------
HRESULT Record::InitDirectSound()
{
    HRESULT hr;

    ZeroMemory( &g_aPosNotify, sizeof(DSBPOSITIONNOTIFY) * 
                               (NUM_REC_NOTIFICATIONS + 1) );
    g_dwCaptureBufferSize = 0;
    g_dwNotifySize        = 0;
    g_pWaveFile           = NULL;

    // Initialize COM
    if( FAILED( hr = CoInitialize(NULL) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("CoInitialize"), hr );

    // Create IDirectSoundCapture using the preferred capture device
    if( FAILED( hr = DirectSoundCaptureCreate( NULL, &g_pDSCapture, NULL ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("DirectSoundCaptureCreate"), hr );
	
	
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FreeDirectSound()
// Desc: Releases DirectSound 
//-----------------------------------------------------------------------------
HRESULT Record::FreeDirectSound()
{
    SAFE_DELETE( g_pWaveFile );

    // Release DirectSound interfaces
    SAFE_RELEASE( g_pDSNotify );
    SAFE_RELEASE( g_pDSBCapture );
    SAFE_RELEASE( g_pDSCapture ); 

    // Release COM
    CoUninitialize();

    return S_OK;
}
//-----------------------------------------------------------------------------
// Name: SaveSoundFile()
// Desc: Called when the user requests to save to a sound file
//-----------------------------------------------------------------------------
VOID Record::SetRecordedFile(LPTSTR filepath) 
{
    HRESULT hr;
    
    SAFE_DELETE( g_pWaveFile );
    g_pWaveFile = new CWaveFile;
    if( NULL == g_pWaveFile )
        return;

    // Get the format of the capture buffer in g_wfxCaptureWaveFormat
    WAVEFORMATEX wfxCaptureWaveFormat;
    ZeroMemory( &wfxCaptureWaveFormat, sizeof(WAVEFORMATEX) );
    g_pDSBCapture->GetFormat( &wfxCaptureWaveFormat, sizeof(WAVEFORMATEX), NULL );

    // Load the wave file
    
	if( FAILED( hr = g_pWaveFile->Open( filepath, &wfxCaptureWaveFormat, WAVEFILE_WRITE ) ) )
    {
        return;
    }

    
}

//-----------------------------------------------------------------------------
// Name: InitNotifications()
// Desc: Inits the notifications on the capture buffer which are handled
//       in WinMain()
//-----------------------------------------------------------------------------
HRESULT Record::InitNotifications()
{
    HRESULT hr; 

    if( NULL == g_pDSBCapture )
        return E_FAIL;

    // Create a notification event, for when the sound stops playing
    if( FAILED( hr = g_pDSBCapture->QueryInterface( IID_IDirectSoundNotify, 
                                                    (VOID**)&g_pDSNotify ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("QueryInterface"), hr );

    // Setup the notification positions
    for( INT i = 0; i < NUM_REC_NOTIFICATIONS; i++ )
    {
        g_aPosNotify[i].dwOffset = (g_dwNotifySize * i) + g_dwNotifySize - 1;
        g_aPosNotify[i].hEventNotify = g_hNotificationEvent;             
    }
    
    // Tell DirectSound when to notify us. the notification will come in the from 
    // of signaled events that are handled in WinMain()
    if( FAILED( hr = g_pDSNotify->SetNotificationPositions( NUM_REC_NOTIFICATIONS, 
                                                            g_aPosNotify ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("SetNotificationPositions"), hr );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: StartRecord()
// Desc: Starts the capture buffer from recording
//-----------------------------------------------------------------------------
HRESULT Record::StartRecord()
{
    HRESULT hr;
    
    // Create a capture buffer, and tell the capture 
    // buffer to start recording   
    if( FAILED( hr = CreateCaptureBuffer( &g_wfxInput ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"), hr );

    if( FAILED( hr = g_pDSBCapture->Start( DSCBSTART_LOOPING ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("Start"), hr );
   
    
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: StopRecord()
// Desc: Stops the capture buffer from recording
//-----------------------------------------------------------------------------
HRESULT Record::StopRecord( )
{
    HRESULT hr;

    
    // Stop the capture and read any data that 
    // was not caught by a notification
    if( NULL == g_pDSBCapture )
        return S_OK;

    // Stop the buffer, and read any data that was not 
    // caught by a notification
    if( FAILED( hr = g_pDSBCapture->Stop() ) )
        return DXTRACE_ERR_MSGBOX( TEXT("Stop"), hr );

    if( FAILED( hr = RecordCapturedData() ) )
        return DXTRACE_ERR_MSGBOX( TEXT("RecordCapturedData"), hr );

    // Close the wav file
    SAFE_DELETE( g_pWaveFile );
    

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: RecordCapturedData()
// Desc: Copies data from the capture buffer to the output buffer 
//-----------------------------------------------------------------------------
HRESULT Record::RecordCapturedData() 
{
    HRESULT hr;
    VOID*   pbCaptureData    = NULL;
    DWORD   dwCaptureLength;
    VOID*   pbCaptureData2   = NULL;
    DWORD   dwCaptureLength2;
    UINT    dwDataWrote;
    DWORD   dwReadPos;
    DWORD   dwCapturePos;
    LONG lLockSize;

    if( NULL == g_pDSBCapture )
        return S_FALSE;
    if( NULL == g_pWaveFile )
        return S_FALSE;

    if( FAILED( hr = g_pDSBCapture->GetCurrentPosition( &dwCapturePos, &dwReadPos ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("GetCurrentPosition"), hr );

    lLockSize = dwReadPos - g_dwNextCaptureOffset;
    if( lLockSize < 0 )
        lLockSize += g_dwCaptureBufferSize;

    // Block align lock size so that we are always write on a boundary
    lLockSize -= (lLockSize % g_dwNotifySize);

    if( lLockSize == 0 )
        return S_FALSE;

    // Lock the capture buffer down
    if( FAILED( hr = g_pDSBCapture->Lock( g_dwNextCaptureOffset, lLockSize, 
                                          &pbCaptureData, &dwCaptureLength, 
                                          &pbCaptureData2, &dwCaptureLength2, 0L ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("Lock"), hr );

    // Write the data into the wav file
    if( FAILED( hr = g_pWaveFile->Write( dwCaptureLength, 
                                              (BYTE*)pbCaptureData, 
                                              &dwDataWrote ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("Write"), hr );

    // Move the capture offset along
    g_dwNextCaptureOffset += dwCaptureLength; 
    g_dwNextCaptureOffset %= g_dwCaptureBufferSize; // Circular buffer

    if( pbCaptureData2 != NULL )
    {
        // Write the data into the wav file
        if( FAILED( hr = g_pWaveFile->Write( dwCaptureLength2, 
                                                  (BYTE*)pbCaptureData2, 
                                                  &dwDataWrote ) ) )
            return DXTRACE_ERR_MSGBOX( TEXT("Write"), hr );

        // Move the capture offset along
        g_dwNextCaptureOffset += dwCaptureLength2; 
        g_dwNextCaptureOffset %= g_dwCaptureBufferSize; // Circular buffer
    }

    // Unlock the capture buffer
    g_pDSBCapture->Unlock( pbCaptureData,  dwCaptureLength, 
                           pbCaptureData2, dwCaptureLength2 );


    return S_OK;
}
//-----------------------------------------------------------------------------
// Name: GetWaveFormat()
//-----------------------------------------------------------------------------
VOID Record::GetWaveFormat( DWORD index, WAVEFORMATEX* pwfx )
{
    INT iSampleRate = index / 4;
    INT iType = index % 4;
	
    switch( iSampleRate )
    {
        case 0: pwfx->nSamplesPerSec = 44100; break;
        case 1: pwfx->nSamplesPerSec = 22050; break;
        case 2: pwfx->nSamplesPerSec = 11025; break;
        case 3: pwfx->nSamplesPerSec =  8000; break;
    }

    switch( iType )
    {
        case 0: pwfx->wBitsPerSample =  8; pwfx->nChannels = 1; break;
        case 1: pwfx->wBitsPerSample = 16; pwfx->nChannels = 1; break;
        case 2: pwfx->wBitsPerSample =  8; pwfx->nChannels = 2; break;
        case 3: pwfx->wBitsPerSample = 16; pwfx->nChannels = 2; break;
    }
	
    pwfx->nBlockAlign = pwfx->nChannels * ( pwfx->wBitsPerSample / 8 );
    pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
    
}

//-----------------------------------------------------------------------------
// Name: Format()
// Desc: Creates a capture buffer format based on what was selected
//-----------------------------------------------------------------------------
HRESULT Record::Format(DWORD index)
{
    HRESULT       hr;
    
    ZeroMemory( &g_wfxInput, sizeof(g_wfxInput));
    g_wfxInput.wFormatTag = WAVE_FORMAT_PCM;

    GetWaveFormat(index, &g_wfxInput );

    if( FAILED( hr = CreateCaptureBuffer( &g_wfxInput ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"), hr );

    return S_OK;
}
//-----------------------------------------------------------------------------
// Name: CreateCaptureBuffer()
// Desc: Creates a capture buffer and sets the format 
//-----------------------------------------------------------------------------
HRESULT Record::CreateCaptureBuffer( WAVEFORMATEX* pwfxInput )
{
    HRESULT hr;
    DSCBUFFERDESC dscbd;

    SAFE_RELEASE( g_pDSNotify );
    SAFE_RELEASE( g_pDSBCapture );

    // Set the notification size
    g_dwNotifySize = MAX( 1024, pwfxInput->nAvgBytesPerSec / 8 );
    g_dwNotifySize -= g_dwNotifySize % pwfxInput->nBlockAlign;   

    // Set the buffer sizes 
    g_dwCaptureBufferSize = g_dwNotifySize * NUM_REC_NOTIFICATIONS;

    SAFE_RELEASE( g_pDSNotify );
    SAFE_RELEASE( g_pDSBCapture );

    // Create the capture buffer
    ZeroMemory( &dscbd, sizeof(dscbd) );
    dscbd.dwSize        = sizeof(dscbd);
    dscbd.dwBufferBytes = g_dwCaptureBufferSize;
    dscbd.lpwfxFormat   = pwfxInput; // Set the format during creatation

    if( FAILED( hr = g_pDSCapture->CreateCaptureBuffer( &dscbd, 
                                                        &g_pDSBCapture, 
                                                        NULL ) ) )
        return DXTRACE_ERR_MSGBOX( TEXT("CreateCaptureBuffer"), hr );

    g_dwNextCaptureOffset = 0;

    if( FAILED( hr = InitNotifications() ) )
        return DXTRACE_ERR_MSGBOX( TEXT("InitNotifications"), hr );

    return S_OK;
}
