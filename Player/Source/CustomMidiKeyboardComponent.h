#pragma once

#include <JuceHeader.h>

class CustomMidiKeyboardComponent : public juce::MidiKeyboardComponent
{
public:

    CustomMidiKeyboardComponent(MidiKeyboardState &state, Orientation orientation) :
    MidiKeyboardComponent(state, orientation) { }

    void drawWhiteNote (int midiNoteNumber, Graphics& g, Rectangle<float> area,
                        bool isDown, bool isOver, Colour lineColour, Colour textColour) override
    {
        auto c = Colours::transparentWhite;

        //if (isDown)  c = findColour (keyDownOverlayColourId);
        if (isDown) c = getColourForMidiNote(midiNoteNumber);
        //if (isOver)  c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));
        if (isOver) c = getColourForMidiNote(midiNoteNumber);

        g.setColour (c);
        g.fillRect (area);

        const auto currentOrientation = getOrientation();

        auto text = getWhiteNoteText (midiNoteNumber);

        if (text.isNotEmpty())
        {
            auto fontHeight = jmin (12.0f, getKeyWidth() * 0.9f);

            g.setColour (textColour);
            g.setFont (Font (fontHeight).withHorizontalScale (0.8f));

            switch (currentOrientation)
            {
                case horizontalKeyboard:            g.drawText (text, area.withTrimmedLeft (1.0f).withTrimmedBottom (2.0f), Justification::centredBottom, false); break;
                case verticalKeyboardFacingLeft:    g.drawText (text, area.reduced (2.0f), Justification::centredLeft,   false); break;
                case verticalKeyboardFacingRight:   g.drawText (text, area.reduced (2.0f), Justification::centredRight,  false); break;
                default: break;
            }
        }

        if (! lineColour.isTransparent())
        {
            g.setColour (lineColour);

            switch (currentOrientation)
            {
                case horizontalKeyboard:            g.fillRect (area.withWidth (1.0f)); break;
                case verticalKeyboardFacingLeft:    g.fillRect (area.withHeight (1.0f)); break;
                case verticalKeyboardFacingRight:   g.fillRect (area.removeFromBottom (1.0f)); break;
                default: break;
            }

            if (midiNoteNumber == getRangeEnd())
            {
                switch (currentOrientation)
                {
                    case horizontalKeyboard:            g.fillRect (area.expanded (1.0f, 0).removeFromRight (1.0f)); break;
                    case verticalKeyboardFacingLeft:    g.fillRect (area.expanded (0, 1.0f).removeFromBottom (1.0f)); break;
                    case verticalKeyboardFacingRight:   g.fillRect (area.expanded (0, 1.0f).removeFromTop (1.0f)); break;
                    default: break;
                }
            }
        }
    }

    void drawBlackNote (int midiNoteNumber, Graphics& g, Rectangle<float> area,
                        bool isDown, bool isOver, Colour noteFillColour) override
    {
        auto c = noteFillColour;

        //if (isDown)  c = c.overlaidWith (findColour (keyDownOverlayColourId));
        if (isDown) c = getColourForMidiNote(midiNoteNumber);
        //if (isOver)  c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));
        if (isOver) c = getColourForMidiNote(midiNoteNumber);

        g.setColour (c);
        g.fillRect (area);

        if (isDown)
        {
            g.setColour (noteFillColour);
            g.drawRect (area);
        }
        else
        {
            g.setColour (c.brighter());
            auto sideIndent = 1.0f / 8.0f;
            auto topIndent = 7.0f / 8.0f;
            auto w = area.getWidth();
            auto h = area.getHeight();

            switch (getOrientation())
            {
                case horizontalKeyboard:            g.fillRect (area.reduced (w * sideIndent, 0).removeFromTop   (h * topIndent)); break;
                case verticalKeyboardFacingLeft:    g.fillRect (area.reduced (0, h * sideIndent).removeFromRight (w * topIndent)); break;
                case verticalKeyboardFacingRight:   g.fillRect (area.reduced (0, h * sideIndent).removeFromLeft  (w * topIndent)); break;
                default: break;
            }
        }
    }
private:
    juce::Colour getColourForMidiNote(int midiNumber)
    {
        juce::Colour c = juce::Colours::white;
        switch (midiNumber)
        {
            case 0:
            case 12:
            case 24:
            case 36:
            case 48:
            case 60:
            case 72:
            case 84:
            case 96:
            case 108:
            case 120:
                c = Colours::red;
                break;
            case 1:
            case 13:
            case 25:
            case 37:
            case 49:
            case 61:
            case 73:
            case 85:
            case 97:
            case 109:
            case 121:
                c = Colours::orangered;
                break;
            case 2:
            case 14:
            case 26:
            case 38:
            case 50:
            case 62:
            case 74:
            case 86:
            case 98:
            case 110:
            case 122:
                c = Colours::orange;
                break;
            case 3:
            case 15:
            case 27:
            case 39:
            case 51:
            case 63:
            case 75:
            case 87:
            case 99:
            case 111:
            case 123:
                c = Colours::yellow;
                break;
            case 4:
            case 16:
            case 28:
            case 40:
            case 52:
            case 64:
            case 76:
            case 88:
            case 100:
            case 112:
            case 124:
                c = Colours::greenyellow;
                break;
            case 5:
            case 17:
            case 29:
            case 41:
            case 53:
            case 65:
            case 77:
            case 89:
            case 101:
            case 113:
            case 125:
                c = Colours::lime;
                break;
            case 6:
            case 18:
            case 30:
            case 42:
            case 54:
            case 66:
            case 78:
            case 90:
            case 102:
            case 114:
            case 126:
                c = Colours::springgreen;
                break;
            case 7:
            case 19:
            case 31:
            case 43:
            case 55:
            case 67:
            case 79:
            case 91:
            case 103:
            case 115:
            case 127:
                c = Colours::aqua;
                break;
            case 8:
            case 20:
            case 32:
            case 44:
            case 56:
            case 68:
            case 80:
            case 92:
            case 104:
            case 116:
                c = Colours::dodgerblue;
                break;
            case 9:
            case 21:
            case 33:
            case 45:
            case 57:
            case 69:
            case 81:
            case 93:
            case 105:
            case 117:
                c = Colours::mediumpurple;
                break;
            case 10:
            case 22:
            case 34:
            case 46:
            case 58:
            case 70:
            case 82:
            case 94:
            case 106:
            case 118:
                c = Colours::blueviolet;
                break;
            case 11:
            case 23:
            case 35:
            case 47:
            case 59:
            case 71:
            case 83:
            case 95:
            case 107:
            case 119:
                c = Colours::violet;
                break;
        }

        return c;
    }
};