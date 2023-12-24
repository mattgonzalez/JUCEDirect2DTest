/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../../../Assets/DemoUtilities.h"
#include "JUCEDemos.h"

#include "../../../Assets/AudioLiveScrollingDisplay.h"

//==============================================================================
#if JUCE_MAC || JUCE_WINDOWS || JUCE_IOS || JUCE_ANDROID
 #include "../../../GUI/AccessibilityDemo.h"
#endif
#include "../../../GUI/AnimationAppDemo.h"
#include "../../../GUI/AnimationDemo.h"
#include "../../../GUI/BouncingBallWavetableDemo.h"
#if JUCE_USE_CAMERA && ! (JUCE_LINUX || JUCE_BSD)
 #include "../../../GUI/CameraDemo.h"
#endif
#if ! JUCE_ANDROID
 #include "../../../GUI/CodeEditorDemo.h"
#endif
#include "../../../GUI/ComponentDemo.h"
#include "../../../GUI/ComponentTransformsDemo.h"
#include "../../../GUI/DialogsDemo.h"
#include "../../../GUI/FlexBoxDemo.h"
#include "../../../GUI/FontsDemo.h"
#include "../../../GUI/GraphicsDemo.h"
#include "../../../GUI/GridDemo.h"
#include "../../../GUI/ImagesDemo.h"
#include "../../../GUI/KeyMappingsDemo.h"
#include "../../../GUI/LookAndFeelDemo.h"
#include "../../../GUI/MDIDemo.h"
#include "../../../GUI/MenusDemo.h"
#include "../../../GUI/MultiTouchDemo.h"
#if JUCE_OPENGL
 #include "../../../GUI/OpenGLAppDemo.h"
 #include "../../../GUI/OpenGLDemo.h"
 #include "../../../GUI/OpenGLDemo2D.h"
#endif
#include "../../../GUI/PropertiesDemo.h"
#if ! (JUCE_LINUX || JUCE_BSD)
 #include "../../../GUI/VideoDemo.h"
#endif
#include "../../../GUI/WebBrowserDemo.h"
#include "../../../GUI/WidgetsDemo.h"
#include "../../../GUI/WindowsDemo.h"

void registerDemos_Two() noexcept
{
   #if JUCE_MAC || JUCE_WINDOWS || JUCE_IOS || JUCE_ANDROID
    REGISTER_DEMO (AccessibilityDemo,         GUI, false)
   #endif
    REGISTER_DEMO (AnimationAppDemo,          GUI, false)
    REGISTER_DEMO (AnimationDemo,             GUI, false)
    REGISTER_DEMO (BouncingBallWavetableDemo, GUI, false)
   #if JUCE_USE_CAMERA && ! (JUCE_LINUX || JUCE_BSD)
    REGISTER_DEMO (CameraDemo,                GUI, true)
   #endif
   #if ! JUCE_ANDROID
    REGISTER_DEMO (CodeEditorDemo,            GUI, false)
   #endif
    REGISTER_DEMO (ComponentDemo,             GUI, false)
    REGISTER_DEMO (ComponentTransformsDemo,   GUI, false)
    REGISTER_DEMO (DialogsDemo,               GUI, false)
    REGISTER_DEMO (FlexBoxDemo,               GUI, false)
    REGISTER_DEMO (FontsDemo,                 GUI, false)
    REGISTER_DEMO (GraphicsDemo,              GUI, false)
    REGISTER_DEMO (GridDemo,                  GUI, false)
    REGISTER_DEMO (ImagesDemo,                GUI, false)
    REGISTER_DEMO (KeyMappingsDemo,           GUI, false)
    REGISTER_DEMO (LookAndFeelDemo,           GUI, false)
    REGISTER_DEMO (MDIDemo,                   GUI, false)
    REGISTER_DEMO (MenusDemo,                 GUI, false)
    REGISTER_DEMO (MultiTouchDemo,            GUI, false)
   #if JUCE_OPENGL
    REGISTER_DEMO (OpenGLAppDemo,             GUI, true)
    REGISTER_DEMO (OpenGLDemo2D,              GUI, true)
    REGISTER_DEMO (OpenGLDemo,                GUI, true)
   #endif
    REGISTER_DEMO (PropertiesDemo,            GUI, false)
   #if ! (JUCE_LINUX || JUCE_BSD)
    REGISTER_DEMO (VideoDemo,                 GUI, true)
   #endif
   #if JUCE_WEB_BROWSER
    REGISTER_DEMO (WebBrowserDemo,            GUI, true)
   #endif
    REGISTER_DEMO (WidgetsDemo,               GUI, false)
    REGISTER_DEMO (WindowsDemo,               GUI, false)
}

CodeEditorComponent::ColourScheme getDarkColourScheme()
{
    return getDarkCodeEditorColourScheme();
}

CodeEditorComponent::ColourScheme getLightColourScheme()
{
    return getLightCodeEditorColourScheme();
}
