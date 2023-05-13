#pragma once

#include <JuceHeader.h>
#include "Paths.h"

class PlaylistComponent    : public juce::Component,
                                  public juce::TableListBoxModel
{
public:
    PlaylistComponent(std::function<void(String)>& trackSelectFn) : trackSelected(trackSelectFn)
    {
        /* debugPlaylistLoading(); */ //Replaced by explicit load function called from outside of this component

        addAndMakeVisible (table);

        table.setColour (juce::ListBox::outlineColourId, juce::Colours::grey);
        table.setOutlineThickness (1);
        table.setMultipleSelectionEnabled (false);
    }

    int getNumRows() override
    {
        return numRows;
    }

    int getSelectedRow()
    {
        return selectedRow;
    }

    void paintRowBackground (juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        auto alternateColour = getLookAndFeel().findColour (juce::ListBox::backgroundColourId)
                .interpolatedWith (getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);
        if (rowIsSelected)
            g.fillAll (juce::Colours::lightblue);
        else if (rowNumber % 2)
            g.fillAll (alternateColour);
    }

    void paintCell (juce::Graphics& g, int rowNumber, int columnId,
                    int width, int height, bool rowIsSelected) override
    {
        g.setColour (rowIsSelected ? juce::Colours::darkblue : getLookAndFeel().findColour (juce::ListBox::textColourId));
        g.setFont (font);

        if (auto* rowElement = dataList->getChildElement (rowNumber))
        {
            auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

            g.drawText (text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        }

        g.setColour (getLookAndFeel().findColour (juce::ListBox::backgroundColourId));
        g.fillRect (width - 1, 0, 1, height);
    }

    void sortOrderChanged (int newSortColumnId, bool isForwards) override
    {
        if (newSortColumnId != 0)
        {
            DataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            dataList->sortChildElements (sorter);

            table.updateContent();
        }
    }

    Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected,
                                        Component* existingComponentToUpdate) override
    {
        if (isRowSelected)
        {
            auto title = dataList->getChildElement(rowNumber)->getStringAttribute ("Title");
            selectedRow = rowNumber;
            trackSelected(title);
        }

        if (columnId == 9)  // [8]
        {
            auto* selectionBox = static_cast<SelectionColumnCustomComponent*> (existingComponentToUpdate);

            if (selectionBox == nullptr)
                selectionBox = new SelectionColumnCustomComponent (*this);

            selectionBox->setRowAndColumn (rowNumber, columnId);
            return selectionBox;
        }

        if (columnId == 8)
        {
            auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

            if (textLabel == nullptr)
                textLabel = new EditableTextCustomComponent (*this);

            textLabel->setRowAndColumn (rowNumber, columnId);
            return textLabel;
        }

        jassert (existingComponentToUpdate == nullptr);
        return nullptr;
    }

    int getColumnAutoSizeWidth (int columnId) override
    {
        if (columnId == 9)
            return 50;

        int widest = 32;

        for (auto i = getNumRows(); --i >= 0;)
        {
            if (auto* rowElement = dataList->getChildElement (i))
            {
                auto text = rowElement->getStringAttribute (getAttributeNameForColumnId (columnId));

                widest = juce::jmax (widest, font.getStringWidth (text));
            }
        }

        return widest + 8;
    }

    int getSelection (const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getIntAttribute ("Select");
    }

    void setSelection (const int rowNumber, const int newSelection)
    {
        dataList->getChildElement (rowNumber)->setAttribute ("Select", newSelection);
    }

    juce::String getText (const int columnNumber, const int rowNumber) const
    {
        return dataList->getChildElement (rowNumber)->getStringAttribute (getAttributeNameForColumnId (columnNumber));
    }

    void setText (const int columnNumber, const int rowNumber, const juce::String& newText)
    {
        const auto& columnName = table.getHeader().getColumnName (columnNumber);
        dataList->getChildElement (rowNumber)->setAttribute (columnName, newText);
    }

    //==============================================================================
    void resized() override
    {
        table.setBoundsInset (juce::BorderSize<int> (0));
    }

    void loadData (juce::File playlistXml)
    {
        if (playlistXml == juce::File() || ! playlistXml.exists())
            return;

        playlistData = juce::XmlDocument::parse (playlistXml);

        dataList   = playlistData->getChildByName ("DATA");
        columnList = playlistData->getChildByName ("HEADERS");

        numRows = dataList->getNumChildElements();

        if (columnList != nullptr)
        {
            for (auto* columnXml : columnList->getChildIterator())
            {
                //TODO: There is a jassert that gets triggered here that will require further debugging to resolve
                table.getHeader().addColumn (columnXml->getStringAttribute ("name"),
                                             columnXml->getIntAttribute ("columnId"),
                                             columnXml->getIntAttribute ("width"),
                                             50,
                                             400,
                                             juce::TableHeaderComponent::defaultFlags);
            }
        }

        table.getHeader().setSortColumnId (1, true);

        resized();
    }

    juce::TableListBox table  { {}, this };
private:
    juce::Font font           { 14.0f };

    std::unique_ptr<juce::XmlElement> playlistData;
    juce::XmlElement* columnList = nullptr;
    juce::XmlElement* dataList = nullptr;
    int numRows = 0;
    int selectedRow = -1;
    std::function<void(String)>& trackSelected;

    //==============================================================================
    class EditableTextCustomComponent  : public juce::Label
    {
    public:
        EditableTextCustomComponent (PlaylistComponent& td)
                : owner (td)
        {
            setEditable (false, true, false);
        }

        void mouseDown (const juce::MouseEvent& event) override
        {
            owner.table.selectRowsBasedOnModifierKeys (row, event.mods, false);

            Label::mouseDown (event);
        }

        void textWasEdited() override
        {
            owner.setText (columnId, row, getText());
        }

        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText (owner.getText(columnId, row), juce::dontSendNotification);
        }

    private:
        PlaylistComponent& owner;
        int row, columnId;
        juce::Colour textColour;
    };

    //==============================================================================
    class SelectionColumnCustomComponent    : public Component
    {
    public:
        SelectionColumnCustomComponent (PlaylistComponent& td)
                : owner (td)
        {
            addAndMakeVisible (toggleButton);

            toggleButton.onClick = [this] { owner.setSelection (row, (int) toggleButton.getToggleState()); };
        }

        void resized() override
        {
            toggleButton.setBoundsInset (juce::BorderSize<int> (2));
        }

        void setRowAndColumn (int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            toggleButton.setToggleState ((bool) owner.getSelection (row), juce::dontSendNotification);
        }

    private:
        PlaylistComponent& owner;
        juce::ToggleButton toggleButton;
        int row, columnId;
    };

    //==============================================================================
    class DataSorter
    {
    public:
        DataSorter (const juce::String& attributeToSortBy, bool forwards)
                : attributeToSort (attributeToSortBy),
                  direction (forwards ? 1 : -1)
        {}

        int compareElements (juce::XmlElement* first, juce::XmlElement* second) const
        {
            auto result = first->getStringAttribute (attributeToSort)
                    .compareNatural (second->getStringAttribute (attributeToSort));

            if (result == 0)
                result = first->getStringAttribute ("Title")
                        .compareNatural (second->getStringAttribute ("Title"));

            return direction * result;
        }

    private:
        juce::String attributeToSort;
        int direction;
    };

    //==============================================================================

    juce::String getAttributeNameForColumnId (const int columnId) const
    {
        for (auto* columnXml : columnList->getChildIterator())
        {
            if (columnXml->getIntAttribute ("columnId") == columnId)
                return columnXml->getStringAttribute ("name");
        }

        return {};
    }

    void debugPlaylistLoading()
    {
        auto pwd = walkDebugDirectoryToExamplePlaylistXML();

        if(pwd.existsAsFile())
        {
            loadData(pwd);
        }
        else
        {
            const auto callback = [this] (const juce::FileChooser& chooser)
            {
                loadData (chooser.getResult());
            };
            fileChooser.launchAsync (  juce::FileBrowserComponent::openMode
                                       | juce::FileBrowserComponent::canSelectFiles,
                                       callback);
        }
    }


    juce::FileChooser fileChooser { "Browse for Playlist.xml",
                                    juce::File::getSpecialLocation (juce::File::invokedExecutableFile) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistComponent)
};