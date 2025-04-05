#pragma once
#include <ctime>
#include <chrono>
#include <string>
#include "Windows.h"

//#include ".benchP.cpp"

class benchP
{
private:
    std::clock_t c_start;
    std::clock_t c_end;

    std::chrono::steady_clock::time_point t_start;
    std::chrono::steady_clock::time_point t_end;

    void setStartTime();

    void setEndTime();

    // even index pipes are for reading
    // odd index pipes are for writing
    // first 2 are for input
    // last 2 are for output
    bool initPipes(HANDLE childPipe[4]);

public:
    std::string execFile;
    std::string args;
    std::stringstream buffer;

    benchP(std::string executable, std::string arguments) :
        execFile(executable),
        args(arguments),
        c_start(),
        c_end(),
        t_start(),
        t_end(),
        buffer()
    {};

    benchP(std::string executable) :
        benchP(executable, "")
    {};

    benchP() :
        benchP("")
    {};

    double getCPUTime() const;

    double getWallTime() const;

    bool runChild();
};

void compareOutput(const benchP& p1, const benchP& p2);
std::ostream& operator<<(std::ostream& out, const benchP& process);
