////////////////////////////////////////////////////////////
// NCVideoInput.h : Declaration of class CNCVideoInput;
//
// Synopsis:
//
// This class is designed to provide simple interface for
// previewing video from any Video-Input device available in
// the system, such as VCR or a Web Camera, plus grabbing a
// still image frame at any given moment. Implementation is
// based on the Microsoft® DirectShow® platform as part of
// Microsoft® DirectX® 9.0;
//
// System requirements: Windows 98/Me/2000/XP, DirectX® 9.0;
//
// Supported compilation environments:
// - VS6 + DirectX 9.0 SDK (Prior to Summer 2004);
// - VS.NET 2003 + DirectX 9.0 SDK (Summer 2004 and on);
//
// Copyright (c) 2003-2005, Tooltips.Net L.L.C.
// Design and implementation: Vitaly Tomilov;
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


#ifndef NC_VIDEO_INPUT
#define NC_VIDEO_INPUT


/////////////////////////////////
// Standard DirectShow® includes:
//
#include <dshow.h>

#include <qedit.h>	// Declarations of all COM interfaces implemented in the
					// qedit.dll in-process COM server;
					// You might need to redefine all necessary includes from
					// this file instead of including the file itself in case
					// your code is also using Microsoft® Direct3D® headers later
					// than version 7, as the two are reportedly conflicting.


////////////////////////////////////////////
// class CNCVideoInput;
//
// Derivation from ISampleGrabberCB allows
// grabbing image frames from video inputs.
class CNCVideoInput:private ISampleGrabberCB
{
private:


	///////////////////////////////////////////////
	// COM Methods implemented by ISampleGrabberCB;
	///////////////////////////////////////////////


	/////////////////////////////////
	// Method AddRef;
	//
	// Implements reference increment
	// to the interface;
    STDMETHODIMP_(ULONG)AddRef();


	/////////////////////////////////
	// Method Release;
	//
	// Implements reference decrement
	// to the interface;
    STDMETHODIMP_(ULONG)Release();


	//////////////////////////////////////////////////////////
	// Method QueryInterface;
	//
	// Implements retrieval of supported interface pointers:
	//
	// - IUnknown;
	// - ISampleGrabberCB;
	// - IVideoWindow;
	// - IMediaControl;
	// - IAMExtDevice;
	//
	// You can pass pointer to this class into any COM server,
	// both in-process and out-of-process. However, since the
	// lifetime of this COM interface is the same as that of the
	// class, it doesn't implement the actual reference counting,
	// which means the external module should never try to keep
	// the interface pointer it uses; It must request the interface
	// just before using it, use it, and then release immediately.
	// It also would be a good practice to implement additional
	// synchronization with lifetime of the class for better
	// reliability;
	// You can pass pointer to this class as type IUnknown* or
	// ISampleGrabberCB*, and the external module will be able to
	// query the other interfaces as it needs them;
	//
	// Because such interfaces as IVideoWindow, IMediaControl and
	// IAMExtDevice are client interfaces, requesting them will
	// return E_NOINTERFACE when they are not currently available.
    STDMETHODIMP QueryInterface(REFIID /*riid*/, void ** /*ppv*/);


	//////////////////////////////////////////
	// Method SampleCB;
	//
	// Grabs image frames using image samples
	// via interface IMediaSample;
	//
	// Parameters ////////////////////////////
	//
	// SampleTime - exact time when the frame
	// was read from the video stream;
	//
	// pSample - pointer to the sample interface
	// that contains the image frame.
    STDMETHODIMP SampleCB(double /*SampleTime*/, IMediaSample * /*pSample*/);


	/////////////////////////////////////////
	// Method BufferCB;
	//
	// Grabs an image frame as a sequence of
	// bytes.
	//
	// Parameters ////////////////////////////
	//
	// SampleTime - exact time when the frame
	// was read from the video stream;
	//
	// pBuffer - pointer to the image buffer;
	//
	// nBufferSize - size of the image buffer.
    STDMETHODIMP BufferCB(double /*SampleTime*/, BYTE * /*pBuffer*/, long /*nBufferSize*/);


public:


///////////////////////////////////////
// struct deviceDescr;
//
// Contains all available information
// about each Video-Input device found
// in the system.
struct deviceDescr
{
public:

	////////////////////////////////////////////
	// Method GetName;
	//
	// Returns best name to be displayed in the
	// application UI for the device;
	// If none available (unlikely), it returns
	// NULL.
	TCHAR * GetName();


    ////////////////////////////////////////////
	// Property m_strFriendlyName;
	//
	// Available for every device, it contains
	// human-readable name for the device, or
	// NULL, if failed to retrieve (unlikely).
	TCHAR * m_strFriendlyName;


	///////////////////////////////////////////
	// Property m_strDescription;
	//
	// Available only for DV and D-VHS/MPEG
	// camcorder devices.
	// It contains a description of the device
	// which is more specific than property
	// m_strFriendlyName.
	// When unavailable, this property contains
	// NULL.
	TCHAR * m_strDescription;


	///////////////////////////////////////////
	// Property m_strDevicePath;
	//
	// Not a human-readable string, but is
	// guaranteed to be unique per device. You
	// can use this property to distinguish
	// between two or more instances of the
	// same model of device.
	// When unavailable, this property contains
	// NULL (unlikely).
	TCHAR * m_strDevicePath;

	friend class CNCVideoInput;	// Only class CNCVideoInput should control
								// instantiation of type deviceDescr.

private:


	////////////////////////
	// Default Constructor;
	//
	deviceDescr();

	
	////////////////////////
	// Destructor;
	//
	~deviceDescr();


///////////////////
// Properties:
///////////////////


	deviceDescr * m_pNext;	// Pointer to the next instance of deviceDescr in
							// the list, or NULL, if this is the last element.

	IMoniker * m_pMoniker;	// Moniker capable of binding to the filter that
							// represents this device.
};


public:


/////////////////////////////////////////////////////////////////////
// enum viGrabDest;
//
// Defines all supported ways of grabbing an image frame.
// See method GrabCurrentFrame.
enum viGrabDest
{
	grabIntoBitmap = 0,		// Grab frame as a device-independent bitmap,
							// compatible with the primary display;
	grabIntoStream = 1,		// Grab frame as IStream with DI bitmap in it;
	grabIntoSequential = 2,	// Grab frame as ISequentialStream with DI bitmap in it;
	grabIntoPicture = 3,	// Grab frame as IPicture with DI bitmap in it;
};


/////////////////////////////////////////////////////////////////////
// enum viState;
//
// Defines current state of the selected Video-Input device.
enum viState
{
	vsRunning = 0,	// Video-Input is in the preview mode;
	vsStopped = 1,	// Video-Input is stopped;
	vsPaused = 2	// Video-Input rendering has been paused;
};


//////////////////////////////////////////////////////////////////////
// enum viClientPos;
//
// Describes all possible ways of adjusting dimensions of the Video
// Window within the client area. It is to be used primarily with
// function SetVideoWindowPos(...)
//////////////////////////////////////////////////////////////////////
// Notes:
//
// 1. Dimension of Video-Input refers herein to physical resolution
//    currently set in your Video-Input Device, while dimension of Video
//    Window refers to the size of a DirectShow control in which video
//    is being rendered.
// 2. Adjusting size and proportion of the Video Window to anything
//    different from the Video-Input resolution may slow down the
//    rendering process, and increase the CPU consumption. Therefore,
//    the preference should be given to parameter posKeepSize, whilst
//    adjusting the client area to fit the Video-Input resolution
//    where necessary.
//    Alternatively, implement your own positioning and resizing based
//    on the values returned by methods GetMinIdealImageSize and
//    GetMaxIdealImageSize of interface IVideoWindow, available through
//    method CNCVideoInput::GetVideoWindow();
// 3. Current resolution of the selected Video-Input Device can always
//    be retrieved via function GetResolution().
enum viClientPos
{
	posDefault			= 0,	// Position Video Window in the left top corner
								// of the client area, and do not resize it.
								// Note: Even dimension of the Video Window
								// is not adjusted to match the Video-Input
								// resolution, so they can be different.

	posStretchNoRatio	= 1,	// Stretch Video Window over entire client area,
								// ignoring proportions of the original Video-Input.

	posStretchWithRatio	= 2,	// Adjust Video Window to fit the client's area, while
								// keeping the proportions of the original Video-Input.
								// Also align Video Window in the middle of the client.

	posKeepWithRatio	= 3,	// If the Video-Input fits into the client's area, the
								// Video Window will keep its original size, and be
								// aligned in the middle of the client. But if it doesn't,
								// the Video Window will be resized to fit the client,
								// while keeping the Video-Input ratio.
			
	posKeepSize			= 4		// Never resize the Video Window, even if it is larger than
								// the client area. But if Video Window fits into the client,
								// it will be aligned in the middle.
};


public:


	///////////////////////////////////////////
	// Initialization Constructor;
	//
	// Parameters: ////////////////////////////
	//
	// bSelectDefault - tells the constructor
	// to initialize all COM interfaces, search
	// for Video-Input devices, and select the
	// the first found device.
	//
	// Note: Although available as an option,
	// it is recommended to avoid such way of
	// video initialization, unless memory for
	// the class is allocated manually.
	CNCVideoInput(bool bSelectDefault = false);

	
	/////////////////////////
	// Virtual Destructor;
	//
	virtual ~CNCVideoInput();


///////////////////
// Methods:
///////////////////


	/////////////////////////////////////////////
	// Method EnumerateDevices;
	//
	// Updates the list of all available
	// Video-Input devices in the system.
	//
	// Returns: /////////////////////////////////
	//
	// Total number of found Video-Input devices.
	// It returns 0 when failed or no Video-Input
	// devices found.
	long EnumerateDevices();


	////////////////////////////////////////////
	// Method CountDevices;
	//
	// Returns total number of Video-Input
	// devices found in the system with the last
	// invocation of method EnumerateDevices();
	long CountDevices();


	/////////////////////////////////////////////////
	// Method SelectDevice;
	//
	// Selects and activates a video-input device
	// with the specified index. It also calls
	// method Detach() in the beginning;
	//
	// Parameters: //////////////////////////////////
	//
	// nDeviceIndex - index of the device to be
	// selected. It must be within the range of
	// [0, CountDevices() - 1]; If nDeviceIndex is -1,
	// the method deselects the currently selected
	// device.
	//
	// Returns: /////////////////////////////////////
	//
	// true, if selected successfully;
	//
	/////////////////////////////////////////////////
	// NOTE:
	//
	// Although it doesn't consume much of the CPU
	// resources, selection of a Video-Input device
	// is a time-consuming procedure, can take couple
	// seconds or even more, depending on the PC
	// configuration;
	//
	// Comparative example:
	//
	// 2.8GhzP4,DDR400,Logitech WebCam - 2 seconds;
	bool SelectDevice(long nDeviceIndex);


	///////////////////////////////////////////////
	// Method GetSelected;
	//
	// Returns index of the currently selected
	// Video-Input device, or -1, if none selected;
	long GetSelected();


	/////////////////////////////////////////////
	// Method ReleaseInterfaces;
	//
	// Calls method Detach(), and then releases
	// all COM interfaces and buffers created
	// for the class. It is called automatically
	// from destructor, leaving however the
	// option to uninitialize the class whenever
	// needed.
	//
	// NOTE: ///////////////////////////////////
	//
	// The class acquires the state of newly
	// created, i.e. you can then call method
	// EnumerateDevices, and so on as usual.
	void ReleaseInterfaces();


	//////////////////////////////////////////////////
	// Method Attach;
	//
	// Attaches video rendering stream to the
	// specified window.
	//
	// Parameters: ///////////////////////////////////
	//
	// hWnd - handle of the window that expects video
	// stream to be rendered right into it.
	// Please note that when attaching to a dialog box
	// control, it is normally of type PICTURE set as
	// "Rectangle" with "Black" color for background;
	//
	// clientPos - specify optionally how you would
	// like to position and size the video control;
	//
	// hMessageDrain - handle of the window to forward
	// all messages from the Video Window control;
	// Among important ones mouse events are, like
	// WM_CONTEXTMENU, WM_LBUTTONDOWN, and so on, see
	// documentation on IVideoWindow::MessageDrain in
	// MSDN;
	// When hMessageDrain is NULL, the method will try
	// to use parent window of hWnd, and if such doesn't
	// exist - it will use hWnd itself; Passing NULL is
	// the simplest solution when attaching video to a
	// static control on a dialog box;
	//
	// Returns: //////////////////////////////////////
	//
	// true, if successful;
	bool Attach(HWND hWnd, viClientPos clientPos = posDefault, HWND hMessageDrain = NULL);


	////////////////////////////////////////////////////
	// Method Detach;
	//
	// Calls method Stop, and then detaches video stream
	// from the window attached by the previous call to
	// method Attach();
	//
	// Returns: ////////////////////////////////////////
	//
	// Handle of the attached window, or NULL, if there
	// was none.
	HWND Detach();


	////////////////////////////////////////////////////
	// Method IsAttached;
	//
	// Returns true, if currently attached to any window.
	bool IsAttached();


	////////////////////////////////////////////////////
	// Method GetState;
	//
	// Returns current state of the selected Video-Input
	// device, if any;
	viState GetState();


	////////////////////////////////////////////////////
	// Method Run;
	//
	// Initiates playback for the currently selected
	// Video Input device into the attached window.
	//
	// Returns: ////////////////////////////////////////
	//
	// true, if successful;
	// false, if no device selected or no window attached;
	bool Run();


	////////////////////////////////////////////
	// Method Stop;
	//
	// Stops video playback, if there was any.
	//
	// Parameters: /////////////////////////////
	//
	// bStopWhenReady - indicates whether it should
	// pause the filter graph to allow filters to
	// queue data, and only then stop the filter graph;
	// When false, the rendering stops immediately;
	//
	// Returns: ////////////////////////////////
	//
	// true, if successful.
	bool Stop(bool bStopWhenReady = false);


	////////////////////////////////////////////
	// Method Pause;
	//
	// Pauses current playback, if there was any;
	//
	// Returns: /////////////////////////////////
	//
	// true, if successful;
	bool Pause();


	///////////////////////////////////////////////
	// Method GetResolution;
	//
	// Returns resolution in the currently selected
	// Video-Input device as set in the system.
	// It is also the size of the image that can be
	// returned from method GrabCurrentFrame;
	SIZE GetResolution();

	
	//////////////////////////////////////////
	// Method SetVideoWindowPos;
	//
	// Adjusts dimensions of the Video Window
	// control as specified by the parameters;
	//
	// Parameters: ///////////////////////////
	//
	// clientPos - tells how to adjust size and
	// position of the Video Window control;
	// See enum viClientPos for details;
	//
	// lpRect - if not NULL, it will contain
	// new dimensions of the Video Window as set
	// by the method.
	//
	// Returns: ///////////////////////////////
	//
	// true, if successful.
	bool SetVideoWindowPos(viClientPos clientPos = posDefault, LPRECT lpRect = NULL);


	////////////////////////////////////////////
	// Method GrabCurrentFrame;
	//
	// Grabs a single frame from the currently
	// active Video Input. It can only be called
	// when live video is being previewed, i.e.
	// when m_viState == vsRunning.
	// All images are returned with the same
	// resolution for a particular device, and
	// can be retrieved via method GetResolution();
	//
	// Parameters: /////////////////////////////
	//
	// grabDest - format in which the
	// frame must be returned. For details
	// see definition of enum viGrabDest.
	//
	// nSize - pointer to a long that receives
	// the number of bytes that was copied into
	// the returned object. This parameter is
	// ignored, and will contain 0 when grabDest
	// is grabIntoBitmap;
	//
	// Returns: ////////////////////////////////
	//
	// A value of type that depends on parameter
	// grabDest, and must be recasted accordingly,
	// as outlined below:
	//
	// grabIntoBitmap - HBITMAP
	// grabIntoStream - IStream*
	// grabIntoSequential - ISequentialStream*
	// grabIntoPicture - IPicture*
	// 
	// When the function fails it returns NULL.
	//
	// IMPORTANT: The returned object is to be
	// released/destroyed when no longer needed,
	// in the way consistent with its type.
	LPVOID GrabCurrentFrame(viGrabDest grabDest = grabIntoBitmap, long * nSize = NULL);


	//////////////////////////////////////////
	// Method GetDeviceDescr;
	//
	// Returns pointer to a device descriptor;
	//
	// Parameters: ///////////////////////////
	//
	// nDeviceIndex - index of the device
	// descriptor to be returned. It must be
	// within the range of [0, CountDevices() - 1],
	// or the function will return NULL.
	deviceDescr * GetDeviceDescr(long nDeviceIndex);


	//////////////////////////////////////
	// Method GetVideoWindow;
	//
	// Returns pointer to the Video Window
	// interface, i.e. IVideoWindow.
	// When interface is unavailable the
	// return value is NULL.
	IVideoWindow * GetVideoWindow();


	//////////////////////////////////////
	// Method GetMediaControl;
	//
	// Returns pointer to the Media Control
	// interface, i.e. IMediaControl.
	// When interface is unavailable the
	// return value is NULL.
	IMediaControl * GetMediaControl();


	///////////////////////////////////////////////
	// Method GetExternalDevice;
	//
	// Returns a pointer to the base interface that
	// controls the currently selected device, i.e.
	// interface IAMExtDevice.
	//
	// Use the returned interface for high-level
	// access to the selected device and its
	// parameters, such as Device Capabilities and
	// others. For details on interface IAMExtDevice
	// read MSDN.
	//
	// Please note, however that within DirectX 9.0
	// this interface is available only for VCR-s.
	//
	// When the interface is not supported or
	// unavailable otherwise, the method returns
	// NULL.
	IAMExtDevice * GetExternalDevice();


private:


//////////////////
// Methods:
//////////////////


	/////////////////////////////////
	// Method ReleaseDevices;
	//
	// Releases the list of all found
	// devices.
	void ReleaseDeviceList();


//////////////////
// Properties:
//////////////////


	//////////////////////////////////////////////////////////////////
	// List and total number of all available descriptors since
	// the last invocation of method EnumerateDevices().
	// Each descriptor represents a Video-Input device;
	//
	deviceDescr * m_pDevices;	// One-way list of device descriptors;
	long m_nDevices;			// Number of elements in the list;

	
	////////////////////////////////////////////////////////////////
	// Set of COM interfaces to preview live Video-Input, manage it,
	// grab a still image frame, and access advanced settings of
	// the selected device; All these interfaces belong to the
	// Microsoft® DirectShow® platform; Each interface here is
	// preambled with URL to Microsoft's online MSDN documentation
	// on it;
	//
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directshow/htm/ivideowindowinterface.asp
	IVideoWindow * m_pVW;	// Video Window control to render video stream;

	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directshow/htm/imediacontrolinterface.asp
	IMediaControl * m_pMC;	// Data Flow Control;

	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directshow/htm/isamplegrabberinterface.asp
	ISampleGrabber * m_pGrabber; // Grabs image frames from video stream;

	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directshow/htm/iamextdeviceinterface.asp
	IAMExtDevice * m_pExternal;	// Device Controlling Interface (For VCR-s only);


	///////////////////////////////////////////////////////////////
	// These properties control the buffer that stores bytes of the
	// last grabbed image frame;
	//
	BYTE * m_pBuffer;		// Pointer to the buffer of image bytes;
	long m_nBufferLength;	// Current buffer length;
	long m_nBufferUsed;		// Number of bytes in the buffer representing
							// the last grabbed image;

	
	/////////////////////////////////////////////////////////////////////////
	// Images are grabbed from within a separate thread, and we need
	// a way to synchronize image request->response between threads.
	// The two properties below allow us to do it;
	//
	HANDLE m_hEvent;	// Event "GOT THE FRAME" to signal from the thread
						// that grabs the image frame;
	bool m_bGrabNow;	// Flag for the thread that grabs images to actually
						// grab one immediately;
	//
	/////////////////////////////////////////////////////////////////////////


	SIZE m_Size;			// Resolution in pixels of the currently selected
							// Video-Input device;

	HWND m_hWnd;			// Handle of the window where video is being rendered; 

	long m_nSelDevice;		// Index of the currently selected device, or -1.

	viState m_viState;		// Current state of the selected Video-Input device.
};

#endif
