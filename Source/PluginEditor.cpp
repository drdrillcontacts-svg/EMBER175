#include "PluginEditor.h"

namespace
{
    const juce::Colour bg        { 0xff111110 };
    const juce::Colour panel     { 0xff201e1a };
    const juce::Colour panelHi   { 0xff2b2721 };
    const juce::Colour brass     { 0xffc18b4a };
    const juce::Colour brassLite { 0xfff0cf98 };
    const juce::Colour cream     { 0xffeee4d3 };
    const juce::Colour muted     { 0xffa99a84 };
}

Ember175AudioProcessorEditor::Ember175AudioProcessorEditor(Ember175AudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(900, 620);
    setResizable(true, true);
    setResizeLimits(780, 560, 1200, 820);

    title.setText("EMBER 175", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(32.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, brassLite);
    title.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(title);

    subtitle.setText("TUBE DYNAMICS  /  REFERENCE LAB", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, muted);
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitle.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subtitle);

    setupSlider(input, "INPUT", "input");
    setupSlider(threshold, "THRESHOLD", "threshold");
    setupSlider(attack, "ATTACK", "attack");
    setupSlider(release, "RELEASE", "release");
    setupSlider(headroom, "HEADROOM", "headroom");
    setupSlider(color, "TUBE COLOR", "color");
    setupSlider(mix, "DRY / WET", "mix");
    setupSlider(output, "OUTPUT", "output");

    bypass.setButtonText("BYPASS");
    stereoLink.setButtonText("STEREO LINK");
    for (auto* b : { &bypass, &stereoLink })
    {
        b->setColour(juce::ToggleButton::textColourId, cream);
        b->setColour(juce::ToggleButton::tickColourId, brass);
        b->setColour(juce::ToggleButton::tickDisabledColourId, muted);
        addAndMakeVisible(*b);
    }
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters, "bypass", bypass);
    linkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters, "stereoLink", stereoLink);

    meterLabel.setColour(juce::Label::textColourId, brassLite);
    meterLabel.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    meterLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(meterLabel);

    footerLabel.setText("INDEPENDENT DSP RESEARCH BUILD  •  NOT AFFILIATED WITH UNIVERSAL AUDIO", juce::dontSendNotification);
    footerLabel.setColour(juce::Label::textColourId, muted);
    footerLabel.setFont(juce::Font(juce::FontOptions(9.5f)));
    footerLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(footerLabel);
    startTimerHz(20);
}

void Ember175AudioProcessorEditor::setupSlider(juce::Slider& s, const juce::String& labelText, const juce::String& id)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 22);
    s.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                          juce::MathConstants<float>::pi * 2.75f, true);
    s.setColour(juce::Slider::rotarySliderFillColourId, brass);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff51483b));
    s.setColour(juce::Slider::thumbColourId, brassLite);
    s.setColour(juce::Slider::textBoxTextColourId, cream);
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff171614));
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff51483b));
    addAndMakeVisible(s);

    auto l = std::make_unique<juce::Label>();
    l->setText(labelText, juce::dontSendNotification);
    l->setColour(juce::Label::textColourId, muted);
    l->setJustificationType(juce::Justification::centred);
    l->setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
    addAndMakeVisible(*l);
    labels.push_back(std::move(l));
    attachments.push_back(std::make_unique<Attachment>(processor.parameters, id, s));
}

void Ember175AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    const auto bounds = getLocalBounds().toFloat().reduced(10.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff30281f), bounds.getX(), bounds.getY(),
                                           juce::Colour(0xff171614), bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 18.0f);
    g.setColour(juce::Colour(0xff675039));
    g.drawRoundedRectangle(bounds, 18.0f, 1.2f);

    // Header plaque and divider
    g.setColour(brass);
    g.fillRoundedRectangle(28.0f, 29.0f, 4.0f, 52.0f, 2.0f);
    g.setColour(juce::Colour(0xff51402d));
    g.drawLine(28.0f, 96.0f, (float)getWidth() - 28.0f, 96.0f, 1.0f);

    // Main control deck
    auto deck = juce::Rectangle<float>(28.0f, 112.0f, (float)getWidth() - 56.0f, (float)getHeight() - 254.0f);
    g.setColour(panel);
    g.fillRoundedRectangle(deck, 13.0f);
    g.setColour(juce::Colour(0xff4b3d2d));
    g.drawRoundedRectangle(deck, 13.0f, 1.0f);

    // Subtle divider between the two rows of controls
    g.setColour(juce::Colour(0xff3b3126));
    g.drawLine(deck.getX() + 18.0f, deck.getCentreY(), deck.getRight() - 18.0f, deck.getCentreY(), 1.0f);

    // Bottom meter bay
    const float bayY = (float)getHeight() - 82.0f;
    g.setColour(juce::Colour(0xff181715));
    g.fillRoundedRectangle(28.0f, bayY, (float)getWidth() - 56.0f, 42.0f, 8.0f);
    g.setColour(juce::Colour(0xff4b3d2d));
    g.drawRoundedRectangle(28.0f, bayY, (float)getWidth() - 56.0f, 42.0f, 8.0f, 1.0f);

    const float meterX = (float)getWidth() * 0.50f;
    const float meterY = bayY + 12.0f;
    const float meterW = (float)getWidth() * 0.25f;
    const float meterH = 12.0f;
    g.setColour(juce::Colour(0xff39342d));
    g.fillRoundedRectangle(meterX, meterY, meterW, meterH, 3.0f);
    const float gr = juce::jlimit(0.0f, 18.0f, processor.getGainReductionDb());
    const float fillW = meterW * gr / 18.0f;
    g.setColour(gr > 12.0f ? juce::Colour(0xffc86d45) : brass);
    if (fillW > 0.0f)
        g.fillRoundedRectangle(meterX, meterY, fillW, meterH, 3.0f);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    g.drawText("GR", meterX - 29.0f, meterY - 1.0f, 23.0f, 14.0f, juce::Justification::centredLeft);

    // Decorative corner screws
    g.setColour(juce::Colour(0xff7a6a53));
    for (auto pt : { juce::Point<float>(22, 22), juce::Point<float>((float)getWidth() - 22, 22),
                     juce::Point<float>(22, (float)getHeight() - 22),
                     juce::Point<float>((float)getWidth() - 22, (float)getHeight() - 22) })
    {
        g.fillEllipse(pt.x - 3.0f, pt.y - 3.0f, 6.0f, 6.0f);
        g.setColour(juce::Colour(0xff29241e));
        g.drawLine(pt.x - 2.0f, pt.y, pt.x + 2.0f, pt.y, 1.0f);
        g.setColour(juce::Colour(0xff7a6a53));
    }
}

void Ember175AudioProcessorEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    title.setBounds(43, 25, 300, 38);
    subtitle.setBounds(45, 63, 420, 20);

    juce::Slider* sliders[] = { &input, &threshold, &attack, &release, &headroom, &color, &mix, &output };
    const int left = 42;
    const int right = w - 42;
    const int colW = (right - left) / 4;
    const int deckTop = 119;
    const int rowH = (h - 254 - 18) / 2;
    for (int i = 0; i < 8; ++i)
    {
        const int row = i / 4;
        const int col = i % 4;
        const int x = left + col * colW;
        const int y = deckTop + row * rowH;
        labels[(size_t)i]->setBounds(x, y + 3, colW, 22);
        sliders[i]->setBounds(x + (colW - 112) / 2, y + 24, 112, juce::jmax(112, rowH - 28));
    }
    const int controlsY = h - 121;
    bypass.setBounds(45, controlsY, 112, 26);
    stereoLink.setBounds(165, controlsY, 145, 26);
    meterLabel.setBounds(w - 245, controlsY, 190, 26);
    footerLabel.setBounds(250, h - 31, w - 300, 16);
}

void Ember175AudioProcessorEditor::timerCallback()
{
    meterLabel.setText("GAIN REDUCTION  " + juce::String(processor.getGainReductionDb(), 1) + " dB", juce::dontSendNotification);
    repaint();
}
