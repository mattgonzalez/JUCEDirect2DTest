# JUCE Direct2D Test

This repostitory is intended as a companion for the JUCE Direct2D renderer. It
contains small PIPs intended to demonstrate and test the renderer.

Each PIP is a single header file that you can load into the JUCE Projucer and
automatically generate a project. Just open the PIP with the Projucer, or
drag-and-drop the PIP onot the Projucer app.

## Using Direct2D with JUCE Direct2D

You'll need the TODO branch of the offical JUCE repository:

TODO link goes here

All you need to do is #define JUCE_DIRECT2D=1 in your app and rebuild.
Or, build and run one of the PIPs, which already have JUCE_DIRECT2D defined.

Painting with Direct2D works just like painting with standard JUCE; you shouldn't
have to make any code changes for your application to run. The PIPs demonstrate
how to improve performance using cached images and paths, and how to handle the
DirectX graphics adapter unexpectedly vanishing.


## Test PIPs

There are currently three PIPS in the repository that demonstrate using
JUCE Graphics methods with Direct2D.


### Image Draw Test

The Image Draw Test creates and paints a JUCE Image using Graphics::drawImageAt,
Graphics::drawImageWithin, or Graphics::drawImageTransformed. It also shows how
by simply keeping the Image as a member variable in the Component, the renderer
can cache the Image in the GPU for significantly faster performance.

The Image Draw Test also demonstrates how to handle recreating image data in the
event of a display or adapter change.

### Image Edit Test

Similarly, the Image Edit Test creates a JUCE Image and then a second modified
Image using Image::clear, Image::rescaled, Image::createCopy,
Image::convertedToFormat, or Image::getClipedImage.

### Path Draw Test

The Path Draw Test creates and draws either a filled Path or a stroked Path. It also
demonstrates how, much like the Image tests, keeping a Path as a member variable
allows the renderer to cache the Path in the GPU.


## Optimizing for Direct2D


## Direct2D Images



## DXGI adapter changes

