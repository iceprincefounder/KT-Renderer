#pragma once

#include <iostream>
#include <cstring>
#include <stdio.h>
#include <ctime>
#include <string>

using namespace std;

namespace kt{

class Log
{
public:
    Log(){ m_startTime = getCurrentDateTime();}

    virtual void logging(const char* message)
    {
        if (m_rendererInfo.empty())
            m_rendererInfo = "kt Renderer beta";
        string currentTime = getCurrentDateTime();
        printf("[%s]%s\n", currentTime.c_str(), message);
        m_currentTime = currentTime;
    }

    // virtual void warning(const char* message);
    // virtual void error(const char* message);
    void setRendererInfo(const char* message)
    {
        m_rendererInfo = message;
    }

    const string getCurrentDateTime() {
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[80];
        tstruct = *localtime(&now);
        // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
        // for more information about date/time format
        strftime(buf, sizeof(buf), "%X", &tstruct);
        return buf;
    }

private:
    string m_startTime;
    string m_endTime;
    string m_currentTime;
    string m_rendererInfo;
};

} // end namespace kt