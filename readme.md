# VeryNeg

VeryNeg is a standalone open source application for the conversion of scanned film negatives.
It lets you import multiple negatives, convert the negative automatically or to select a custom scan area or sample the border and densest parts. You can also specify the gamma baked into the scan.
You also have the option to crop and orient the image and to apply automatic or manual color correction, intensity edits and sharpening.
You can hold (copy) settings from one negative to another and then export one or all of the negatives.
VeryNEg provides non-destructive editing and saves edits to a sidecart .neg file automatically.

VeryNeg was developed originally as a part of my master's thesis research for Master of Arts from Visual Communication Design, Aalto University. It is currently in beta development and still has many bugs, unsupported features etc. I don't even know if it works on any other machine than my own...

## Current state of the application:

Version 0.5.0
Mac only with MacOS 15 minimum required currently, can support older systems in the future and Windows and Linux as well.
Supports 16bit tiff files from flatbed scanner only (I guess you could convert your raw's into tiffs as a kind of a hack).
Only color negative film currently, BW will come in the future and should be easy to implement.
Please use a color corrected workflow for best results: When scanning, convert the colors to a known color space, for example AdobeRGB. I scan with an Epson scanner with ColorSync from Epson Standard to AdobeRGB.
At the moment supports only high DPI displays properly but this is an easy fix in the future.

## Technologies used

Written in C++.

### Libraries:
SFML, RmlUi, OpenImageIO, Little-CMS, nlohmann/json, portable-file-dialogs.


### Architecture:
Uses a basic Model-View-Controller.
Model owns a ColorProfiler and Negatives that reference the ColorProfiler.
Negatives use imageAlgorithms.


## Known issues:

Reset buttons don't work reliably.
Sometimes undo-redo won't work.
Crop won't show up on the preview.
The whole architecture has to be reconsidered and split into submodules. Now the classes are very crowded and hard to scale.
Should be easy to compile for Windows and Linux as well in the future.
The way each Negative object loads the full size image to memory has to be improved, since currently that uses a lot of memory.
Importing images is slow and blocks the main thread.
Exporting images is slow and has to be improved.
Lack of camera raw file support, planned for the future.

Please consider that this is my first serious application and C++ project, so be kind :) All feedback or bug or error reports are appreciated. Make sure to take a look at my thesis as well. It explains some parts of VeryNeg in more detail.