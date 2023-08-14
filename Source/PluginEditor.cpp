/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    This code has been referenced and adapted from Schiermeyer (2021a; 2021b).

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <array>

/*
  ==============================================================================
    **************************************************************************
    NOTE TO READER: This following section of code was explicitly referenced from
    Schiermeyer (2021a; 2021b) to demonstrate the spectrum in the plugin. This
    code block has not be originally created by the author. The end of the block
    will be clearly stated for ease of reference to the reader.
    **************************************************************************
  ==============================================================================
*/

SpectrumAnalyser::SpectrumAnalyser(One_MBCompAudioProcessor& p) :
audioProcessor(p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for( auto param : params )
    {
        param->addListener(this);
    }
    
    startTimerHz(60);
}

SpectrumAnalyser::~SpectrumAnalyser()
{
    const auto& params = audioProcessor.getParameters();
    for( auto param : params )
    {
        param->removeListener(this);
    }
}

void SpectrumAnalyser::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    drawBackgroundGrid(g);
    
    auto responseArea = getAnalysisArea();
    
    if( shouldShowFFTAnalysis )
    {
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        
        g.setColour(Colour(97u, 18u, 167u)); //purple-
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
        
        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        
        g.setColour(Colour(215u, 201u, 134u));
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }
    
    Path border;
    
    border.setUsingNonZeroWinding(false);
    
    border.addRoundedRectangle(getRenderArea(), 4);
    border.addRectangle(getLocalBounds());
    
    g.setColour(Colours::black);
    
    g.fillPath(border);
    
    drawTextLabels(g);
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
}

std::vector<float> SpectrumAnalyser::getFrequencies()
{
    return std::vector<float>
    {
        20, /*30, 40,*/ 50, 100,
        200, /*300, 400,*/ 500, 1000,
        2000, /*3000, 4000,*/ 5000, 10000,
        20000
    };
}

std::vector<float> SpectrumAnalyser::getGains()
{
    return std::vector<float>
    {
        -24, -12, 0, 12, 24
    };
}

std::vector<float> SpectrumAnalyser::getXs(const std::vector<float> &freqs, float left, float width)
{
    std::vector<float> xs;
    for( auto f : freqs )
    {
        auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
        xs.push_back( left + width * normX );
    }
    
    return xs;
}

void SpectrumAnalyser::drawBackgroundGrid(juce::Graphics &g)
{
    using namespace juce;
    auto freqs = getFrequencies();
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    
    auto xs = getXs(freqs, left, width);
    
    g.setColour(Colours::dimgrey);
    for( auto x : xs )
    {
        g.drawVerticalLine(x, top, bottom);
    }
    
    auto gain = getGains();
    
    for( auto gDb : gain )
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey );
        g.drawHorizontalLine(y, left, right);
    }
}

void SpectrumAnalyser::drawTextLabels(juce::Graphics &g)
{
    using namespace juce;
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    
    auto freqs = getFrequencies();
    auto xs = getXs(freqs, left, width);
    
    for( int i = 0; i < freqs.size(); ++i )
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if( f > 999.f )
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if( addK )
            str << "k";
        str << "Hz";
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;

        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);
        
        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }
    
    auto gain = getGains();

    for( auto gDb : gain )
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        
        String str;
        if( gDb > 0 )
            str << "+";
        str << gDb;
        
        auto textWidth = g.getCurrentFont().getStringWidth(str);
        
        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);
        
        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey );
        
        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
        
        str.clear();
        str << (gDb - 24.f);

        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
    }
}

void SpectrumAnalyser::resized()
{
    using namespace juce;
}

void SpectrumAnalyser::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;
    while( leftChannelFifo->getNumCompleteBuffersAvailable() > 0 )
    {
        if( leftChannelFifo->getAudioBuffer(tempIncomingBuffer) )
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                                              monoBuffer.getReadPointer(0, size),
                                              monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                                              tempIncomingBuffer.getReadPointer(0, 0),
                                              size);
            
            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }
    
    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / double(fftSize);

    while( leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0 )
    {
        std::vector<float> fftData;
        if( leftChannelFFTDataGenerator.getFFTData( fftData) )
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }
    
    while( pathProducer.getNumPathsAvailable() > 0 )
    {
        pathProducer.getPath( leftChannelFFTPath );
    }
}

void SpectrumAnalyser::timerCallback()
{
    if( shouldShowFFTAnalysis )
    {
        auto fftBounds = getAnalysisArea().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();
        
        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if( parametersChanged.compareAndSetBool(false, true) )
    {
        
    }
    
    repaint();
}

juce::Rectangle<int> SpectrumAnalyser::getRenderArea()
{
    auto bounds = getLocalBounds();
    
    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);
    
    return bounds;
}


juce::Rectangle<int> SpectrumAnalyser::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}

/*
  ==============================================================================
    **************************************************************************
                            END OF SPECTRUM CODE BLOCK
                            (Schiermeyer, 2021a; 2021b)
    **************************************************************************
  ==============================================================================
*/

ControlBar::ControlBar(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace PluginParameters;
    const auto& parameters = GetParameters();
    
    // Use only the button label to initialize the ToggleButton.
    bypassButton1 = std::make_unique<Buttons>("X");
    soloButton1 = std::make_unique<Buttons>("S");
    muteButton1 = std::make_unique<Buttons>("M");
    titleLabel1.setText("LB", juce::NotificationType::dontSendNotification);
        
    bypassButton2 = std::make_unique<Buttons>("X");
    soloButton2 = std::make_unique<Buttons>("S");
    muteButton2 = std::make_unique<Buttons>("M");
    titleLabel2.setText("MB", juce::NotificationType::dontSendNotification);

    bypassButton3 = std::make_unique<Buttons>("X");
    soloButton3 = std::make_unique<Buttons>("S");
    muteButton3 = std::make_unique<Buttons>("M");
    titleLabel3.setText("HB", juce::NotificationType::dontSendNotification);

    auto makeAttachmentHelper = [&parameters, &apvts](auto& attachment, const auto& name, auto& button)
    {
        makeBtnAttachment(attachment, apvts, parameters, name, button);
    };
        
    makeAttachmentHelper(bypassButtonAttachment1, ParamNames::Bypass_LB, *bypassButton1);
    makeAttachmentHelper(soloButtonAttachment1, ParamNames::Solo_LB, *soloButton1);
    makeAttachmentHelper(muteButtonAttachment1, ParamNames::Mute_LB, *muteButton1);
    
    makeAttachmentHelper(bypassButtonAttachment2, ParamNames::Bypass_MB, *bypassButton2);
    makeAttachmentHelper(soloButtonAttachment2, ParamNames::Solo_MB, *soloButton2);
    makeAttachmentHelper(muteButtonAttachment2, ParamNames::Mute_MB, *muteButton2);
    
    makeAttachmentHelper(bypassButtonAttachment3, ParamNames::Bypass_HB, *bypassButton3);
    makeAttachmentHelper(soloButtonAttachment3, ParamNames::Solo_HB, *soloButton3);
    makeAttachmentHelper(muteButtonAttachment3, ParamNames::Mute_HB, *muteButton3);
    
    
    // Add buttons and title labels to the component
    addAndMakeVisible(*bypassButton1);
    addAndMakeVisible(*soloButton1);
    addAndMakeVisible(*muteButton1);
    addAndMakeVisible(titleLabel1);
    
    addAndMakeVisible(*bypassButton2);
    addAndMakeVisible(*soloButton2);
    addAndMakeVisible(*muteButton2);
    addAndMakeVisible(titleLabel2);
    
    addAndMakeVisible(*bypassButton3);
    addAndMakeVisible(*soloButton3);
    addAndMakeVisible(*muteButton3);
    addAndMakeVisible(titleLabel3);
}

void ControlBar::resized()
{
    auto bounds = getLocalBounds();
    auto buttonWidth = bounds.getWidth() / 9;
    auto buttonHeight = 20;  // Adjust this as needed
    auto titleHeight = 18;   // Adjust this as needed

    // Position the low group
    titleLabel1.setBounds(bounds.removeFromTop(titleHeight));
    bypassButton1->setBounds(bounds.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    soloButton1->setBounds(bounds.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    muteButton1->setBounds(bounds.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    
    // Reset bounds for mid group
    bounds = getLocalBounds();
    auto midColumn = bounds.removeFromRight(335);
    // Position the mid group
    titleLabel2.setBounds(midColumn.removeFromTop(titleHeight));
    bypassButton2->setBounds(midColumn.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    soloButton2->setBounds(midColumn.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    muteButton2->setBounds(midColumn.removeFromLeft(buttonWidth).withHeight(buttonHeight));

    // Reset bounds for high group
    bounds = getLocalBounds();
    auto rightColumn = bounds.removeFromRight(175);  // Adjust this as needed
    // Position the high group
    titleLabel3.setBounds(rightColumn.removeFromTop(titleHeight));
    bypassButton3->setBounds(rightColumn.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    soloButton3->setBounds(rightColumn.removeFromLeft(buttonWidth).withHeight(buttonHeight));
    muteButton3->setBounds(rightColumn.removeFromLeft(buttonWidth).withHeight(buttonHeight));
}


void ControlBar::paint(juce::Graphics& g)
{
    using namespace juce;
    auto bounds = getLocalBounds();
    g.setColour(Colours::grey);
    g.fillAll();
    
    auto localBounds = bounds;
    
    bounds.reduce(3,3);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);
    
    g.drawRect(localBounds);
}

//==============================================================================

template<typename T>
bool addKilohertz(T& value)
{
    if( value > static_cast<T>(999) )
    {
        value /= static_cast<T>(1000);
        return true;
    }
    return false;
}


juce::String getValString(const juce::RangedAudioParameter& param,
                          bool getLow,
                          juce::String suffix)
{
    juce::String string;
    
    auto value = getLow ? param.getNormalisableRange().start : param.getNormalisableRange().end;
    
    bool useK = addKilohertz(value);
    string << value;
    
    if( useK )
    {
        string << "k";
    }
    
    string << suffix;
    
    return string;
}
//==============================================================================

void LookAndFeel::drawRotarySlider(juce::Graphics &g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider &slider)
{
    using namespace juce;
    auto bounds = Rectangle<float>(x, y, width, height);
    
    auto enabled = slider.isEnabled();
    
    g.setColour(enabled ? Colour(236u, 114u, 41u) : Colours::darkgrey);
    g.fillEllipse(bounds);
    
    g.setColour(enabled ? Colour(46u, 48u, 45u) : Colours::grey);
    g.drawEllipse(bounds, 1.f);
    
    if(auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);
        
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        g.setColour(enabled ? Colours::black : Colours::darkgrey);
        g.fillRect(r);
        
        g.setColour(enabled ? Colours::white : Colours::lightgrey);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    if(auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
    {
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds();
        auto size = jmin(bounds.getWidth(), bounds.getHeight() - 6);
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float ang = 30.f;
        size -= 6;
        
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());
        
        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
        
        auto colour = toggleButton.getToggleState() ? Colours::dimgrey : Colours::red;
        
        g.setColour(colour);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2);
    }
}
//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
    auto bounds = getLocalBounds();
    
    g.setColour(Colours::darkkhaki);
    g.drawFittedText(getName(), bounds.removeFromTop(getTextHeight() + 2), Justification::centredTop, 1);
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(Colour(0u, 172u, 1u));
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for(int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
        
    }
    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    bounds.removeFromTop(getTextHeight());
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight();
    
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), bounds.getCentreY());
//    r.setY(2);
    r.setY(bounds.getY());
    
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if(auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    bool addK = false;
    
    if(auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        
//        if(val > 999.f)
//        {
//            val /= 1000.f;
//            addK = true;
//        }
        
        addK = addKilohertz(val);
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse;
    }
    
    if(suffix.isNotEmpty())
    {
        str << " ";
        if(addK)
        {
            str << "k";
        }
        str << suffix;
    }
    
    return str;
}

//==============================================================================

Placeholder::Placeholder()
{
    juce::Random r;
    customColour = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}

//==============================================================================
CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace PluginParameters;
    const auto& parameters = GetParameters();
    
    auto getParamHelper = [&parameters, &apvts](const auto& name) -> auto&
    {
        return getParameter(apvts, parameters, name);
    };
    
    // Create and assign parameters
    atkSlider1 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Attack_LB),"ms", "Att-LB");
    relSlider1 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Release_LB),"ms", "Rel-LB");
    thresSlider1 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Threshold_LB),"dB", "Thr-LB");
    ratiSlider1 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Ratio_LB),"Amt", "Rat-LB");
    
    atkSlider2 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Attack_MB),"ms", "Att-MB");
    relSlider2 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Release_MB),"ms", "Rel-MB");
    thresSlider2 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Threshold_MB),"dB", "Thr-MB");
    ratiSlider2 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Ratio_MB),"Amt", "Rat-MB");
    
    atkSlider3 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Attack_HB),"ms", "Att-HB");
    relSlider3 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Release_HB),"ms", "Rel-HB");
    thresSlider3 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Threshold_HB),"dB", "Thr-HB");
    ratiSlider3 = std::make_unique<RotarySliderWL2>(getParamHelper(ParamNames::Ratio_HB),"Amt", "Rat-HB");
    
    auto makeAttachmentHelper = [&parameters, &apvts](auto& attachment, const auto& name, auto& slider)
    {
        makeAttachment(attachment, apvts, parameters, name, slider);
    };
    
    makeAttachmentHelper(atkSlider1_Attachment, ParamNames::Attack_LB, *atkSlider1);
    makeAttachmentHelper(relSlider1_Attachment, ParamNames::Release_LB, *relSlider1);
    makeAttachmentHelper(thresSlider1_Attachment, ParamNames::Threshold_LB, *thresSlider1);
    makeAttachmentHelper(ratiSlider1_Attachment, ParamNames::Ratio_LB, *ratiSlider1);
    
    makeAttachmentHelper(atkSlider2_Attachment, ParamNames::Attack_MB, *atkSlider2);
    makeAttachmentHelper(relSlider2_Attachment, ParamNames::Release_MB, *relSlider2);
    makeAttachmentHelper(thresSlider2_Attachment, ParamNames::Threshold_MB, *thresSlider2);
    makeAttachmentHelper(ratiSlider2_Attachment, ParamNames::Ratio_MB, *ratiSlider2);
    
    makeAttachmentHelper(atkSlider3_Attachment, ParamNames::Attack_HB, *atkSlider3);
    makeAttachmentHelper(relSlider3_Attachment, ParamNames::Release_HB, *relSlider3);
    makeAttachmentHelper(thresSlider3_Attachment, ParamNames::Threshold_HB, *thresSlider3);
    makeAttachmentHelper(ratiSlider3_Attachment, ParamNames::Ratio_HB, *ratiSlider3);
    
    addLabelPairs(atkSlider1->labels, getParamHelper(ParamNames::Attack_LB), "ms");
    addLabelPairs(relSlider1->labels, getParamHelper(ParamNames::Release_LB), "ms");
    addLabelPairs(thresSlider1->labels, getParamHelper(ParamNames::Threshold_LB), "dB");
    addLabelPairs(ratiSlider1->labels, getParamHelper(ParamNames::Ratio_LB), "Amt");
    
    addLabelPairs(atkSlider2->labels, getParamHelper(ParamNames::Attack_MB), "ms");
    addLabelPairs(relSlider2->labels, getParamHelper(ParamNames::Release_MB), "ms");
    addLabelPairs(thresSlider2->labels, getParamHelper(ParamNames::Threshold_MB), "dB");
    addLabelPairs(ratiSlider2->labels, getParamHelper(ParamNames::Ratio_MB), "Amt");
    
    addLabelPairs(atkSlider3->labels, getParamHelper(ParamNames::Attack_HB), "ms");
    addLabelPairs(relSlider3->labels, getParamHelper(ParamNames::Release_HB), "ms");
    addLabelPairs(thresSlider3->labels, getParamHelper(ParamNames::Threshold_HB), "dB");
    addLabelPairs(ratiSlider3->labels, getParamHelper(ParamNames::Ratio_HB), "Amt");
    
    addAndMakeVisible(*atkSlider1);
    addAndMakeVisible(*relSlider1);
    addAndMakeVisible(*thresSlider1);
    addAndMakeVisible(*ratiSlider1);
    
    addAndMakeVisible(*atkSlider2);
    addAndMakeVisible(*relSlider2);
    addAndMakeVisible(*thresSlider2);
    addAndMakeVisible(*ratiSlider2);
    
    addAndMakeVisible(*atkSlider3);
    addAndMakeVisible(*relSlider3);
    addAndMakeVisible(*thresSlider3);
    addAndMakeVisible(*ratiSlider3);
    
}

void CompressorBandControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    using namespace juce;

    // Main FlexBox to contain the Control FlexBoxes
    FlexBox mainFlex;
    mainFlex.flexDirection = FlexBox::Direction::column;  // Each group in a new line
    mainFlex.justifyContent = FlexBox::JustifyContent::center;

    // Individual Control FlexBoxes (for better structuring)
    FlexBox controlFlex1, controlFlex2, controlFlex3;

    // Configure as rows
    controlFlex1.flexDirection = FlexBox::Direction::row;
    controlFlex1.items.addArray({
        FlexItem(*atkSlider1).withFlex(1),
        FlexItem(*relSlider1).withFlex(1),
        FlexItem(*thresSlider1).withFlex(1),
        FlexItem(*ratiSlider1).withFlex(1)
        });
    
    controlFlex2.flexDirection = FlexBox::Direction::row;
    controlFlex2.items.addArray({
        FlexItem(*atkSlider2).withFlex(1),
        FlexItem(*relSlider2).withFlex(1),
        FlexItem(*thresSlider2).withFlex(1),
        FlexItem(*ratiSlider2).withFlex(1)
        });
    
    controlFlex3.flexDirection = FlexBox::Direction::row;
    controlFlex3.items.addArray({
        FlexItem(*atkSlider3).withFlex(1),
        FlexItem(*relSlider3).withFlex(1),
        FlexItem(*thresSlider3).withFlex(1),
        FlexItem(*ratiSlider3).withFlex(1)
        });
    
    mainFlex.items.addArray({
        FlexItem(controlFlex1).withFlex(1),
        FlexItem(controlFlex2).withFlex(1),
        FlexItem(controlFlex3).withFlex(1)
    });

    // Perform layout within the component's bounds
    mainFlex.performLayout(bounds);
}

void CompressorBandControls::paint(juce::Graphics& g)
{
    using namespace juce;
    auto bounds = getLocalBounds();
    g.setColour(Colours::grey);
    g.fillAll();
    
    auto localBounds = bounds;
    
    bounds.reduce(3,3);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);
    
    g.drawRect(localBounds);
}


//==============================================================================
GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace PluginParameters;
    const auto& parameters = GetParameters();
    
    auto getParamHelper = [&parameters, &apvts](const auto& name) -> auto&
    {
        return getParameter(apvts, parameters, name);
    };
    
    auto& gainInputParameter = getParamHelper(ParamNames::Gain_Input);
    auto& lowMidParameter = getParamHelper(ParamNames::Low_Mid_XO_Frequency);
    auto& midHighParameter = getParamHelper(ParamNames::Mid_High_XO_Frequency);
    auto& gainOutputParameter = getParamHelper(ParamNames::Gain_Output);
    
    inputGainSlider = std::make_unique<RotarySliderWL>(gainInputParameter,"dB", "Input Gain");
    lowMidCrossoverSlider = std::make_unique<RotarySliderWL>(lowMidParameter,"Hz", "Low-Mid Range");
    midHighCrossoverSlider = std::make_unique<RotarySliderWL>(midHighParameter,"Hz", "Mid-Hi Range");
    outputGainSlider = std::make_unique<RotarySliderWL>(gainOutputParameter,"dB", "Output Gain");
    
    auto makeAttachmentHelper = [&parameters, &apvts](auto& attachment,
                                                      const auto& name,
                                                      auto& slider)
    {
        makeAttachment(attachment, apvts, parameters, name, slider);
    };
    
    makeAttachmentHelper(inputGainSliderAttachment,
                         ParamNames::Gain_Input,
                         *inputGainSlider);
    makeAttachmentHelper(lowMidCrossoverSliderAttachment,
                         ParamNames::Low_Mid_XO_Frequency,
                         *lowMidCrossoverSlider);
    makeAttachmentHelper(midHighCrossoverSliderAttachment,
                         ParamNames::Mid_High_XO_Frequency,
                         *midHighCrossoverSlider);
    makeAttachmentHelper(outputGainSliderAttachment,
                         ParamNames::Gain_Output,
                         *outputGainSlider);
    
    addLabelPairs(inputGainSlider->labels,
                  gainInputParameter,
                  "dB");
    addLabelPairs(lowMidCrossoverSlider->labels,
                  lowMidParameter,
                  "Hz");
    addLabelPairs(midHighCrossoverSlider->labels,
                  midHighParameter,
                  "Hz");
    addLabelPairs(outputGainSlider->labels,
                  gainOutputParameter,
                  "dB");
    
    addAndMakeVisible(*inputGainSlider);
    addAndMakeVisible(*lowMidCrossoverSlider);
    addAndMakeVisible(*midHighCrossoverSlider);
    addAndMakeVisible(*outputGainSlider);
}


void GlobalControls::paint(juce::Graphics& g)
{
    using namespace juce;
    auto bounds = getLocalBounds();
    g.setColour(Colours::grey);
    g.fillAll();
    
    auto localBounds = bounds;
    
    bounds.reduce(3,3);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 3);
    
    g.drawRect(localBounds);
    
}

void GlobalControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    using namespace juce;
    
    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;
    
    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);
    
    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inputGainSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMidCrossoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighCrossoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outputGainSlider).withFlex(1.f));
    flexBox.items.add(endCap);
    
    flexBox.performLayout(bounds);
}

One_MBCompAudioProcessorEditor::One_MBCompAudioProcessorEditor (One_MBCompAudioProcessor& processor)
    : AudioProcessorEditor (&processor), audioProcessor (processor)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(controlBar);
    addAndMakeVisible(specAnalyser);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);
    
    setSize (500, 600);
}

One_MBCompAudioProcessorEditor::~One_MBCompAudioProcessorEditor()
{
}

//==============================================================================
void One_MBCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
//
//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
//
    g.fillAll(juce::Colours::black);
}

void One_MBCompAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    
    controlBar.setBounds(bounds.removeFromTop(40));
    
    bandControls.setBounds(bounds.removeFromBottom(225));
    
    specAnalyser.setBounds(bounds.removeFromTop(225));
    
    globalControls.setBounds(bounds);
    
}
