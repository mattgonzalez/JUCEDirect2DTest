#pragma once

struct Effect : public juce::Direct2DImageEffectFilter
{
    Effect(uint8_t const* effectID) :
        Direct2DImageEffectFilter(effectID)
    {
    }

    ~Effect() override = default;

    void configureDirect2DEffect(ID2D1Effect* effect)
    {
        jassert(configure);
        configure(effect);
    }

    std::function<void(ID2D1Effect* effect)> configure;
};

juce::Image applyEffect(juce::Rectangle<int> size,
    const GUID& effectID,
    juce::Array<juce::Image> inputs,
    std::function<void(ID2D1Effect* effect)> configureEffect)
{
    Effect effect{ (uint8_t const*)&effectID };

    for (auto input : inputs)
    {
        effect.addInput(input);
    }

    effect.configure = configureEffect;

    auto outputImage = juce::Image{ juce::Image::ARGB, size.getWidth(), size.getHeight(), true };

    {
        juce::Graphics g{ outputImage };
        g.getInternalContext().applyEffect(effect, size.toFloat());
    }

    return outputImage;
}

juce::Image applyBlur(juce::Image const& sourceImage)
{
    return applyEffect(sourceImage.getBounds(),
        CLSID_D2D1GaussianBlur,
        {
            sourceImage
        },
        [](ID2D1Effect* effect)
        {
            effect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 1.0f);
        });
}

juce::Image applyEdgeDetect(juce::Image const& sourceImage)
{
    return applyEffect(sourceImage.getBounds(),
        CLSID_D2D1EdgeDetection,
        {
            sourceImage
        },
        [](ID2D1Effect* effect)
        {
            effect->SetValue(D2D1_EDGEDETECTION_PROP_STRENGTH, 0.0f);
            effect->SetValue(D2D1_EDGEDETECTION_PROP_BLUR_RADIUS, 1.0f);
            effect->SetValue(D2D1_EDGEDETECTION_PROP_MODE, D2D1_EDGEDETECTION_MODE_PREWITT);
        });
}

juce::Image applyArithmeticComposite(juce::Image const& first, juce::Image const& second, float c1, float c2, float c3, float c4)
{
    return applyEffect(first.getBounds(),
        CLSID_D2D1ArithmeticComposite,
        {
            first, second
        },
        [=](ID2D1Effect* effect)
        {
            effect->SetValue(D2D1_ARITHMETICCOMPOSITE_PROP_COEFFICIENTS, D2D1::Vector4F(c1, c2, c3, c4));
        });
}

juce::Image firstMinusSecond(juce::Image const& first, juce::Image const& second)
{
    return applyArithmeticComposite(first, second, 0.0f, 1.0f, -1.0f, 0.0f);
}

juce::Image firstPlusSecond(juce::Image const& first, juce::Image const& second)
{
    return applyArithmeticComposite(first, second, 0.0f, 1.0f, 1.0f, 0.0f);
}

juce::Image applyGamma(juce::Image const& image, float exponent)
{
    return applyEffect(image.getBounds(),
        CLSID_D2D1GammaTransfer,
        {
            image
        },
        [=](ID2D1Effect* effect)
        {
            effect->SetValue(D2D1_GAMMATRANSFER_PROP_RED_EXPONENT, exponent);
            effect->SetValue(D2D1_GAMMATRANSFER_PROP_GREEN_EXPONENT, exponent);
            effect->SetValue(D2D1_GAMMATRANSFER_PROP_BLUE_EXPONENT, exponent);
        });
}
