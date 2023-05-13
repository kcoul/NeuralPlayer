#pragma once

#include <JuceHeader.h>

static juce::File walkDebugDirectoryToExamplePlaylistXML()
{
    auto pwd = File::getCurrentWorkingDirectory();
    while (true) //Ascend to build inputFolder
    {
        pwd = pwd.getParentDirectory();
        if (pwd.getFileName().endsWith("cmake-build-debug") ||
            pwd.getFileName().endsWith("cmake-build-release") ||
            pwd.getFileName().endsWith("cmake-build-relwithdebinfo"))
            break;
    }

    pwd = pwd.getParentDirectory();
    pwd = pwd.getChildFile("Player/Resources/Playlist.xml");

    return pwd;
}

static juce::File walkDebugDirectoryToSpleeterRTBin()
{
    auto pwd = File::getCurrentWorkingDirectory();

    while (true) //Ascend to build inputFolder
    {
        pwd = pwd.getParentDirectory();
        if (pwd.getFileName().endsWith("cmake-build-debug") ||
            pwd.getFileName().endsWith("cmake-build-release") ||
            pwd.getFileName().endsWith("cmake-build-relwithdebinfo"))
            break;
    }

    //Descend to SpleeterRTBin location
    pwd = pwd.getChildFile("SpleeterRT/Executable/SpleeterRTBin");

    return pwd;
}

static juce::File walkDebugDirectoryToProjectMSDL()
{
    auto pwd = File::getCurrentWorkingDirectory();

    while (true) //Ascend to build inputFolder
    {
        pwd = pwd.getParentDirectory();
        if (pwd.getFileName().endsWith("cmake-build-debug") ||
            pwd.getFileName().endsWith("cmake-build-release") ||
            pwd.getFileName().endsWith("cmake-build-relwithdebinfo"))
            break;
    }

    pwd = pwd.getParentDirectory();

    //Descend to ProjectM-SDL location
    pwd = pwd.getChildFile("cmake-build-debug/Visualizers/projectM/frontend-sdl2/src/projectMSDL");

    return pwd;
}
