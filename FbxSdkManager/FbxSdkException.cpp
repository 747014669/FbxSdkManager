#include "FbxSdkException.h"
#include <iostream>
#include <ctime>
#include <iomanip>

bool FbxErrorHandler::s_quietMode = false;
std::string FbxErrorHandler::s_lastError;

void FbxErrorHandler::LogError(const std::string& error)
{
    s_lastError = error;
    
    if (!s_quietMode)
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cerr << "[ERROR] " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") 
                  << " - " << error << std::endl;
    }
}

void FbxErrorHandler::LogWarning(const std::string& warning)
{
    if (!s_quietMode)
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cerr << "[WARN]  " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") 
                  << " - " << warning << std::endl;
    }
}

void FbxErrorHandler::LogInfo(const std::string& info)
{
    if (!s_quietMode)
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cout << "[INFO]  " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") 
                  << " - " << info << std::endl;
    }
}