#pragma once

#include <JuceHeader.h>

static String tryParseDuration(double duration)
{
    auto secondsToMins = duration / 60;

    auto minsToHours = secondsToMins / 60;

    String out;

    if (minsToHours > 1)
    {
        auto hours = (int)minsToHours;
        if (hours < 10)
            out.append("0", 1);
        out.append(String(hours), 2);
        out.append(":", 1);

        auto remainingMins = (int)secondsToMins % 60;
        if (remainingMins < 10)
            out.append("0", 1);
        out.append((String)remainingMins, 2);
        out.append(":", 1);

        auto remainingSeconds = (int)duration % 60;
        if (remainingSeconds < 10)
            out.append("0", 1);
        out.append((String)remainingSeconds, 2);

        return out;
    }
    else if (secondsToMins > 1)
    {
        auto minutes = (int)secondsToMins;
        if (minutes < 10)
            out.append("0", 1);
        out.append((String)minutes, 2);
        out.append(":", 1);

        auto remainingSeconds = (int)duration % 60;
        if (remainingSeconds < 10)
            out.append("0", 1);
        out.append((String)remainingSeconds, 2);

        return out;
    }
    else if (duration > 0)
    {
        out.append("00:", 3);

        auto remainingSeconds = (int)duration % 60;
        if (remainingSeconds < 10)
            out.append("0", 1);
        out.append((String)remainingSeconds, 2);

        return out;
    }

    //Invalid duration
    return "";
}