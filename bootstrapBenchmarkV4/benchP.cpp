#include "benchP.hpp"
#include <iostream>
#include <ostream>
#include <algorithm>
#include "windows.h"


void benchP::setStartTime() {
    c_start = std::clock();
    t_start = std::chrono::high_resolution_clock::now();
}

void benchP::setEndTime() {
    this->c_end = std::clock();
    this->t_end = std::chrono::high_resolution_clock::now();
}

double benchP::getCPUTime() const { return 1000.0 * (this->c_end - this->c_start) / CLOCKS_PER_SEC; }

double benchP::getWallTime() const { return std::chrono::duration<double, std::milli>(this->t_end - this->t_start).count(); }

// even index pipes are for reading
// odd index pipes are for writing
// first 2 are for input
// last 2 are for output
bool benchP::initPipes(HANDLE childPipe[4])
{
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&childPipe[0], &childPipe[1], &saAttr, 0)) {
        std::cerr << "create Input pipe Failed" << std::endl;
        return false;
    }
    // Ensure the write handle to the pipe for STDIN is not inherited. - do not inherit on the pipe so we can write to the
    if (!SetHandleInformation(childPipe[1], HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "failed to set input read pipe handle info" << std::endl;
        return false;
    }
    if (!CreatePipe(&childPipe[2], &childPipe[3], &saAttr, 0)) {
        std::cerr << "create Output pipe Failed" << std::endl;
        return false;
    }
    // Ensure the read handle to the pipe for STDOUT is not inherited - do not inherit on the pipe so we can read from the pipe
    if (!SetHandleInformation(childPipe[2], HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "failed to set output write pipe Handle info" << std::endl;
        return false;
    }
    return true;
}

bool benchP::runChild()
{
    constexpr size_t BufSize = 4096;

    // unsigned types from windows.h to count bits read/written
    DWORD dwRead, dwWritten;
    std::string input{ this->buffer.str() };

    this->buffer.str(std::string());
    //clears stringstream

    const char* cInput{input.c_str()};

    //child I/O pipe
    //childPipe[0] is used for reading
    //childPipe[1] is used for writing
    HANDLE cPipes[4]{ NULL };

    STARTUPINFOA sInfo;
    PROCESS_INFORMATION pi;

    //INITIALIZATION
    if (!initPipes(cPipes)) // each pipe has its own error message
        return false;


    ZeroMemory(&sInfo, sizeof(sInfo));
    sInfo.cb = sizeof(sInfo);
    sInfo.hStdError = cPipes[3];
    sInfo.hStdOutput = cPipes[3];
    sInfo.hStdInput = cPipes[0];
    sInfo.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    //this->execFile = std::filesystem::current_path().append(this->execFile);
    std::string args{ this->execFile + " " + this->args };
    const char *wArgs{ args.c_str() };

    WriteFile(cPipes[1], cInput, input.size(), &dwWritten, NULL);
    CloseHandle(cPipes[1]);

     this->setStartTime();

    // process start
    if (!CreateProcessA(NULL,   // No module name (use command line)
        const_cast<LPSTR>(wArgs),            // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        TRUE,           // Set handle inheritance to TRUE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &sInfo,            // Pointer to STARTUPINFO structure
        &pi)            // Pointer to PROCESS_INFORMATION structure
        )
    {
        // error 2, cannot find file specified
        //for (size_t i{}; i < 4; ++i)                                    //close pipes 0-3
            //CloseHandle(cPipes[i]);

        CloseHandle(cPipes[0]);
        //CloseHandle(cPipes[1]);
        CloseHandle(cPipes[2]);
        CloseHandle(cPipes[3]);

        std::cerr << "failed to start child: " << GetLastError() << std::endl;
        return false;
    }

    //we should write into the process while its running...
    //add write functionality into cPipes[1]

    //CloseHandle(cPipes[0]);

    // becuase the program creates another process, they may execute simutaneously so we wait for child to stop then stop the timer. 
    WaitForSingleObject(pi.hProcess, INFINITE);

    // end time
    this->setEndTime();

    //force child to stop writing so we can read from the pipe
    CloseHandle(cPipes[3]);

    char chBuf[BufSize];
    //read
    //HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    bool bSuccess = ReadFile(cPipes[2], chBuf, BufSize, &dwRead, NULL);
    chBuf[dwRead] = '\0';

    this->buffer << chBuf;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(cPipes[2]);
    return true;
}

std::ostream& operator<<(std::ostream& out, const benchP& process) {
    out << std::fixed << std::setprecision(2) << process.execFile << " " << process.args
        << " CPU time: " << process.getCPUTime() << "ms. Wall time: " << process.getWallTime() << "ms.";
    return out;
}


// p1 should be the professor program
// p2 should be the student program
// function should output the student program, but highlight red when printing unexpected values,
// high light green when outputting additional words. 
// by reading the tokens one at a time, we can concatenate the ansi color formatting around tokens
void compareOutput(const benchP& p1, const benchP& p2)
{
    std::string p2token{ p2.buffer.str() };
    std::string p1token{ p1.buffer.str() };

    std::replace(p2token.begin(), p2token.end(), '\r', ' ');
    std::replace(p1token.begin(), p1token.end(), '\r', ' ');

    std::stringstream p1Buffer{ p1token };
    std::stringstream p2Buffer{ p2token };

    while (std::getline(p2Buffer, p2token))
    {
        std::getline(p1Buffer, p1token);
        std::stringstream p2Line{ p2token };
        std::stringstream p1Line{ p1token };

        while (std::getline(p2Line, p2token, ' '))
        {
            std::getline(p1Line, p1token, ' ');
            if (p1token == p2token)
                std::cout << p2token << ' ';
            else
                std::cout << "\033[41m" << p2token << " " << "\033[40m";
        }
        std::cout << std::endl;
    }
}