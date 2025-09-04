#pragma once
#include <exception>
#include <string>

/**
 * @brief FBX SDK相关异常类
 */
class FbxSdkException : public std::exception
{
public:
    enum ErrorCode
    {
        NONE = 0,
        MANAGER_CREATE_FAILED,
        SCENE_CREATE_FAILED,
        FILE_NOT_FOUND,
        FILE_FORMAT_ERROR,
        INVALID_FILE_VERSION,
        GEOMETRY_NOT_FOUND,
        MATERIAL_NOT_FOUND,
        MEMORY_ALLOCATION_FAILED,
        TRIANGULATION_FAILED,
        UNKNOWN_ERROR
    };

    FbxSdkException(ErrorCode code, const std::string& message)
        : m_errorCode(code), m_message(message)
    {
        m_fullMessage = "[FbxSdkError " + std::to_string(code) + "] " + message;
    }

    const char* what() const noexcept override
    {
        return m_fullMessage.c_str();
    }

    ErrorCode getErrorCode() const { return m_errorCode; }
    const std::string& getMessage() const { return m_message; }

private:
    ErrorCode m_errorCode;
    std::string m_message;
    std::string m_fullMessage;
};

/**
 * @brief 错误处理工具类
 */
class FbxErrorHandler
{
public:
    static void SetQuietMode(bool quiet) { s_quietMode = quiet; }
    static bool IsQuietMode() { return s_quietMode; }
    
    static void LogError(const std::string& error);
    static void LogWarning(const std::string& warning);
    static void LogInfo(const std::string& info);

    static std::string GetLastError() { return s_lastError; }
    static void ClearLastError() { s_lastError.clear(); }

private:
    static bool s_quietMode;
    static std::string s_lastError;
};