# JUCE Direct2D Test

This repostitory is intended as a companion for the JUCE Direct2D renderer. It
contains small PIPs intended to demonstrate and test the renderer.

Each PIP is a single header file that you can load into the JUCE Projucer and
automatically generate a project. Just open the PIP with the Projucer, or
drag-and-drop the PIP onot the Projucer app.

## Using Direct2D with JUCE Direct2D

You'll need the the direct2d branch of the offical JUCE repository:

TODO link goes here

Just pull, switch to that branch, and rebuild. Or, build and run one of the PIPs.

Painting with Direct2D works just like painting with standard JUCE; you shouldn't
have to make any code changes for your application to run. The PIPs demonstrate
how to improve performance using cached images and paths, and how to handle the
DirectX graphics adapter unexpectedly vanishing.

For more details on working with Direct2D, please refer to the wiki for this 
repository.

## Issues

Please submit any issues to the issue tracker on this repository, along with a 
DirectX diagnostic report for your system. To export the diagnostic report, run
the DirectX Diagnostic Tool. You can find the DirectX Diagnostic Tool app by searching 
your Start menu. Alternatively, open the Run command dialog or a command prompt 
and enter "dxdiag".

Please submit the exported report along with your issue


## Test PIPs

The PIPs in the repository demonstrate and validate using JUCE Graphics methods
with Direct2D.

### Select Renderer

This PIP explains how to switch between Direct2D mode and the software renderer.

### Image Draw Test

The Image Draw Test creates and paints a JUCE Image using Graphics::drawImageAt,
Graphics::drawImageWithin, or Graphics::drawImageTransformed. It also shows how
by simply keeping the Image as a member variable in the Component, the renderer
can cache the Image in the GPU for significantly faster performance.

The Image Draw Test also shows how to handle recreating image data in the
event of a display or adapter change.

### Image Edit Test

Similarly, the Image Edit Test creates a JUCE Image and then a second modified
Image using either Image::clear, Image::rescaled, Image::createCopy,
Image::convertedToFormat, or Image::getClipedImage.

### Path Draw Test

The Path Draw Test creates and draws either a filled Path or a stroked Path. It also
lays out how, much like the Image tests, keeping a Path as a member variable
allows the renderer to cache the Path in the GPU.


