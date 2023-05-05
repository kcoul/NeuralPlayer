#include "AudioThumbnailComponent.h"

AudioThumbnailComponent::AudioThumbnailComponent(
    AudioFormatManager& formatManager,
    AudioTransportSource& transport,
    AudioThumbnailCache& thumbCache,
    std::function<void()>& relayAllNotesOff,
    const File& existingFile) :
        transportSource(transport),
        scrollbar(false),
        thumbnail(512, formatManager, thumbCache),
        isFollowingTransport(false),
        allNotesOff(relayAllNotesOff)
{
    thumbnail.addChangeListener(this);

    addAndMakeVisible(scrollbar);
    scrollbar.setRangeLimits(visibleRange);
    scrollbar.setAutoHide(false);
    scrollbar.addListener(this);

    currentPositionMarker.setFill(Colours::white.withAlpha(0.85f));
    addAndMakeVisible(currentPositionMarker);

    setFile(existingFile);
}

AudioThumbnailComponent::~AudioThumbnailComponent()
{
    scrollbar.removeListener(this);
    thumbnail.removeChangeListener(this);
}

void AudioThumbnailComponent::setFile(const File& file)
{
    if (file.existsAsFile())
    {
        thumbnail.setSource(new FileInputSource(file));
        const Range<double> newRange(0.0, thumbnail.getTotalLength());
        scrollbar.setRangeLimits(newRange);
        setRange(newRange);

        startTimerHz(40);
    }
}

File AudioThumbnailComponent::getLastDroppedFile() const noexcept
{
    return lastFileDropped;
}

void AudioThumbnailComponent::setZoomFactor(double amount)
{
    if (thumbnail.getTotalLength() > 0)
    {
        const double newScale = jmax(0.001, thumbnail.getTotalLength() * (1.0 - jlimit(0.0, 0.99, amount)));
        const double timeAtCentre = xToTime(getWidth() / 2.0f);
        setRange(Range<double>(timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5));
    }
}

void AudioThumbnailComponent::setRange(Range<double> newRange)
{
    visibleRange = newRange;
    scrollbar.setCurrentRange(visibleRange);
    updateCursorPosition();
    repaint();
}

void AudioThumbnailComponent::setFollowsTransport(bool shouldFollow)
{
    isFollowingTransport = shouldFollow;
}

void AudioThumbnailComponent::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
    g.setColour(Colours::lightblue);

    if (thumbnail.getTotalLength() > 0.0)
    {
        Rectangle<int> thumbArea(getLocalBounds());
        thumbArea.removeFromBottom(scrollbar.getHeight() + 4);
        thumbnail.drawChannels(g, thumbArea.reduced (2),
            visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
    }
    else
    {
        g.setFont(14.0f);
        g.drawFittedText("(No audio file selected)", getLocalBounds(), Justification::centred, 2);
    }
}

void AudioThumbnailComponent::resized()
{
    scrollbar.setBounds(getLocalBounds().removeFromBottom(14).reduced(2));
}

void AudioThumbnailComponent::changeListenerCallback(ChangeBroadcaster*)
{
    // this method is called by the thumbnail when it has changed, so we should repaint it..
    repaint();
}

void AudioThumbnailComponent::filesDropped(const StringArray& files, int /*x*/, int /*y*/)
{
    auto result = File(files[0]); //Only array element guaranteed to exist, just load one for now.
    if (result.getFileExtension() == ".wav")
    {
        lastFileDropped = result;
        sendChangeMessage();
    }
}

void AudioThumbnailComponent::mouseDown(const MouseEvent& e)
{
    mouseDrag(e);
}

void AudioThumbnailComponent::mouseDrag(const MouseEvent& e)
{
    if (canMoveTransport())
    {
        transportSource.setPosition(jmax(0.0, xToTime((float)e.x)));
        allNotesOff();
    }
}

void AudioThumbnailComponent::mouseUp(const MouseEvent&)
{
    transportSource.start();
}

void AudioThumbnailComponent::mouseWheelMove(const MouseEvent&, const MouseWheelDetails& wheel)
{
    if (thumbnail.getTotalLength() > 0.0)
    {
        double newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
        newStart = jlimit(0.0, jmax(0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

        if (canMoveTransport())
            setRange(Range<double>(newStart, newStart + visibleRange.getLength()));

        repaint();
    }
}

float AudioThumbnailComponent::timeToX(const double time) const
{
    return getWidth() * (float)((time - visibleRange.getStart()) / (visibleRange.getLength()));
}

double AudioThumbnailComponent::xToTime(const float x) const
{
    return (x / getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
}

bool AudioThumbnailComponent::canMoveTransport() const noexcept
{
    return !(isFollowingTransport && transportSource.isPlaying());
}

void AudioThumbnailComponent::scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    if (scrollBarThatHasMoved == &scrollbar)
        if (!(isFollowingTransport && transportSource.isPlaying()))
            setRange(visibleRange.movedToStartAt(newRangeStart));
}

void AudioThumbnailComponent::timerCallback()
{
    if (canMoveTransport())
        updateCursorPosition();
    else
        setRange(visibleRange.movedToStartAt(transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
}

void AudioThumbnailComponent::updateCursorPosition()
{
    currentPositionMarker.setVisible(transportSource.isPlaying() || isMouseButtonDown());

    auto x = jmax(0.0f, timeToX(transportSource.getCurrentPosition()) - 0.75f);
    currentPositionMarker.setRectangle(Rectangle<float>(x, 0,
        1.5f, (float)(getHeight() - scrollbar.getHeight())));
}
