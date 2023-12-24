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

#pragma once

#include <JuceHeader.h>
#include "../Demos/JUCEDemos.h"

struct DemoContent;
struct CodeContent;

//==============================================================================
class DemoContentComponent final : public TabbedComponent
{
public:
    DemoContentComponent (Component& mainComponent, std::function<void (bool)> demoChangedCallback);
    ~DemoContentComponent() override;

    void resized() override;

    void setDemo (const String& category, int selectedDemoIndex);
    void clearCurrentDemo();
    int getCurrentDemoIndex() const noexcept      { return currentDemoIndex; }

    bool isShowingHomeScreen() const noexcept;
    void showHomeScreen();

    void setTabBarIndent (int indent) noexcept    { tabBarIndent = indent; }

private:
    std::function<void (bool)> demoChangedCallback;

    std::unique_ptr<DemoContent> demoContent;

   #if ! (JUCE_ANDROID || JUCE_IOS)
    std::unique_ptr<CodeContent> codeContent;
   #endif

    String currentDemoCategory;
    int currentDemoIndex = -1;
    int tabBarIndent = 0;

    //==============================================================================
    void updateLookAndFeel();
    void lookAndFeelChanged() override;

    String trimPIP (const String& fileContents);
    void ensureDemoIsShowing();
};
