/*
    DirectShow Graph Manager Object

    This is the object that grabs frames from a WDM device.

    This code borrows heavily from the implementation in the
    DirectShow Video Processing Library (DSVL).  The original
    copyright is preserved below.

    $Id: rvDSCamera.cpp 28 2010-03-09 23:49:39Z mike $
*/

/* ========================================================================
*  PROJECT: DirectShow Video Processing Library (DSVL)
*  File:    OpenGL/GLUT shared memory buffer sample
*  Version: 0.0.8 (05/13/2005)
*  ========================================================================
*  Author:  Thomas Pintaric, Vienna University of Technology
*  Contact: pintaric@ims.tuwien.ac.at http://ims.tuwien.ac.at/~thomas
*  =======================================================================
*
*  Copyright (C) 2005  Vienna University of Technology
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  For further information please contact Thomas Pintaric under
*  <pintaric@ims.tuwien.ac.at> or write to Thomas Pintaric,
*  Vienna University of Technology, Favoritenstr. 9-11/E188/2, A-1040
*  Vienna, Austria.
* ========================================================================*/


#include "rvDSCamera.h"

DEFINE_EVENT_TYPE(wxEVT_CAMERA_IMAGE_READY)

static HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter, LPWSTR filterNameSubstring, bool matchDeviceName)
{
    HRESULT hr;
    IBaseFilter *pSrc = NULL;
    CComPtr <IMoniker> pMoniker = NULL;
    ULONG cFetched;

    // Create the system device enumerator
    CComPtr <ICreateDevEnum> pDevEnum =NULL;
    hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void ** ) &pDevEnum);
    if (FAILED(hr)) return hr;

    // Create an enumerator for the video capture devices
    CComPtr <IEnumMoniker> pClassEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
    if (FAILED(hr)) return hr;

    // If there are no enumerators for the requested type, then
    // CreateClassEnumerator will succeed, but pClassEnum will be NULL.
    // No video capture device was detected as we need at least one WDM
    // video capture device such as a USB web camera.
    if (pClassEnum == NULL) return E_FAIL;

    // Walk through the enumeration of each device.
    while (S_OK == (pClassEnum->Next (1, &pMoniker, &cFetched)))
    {
        CComPtr <IPropertyBag> pProp;
        pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pProp);

        // Try to match the friendly name.
        VARIANT varName;
        VariantInit(&varName);
        HRESULT name_hr = pProp->Read(L"FriendlyName", &varName, 0);

        if (filterNameSubstring != NULL)
        {
            // Bind Moniker to a filter object
            hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);

            // Are we matching the device name or friendly name?
            if (matchDeviceName)
            {
                LPOLESTR strName = NULL;

                // Get the name of the device.
                hr = pMoniker->GetDisplayName(NULL, NULL, &strName);

                // Do we match the device name?
                if (wcsstr(strName, filterNameSubstring) != NULL)
                {
                    // We matched the device name.
                    *ppSrcFilter = pSrc;
                    return S_OK;
                }
                else
                {
                    // We didn't match the device name.
                    pSrc->Release();
                }
            }
            else // matchFriendlyName
            {
                // Do we match the friendly name?
                if (SUCCEEDED(name_hr) && wcsstr(varName.bstrVal, filterNameSubstring) != NULL)
                {
                    // We matched the friendly name.
                    *ppSrcFilter = pSrc;
                    return S_OK;
                }
                else
                {
                    // We didn't match the friendly name.
                    pSrc->Release();
                }
            }
        }
        VariantClear(&varName);

        // Release for the next loop.
        pMoniker = NULL;
    }

    pClassEnum->Reset();

    // Use the first video capture device on the device list.
    // Note that if the Next() call succeeds but there are no monikers,
    // it will return S_FALSE (which is not a failure).  Therefore, we
    // check that the return code is S_OK instead of using SUCCEEDED() macro.
    if (S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched)))
    {
        // Bind Moniker to a filter object
        hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);

        // Return failure if we couldn't bind moniker to filter object.
        if (FAILED(hr)) return hr;
    }
    else
    {
        // Unable to access video capture device.
        return E_FAIL;
    }

    // Copy the found filter pointer to the output parameter.
    // Do NOT Release() the reference, since it will still be used
    // by the calling function.
    *ppSrcFilter = pSrc;

    return hr;
}


static HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION dir, int number, CComPtr <IPin> &pRetPin)
{
    ULONG fetched;
    HRESULT hr;

    // By default the returned pin interfaces is NULL.
    pRetPin = NULL;

    // Get the enum pins.
    IEnumPins* pEnumPins = NULL;
    hr = pFilter->EnumPins(&pEnumPins);
    if (FAILED(hr)) return E_FAIL;

    // Reset the enumeration.
    pEnumPins->Reset();

    // Get the first pin.
    IPin* pPin = NULL;
    hr = pEnumPins->Next(1, &pPin, &fetched);
    if (FAILED(hr)) return E_FAIL;

    // Get he pin info for the rest pin.
    PIN_INFO pinInfo;
    pPin->QueryPinInfo(&pinInfo);

    // Reset the pin number.
    int n = 0;

    // Loop over each pin.
    do
    {
        // The pFilter member has an outstanding ref count
        // so release it here as we don't use it anyways!
        pinInfo.pFilter->Release();
        pinInfo.pFilter = NULL;

        // Is the pin going in the right direction?
        if (pinInfo.dir == dir)
        {
            // Yes. Increment the pin number.
            n++;

            // Is this the correct pin?
            if (n == number)
            {
                // Yes. Return the pin.
                pEnumPins->Release();
                pRetPin = pPin;

                // Return success.
                return S_OK;
            }
            else
            {
                // No. Release the pin.
                pPin->Release();

                // Query the next pin.
                pEnumPins->Next(1, &pPin, &fetched);

                // Was there another pin to fetch?
                if (fetched == 0)
                {
                    // Don't return a pin.
                    pEnumPins->Release();

                    // Return failure.
                    return E_FAIL;
                }

                // Query the next set of pin information.
                pPin->QueryPinInfo(&pinInfo);
            }
        }
        else
        {
            // No. Release the pin.
            pPin->Release();

            // Query the next pin.
            pEnumPins->Next(1, &pPin, &fetched);

            // Was there another pin to fetch?
            if (fetched == 0)
            {
                // Don't return a pin.
                pEnumPins->Release();

                // Return failure.
                return E_FAIL;
            }

            // Query the next set of pin information.
            pPin->QueryPinInfo(&pinInfo);
        }
    } while (pPin != NULL);

    // Release the enumerator.
    pEnumPins->Release();

    return E_FAIL;
}

static bool CanDeliverDV(IPin *pPin)
// Checks if a pin can deliver compressed DV formats.  Refer to Microsoft
// DirectX Documentation, "DV Video Subtypes" for more information.
//
// FOURCC   GUID                Data Rate   Description
// 'dvsl'   MEDIASUBTYPE_dvsl   12.5 Mbps   SD-DVCR 525-60 or SD-DVCR 625-50
// 'dvsd'   MEDIASUBTYPE_dvsd   25 Mbps     SDL-DVCR 525-60 or SDL-DVCR 625-50
// 'dvhd'   MEDIASUBTYPE_dvhd   50 Mbps     HD-DVCR 1125-60 or HD-DVCR 1250-50
{
    HRESULT hr;

    // Create an interface to enumerate media types on this pin.
    CComPtr <IEnumMediaTypes> enumMediaTypes = NULL;
    hr = pPin->EnumMediaTypes(&enumMediaTypes);
    if (FAILED(hr)) return false;

    // Reset the enumerator.
    enumMediaTypes->Reset();

    AM_MEDIA_TYPE *pMediaType = NULL;

    // Enumerate the media types for the pin.
    while(S_OK == (hr = enumMediaTypes->Next(1, &pMediaType, NULL)))
    {
        // Is this a digital video media type?
        if((pMediaType->subtype == MEDIASUBTYPE_dvsl) ||
            (pMediaType->subtype == MEDIASUBTYPE_dvsd) ||
            (pMediaType->subtype == MEDIASUBTYPE_dvhd))
        {
            // Free the media type.
            DeleteMediaType(pMediaType);

            // Yes, this pin supports a compressed DV format.
            return true;
        }
        else
        {
            // Free the media type.
            DeleteMediaType(pMediaType);
        }
    }

    // No, this pin does not support a compressed DV format.
    return false;
}

static double RoundDouble(double doValue, int nPrecision)
{
    static const double doBase = 10.0f;
    double doComplete5, doComplete5i;

    doComplete5 = doValue * pow(doBase, (double) (nPrecision + 1));

    if(doValue < 0.0f)
        doComplete5 -= 5.0f;
    else
        doComplete5 += 5.0f;

    doComplete5 /= doBase;
    modf(doComplete5, &doComplete5i);

    return doComplete5i / pow(doBase, (double) nPrecision);
}

static double avg2fps(REFERENCE_TIME avgTimePerFrame, int precision)
{
    return RoundDouble(10000000.0 / (double) avgTimePerFrame, precision);
}

static REFERENCE_TIME fps2avg(double fps)
{
    return ((REFERENCE_TIME) (1.0 / fps * 10000000.0));
}

static HRESULT MatchMediaTypes(IPin *pPin, long width, long height, double frameRate, GUID subtype, AM_MEDIA_TYPE *pRequestMediaType)
// Overwrite req_mt with the best match.
{
    HRESULT hr;
    AM_MEDIA_TYPE *pMediaType = NULL;

    // Get the interface to enumerate the media types on this pin.
    CComPtr<IEnumMediaTypes> pEnumMediaTypes;
    pPin->EnumMediaTypes(&pEnumMediaTypes);

    // Reset the enumeration.
    pEnumMediaTypes->Reset();

    // Enumerate each media type on this pin.
    while (S_OK == (hr = pEnumMediaTypes->Next(1, &pMediaType, NULL)))
    {
        // Point to the video information header.
        VIDEOINFOHEADER *pVideoInfo = (VIDEOINFOHEADER *) pMediaType->pbFormat;

        // Do we match the video information.
        if(((pMediaType->subtype == subtype) || (subtype == GUID_NULL)) &&
           ((pVideoInfo->bmiHeader.biWidth == width)  || (width == 0)) &&
           ((pVideoInfo->bmiHeader.biHeight == height) || (height == 0)) &&
           ((avg2fps(pVideoInfo->AvgTimePerFrame, 3) == RoundDouble(frameRate, 3)) || (frameRate == 0.0)))
        {
            // Found a match!
            CopyMediaType(pRequestMediaType, pMediaType);
            DeleteMediaType(pMediaType);
            return S_OK;
        }
        else
        {
            // Delete the media type.
            DeleteMediaType(pMediaType);
        }
    }

    // No match found.
    return E_FAIL;
}

HRESULT AutoConnectFilters(IBaseFilter *pOutFilter, int outPinNum, IBaseFilter *pInFilter, int inPinNum, IGraphBuilder *pGraphBuilder)
{
    HRESULT hr;
    CComPtr <IPin> pInPin = NULL;
    CComPtr <IPin> pOutPin = NULL;

    // Get the output pin.
    hr = GetPin(pOutFilter, PINDIR_OUTPUT, outPinNum, pOutPin);
    if (FAILED(hr)) return hr;

    // Get the input pin.
    hr = GetPin(pInFilter, PINDIR_INPUT, inPinNum, pInPin);
    if (FAILED(hr)) return hr;

    // Make sure the pins are non-null.
    if (pOutPin == NULL || pInPin == NULL) return E_FAIL;

    // Connect the output pin to the input pin.
    hr = pGraphBuilder->Connect(pOutPin, pInPin);
    if (FAILED(hr)) return E_FAIL;

    return S_OK;
}

rvDSCamera::rvDSCamera() :
        m_captureGraphBuilder(NULL),
        m_graphBuilder(NULL),
        m_mediaControl(NULL),
        m_mediaEvent(NULL),
        m_mediaSeeking(NULL),
        m_cameraControl(NULL),
        m_droppedFrames(NULL),
        m_videoControl(NULL),
        m_videoProcAmp(NULL),
        m_sourceFilter(NULL),
        m_decoderFilter(NULL),
        m_rendererFilter(NULL),
        m_grabberFilter(NULL),
        m_sampleGrabber(NULL),
        m_capturePin(NULL),
        m_sync(NULL),
        m_syncName(_T("SyncEvent"))
{
    m_refCount = 0;
    m_graphInitialized = false;
    m_imageHandler = NULL;
    m_imageBufferList = rvLinkedList_New(16, sizeof(rvDSCameraBuffer), 0);
    m_sync = CreateEvent(NULL, TRUE, 0, _T("SyncEvent"));
}

rvDSCamera::~rvDSCamera()
{
    // Get the first buffered image.
    rvDSCameraBuffer *pImageBuffer = (rvDSCameraBuffer *) rvLinkedList_Next(m_imageBufferList, NULL);

    // Loop over each buffered image.
    while (pImageBuffer != NULL)
    {
        // Save the next buffered image.
        rvDSCameraBuffer *pNextBuffer = (rvDSCameraBuffer *) rvLinkedList_Next(m_imageBufferList, pImageBuffer);

        // Yes, release this object.
        pImageBuffer->pMediaSample->Release();

        // Remove from the list.
        rvLinkedList_Delete(m_imageBufferList, pImageBuffer, NULL);

        // Get the next buffered image.
        pImageBuffer = pNextBuffer;
    }

    // Free the memory buffer list.
    rvLinkedList_Free(m_imageBufferList, NULL);
}

HRESULT WINAPI rvDSCamera::QueryInterface(REFIID iid, void** ppvObject )
{
    // Return requested interface
    if (IID_IUnknown == iid)
    {
        *ppvObject = dynamic_cast<IUnknown*>(this);
    }
    else if (IID_ISampleGrabberCB == iid)
    {
        // Sample grabber callback object
        *ppvObject = dynamic_cast<ISampleGrabberCB*>(this);
    }
    else
    {
        // No interface for requested iid - return error.
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    // Increment our reference count.
    this->AddRef();

    return S_OK;
}

ULONG WINAPI rvDSCamera::AddRef()
{
    return m_refCount++;
}

ULONG WINAPI rvDSCamera::Release()
{
    if (m_refCount > 0) m_refCount--;
    return m_refCount;
}

HRESULT WINAPI rvDSCamera::SampleCB(double SampleTime, IMediaSample *pMediaSample)
{
    // Lock this code block as a critical section.
    CAutoLock cObjectLock(&m_critSection);

    // Get the first buffered image.
    rvDSCameraBuffer *pImageBuffer = (rvDSCameraBuffer *) rvLinkedList_Next(m_imageBufferList, NULL);

    // Loop over each buffered image.
    while (pImageBuffer != NULL)
    {
        // Save the next buffered image.
        rvDSCameraBuffer *pNextBuffer = (rvDSCameraBuffer *) rvLinkedList_Next(m_imageBufferList, pImageBuffer);

        // Should we release this image buffer?
        if (pImageBuffer->useCount == 0)
        {
            // Yes, release this object.
            pImageBuffer->pMediaSample->Release();

            // Remove from the list.
            rvLinkedList_Delete(m_imageBufferList, pImageBuffer, NULL);
        }

        // Get the next buffered image.
        pImageBuffer = pNextBuffer;
    }

    // Get the sample start and end time.  We really just care about the start time.
    REFERENCE_TIME t_start, t_end;
    pMediaSample->GetMediaTime(&t_start, &t_end);

    // Add a reference to the media sample.
    pMediaSample->AddRef();

    // Fill in the image buffer.
    rvDSCameraBuffer imageBuffer;
    imageBuffer.useCount = 0;
    imageBuffer.timeStamp = t_start;
    imageBuffer.pMediaSample = pMediaSample;

    // Insert into the list of image buffers.
    rvLinkedList_Append(m_imageBufferList, &imageBuffer);

    // Signal that we have an image.
    SetEvent(m_sync);

    // Send an event that we are ready with a new image.
    if (m_imageHandler != NULL)
    {
        wxCommandEvent event(wxEVT_CAMERA_IMAGE_READY);
        event.SetEventObject(this);
        m_imageHandler->ProcessEvent(event);
    }

    return S_OK;
}

HRESULT WINAPI rvDSCamera::BufferCB(double sampleTimeSec, BYTE* bufferPtr, long bufferLength)
{
    return(E_NOTIMPL);
}


bool rvDSCamera::SetImageHandler(wxEvtHandler *imageHandler)
{
    // Set the image handler.
    m_imageHandler = imageHandler;

    return true;
}

bool rvDSCamera::EnableMemoryBuffer(unsigned int maxConcurrentClients, unsigned int allocatorBuffersPerClient)
{
    HRESULT hr;
    CComPtr <IPin> pSampleGrabberPin = NULL;
    CComPtr <IMemInputPin> pMemInputPin = NULL;
    CComPtr <IMemAllocator> pMemAllocator = NULL;
    CComPtr <ISampleGrabberCB> pSampleGrabberCB = NULL;

    // Lock this code block as a critical section.
    CAutoLock cObjectLock(&m_critSection);

    // Sanity check the arguments.
    if (allocatorBuffersPerClient < MIN_ALLOCATOR_BUFFERS_PER_CLIENT) allocatorBuffersPerClient = MIN_ALLOCATOR_BUFFERS_PER_CLIENT;
    if (maxConcurrentClients <= 0) maxConcurrentClients = 1;

    // Determine the number of allocator buffers.
    unsigned int currentAllocatorBuffers = maxConcurrentClients * allocatorBuffersPerClient;

    // Drill down the interfaces to get the allocator properties.
    hr = GetPin(m_grabberFilter, PINDIR_INPUT, 1, pSampleGrabberPin);
    if (FAILED(hr)) return false;

    hr = pSampleGrabberPin->QueryInterface(IID_IMemInputPin, (void**) &pMemInputPin);
    if (FAILED(hr)) return false;

    hr = pMemInputPin->GetAllocator(&pMemAllocator);
    if (FAILED(hr)) return false;

    hr = pMemAllocator->Decommit();
    if (FAILED(hr)) return false;

    // Update the number of buffers created by the allocator.
    ALLOCATOR_PROPERTIES actualProperties;
    ALLOCATOR_PROPERTIES requestedProperties;
    pMemAllocator->GetProperties(&requestedProperties);
    requestedProperties.cBuffers = currentAllocatorBuffers;
    hr = pMemAllocator->SetProperties(&requestedProperties, &actualProperties);

    // Setup the sample grabber.
    m_sampleGrabber->SetBufferSamples(FALSE);
    m_sampleGrabber->SetOneShot(FALSE);

    // Query our own sample grabber callback interface.
    hr = QueryInterface(IID_ISampleGrabberCB, (void**)&pSampleGrabberCB);
    if (FAILED(hr)) return false;

    // Set the callback interface.
    return (SUCCEEDED(m_sampleGrabber->SetCallback(pSampleGrabberCB,0)) ? true : false);
}

bool rvDSCamera::DisableMemoryBuffer()
{
    // Lock this code block as a critical section.
    CAutoLock cObjectLock(&m_critSection);

    // Get the first buffered image.
    rvDSCameraBuffer *pImageBuffer = (rvDSCameraBuffer *) rvLinkedList_Next(m_imageBufferList, NULL);

    // Loop over each buffered image.
    while (pImageBuffer != NULL)
    {
        // Save the next buffered image.
        rvDSCameraBuffer *pNextBuffer = (rvDSCameraBuffer *) rvLinkedList_Next(m_imageBufferList, pImageBuffer);

        // Yes, release this object.
        pImageBuffer->pMediaSample->Release();

        // Remove from the list.
        rvLinkedList_Delete(m_imageBufferList, pImageBuffer, NULL);

        // Get the next buffered image.
        pImageBuffer = pNextBuffer;
    }

    return (SUCCEEDED(m_sampleGrabber->SetCallback(NULL,0)) ? true : false);
}

bool rvDSCamera::BuildGraph(int width, int height, double frameRate)
{
    HRESULT hr;

    CComPtr <IBaseFilter> pVideoSource = NULL;
    CComPtr <IBaseFilter> pStreamSplitter = NULL;
    CComPtr <IBaseFilter> pVideoDecoder = NULL;
    CComPtr <IBaseFilter> pVideoRenderer = NULL;
    CComPtr <IBaseFilter> pSampleGrabber = NULL;
    CComPtr <IAMStreamConfig> pStreamConfig = NULL;
    CComPtr <IPin> pSampleGrabberPin = NULL;
    CComPtr <IMemInputPin> pMemInputPin = NULL;
    CComPtr <IMemAllocator> pMemAllocator = NULL;

    // Fail if we are already initialized.
    if (m_graphInitialized) return false;

    // Update the width and height.
    m_imageWidth = width;
    m_imageHeight = height;

    // Create the graph builder.
    hr = CoCreateInstance (CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **) &m_graphBuilder);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Create the capture graph builder.
    hr = CoCreateInstance (CLSID_CaptureGraphBuilder2 , NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **) &m_captureGraphBuilder);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // We can't directly create a sample grabber interface, but instead have to
    // create a base filter and then query the sample grabber interface from it.
    hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &(pVideoRenderer));
    if (FAILED(hr)) goto ERROR_HANDLER;
    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &(pSampleGrabber));
    if (FAILED(hr)) goto ERROR_HANDLER;
    hr = pSampleGrabber->QueryInterface(IID_ISampleGrabber, (void**) &m_sampleGrabber);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Configure the media subtype.
    AM_MEDIA_TYPE mediaType;
    ZeroMemory(&mediaType, sizeof(AM_MEDIA_TYPE));
    mediaType.majortype = MEDIATYPE_Video;
    mediaType.formattype = GUID_NULL;
    mediaType.subtype = MEDIASUBTYPE_RGB24;
    hr = m_sampleGrabber->SetMediaType(&mediaType);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Obtain interfaces for media control.
    hr = m_graphBuilder->QueryInterface(IID_IMediaControl,(LPVOID *) &m_mediaControl);
    if (FAILED(hr)) goto ERROR_HANDLER;
    hr = m_graphBuilder->QueryInterface(IID_IMediaEvent, (LPVOID *) &m_mediaEvent);
    if (FAILED(hr)) goto ERROR_HANDLER;
    hr = m_captureGraphBuilder->SetFiltergraph(m_graphBuilder);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Find the video capture device.
    hr = FindCaptureDevice(&pVideoSource, L"PGR", false);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Add the video source.
    hr = m_graphBuilder->AddFilter(pVideoSource, L"Video Source");
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Set the source filter interface.
    m_sourceFilter = pVideoSource;

    // Get the first output pin from the video source filter.
    hr = GetPin(pVideoSource, PINDIR_OUTPUT, 1, m_capturePin);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Get the stream configuration interface.
    hr = m_captureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, NULL, pVideoSource, IID_IAMStreamConfig, (void**) &pStreamConfig);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Does the video capture pin support compressed DV?  If so,
    // WDM DV Video Capture Devices (such as consumer miniDV camcorders)
    // will require special handling. Since the WDM source will only
    // offer DVRESOLUTION_FULL, any changes must be made through the
    // IIPDVDec interface on the DV Decoder filter.
    if (CanDeliverDV(m_capturePin))
    {
        // Yes, this pin does support compressed DV.
        int dvRes;
        CComPtr<IIPDVDec> pDVDec;

        // Create an instance of the video decoder.
        CoCreateInstance(CLSID_DVVideoCodec, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &(pVideoDecoder));
        if (FAILED(hr)) goto ERROR_HANDLER;

        // Save the decode filter.
        m_decoderFilter = pVideoDecoder;

        // Insert the DV decoder (CLSID_DVVideoCodec) into our graph.
        hr = m_graphBuilder->AddFilter(pVideoDecoder, L"Video Decoder");
        if (FAILED(hr)) goto ERROR_HANDLER;

        // Since we're dealing with DV, there's only a limited range of possible resolutions:
        //
        //   DVDECODERRESOLUTION_720x480 (PAL: 720x576)  = DVRESOLUTION_FULL
        //   DVDECODERRESOLUTION_360x240 (PAL: 360x288)  = DVRESOLUTION_HALF
        //   DVDECODERRESOLUTION_180x120 (PAL: 180x144)  = DVRESOLUTION_QUARTER
        //   DVDECODERRESOLUTION_88x60   (PAL: 88x72)    = DVRESOLUTION_DC
        hr = pVideoDecoder->QueryInterface(IID_IIPDVDec, (void**) &pDVDec);
        if (FAILED(hr)) goto ERROR_HANDLER;

        // Get the default resolution.
        hr = pDVDec->get_IPDisplay(&dvRes);
        if (FAILED(hr)) goto ERROR_HANDLER;

        // Adjust the resolution if we match.
        if ((m_imageWidth == 720) && ((m_imageHeight == 480) || (m_imageHeight == 576))) dvRes = DVRESOLUTION_FULL;
        else if ((m_imageWidth == 360) && ((m_imageHeight == 240) || (m_imageHeight == 288))) dvRes = DVRESOLUTION_HALF;
        else if ((m_imageWidth == 180) && ((m_imageHeight == 120) || (m_imageHeight == 144))) dvRes = DVRESOLUTION_QUARTER;
        else if ((m_imageWidth == 88)  && ((m_imageHeight == 60)  || (m_imageHeight == 72)))  dvRes = DVRESOLUTION_DC;

        // Set the resolution.
        hr = pDVDec->put_IPDisplay(dvRes);
        if (FAILED(hr)) goto ERROR_HANDLER;
    }
    else
    {
        // No, this pin does not support compressed DV.
        AM_MEDIA_TYPE mediaType;

        // Attempt to make a match of the input parameters.
        // XXX hr = MatchMediaTypes(m_capturePin, m_imageWidth, m_imageHeight, frameRate, GUID_NULL, &mediaType);
        hr = MatchMediaTypes(m_capturePin, m_imageWidth, m_imageHeight, frameRate, MEDIASUBTYPE_RGB24, &mediaType);
        if (FAILED(hr)) goto ERROR_HANDLER;

        // Set the media format.
        pStreamConfig->SetFormat(&mediaType);
    }

    if (FAILED(pVideoSource->QueryInterface(IID_IAMCameraControl, (void**) &m_cameraControl)))
        m_cameraControl = NULL;

    if (FAILED(pVideoSource->QueryInterface(IID_IAMDroppedFrames, (void**) &m_droppedFrames)))
        m_droppedFrames = NULL;

    if (FAILED(pVideoSource->QueryInterface(IID_IAMVideoControl, (void**) &m_videoControl)))
        m_videoControl = NULL;

    if (FAILED(pVideoSource->QueryInterface(IID_IAMVideoProcAmp, (void**) &m_videoProcAmp)))
        m_videoProcAmp = NULL;

    hr = m_graphBuilder->AddFilter(pSampleGrabber, L"Sample Grabber");
    if (FAILED(hr)) goto ERROR_HANDLER;

    hr = m_graphBuilder->AddFilter(pVideoRenderer, L"Video Renderer");
    if (FAILED(hr)) goto ERROR_HANDLER;

    hr = AutoConnectFilters(pVideoSource, 1, pSampleGrabber, 1, m_graphBuilder);
    if (FAILED(hr)) goto ERROR_HANDLER;

    hr = AutoConnectFilters(pSampleGrabber, 1, pVideoRenderer, 1, m_graphBuilder);
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Have the Filter Graph Manager to choose a reference clock using its default algorithm.
    m_graphBuilder->SetDefaultSyncSource();

    // Drill down the interfaces to get the allocator properties.
    hr = GetPin(pSampleGrabber, PINDIR_INPUT, 1, pSampleGrabberPin);
    if (FAILED(hr)) goto ERROR_HANDLER;

    hr = pSampleGrabberPin->QueryInterface(IID_IMemInputPin, (void**) &pMemInputPin);
    if (FAILED(hr)) goto ERROR_HANDLER;

    hr = pMemInputPin->GetAllocator(&pMemAllocator);
    if (FAILED(hr)) goto ERROR_HANDLER;

    hr = pMemAllocator->Decommit();
    if (FAILED(hr)) goto ERROR_HANDLER;

    // Update the number of buffers created by the allocator.
    long numAlloc = (MIN_ALLOCATOR_BUFFERS_PER_CLIENT * DEF_CONCURRENT_CLIENTS);
    ALLOCATOR_PROPERTIES actualProperties;
    ALLOCATOR_PROPERTIES requestedProperties;
    pMemAllocator->GetProperties(&requestedProperties);
    if (requestedProperties.cBuffers != numAlloc) requestedProperties.cBuffers = numAlloc;
    hr = pMemAllocator->SetProperties(&requestedProperties, &actualProperties);

    // Set the grabber filter.
    m_grabberFilter = pSampleGrabber;

    // We are now initialized.
    m_graphInitialized = true;

    // Success.
    return true;

ERROR_HANDLER:

    // Release the interfaces we hold.
    if (m_mediaControl != NULL) { m_mediaControl.Release(); m_mediaControl = NULL; }
    if (m_graphBuilder != NULL) { m_graphBuilder.Release(); m_graphBuilder = NULL; }
    if (m_captureGraphBuilder != NULL) { m_captureGraphBuilder.Release(); m_captureGraphBuilder = NULL; }
    if (m_mediaEvent != NULL) { m_mediaEvent.Release(); m_mediaEvent = NULL; }
    if (m_mediaSeeking != NULL) { m_mediaSeeking.Release(); m_mediaSeeking = NULL; }
    if (m_cameraControl != NULL) { m_cameraControl.Release(); m_cameraControl = NULL; }
    if (m_droppedFrames != NULL) { m_droppedFrames.Release(); m_droppedFrames = NULL; }
    if (m_videoControl != NULL) { m_videoControl.Release(); m_videoControl = NULL; }
    if (m_videoProcAmp != NULL) { m_videoProcAmp.Release(); m_videoProcAmp = NULL; }
    if (m_sourceFilter != NULL) { m_sourceFilter.Release(); m_sourceFilter = NULL; }
    if (m_capturePin != NULL) { m_capturePin.Release(); m_capturePin = NULL; }
    if (m_decoderFilter != NULL) { m_decoderFilter.Release(); m_decoderFilter = NULL; }
    if (m_rendererFilter != NULL) { m_rendererFilter.Release(); m_rendererFilter = NULL; }
    if (m_grabberFilter != NULL) { m_grabberFilter.Release(); m_grabberFilter = NULL; }
    if (m_sampleGrabber != NULL) { m_sampleGrabber.Release(); m_sampleGrabber = NULL; }

    // Return failure.
    return false;
}

bool rvDSCamera::ReleaseGraph()
{
    HRESULT hr;

    // Lock this code block as a critical section.
    CAutoLock cObjectLock(&m_critSection);

    // Fail if we are not initialized.
    if (!m_graphInitialized) return false;

    // Stop all media.
    hr = m_mediaControl->Stop();

    // Initialize the filters pointer.
    IEnumFilters *pEnumFilters = NULL;

    // Enumerate the filters in the graph.
    hr = m_graphBuilder->EnumFilters(&pEnumFilters);
    if (SUCCEEDED(hr))
    {
        IBaseFilter *pFilter = NULL;
        while (S_OK == pEnumFilters->Next(1, &pFilter, NULL))
        {
            FILTER_INFO filterInfo;
            HRESULT hrFilterInfo = pFilter->QueryFilterInfo(&filterInfo);

            // Remove the filter.
            hr = m_graphBuilder->RemoveFilter(pFilter);
            wxASSERT(SUCCEEDED(hr));

            // Did we succeed getting the filter information.
            if (SUCCEEDED(hrFilterInfo))
            {
                // Release the graph associated with the filter.
                filterInfo.pGraph->Release();
            }

            // Reset the enumerator.
            pEnumFilters->Reset();
            pFilter->Release();
        }

        // Release the filters enumerator.
        pEnumFilters->Release();
    }

    // Release the interfaces we hold.
    m_mediaControl.Release(); m_mediaControl = NULL;
    m_graphBuilder.Release(); m_graphBuilder = NULL;
    if (m_captureGraphBuilder != NULL) { m_captureGraphBuilder.Release(); m_captureGraphBuilder = NULL; }
    if (m_mediaEvent != NULL) { m_mediaEvent.Release(); m_mediaEvent = NULL; }
    if (m_mediaSeeking != NULL) { m_mediaSeeking.Release(); m_mediaSeeking = NULL; }
    if (m_cameraControl != NULL) { m_cameraControl.Release(); m_cameraControl = NULL; }
    if (m_droppedFrames != NULL) { m_droppedFrames.Release(); m_droppedFrames = NULL; }
    if (m_videoControl != NULL) { m_videoControl.Release(); m_videoControl = NULL; }
    if (m_videoProcAmp != NULL) { m_videoProcAmp.Release(); m_videoProcAmp = NULL; }
    if (m_sourceFilter != NULL) { m_sourceFilter.Release(); m_sourceFilter = NULL; }
    if (m_capturePin != NULL) { m_capturePin.Release(); m_capturePin = NULL; }
    if (m_decoderFilter != NULL) { m_decoderFilter.Release(); m_decoderFilter = NULL; }
    if (m_rendererFilter != NULL) { m_rendererFilter.Release(); m_rendererFilter = NULL; }
    if (m_grabberFilter != NULL) { m_grabberFilter.Release(); m_grabberFilter = NULL; }
    if (m_sampleGrabber != NULL) { m_sampleGrabber.Release(); m_sampleGrabber = NULL; }

    // We are no longer initialized.
    m_graphInitialized = false;

    // Success.
    return true;
}

bool rvDSCamera::IsGraphInitialized()
{
    return m_graphInitialized;
}

bool rvDSCamera::Run()
{
    // Fail if we are not initialized.
    if (!m_graphInitialized) return false;

    HRESULT hr = m_mediaControl->Run();
    if(FAILED(hr)) return false;

    return true;
}

bool rvDSCamera::Pause()
{
    // Fail if we are not initialized.
    if (!m_graphInitialized) return false;

    HRESULT hr = m_mediaControl->Pause();
    if(FAILED(hr)) return false;

    return true;
}

bool rvDSCamera::Stop(bool forcedStop)
{
    // Fail if we are not initialized.
    if (!m_graphInitialized) return false;

    HRESULT hr = (forcedStop ? m_mediaControl->Stop() : m_mediaControl->StopWhenReady());
    if(FAILED(hr)) return false;

    return true;
}

bool rvDSCamera::WaitForNextImage(long milliseconds)
{
    MSG uMsg;
    memset(&uMsg,0,sizeof(uMsg));
    bool exitMainLoop = false;

    do
    {
        // Wait to receive the event that indicates a new sample has been received.
        DWORD dw = MsgWaitForMultipleObjectsEx(1, &m_sync, milliseconds, QS_ALLEVENTS, 0);

        // Did we receive the sample?
        if (dw == WAIT_OBJECT_0 || dw == WAIT_TIMEOUT)
        {
            ResetEvent(m_sync);
            return true;
        }
        // Process a windows message.
        else while(PeekMessage(&uMsg, NULL, 0, 0, PM_NOREMOVE))
        {
            // Receive the Windows message.
            if (!GetMessage(&uMsg, NULL, 0, 0))
            {
                exitMainLoop = true;
            }
            else
            {
                // Process the Windows message.
                TranslateMessage(&uMsg);
                DispatchMessage(&uMsg);
            }
        }
    } while(!exitMainLoop);

    return false;
}

bool rvDSCamera::CheckoutIplImage(IplImage *image)
{
    // Initialize the image header with three channels, origin in the bottom left and alignment of 4.
    cvInitImageHeader(image, cvSize(m_imageWidth, m_imageHeight), IPL_DEPTH_8U, 3, IPL_ORIGIN_BL, 4);

    // Initialize the image data and image data origin pointers.
    image->imageData = NULL;
    image->imageDataOrigin = NULL;

    // Lock this code block as a critical section.
    CAutoLock cObjectLock(&m_critSection);

    // Find the latest image buffer.
    rvDSCameraBuffer *pImageBuffer = (rvDSCameraBuffer *) rvLinkedList_Prev(m_imageBufferList, NULL);

    // Return if we didn't find a previous image buffer.
    if (pImageBuffer == NULL) return false;

    // Increment the use count for the image buffer.
    ++pImageBuffer->useCount;

    // Get the buffer pointer to the image buffer.
    HRESULT hr = pImageBuffer->pMediaSample->GetPointer((BYTE **) &image->imageData);
    if (FAILED(hr)) return false;

    // Point to the image buffer.  This is not used in the structure because the
    // caller should never free the data, but rather call the CheckinIplImage method.
    image->imageDataOrigin = (char *) pImageBuffer;

    return true;
}

bool rvDSCamera::CheckinIplImage(IplImage *image, bool forceRelease)
{
    // Return if the handle is null.
    if (image->imageDataOrigin == NULL) return false;

    // Lock this code block as a critical section.
    CAutoLock cObjectLock(&m_critSection);

    // Get the image buffer.
    rvDSCameraBuffer *pImageBuffer = (rvDSCameraBuffer *) image->imageDataOrigin;

    // Decrement or zero out the use count.
    if (!forceRelease)
    {
        // Decrement if greater than zero.
        if (pImageBuffer->useCount > 0) --pImageBuffer->useCount;
    }
    else
    {
        // Force the use count to zero.
        pImageBuffer->useCount = 0;
    }

    return true;
}

static HRESULT DisplayPinProperties(CComPtr <IPin> pSrcPin, HWND hWnd)
{
    CComPtr <ISpecifyPropertyPages> pPages = NULL;

    // Get the property pages interface.
    HRESULT hr = pSrcPin->QueryInterface(IID_ISpecifyPropertyPages, (void**) &pPages);
    if (SUCCEEDED(hr))
    {
        // Get the filter's name and IUnknown pointer.
        PIN_INFO PinInfo;
        pSrcPin->QueryPinInfo(&PinInfo);

        // Display the property frame.
        CAUUID caGUID;
        pPages->GetPages(&caGUID);
        OleCreatePropertyFrame(hWnd, 0, 0, L"Property Sheet", 1, (IUnknown **) &(pSrcPin.p),
                               caGUID.cElems, caGUID.pElems, 0, 0, NULL);

        // Cleanup.
        CoTaskMemFree(caGUID.pElems);
        PinInfo.pFilter->Release();
    }
    else return(hr);

    return(S_OK);
}

static HRESULT DisplayFilterProperties(IBaseFilter *pBaseFilter, HWND hWnd)
{
    CComPtr <ISpecifyPropertyPages> pPages = NULL;

    // Get the property pages interface.
    HRESULT hr = pBaseFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **) &pPages);
    if (SUCCEEDED(hr))
    {
        // Get the filter's name and IUnknown pointer.
        FILTER_INFO filterInfo;
        pBaseFilter->QueryFilterInfo(&filterInfo);

        // Display the property frame.
        CAUUID caGUID;
        pPages->GetPages(&caGUID);
        OleCreatePropertyFrame(hWnd, 0, 0, filterInfo.achName, 1, (IUnknown **) &pBaseFilter,
                               caGUID.cElems, caGUID.pElems, 0, 0, NULL);

        // Clean up.
        if (filterInfo.pGraph != NULL) filterInfo.pGraph->Release();
        CoTaskMemFree(caGUID.pElems);
    }
    return(hr);
}

static DWORD WINAPI TShowPinProperties(LPVOID parameters)
{
    HWND hWnd = (HWND)((LPVOID *) parameters)[0];
    IPin *pSrcPin = (IPin *)((LPVOID *) parameters)[1];
    CoTaskMemFree(parameters);
    return DisplayPinProperties(pSrcPin, hWnd);
}

static DWORD WINAPI TShowFilterProperties(LPVOID parameters)
{
    HWND hWnd = (HWND)((LPVOID *) parameters)[0];
    IBaseFilter *pBaseFilter = (IBaseFilter *)((LPVOID *) parameters)[1];
    CoTaskMemFree(parameters);
    return DisplayFilterProperties(pBaseFilter, hWnd);
}

bool rvDSCamera::ShowPinProperties(HWND hWnd)
{
    DWORD dwThreadID;
    LPVOID *parameters = (LPVOID *) CoTaskMemAlloc(sizeof(LPVOID) * 2);
    parameters[0] = (LPVOID) hWnd;
    parameters[1] = (LPVOID) m_capturePin;
    HANDLE hPropThread = CreateThread(NULL, 0, TShowPinProperties, (LPVOID) parameters, 0, &dwThreadID);
    if (hPropThread == NULL) return false;
    return true;
}

bool rvDSCamera::ShowFilterProperties(HWND hWnd)
{
    DWORD dwThreadID;
    LPVOID *parameters = (LPVOID *) CoTaskMemAlloc(sizeof(LPVOID) * 2);
    parameters[0] = (LPVOID) hWnd;
    parameters[1] = (LPVOID) m_sourceFilter;
    HANDLE hPropThread = CreateThread(NULL, 0, TShowFilterProperties, (LPVOID) parameters, 0, &dwThreadID);
    if (hPropThread == NULL) return false;
    return true;
}

