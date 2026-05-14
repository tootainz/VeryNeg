VeryNeg

VeryNeg is a standalone open source appliaction for the conversion of scanned film negatives.
It lets you import multipel negatives. Covner the negatvie automatically or to select custom scan area and border or densest part sample, scan gamma.
Lets you crop adn orient the image and to apply autoamtic or manual colro correction, intensity edtis adn sharpening.
Can hold (copy) settigns from one negatvie to another. adn then export one or all negatives.
Non destructive editing and saves edits to a sidecart .neg file automatically

VeryNeg was developed originally as part of my masters thesis research

Currently supported features:

Version 0.5.0
Mac only
16bit tiff files from flatbed scanner only (I guess you could convert your raw's into tiffs as a kind of a hack)
only color negative film
please use a color corrected workflow for best results: when scannign covnert the colros to a knows space, for example AdobeRGB.
At the moment supports only high dpi displays properly but this is an easy fix in the future

Written in C++

uses libraries:

SFML, RmlUi, OpenImageIO, LittleCMS, Nlohmann-json, portable file dialogs


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