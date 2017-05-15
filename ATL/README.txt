Active Template Library (ATL)
=============================

This directory contains the Active Template Library (ATL) that used to be 
included with the Windows Platform SDK prior to 2005.  This code as been pulled 
out from these earlier versions of the Windows Platform SDK and included in our 
source code repository so we can continue to use it with the Visual C++ Express 
Edition which doesn't include support for the ATL library.  This code has been 
modified so as to not require the atlthunk.lib which is not distributed with the 
Visual C++ Express Edition.

What’s ATL? The ATL is a set of template-based C++ classes to assist with 
creating small, fast Component Object Model (COM) objects. It has special 
support for key COM features, including stock implementations, dual interfaces, 
standard COM enumerator interfaces, connection points, tear-off interfaces, and 
ActiveX controls.

This code is included here because it is required by applications that interact 
with the DirectShow COM objects to obtain image frames from a video stream for 
vision processing.

