//////////////////////////////////////////////////////////////
// NCVideoInput.cpp : Implementation of class CNCVideoInput;
//
// Copyright (c) 2003-2005, Tooltips.Net L.L.C.
// Design and implementation: Vitaly Tomilov
//
// Permission to use, copy, modify, but NOT distribute this software without
// fee is hereby granted, subject to the following terms and conditions also
// stated in the END USER LICENSE AGREEMENT for Neat-CPP class libraries
// Version 1.00;
//
// 1. Source Code of this software may only be used solely for internal
//    business needs including but not limited to preparation of derivative
//    works. Under no circumstances the Source Code, either in original or
//    modified form, may be distributed or transferred outside of the business
//    entity or individual person capacity to which this software was licensed.
//
// 2. This software and derivative works in its compiled (binary object code)
//    form may be distributed either commercially or in not-for-profit fashion
//    to any third party including retail, provided that its use in the
//    distributing software package is limited to accessory, secondary means
//    and neither constitute a main part not determine a purpose of such
//    distribution.
//
// 3. All copies of the software, derivative works or modified versions,
//    and any portions thereof, must include this entire copyright and
//    permission notice, without modification. The full notice must also
//    appear in supporting documentation
//
// Last updated: 29.01.2005;
//
// Library support web site: http://www.NeatCPP.com
//////////////////////////////////////////////////////////////


#include "stdafx.h"			// Precompiled header;
#include "NCVideoInput.h"	// Declaration of the class;
#include <atlbase.h>		// For Smart Pointers usage;


//////////////////////////
// Defining a safe way to
// release COM pointers:
//
#ifndef NC_SAFE_RELEASE
#define NC_SAFE_RELEASE(Ptr)\
	if(Ptr)\
    {\
		Ptr->Release( );\
		Ptr = NULL;\
    }
#endif


///////////////////////////////////////////////////////////////
// Implementation of struct CNCVideoInput::deviceDescr
///////////////////////////////////////////////////////////////


////////////////////////////////////////
// Default Constructor Implementation;
//
CNCVideoInput::deviceDescr::deviceDescr()
{
	::memset(this, 0, sizeof(deviceDescr));	// Reset all properties
}


/////////////////////////////////////////////
// Method GetName;
//
// If will return description of the device,
// if it is available, and if it is not - 
// friendly name of the device.
TCHAR * CNCVideoInput::deviceDescr::GetName()
{
	return m_strDescription?m_strDescription:m_strFriendlyName;
}


///////////////////////////////
// Destructor implementation;
//
CNCVideoInput::deviceDescr::~deviceDescr()
{
	////////////////////////////////
	// Releasing all text parameters
	// that were allocated:
	//
	if(m_strFriendlyName)
		delete []m_strFriendlyName;
	if(m_strDescription)
		delete []m_strDescription;
	if(m_strDevicePath)
		delete []m_strDevicePath;

	/////////////////////////////////////
	// Releasing the Video Input moniker:
	//
	NC_SAFE_RELEASE(m_pMoniker)
}


///////////////////////////////////////////////////////////////
// Implementation of class CNCVideoInput
///////////////////////////////////////////////////////////////


////////////////////////////////////////
// Constructor Implementation;
//
CNCVideoInput::CNCVideoInput(bool bSelectDefault)
{
	//////////////////////////////////////////////
	// Legal Notice:
	//
#ifdef _VI_DEMO
	::MessageBox(NULL, _T("NCVideoInput.lib is to be linked with demo projects only!\nFor commercial products you must purchase the source code\nfrom www.neatcpp.com"), _T("Legal Notice"), MB_OK|MB_ICONINFORMATION);
#endif
	//
	//////////////////////////////////////////////

	m_viState = vsStopped;					// Video is stopped
	::memset(&m_Size, 0, sizeof(m_Size));	// Reset size of the video

	m_hWnd = NULL;							// No window attached
	m_pBuffer = NULL;						// No buffer allocated
	m_pGrabber = NULL;						// No Grabber interface available
	m_pMC = NULL;							// No Media Control available
	m_pVW = NULL;							// No Video Window available
	m_pDevices = NULL;						// No devices found yet
	m_pExternal = NULL;						// No external device control

	m_nDevices = 0;							// No devices found
	m_nBufferLength = 0;					// No buffer allocated
	m_nBufferUsed = 0;						// No bytes used in the buffer
	m_nSelDevice = -1;						// No device selected
	m_bGrabNow = false;						// Do not grab any image frame

	// Creating event "GOT THE FRAME":
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	if(bSelectDefault)	// If required to select the default device;
	{
		if(EnumerateDevices())	// Enumerate all available Video-Input devices;
			SelectDevice(0);	// Select the first found device;
	}
}


///////////////////////////////
// Destructor Implementation;
//
CNCVideoInput::~CNCVideoInput()
{
	ReleaseInterfaces();		// Release all COM interfaces, if not released yet;
	::CloseHandle(m_hEvent);	// Destroy the event handler used for grabbing images;
}


////////////////////////////////////////////////////////////////////////////
// Implementation of COM methods of the class;
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// Method BufferCB;
//
// Implements a COM event handler called for each frame so we could be ready
// to grab any frame we want. The method will verify flag m_bGrabNow to know
// when some other thread is interested in grabbing the current frame. And just
// as the flag is set it will copy the current frame into the buffer, and
// signal back to the thread that wanted the image, so it knows that buffer
// already contains the current frame.
//
// Please note that on Windows 9x systems, you are not allowed to call most
// of the Windows API functions in this callback.  Why not?  Because the
// video renderer might hold the global Win16 lock so that the video surface
// can be locked while you copy its data. This is not an issue on Windows 2000/XP,
// but is a limitation on Win95,98,98SE, and ME. Calling a 16-bit legacy function
// could lock the system, because it would wait forever for the Win16 lock, which
// would be forever held by the video renderer.
//
STDMETHODIMP CNCVideoInput::BufferCB(double /*SampleTime*/, BYTE * pBuffer, long nBufferSize)
{
	if(m_bGrabNow)								// If requested to grab a frame;
	{
		m_nBufferUsed = 0;						// No bytes used in the buffer
		m_bGrabNow = false;						// Reset the flag;
		if(nBufferSize > m_nBufferLength)		// If buffer isn't large enough;
		{
			delete []m_pBuffer;					// Release the image buffer;
			m_pBuffer = new BYTE[nBufferSize];	// Allocate new memory for the buffer;
			if(m_pBuffer)						// If successfully allocated;
				m_nBufferLength = nBufferSize;	// Save actual size of the buffer;
		}
		if(m_pBuffer)							// If buffer was allocated successfully;
		{
			::memcpy(m_pBuffer, pBuffer, nBufferSize);	// Copy bytes into the buffer;
			m_nBufferUsed = nBufferSize;				// Save new buffer size;
		}
		else
			m_nBufferLength = 0;				// Set buffer size to 0;

		::SetEvent(m_hEvent);					// Signal event "GOT THE FRAME";
	}
	return S_OK;								// Return success;
}


///////////////////////////////////////////////////////
// Method SampleCB;
//
// This method needs implementation only when image
// frames are to be grabbed as samples, i.e. when method
// ISampleGrabber::SetCallback is called passing 0 as
// the second parameter. However, we are calling method
// ISampleGrabber::SetCallback passing 1 from within
// methods Attach() and Detach(), which means method
// SampleCB doesn't need any implementation.
STDMETHODIMP CNCVideoInput::SampleCB(double /*SampleTime*/, IMediaSample * /*pSample*/)
{
	return E_NOTIMPL;	// This method has no implementation;
}


///////////////////////////////////////////////////////
// Method QueryInterface;
//
// Implements access to interfaces IID_ISampleGrabberCB
// and IID_IUnknown.
// We do not increment reference to any of the returned
// interfaces, because their lifetime is limited by that
// of the class;
STDMETHODIMP CNCVideoInput::QueryInterface(REFIID riid, void ** ppv)
{
	if(riid == IID_ISampleGrabberCB || riid == IID_IUnknown)
	{
		*ppv = this;		// Set the interface pointer
		return S_OK;		// Return "Success"
	}
	if(riid == IID_IVideoWindow && m_pVW)
	{
		*ppv = m_pVW;		// Set the interface pointer
		return S_OK;		// Return "Success"
	}
	if(riid == IID_IMediaControl && m_pMC)
	{
		*ppv = m_pMC;		// Set the interface pointer
		return S_OK;		// Return "Success"
	}
	if(riid == IID_IAMExtDevice && m_pExternal)
	{
		*ppv = m_pExternal;	// Set the interface pointer
		return S_OK;		// Return "Success"
	}
	return E_NOINTERFACE;	// Return "No such interface supported"
}


////////////////////////////////////////////////////
// Method AddRef;
//
// Dummy implementation of reference incrementing.
// The actual implementation is not needed, because
// the object must have the same life length as the
// class implementing the method.
STDMETHODIMP_(ULONG)CNCVideoInput::AddRef()
{
	return 1;	// Just as long as it is not 0
}


////////////////////////////////////////////////////
// Method Release;
//
// Dummy implementation of reference decrementing.
// The actual implementation is not needed, because
// the object must have the same life length as the
// class implementing the method.
STDMETHODIMP_(ULONG)CNCVideoInput::Release()
{
	return 1;	// Just as long as it is not 0
}


////////////////////////////////////////////////////////////////////////////
// Implementation of C++ methods of the class;
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// Method EnumerateDevices;
//
// Initializes the list of devices with all available
// in the system that qualifies as a Video-Input device;
long CNCVideoInput::EnumerateDevices()
{
	// Release the previous list of devices:
	ReleaseDeviceList();

	// Create a System Device Enumerator:
	CComPtr<ICreateDevEnum> pSysDevEnum;
	if(pSysDevEnum.CoCreateInstance(CLSID_SystemDeviceEnum) != S_OK)
		return 0;

	// Obtain a class enumerator for the video capture category:
	CComPtr<IEnumMoniker> pEnumCat;
	if(pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0) != S_OK)
		return 0;

	IMoniker *pMoniker = NULL;	// Device moniker;
	deviceDescr * pInfo = NULL;	// Device descriptor pointer;
	while(pEnumCat->Next(1, &pMoniker, NULL) == S_OK)	// While found next device monkier;
	{
		IPropertyBag * pPropBag;

		// Getting access to the storage where the Property Bag resides:
		if(pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag)) != S_OK)
		{
			pMoniker->Release();// Release unsuccessful moniker;
			continue;			// Skip this moniker, maybe next one will work;
		}
		m_nDevices ++;			// Increment found devices;
		if(m_pDevices)			// If found at least one device;
		{
			pInfo->m_pNext = new deviceDescr;	// Attaching a new descriptor as the next element;
			pInfo = pInfo->m_pNext;				// Save pointer to the next element;
		}
		else	// This is the first Video-Input device found;
		{
			pInfo = new deviceDescr;	// New instance of device descriptor;
			m_pDevices = pInfo;			// Save directly into the list pointer;
		}

		/////////////////////////////////////////////
		// Retrieving all text information available
		// for video devices, i.e. "FriendlyName",
		// "Description" and "DevicePath":
		//
		TCHAR ** InfoParam[] = {&pInfo->m_strFriendlyName, &pInfo->m_strDescription, &pInfo->m_strDevicePath};
		LPCWSTR ParamNames[] = {L"FriendlyName", L"Description", L"DevicePath"};
		VARIANT varParam; // Variant to read in text strings;
		for(int i = 0;i < sizeof(ParamNames) / sizeof(TCHAR*);i ++)
		{
			::VariantInit(&varParam); // Initializing the variant;
			if(pPropBag->Read(ParamNames[i], &varParam, 0) == S_OK)	// If read the property successfully;
			{
				USES_CONVERSION;						// Allow valid W2T conversion;
				TCHAR * pText = W2T(varParam.bstrVal);	// Text string with property value;
				long L = (long)_tcslen(pText) + 1;		// String length + 1;
				*InfoParam[i] = new TCHAR[L];			// Allocate memory for the string;
				_tcsncpy(*InfoParam[i], pText, L);		// Copy the string;
				::VariantClear(&varParam);				// Free all variant buffers;
			}
		}
		pInfo->m_pMoniker = pMoniker;	// Save the moniker pointer into the current descriptor;
		pPropBag->Release();			// Release the Property Bag interface;
	}

	return m_nDevices;	// Return the number of found Video-Input devices;
}


///////////////////////////////////////////////////
// Method CountDevices;
//
// Simply returns the value of property m_nDevices;
long CNCVideoInput::CountDevices()
{
	return m_nDevices;
}


///////////////////////////////////////////////////
// Method SelectDevice;
//
// Creates a graph builder, filters, and builds the
// graph;
bool CNCVideoInput::SelectDevice(long nDeviceIndex)
{
	Detach(); // Detach from the window, if attached;


	deviceDescr * pInfo = GetDeviceDescr(nDeviceIndex);	// Retrieve device descriptor;
	if(!pInfo && nDeviceIndex != -1)	// If no device with such index found;
		return false;	// Return with error;


	//////////////////////////////////////////////////////////////////
	// Releasing all COM interfaces:
	//
	NC_SAFE_RELEASE(m_pExternal)

	// Releasing the two interfaces below results in releasing
	// whatever Video-Input device currently selected, i.e. the
	// device becomes immediately available right after both m_pMC
	// and m_pVW have been released:
	NC_SAFE_RELEASE(m_pVW)
	NC_SAFE_RELEASE(m_pMC)

	//////////////////////////////////////////////////////////////////
	// <<< The device is now free to be used by other applications >>>
	//////////////////////////////////////////////////////////////////

	NC_SAFE_RELEASE(m_pGrabber)
	//
	//////////////////////////////////////////////////////////////////


	m_nSelDevice = -1;		// Assume No Device selected when this method is done;

	if(nDeviceIndex == -1)	// If asked to deselect any current device;
		return true;		// Return success;


	//////////////////////////////////////////////////////////////
	// Creating a graph of filters. This is the key interface
	// that we use here. Creating it is always successful, unless
	// DirectX itself is unavailable, hence can be verified for
	// that here;
	CComPtr<IGraphBuilder> pGraph;
	if(pGraph.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC) != S_OK) // If failed to create;
		return false;	// DirectX 9.0 is not available on this PC; quit;


	///////////////////////////////////////////////////////////////
	// Binding moniker of the selected object with the base filter,
	// i.e. activating the object represented by the moniker;
	//
	CComPtr<IBaseFilter> pFilter;
	if(pInfo->m_pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pFilter) != S_OK)
		return false;


	//////////////////////////////////////////////
	// Adding the capture filter to the graph:
	//
	if(pGraph->AddFilter(pFilter, L"Cap") != S_OK)	// If couldn't put capture device in graph;
		return false;	// Return with error;


	//////////////////////////////////////////////
    // Creating a sample grabber and querying for
	// interface ISampleGrabber:
	//
	CComPtr<IBaseFilter> pGrabberFilter;
	if(pGrabberFilter.CoCreateInstance(CLSID_SampleGrabber) != S_OK)
		return false; // Couldn't create SampleGrabber (is qedit.dll registered?)
	pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&m_pGrabber); // Getting ISampleGrabber;
	//
	//////////////////////////////////////////////
	

	/////////////////////////////////////////////
	// Set the media type based on the bit depth
	// of the primary display:
	//
	AM_MEDIA_TYPE mt;							// Media Type structure;
	::ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));	// Reset values in mt;
	mt.majortype = MEDIATYPE_Video;				// Use type Video;

	/////////////////////////////////
	// Finding the current bit depth:
	//
	HDC hDC = ::GetDC(NULL);							// Device Context of the primary display;
	int iBitDepth = ::GetDeviceCaps(hDC, BITSPIXEL);	// The number of bits used;
	::ReleaseDC(NULL, hDC);								// Release the Device Context;

	////////////////////////////////////////
	// Specifying subtype for the video,
	// depending on the number of color bits
	// used on the primary display:
	//
	switch(iBitDepth)
	{
	case 8:
		{
			mt.subtype = MEDIASUBTYPE_RGB8;
			break;
		}
	case 16:
		{
			mt.subtype = MEDIASUBTYPE_RGB555;
			break;
		}
	case 24:
		{
			mt.subtype = MEDIASUBTYPE_RGB24;
			break;
		}
	case 32:
		{
			mt.subtype = MEDIASUBTYPE_RGB32;
			break;
		}
	default:
		return false;
	}


	////////////////////////////////////////////////
	// Specifying the media type for the connection
	// on the input pin of the Sample Grabber:
	//
	m_pGrabber->SetMediaType(&mt);


	///////////////////////////////////////////////
	// Adding the grabber to the graph;
	//
	if(pGraph->AddFilter(pGrabberFilter, L"Grabber" ) != S_OK)
		return false; // Couldn't put sample grabber in graph;


	//////////////////////////////////////////
	// Now building the graph itself with the
	// help of interface ICaptureGraphBuilder2:
	//
	CComPtr<ICaptureGraphBuilder2> pCGB2;
	if(pCGB2.CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC) != S_OK)
		return false; // Can't get a ICaptureGraphBuilder2 reference;
	
	if(pCGB2->SetFiltergraph(pGraph) != S_OK)
		return false; // SetGraph failed;


	////////////////////////////////////////////////////
    // If there is a VP pin present on the video device,
	// then put the renderer on CLSID_NullRenderer:
	//
	CComPtr<IPin> pVPPin;
	HRESULT hr = pCGB2->FindPin(pFilter, PINDIR_OUTPUT, &PIN_CATEGORY_VIDEOPORT, NULL, FALSE, 0, &pVPPin);


	///////////////////////////////////////////////////////////
    // If there is a VP pin, put the renderer on NULL Renderer:
	//
    CComPtr<IBaseFilter> pRenderer;
    if(hr == S_OK) // If found the pin with the previous call;
    {
        hr = pRenderer.CoCreateInstance(CLSID_NullRenderer);
        if(hr != S_OK) // If unable to make a NULL renderer;
			return false; // Return with error;

		if(FAILED(pGraph->AddFilter(pRenderer, L"NULL renderer")))
			return false; // Can't add the filter to graph;
    }


	//////////////////////////////////////////////////////////////////
	// This is the point of the actual device activation, which takes
	// approximately 99.9% of time required for the whole method to
	// execute; This is because it requires appropriate device drivers
	// to load, initialize and start working;
	// All steps followed here are consistent with Microsoft examples
	// for searching appropriate category where the pin can be rendered,
	// and then rendering it;
	//
	// Firstly, trying to render the interleaved video pin,
	// i.e. used for Digital Video (DV):
	hr = pCGB2->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Interleaved, pFilter, pGrabberFilter, pRenderer);
	if(FAILED(hr)) // If failed to render interleaved video;
	{
		// Then try to render as media under the Preview category:
		hr = pCGB2->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pFilter, pGrabberFilter, pRenderer);

		if(FAILED(hr)) // If failed as well;
			// Then try render under the Capture category:
			hr = pCGB2->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pFilter, pGrabberFilter, pRenderer);
    }
	if(FAILED(hr)) // If failed to render pins under all categories;
		return false; // Can't build the graph; Return with error;
	//
	////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////
    // Asking for the connection media type to find out
	// what resolution is set in our Video-Input device:
    //
	m_pGrabber->GetConnectedMediaType(&mt); // It shouldn't fail here;
	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*)mt.pbFormat;
	m_Size.cx = vih->bmiHeader.biWidth;
	m_Size.cy = vih->bmiHeader.biHeight;
	// Freeing the media structure:
	if(mt.cbFormat)
		::CoTaskMemFree((PVOID)mt.pbFormat);
	if(mt.pUnk)
		mt.pUnk->Release();
	//
	//////////////////////////////////////////////////////


	/////////////////////////////////////////////////
    // Don't buffer the samples as they pass through:
	m_pGrabber->SetBufferSamples(FALSE); 


	/////////////////////////////////////////////////
    // Grab only one at a time, and stop stream after
    // grabbing one sample:
    m_pGrabber->SetOneShot(FALSE);


	m_nSelDevice = nDeviceIndex;	// Store selected device index;


	/////////////////////////////////////////////
	// Retrieving pointers for Media Control and
	// Video Window interfaces;
	//
	// This code cannot fail once all the above
	// was successful; no verification is needed;
	//
	pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMC);
	pGraph->QueryInterface(IID_IVideoWindow, (void**)&m_pVW);


	///////////////////////////////////////////////////////////////
	// Finding Device Controlling Interface (Works for VCR-s only):
	//
	pCGB2->FindInterface(&PIN_CATEGORY_VIDEOPORT, &MEDIATYPE_Video, pFilter, IID_IAMExtDevice, (void**)&m_pExternal);


	return true;	// Return Success;
}


/////////////////////////////////////////////////////////
// Method GetSelected;
//
// Simply returns the value of property m_nSelDevice;
long CNCVideoInput::GetSelected()
{
	return m_nSelDevice;
}


///////////////////////////////////////////////////
// Method ReleaseInterfaces;
//
// Detaches from Video Window, and releases all COM
// interfaces and memory buffers;
void CNCVideoInput::ReleaseInterfaces()
{
	Detach();				// Detach Video Window;

	ReleaseDeviceList();	// Release all device descriptors;

	if(m_pBuffer)			// If Image Grab buffer was used once;
	{
		delete []m_pBuffer;	// Delete the buffer;
		m_pBuffer = NULL;	// Reset the buffer pointer;
	}


	////////////////////////////////
	// Resetting buffer parameters:
	//
	m_nBufferLength = 0;	// No buffer allocated;
	m_nBufferUsed = 0;		// No bytes in the buffer used;

	m_bGrabNow = false;						// No image grabbing anymore;

	m_nSelDevice = -1;						// No device selected;
	
	::memset(&m_Size, 0, sizeof(m_Size));	// Device resolution is no longer known;

	//////////////////////////////////////////////////////////////////
	// Releasing all COM interfaces:
	//
	NC_SAFE_RELEASE(m_pExternal)

	// Releasing the two interfaces below results in releasing
	// whatever Video-Input device currently selected, i.e. the
	// device becomes immediately available right after both m_pMC
	// and m_pVW have been released:
	NC_SAFE_RELEASE(m_pVW)
	NC_SAFE_RELEASE(m_pMC)

	//////////////////////////////////////////////////////////////////
	// <<< The device is now free to be used by other applications >>>
	//////////////////////////////////////////////////////////////////

	NC_SAFE_RELEASE(m_pGrabber)
	//
	//////////////////////////////////////////////////////////////////
}


/////////////////////////////////////////////////
// Method Attach;
//
// - Sets a new owner for Video Window;
// - Repositions Video Window and sets new style;
// - Sets message forwarding from Video Window;
// - Makes Video Window visible;
bool CNCVideoInput::Attach(HWND hWnd, viClientPos clientPos, HWND hMessageDrain)
{
	////////////////////////////////////////////////////////
	// Already attached, wrong handle or no device selected:
	//
	if(m_hWnd || !::IsWindow(hWnd) || m_nSelDevice == -1)
		return false;	// Return with error;

	/////////////////////////////////////////////////
	// Setting new owner of the Video Window control:
	//
	if(m_pVW->put_Owner((OAHWND)hWnd) != S_OK) // If failed to set new owner;
		return false;	// Return with error;

	m_hWnd = hWnd;	// Save Window Handle of the control
	

	///////////////////////////////////////////////////
	// Setting default Video Window size/position;
	//
	// It is advisable to do here for best performance,
	// because below we make the Video Window visible,
	// which means any next sizing and positioning will
	// happen on a visible window, which may produce
	// slight flickering on slow PC-s;
	//
	SetVideoWindowPos(clientPos);

	
	///////////////////////////////////////////////
	// Forwarding all messages from Video Input to
	// another window;
	// 
	HWND hDrainWnd = hMessageDrain;				// Assume hDrainWnd is a valid window handle;
	if(!::IsWindow(hDrainWnd))					// If it is not;
	{
		hDrainWnd = ::GetParent(m_hWnd);		// Assume it has a valid parent window;
		if(!::IsWindow(hDrainWnd))				// If it doesn't;
			hDrainWnd = hWnd;					// Use the same window;
	}
	m_pVW->put_MessageDrain((OAHWND)hDrainWnd);	// Start forwarding events;
	//
	///////////////////////////////////////////////
	

	/////////////////////////////////////////////////
	// By default, Video Window is top-level, and
	// without any parent; Here we are changing style
	// of Video Window to be a child control with
	// video rendering clipped by dimensions of the
	// containing window:
	//
	m_pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);

	m_pVW->put_Visible(OATRUE); // Making Video Window visible;

	
	////////////////////////////////////////////
	// Initiate callbacks to be made into method
	// BufferCB, so we could grab an image frame
	// at any moment:
	//
	m_pGrabber->SetCallback(this, 1);

	return true;	// Return success;
}


////////////////////////////////////
// Method Detach;
//
// Stops any playback, and detaches
// Video Window from the containing
// control;
HWND CNCVideoInput::Detach()
{
	if(!IsAttached())	// If not attached;
		return NULL;	// There's nothing to detach; return with error;

	
	/////////////////////////////////////////////////
	// We try to stop any video playback, and update
	// the video input, as sometimes it may leave
	// unupdated areas:
	if(!Stop())	// If didn't stop any playback;
		::RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);	// Update the client's area;


	m_pVW->put_Visible(OAFALSE);	// Hide Video Window;

	
	//////////////////////////////////////////////////////////
	// Removing owner of Video Window, i.e. releasing it from
	// being a child control of the window to which we attached
	// the Video Window;
	// This is a tricky action, as changing owner of the window
	// often results in showing the window temporarily, and then
	// switching input focus onto it.
	// We must make sure that when this happens we regain the
	// input focus to the control which had it before the
	// focus was lost:
	//
	HWND hActiveWnd = ::GetActiveWindow();	// Remember currently active window;
	m_pVW->put_Owner((OAHWND)NULL);			// Remove owner of Video Window;
	if(::GetActiveWindow() != hActiveWnd)	// If previous focus is lost;
		SetActiveWindow(hActiveWnd);		// Restore the input focus;
	//
	//////////////////////////////////////////////////////////

	m_pGrabber->SetCallback(NULL, 1); // Cancel all callbacks;

	HWND hWnd = m_hWnd;	// Return result;

	m_hWnd = NULL;		// Reset window handle;

	return hWnd;		// Return previously attached window;
}


///////////////////////////////////
// Method IsAttached;
//
// Returns the Attach status based
// on property m_hWnd;
bool CNCVideoInput::IsAttached()
{
	return (m_hWnd != NULL); // Attached, if m_hWnd is not NULL;
}


//////////////////////////////////////////////////
// Method GetStatus;
//
// Simply returns the value of property m_viState;
CNCVideoInput::viState CNCVideoInput::GetState()
{
	return m_viState;
}


///////////////////////////////////
// Method Run;
//
// Starts playback of the selected 
// Video-Input device;
bool CNCVideoInput::Run()
{
	////////////////////////////////////////////
	// Must be attached, and not yet playing:
	//
	if(IsAttached() && (m_viState != vsRunning))
	{
		m_pMC->Run();			// Start playback;
		m_viState = vsRunning;	// Changing status to "RUNNING PLAYBACK";
		return true;			// Return success;
	}
	return false;				// Return failure;
}


//////////////////////////////////////////////
// Method Stop;
//
// Stops any playback;
bool CNCVideoInput::Stop(bool bStopWhenReady)
{
	if(IsAttached() && m_viState != vsStopped)	// If valid state;
	{
		if(bStopWhenReady)						// If requested to stop only when ready;
			m_pMC->StopWhenReady();				// Stop When Ready;
		else
			m_pMC->Stop();						// Stop immediately;
		
		m_viState = vsStopped;					// Set new state;
		
		::RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);	// Update Video Window;
		
		return true;	// Return Success;
	}

    return false;		// Return Failure;
}


////////////////////////////////
// Method Pause;
//
// Pauses the current playback;
bool CNCVideoInput::Pause()
{
	if(IsAttached() && m_viState == vsRunning) // If valid state;
	{
		m_pMC->Pause();			// Pause the video;
		m_viState = vsPaused;	// Change state;
		return true;			// Return Success;
	}
    return false;				// Return Failure;
}


//////////////////////////////////////
// Method GetResolution;
//
// Simply returns the value of m_Size;
SIZE CNCVideoInput::GetResolution()
{
	return m_Size;
}


///////////////////////////////////////////////
// Method SetVideoWindowPos;
//
// Adjusts size and position of the Video-Input
// window control to best fit the containing
// control;
bool CNCVideoInput::SetVideoWindowPos(viClientPos clientPos, LPRECT lpRect)
{
	if(!IsAttached())	// Must be attached;
		return false;


	bool bResult = true;	// Return result;


	///////////////////////////////////////////
	// Get dimensions of the containing client:
	//
    RECT rc;
	::GetClientRect(m_hWnd, &rc);


	////////////////////////////////////////////
	// Adjust depending on parameter clientPos;
	// See in the declaration of viClientPos what
	// is done in each case;
	//
	switch(clientPos)
	{
	case posDefault:
		{
			m_pVW->put_Left(0);
			m_pVW->put_Top(0);
			break;
		}
	case posStretchWithRatio:
	case posKeepWithRatio:
		{
			if(clientPos == posKeepWithRatio && m_Size.cx <= rc.right && m_Size.cy <= rc.bottom)
			{
				m_pVW->SetWindowPosition(m_Size.cx > rc.right?0:((rc.right - m_Size.cx)/2), m_Size.cy > rc.bottom?0:((rc.bottom - m_Size.cy)/2), m_Size.cx, m_Size.cy);
				break;
			}
			double dRatio = max((double)m_Size.cx/rc.right, (double)m_Size.cy/rc.bottom);
			double x, y, cx, cy;
			if((double)m_Size.cx/rc.right > (double)m_Size.cy/rc.bottom)
			{
				x = 0;
				cx = rc.right;
				cy = m_Size.cy / dRatio;
				y = (rc.bottom - cy)/2;
			}
			else
			{
				y = 0;
				cy = rc.bottom;
				cx = m_Size.cx / dRatio;
				x = (rc.right - cx)/2;
			}
#define DoubleToLong(A)	(A - (long)A >= 0.4445)?((long)A + 1):(long)A
			m_pVW->SetWindowPosition(DoubleToLong(x), DoubleToLong(y), DoubleToLong(cx), DoubleToLong(cy));
#undef DoubleToLong
			break;
		}
	case posKeepSize:
		{
			m_pVW->SetWindowPosition(m_Size.cx > rc.right?0:((rc.right - m_Size.cx)/2), m_Size.cy > rc.bottom?0:((rc.bottom - m_Size.cy)/2), m_Size.cx, m_Size.cy);
			break;
		}
	case posStretchNoRatio:
		{
			m_pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
			break;
		}
	default:
		{
			// Unknown type;
			bResult = false;
			break;
		}
	}

	return bResult; // Return result;
}


//////////////////////////////////////////////////////////////////////
// Method GrabCurrentFrame;
//
// If the video is being previewed, this method sets "GRAB FRAME" flag
// and waits till event "GOT THE FRAME" arrives.
// After that the method simply converts the frame into whatever output
// format is requested, and sets the size of the frame in bytes,
// if grabDest is not grabIntoBitmap.
LPVOID CNCVideoInput::GrabCurrentFrame(viGrabDest grabDest, long * nSize)
{
	if(!m_hEvent || m_viState != vsRunning)	// If the graph is not running,
											// or failed to create event
		return NULL;						// Can't grab a frame now, return;

	::ResetEvent(m_hEvent);		// Set flag "GOT THE FRAME" to FALSE

	m_bGrabNow = true;			// Set flag "GRAB FRAME" ON

	//////////////////////////////////////////////
	// Doing a reasonable calculation here of what
	// should be the maximum time-out for grabbing
	// the frame, depending on the video-input
	// resolution.
	long nWait = m_Size.cx * m_Size.cy / 100; // Time-out in milliseconds

	///////////////////////////////////////////////////
	// Wait for the frame to arrive in method BufferCB,
	// and indicate "GOT THE FRAME" for us...
	if(::WaitForSingleObject(m_hEvent, nWait) == WAIT_TIMEOUT) // If timed-out
	{
		m_bGrabNow = false; // Set flag "GRAB FRAME" OFF to prevent
							// any further attempt to grab a new frame;
		return NULL;		// Return error;
	}

	if(nSize)		// If a valid pointer was passed;
		*nSize = 0;	// Reset the size parameter;
	
	////////////////////////////////////////////////
	// The frame was grabbed successfully, so now we
	// can process the frame buffer here:
	//
	BITMAPINFOHEADER bih;
	memset(&bih, 0, sizeof(bih));
	bih.biSize = sizeof(bih);
	bih.biWidth = m_Size.cx;	// Video Input width;
	bih.biHeight = m_Size.cy;	// Video Input height;
	bih.biPlanes = 1;			// Always fixed (See MSDN);

	HDC hDC = ::GetDC(NULL);	// Display Device Context;
	
	///////////////////////////////////////////////
	// Since we select the same number of bits for
	// rendering as available in the display, we
	// specify the same number of bits here:
	bih.biBitCount = (USHORT)::GetDeviceCaps(hDC, BITSPIXEL);

	LPVOID lpResult = NULL;	// Return result;

	if(grabDest == grabIntoBitmap)
	{
		/////////////////////////////////////////////////////////
		// Creating a bitmap in the Device Context of the Display,
		// which guarantees the fastest possible drawing speed
		// of such bitmap on the screen, as no middle-layer
		// transformation is required;
		lpResult = ::CreateDIBitmap(hDC, &bih, CBM_INIT, m_pBuffer, (BITMAPINFO*)&bih, DIB_RGB_COLORS);
	}
	else // Grabbing into a COM object
	{
		/////////////////////////////////
		// Constructing bitmap in memory:
		//
		BITMAPFILEHEADER bfh;
		::memset(&bfh, 0, sizeof(bfh));
		bfh.bfType = 'MB'; // Inverted bytes for BMP file header;
		bfh.bfSize = sizeof(bfh) + m_nBufferUsed + sizeof(BITMAPINFOHEADER);
		bfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
		
		// Calculating total length of the required buffer:
		long nTotalSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m_nBufferUsed;

		if(nSize)					// If a valid pointer was passed
			*nSize = nTotalSize;	// Set the number of bytes
		
		// Allocating memory:
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, nTotalSize);

		// Locking the global memory for direct access:
		LPBYTE pData = (LPBYTE)::GlobalLock(hGlobal);

		// Copying Bitmap File Header into the buffer:
		::memcpy(pData, &bfh, sizeof(BITMAPFILEHEADER));

		// Incrementing the buffer pointer:
		pData += sizeof(BITMAPFILEHEADER);

		// Copying Bitmap Header Information into the buffer:
		::memcpy(pData, &bih, sizeof(BITMAPINFOHEADER));

		// Incrementing the buffer pointer:
		pData += sizeof(BITMAPINFOHEADER);

		// Copying the bitmap color bits into the buffer:
		::memcpy(pData, m_pBuffer, m_nBufferUsed);
		
		// Unlocking the global memory buffer:
		::GlobalUnlock(hGlobal);

		IStream * pStream = NULL;	// IStream pointer;
		if(::CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK) // If created stream successfully;
		{
			switch(grabDest)
			{
			case grabIntoStream:		// Interface IStream is required
				{
					lpResult = pStream;	// Just set the return value as is;
					break;
				}
			case grabIntoSequential:	// Interface IStreamSequential is required
				{
					// Simply querying for the base interface:
					pStream->QueryInterface(IID_ISequentialStream, &lpResult);
					break;
				}
			case grabIntoPicture:		// Interface IPicture is required
				{
					::OleLoadPicture(pStream, nTotalSize, FALSE, IID_IPicture, (void**)&lpResult);
					break;
				}
			default:
				break;
			}
			
			///////////////////////////////////////////
			// If failed to get the required interface,
			// or IStream is no longer needed:
			//
			if(!lpResult || grabDest != grabIntoStream)
				pStream->Release();	// Release the IStream instance;
		}
		else
			::GlobalFree(hGlobal);	// Release global memory, if failed;
	}

	::ReleaseDC(NULL, hDC);	// Release Display Device Context;

	return lpResult;	// Return the result.
}


//////////////////////////////////
// Method GetDeviceDescr;
//
// Looks up device descriptor with
// specified index;
CNCVideoInput::deviceDescr * CNCVideoInput::GetDeviceDescr(long nDeviceIndex)
{
	if(nDeviceIndex >= m_nDevices || nDeviceIndex < 0) // If the index is within the range;
		return NULL;	// Return with error;

	////////////////////////////
	// Looking for the device
	// with the specified index:
	//
	deviceDescr * pInfo = m_pDevices;
	while(nDeviceIndex--)
		pInfo = pInfo->m_pNext;
	
	return pInfo;	// Return the device descriptor pointer;
}

/////////////////////////////////////
// Method GetVideoWindow;
//
// Simply returns the value of m_pVW;
IVideoWindow * CNCVideoInput::GetVideoWindow()
{
	return m_pVW;
}


/////////////////////////////////////
// Method GetMediaControl;
//
// Simply returns the value of m_pMC;
IMediaControl * CNCVideoInput::GetMediaControl()
{
	return m_pMC;
}


///////////////////////////////////////////
// Method GetExternalDevice;
//
// Simply returns the value of m_pExternal;
IAMExtDevice * CNCVideoInput::GetExternalDevice()
{
	return m_pExternal;
}


//////////////////////////////////////////////////////
// Implementation of private class methods:
//////////////////////////////////////////////////////


///////////////////////////////////
// Method ReleaseDeviceList;
//
// Releases all device descriptors;
void CNCVideoInput::ReleaseDeviceList()
{
	m_nDevices = 0;		// No devices;
	while(m_pDevices)	// If there are devices in the list;
	{
		deviceDescr * pNext = m_pDevices->m_pNext;	// Get the next device in the list;
		delete m_pDevices;							// Delete the device descriptor;
		m_pDevices = pNext;							// Assume the next pointer in the list;
	}
}
