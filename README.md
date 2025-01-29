# D2DL
## Direct2D Direct Layer

Aiming to be a simple abstraction over Direct2D and DirectWrite, mainly
for the purpose of creating embedded plugin (VST, CLAP, AU) GUIs using the
native Windows drawing APIs rather than hacking together some OpenGL slop.

This will eventually expose a generic API which will also be compatible with
a future layer over MacOS graphics APIs.
