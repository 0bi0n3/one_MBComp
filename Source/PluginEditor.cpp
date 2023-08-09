/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    This code has been referenced and adapted from Schiermeyer (2021a; 2021b).

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
CompressorBandControls::CompressorBandControls()
{
    addAndMakeVisible(atkSlider1);
    addAndMakeVisible(atkSlider2);
    addAndMakeVisible(atkSlider3);
    
    addAndMakeVisible(relSlider1);
    addAndMakeVisible(relSlider2);
    addAndMakeVisible(relSlider3);
    
    addAndMakeVisible(thresSlider1);
    addAndMakeVisible(thresSlider2);
    addAndMakeVisible(thresSlider3);
    
    addAndMakeVisible(ratiSlider1);
    addAndMakeVisible(ratiSlider2);
    addAndMakeVisible(ratiSlider3);
}

void CompressorBandControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
        using namespace juce;

        // Individual Control FlexBoxes
        FlexBox controlFlex1, controlFlex2, controlFlex3;

        // Configure them as rows because each control of a group is in one row.
        controlFlex1.flexDirection = FlexBox::Direction::row;
        controlFlex2.flexDirection = FlexBox::Direction::row;
        controlFlex3.flexDirection = FlexBox::Direction::row;

        controlFlex1.items.addArray({
            FlexItem(atkSlider1).withFlex(1),
            FlexItem(relSlider1).withFlex(1),
            FlexItem(thresSlider1).withFlex(1),
            FlexItem(ratiSlider1).withFlex(1)
        });

        controlFlex2.items.addArray({
            FlexItem(atkSlider2).withFlex(1),
            FlexItem(relSlider2).withFlex(1),
            FlexItem(thresSlider2).withFlex(1),
            FlexItem(ratiSlider2).withFlex(1)
        });

        controlFlex3.items.addArray({
            FlexItem(atkSlider3).withFlex(1),
            FlexItem(relSlider3).withFlex(1),
            FlexItem(thresSlider3).withFlex(1),
            FlexItem(ratiSlider3).withFlex(1)
        });

        // Main FlexBox to contain the Control FlexBoxes
        FlexBox mainFlex;
        mainFlex.flexDirection = FlexBox::Direction::column;  // Each group in a new line
        mainFlex.justifyContent = FlexBox::JustifyContent::center;

        mainFlex.items.addArray({
            FlexItem(controlFlex1).withFlex(1),
            FlexItem(controlFlex2).withFlex(1),
            FlexItem(controlFlex3).withFlex(1)
        });

        // Perform layout within the component's bounds
        mainFlex.performLayout(bounds);
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

One_MBCompAudioProcessorEditor::One_MBCompAudioProcessorEditor (One_MBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyser);
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
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void One_MBCompAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    
    controlBar.setBounds(bounds.removeFromTop(40));
    
    bandControls.setBounds(bounds.removeFromBottom(225));
    
    analyser.setBounds(bounds.removeFromTop(225));
    
    globalControls.setBounds(bounds);
    
}
