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

    //Descend to ProjectM-SDL location
#if (JUCE_MAC || JUCE_LINUX)
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/src/projectMSDL");
#elif JUCE_WINDOWS
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/src/projectMSDL.exe");
#endif



    return pwd;
}

//===============================================================
// NeuralVisualizer Reference Packs
//===============================================================

static juce::String walkSourceDirectoryToProjectMSDL_COTC_Presets()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/creamofthecrop_20200216/Presets");

    return pwd.getFullPathName();
}

static juce::String walkSourceDirectoryToProjectMSDL_COTC_Textures()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/creamofthecrop_20200216/Textures");

    return pwd.getFullPathName();
}

static juce::String walkSourceDirectoryToProjectMSDL_Isosceles_Presets()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/isoscelesmashup_20200817/Presets");

    return pwd.getFullPathName();
}

static juce::String walkSourceDirectoryToProjectMSDL_Isosceles_Textures()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/isoscelesmashup_20200817/Textures");

    return pwd.getFullPathName();
}

static juce::String walkSourceDirectoryToProjectMSDL_Tryptonaut_Presets()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/tryptonaut_20120420/Presets");

    return pwd.getFullPathName();
}

static juce::String walkSourceDirectoryToProjectMSDL_Tryptonaut_Textures()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/tryptonaut_20120420/Textures");

    return pwd.getFullPathName();
}

//===============================================================
// NeuralVisualizer Workbench
//===============================================================

static juce::String walkSourceDirectoryToProjectMSDL_NP_Presets()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/neuralplayertesting_20230529/Presets");

    auto str = pwd.getFullPathName();

    return pwd.getFullPathName();
}

static juce::String walkSourceDirectoryToProjectMSDL_NP_Textures()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/neuralplayertesting_20230529/Textures");

    auto str = pwd.getFullPathName();

    return pwd.getFullPathName();
}

static juce::String walkSourceDirectoryToProjectMSDL_NPX_Presets()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/neuralplayerinactive_20230529/Presets");

    return pwd.getFullPathName();
}

static juce::String walkSourceDirectoryToProjectMSDL_NPX_Textures()
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
    pwd = pwd.getChildFile("Visualizers/projectM/frontend-sdl2/neuralplayerinactive_20230529/Textures");

    return pwd.getFullPathName();
}