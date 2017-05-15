/*
    Copyright (C) 2010, Michael P. Thompson

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 2 as 
    specified in the README.txt file or as published by the Free Software 
    Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    $Id: rvDSCamera.h 28 2010-03-09 23:49:39Z mike $
*/

#ifndef _RV_DSCAMERA_H_
#define _RV_DSCAMERA_H_

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
#pragma warning(disable : 4312)

#include "wx/wx.h"
#include "cv.h"
#include "rvLinkedList.h"

// There are problems compiling wxWidgets and DirectX/DirectShow files because
// of name and type conflicts.  The changes below help work around these issues.

// Argh!!!  __WXDEBUG__ is also used by the DirectX/DirectShow SDK to
// '#pragma once' the contents of DXSDK/Samples/C++/DirectShow/BaseClasses/wxdebug.h.
// So if __WXDEBUG__ is defined, then wxdebug.h doesn't get included, and the
// assert macros don't get defined.  You have to #undef __WXDEBUG__ before including
// the directshow baseclass's <streams.h>.
#undef __WXDEBUG__

// Argh again!!!  <streams.h> includes <wxdebug.h> which declares
// some PTCHAR objects. PTCHAR is only conditionally defined in <winnt.h>,
// but this condition isn't true because <tchar.h> defines TCHAR and
// defines the macro _TCHAR_DEFINED which stops winnt.h from defining the
// TCHAR -derived types.  This is fundamentally broken so we just declare
// TCHAR derived types here as they are in <wxdebug.h>.
typedef TCHAR *PTCHAR;

// Triple Argh!!!  The following are to work around the fact dxtrans.h is not
// included in the latest DirectX SDKs, but is included in qedit.h.  The pragma
// avoids the need to comment the include of dxtrans.h from qedit.h.
#pragma include_alias( "dxtrans.h", "qedit.h" )
#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__
#include <qedit.h>

#include <atlbase.h>
#include <dshow.h>
#include <streams.h>
#include <comutil.h>

#define MIN_ALLOCATOR_BUFFERS_PER_CLIENT  3
#define DEF_CONCURRENT_CLIENTS            3

class rvDSCameraBuffer
{
public:
    unsigned int useCount;
    REFERENCE_TIME timeStamp;
    IMediaSample *pMediaSample;
};

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_CAMERA_IMAGE_READY, -1)
END_DECLARE_EVENT_TYPES()

class rvDSCamera : public ISampleGrabberCB, wxObject
{
public:
    //IUnknown interface
    HRESULT WINAPI QueryInterface( REFIID iid, void** ppvObject );
    ULONG WINAPI AddRef();
    ULONG WINAPI Release();

    // ISampleGrabberCB interfaces
    HRESULT WINAPI SampleCB(double SampleTime, IMediaSample *pSample);
    HRESULT WINAPI BufferCB(double sampleTimeSec, BYTE* bufferPtr, long bufferLength);

private:
    bool m_graphInitialized;
    unsigned int m_refCount;
    int m_imageWidth;
    int m_imageHeight;
    rvLinkedList *m_imageBufferList;
    wxEvtHandler *m_imageHandler;

public:
    rvDSCamera();
    ~rvDSCamera();

    // Camera setup.
    bool SetImageHandler(wxEvtHandler *imageHandler = NULL);
    bool EnableMemoryBuffer(unsigned int maxConcurrentClients = DEF_CONCURRENT_CLIENTS, unsigned int allocatorBuffersPerClient = MIN_ALLOCATOR_BUFFERS_PER_CLIENT);
    bool DisableMemoryBuffer();
    bool BuildGraph(int width, int height, double frameRate);
    bool ReleaseGraph();
    bool IsGraphInitialized();

    // Media control.
    bool Run();
    bool Pause();
    bool Stop(bool forcedStop = false);

    // Image buffer management.
    bool WaitForNextImage(long milliseconds);
    bool CheckoutIplImage(IplImage *image);
    bool CheckinIplImage(IplImage *image, bool forceRelease = false);

    // Camera properties.
    bool ShowPinProperties(HWND hWnd);
    bool ShowFilterProperties(HWND hWnd);

private:

    HANDLE m_sync;
    const LPTSTR m_syncName;
    CCritSec m_critSection;

    CComPtr <ICaptureGraphBuilder2> m_captureGraphBuilder;
    CComPtr <IGraphBuilder>         m_graphBuilder;
    CComPtr <IMediaControl>         m_mediaControl;
    CComPtr <IMediaEventEx>         m_mediaEvent;
    CComPtr <IMediaSeeking>         m_mediaSeeking;
    CComPtr <IAMCameraControl>      m_cameraControl;
    CComPtr <IAMDroppedFrames>      m_droppedFrames;
    CComPtr <IAMVideoControl>       m_videoControl;
    CComPtr <IAMVideoProcAmp>       m_videoProcAmp;
    CComPtr <IBaseFilter>           m_sourceFilter;
    CComPtr <IBaseFilter>           m_decoderFilter;
    CComPtr <IBaseFilter>           m_rendererFilter;
    CComPtr <IBaseFilter>           m_grabberFilter;
    CComPtr <ISampleGrabber>        m_sampleGrabber;
    CComPtr <IPin>                  m_capturePin;
};

#endif // _RV_DSCAMERA_H_
