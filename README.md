# JUCE Direct2D Test

This repository is a companion for the JUCE Direct2D beta renderer. It contains small PIPs intended to demonstrate and test the renderer along with documentation in the repository wiki.

Each PIP is a single header file that you can load into the JUCE Projucer and automatically generate a project. Just open the PIP with the Projucer, or drag-and-drop the PIP onto the Projucer app.

## Using Direct2D with JUCE

You'll need the the direct2d branch of the offical JUCE repository:

https://github.com/juce-framework/JUCE/tree/direct2d

Just pull, switch to that branch, and rebuild. Or, build and run one of the PIPs in this repository.

Painting with Direct2D works just like painting with standard JUCE; you shouldn't have to make any code changes for your application to run. The PIPs demonstrate how to improve performance using cached images and paths, and how to handle the DirectX graphics adapter unexpectedly vanishing.

For more details on working with Direct2D, please refer to the wiki for this repository.

## Issues

Please submit any issues to the issue tracker on this repository, along with a DirectX diagnostic report for your system. To export the diagnostic report, run the DirectX Diagnostic Tool. You can find the DirectX Diagnostic Tool app by searching your Start menu. Alternatively, open the Run command dialog or a command prompt and enter "dxdiag".

Please submit the exported report along with your issue


## Test PIPs

### Select Renderer

This PIP shows how to switch between Direct2D and the software renderer.

### Image Draw Test

The Image Draw Test creates and paints a JUCE Image using Graphics::drawImageAt, Graphics::drawImageWithin, or Graphics::drawImageTransformed. It also shows how the renderer can cache the Image in the GPU for significantly faster performance, and how to recreate image data in the event of a display or adapter change.

### Image Edit Test

Similarly, the Image Edit Test creates a JUCE Image and then a second modified Image using either Image::clear, Image::rescaled, Image::createCopy, Image::convertedToFormat, or Image::getClipedImage.

### Path Draw Test

The Path Draw Test creates and draws either a filled Path or a stroked Path. This PIP demonstrates how, much like the Image tests, the renderer can cache the Path in the GPU for better performance.

### FlexBox Animation

The FlexBox Animation test fills the window with child components and positions them using FlexBox. Instead of setting the component bounds, the components are animiated using affine transforms, which allows anti-aliased subpixel positioning for the components. 

### Cached Path Creation Test

This PIP measures how long the renderer takes to create a cached Path by converting a Path to a Direct2D geometry realization. Note that this PIP relies on nonstandard extensions to the JUCE code that likely will not survive the official integration.

