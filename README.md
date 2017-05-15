# RoboTag

RoboTags - Fiducial Markers for Robotic Vision, Navigation and Object Recognition

This is fiducial recognition software intended for robotic vision 
applications.  It is was originally written by Mike Thompson in 
2006 and was inspired by the functionality of the closed source 
ARTag Revision 1 fiducial software.  This original code is now 
outdated by other libraries that have since implemented similar 
functionality, but is maintained here for historical purposes.

See the Presentation folder in this repository for an overview
of how fiducial markers can be used for robotics applications.

This code inspired the fiducial based navigation system 
implemented by Ubiquity Robotics for their robots and the 
associated ROS fiducials package.  Ceiling mounted fiducial 
markers allow a robot to identify its location and orientation. 
It does this by constructing a map of the ceiling fiducials. The 
position of one fiducial needs to be specified, then a map of the 
fiducials is built up from observing pairs of markers in the same 
image. Once the map has been constructed, the robot can identify 
its location by locating itself relative to one or more ceiling 
fiducials.
