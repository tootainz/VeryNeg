# VeryNeg

VeryNeg is a standalone open source application for the conversion of scanned film negatives.
It lets you import multiple negatives, convert the negative automatically or to select a custom scan area or sample the border and densest parts. You can also specify the gamma baked into the scan.
You also have the option to crop and orient the image and to apply automatic or manual color correction, intensity edits and sharpening.
You can hold (copy) settings from one negative to another and then export one or all of the negatives.
VeryNEg provides non-destructive editing and saves edits to a sidecart .neg file automatically.

VeryNeg was developed originally as a part of my master's thesis research. It is in currently beta development and still has many bugs, unsupprted features etc.

## Current state of the application:

Version 0.5.0
Mac only with MacOS 15 minimum required currently, can support older systems in the future and Windows and Linux as well.
Supports 16bit tiff files from flatbed scanner only (I guess you could convert your raw's into tiffs as a kind of a hack).
Only color negative film currently, BW will come in the future and should be easy to implement.
Please use a color corrected workflow for best results: When scanning, convert the colors to a known color space, for example AdobeRGB. I scan with an Epson scanner with ColorSync from Epson Standard to AdobeRGB.
At the moment supports only high DPI displays properly but this is an easy fix in the future.

## Technologies used

Written in C++

Uses libraries:
SFML, RmlUi, OpenImageIO, Little-CMS, nlohmann/json, portable file dialogs


Architecture:

Model View Controller

Model has ColorProfiler and Negatives

Negative uses imageAlgorithms


Known issues:

reet doesnt work

sometimes undo redo wont work

Crop wont show up on the preview

The whole architecture has to be reconsidered adn split into submodules. Now the classes are very crowded and hard to scale

Should be easy to compile for windows and linux as well

The way each Negative object loads the full size image to memeory has to be imporved, sicne currently uses a lot of memory.

Laoding images is slow and blocks the main thread

exporting the images is slow and has to be improved

lack of camera raw file support

Please consider that this is my first serious appliaction and c++ project :)