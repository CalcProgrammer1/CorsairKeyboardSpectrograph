CorsairKeyboardSpectrograph
===========================

Spectrograph audio visualizer for Corsair K70 RGB keyboard for Windows and Linux.  Now also supports the Razer BlackWidow Chroma in Windows via the official SDK.


See these Reddit threads for more information about Corsair K70 USB protocol decoding:

http://www.reddit.com/r/MechanicalKeyboards/comments/2ij2um/corsair_k70_rgb_usb_protocol_reverse_engineering/

http://www.reddit.com/r/MechanicalKeyboards/comments/2imkdx/corsair_k70_rgb_music_visualizer_in_linux/

http://www.reddit.com/r/MechanicalKeyboards/comments/2iwv1i/my_k70_rgb_and_my_rgb_case_fans_rgb_battlestation/

http://www.reddit.com/r/MechanicalKeyboards/comments/2ipync/corsair_k70_rgb_usb_protocol_reverse_engineering/

http://www.reddit.com/r/MechanicalKeyboards/comments/2j6j1w/corsair_k70_rgb_linux_music_visualizer_2/

What is it
----------

This is a simple audio input visualization that uses the K70 LEDs as a "bar graph" of sorts for the audio spectrum.  It also continuously cycles through a dim rainbow pattern as the background for this visualization.  Audio comes from the default input device, so if you want to use it to visualize the output audio you must use some form of a loopback device.  In Linux you can use PulseAudio's monitor inputs to get a digital loopback while in Windows many audio chipsets have an internal "Stereo Mix" or similar loopback.  Failing that, there are programs like Virtual Audio Cable that can do a softare loopback in Windows.

Dependencies
------------

- libusb-0.1 (libusb-win32) - This library is used for communicating with the keyboard USB interface to send the LED commands.  On Windows you must install the libusb filter driver for endpoint 3 of the keyboard while on Linux you must add a udev rule for it or run the program as root.

- libSDL - This library is used for drawing the on-screen spectrum and waveform graph.  This is mostly for development/debugging purposes

- libSDL_gfx - This library provides drawing routines to use with SDL, used for drawing lines and bars


- OpenAL - This library provides the audio input for the program.

Compiling
---------

I made the project in Code::Blocks and included my .cbp project file.  In Linux you just need to install the requisite dependencies from your package manager and it should work.  In Windows, download the libraries and extract to C:\libraries\, you may need to edit the paths in the Code::Blocks project to reflect the names of your libraries directory if you're building with different versions of the libraries than I used.
