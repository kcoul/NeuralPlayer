#pragma once

//Watched thread variables
struct WatchedVars
{
    String returnedText;
    String currentFileName;
    int currentFileIndex = 0;
    int numFiles = 0;
};
