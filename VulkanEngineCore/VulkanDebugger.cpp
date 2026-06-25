#include "VulkanDebugger.h"
LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;


VulkanDebugger::VulkanDebugger()
{
}

VulkanDebugger::~VulkanDebugger()
{
}

VkBool32 VKAPI_CALL VulkanDebugger::DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT  MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* UserData)
{
    const char* severityStr = "";
    const char* colorCode = "";

    switch (MessageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        severityStr = "VERBOSE";
        colorCode = "\033[34m";  // blue
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        severityStr = "INFO";
        colorCode = "\033[32m";  // green
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        severityStr = "WARNING";
        colorCode = "\033[33m";  // yellow
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        severityStr = "ERROR";
        colorCode = "\033[31m";  // red
        break;
    default:
        severityStr = "UNKNOWN";
        colorCode = "\033[35m";  // magenta
        break;
    }

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD originalAttributes = 7;

    if (hConsole != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hConsole, &consoleInfo)) originalAttributes = consoleInfo.wAttributes;

    WORD color = originalAttributes;
    if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)   color = FOREGROUND_RED;
    else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) color = FOREGROUND_RED | FOREGROUND_GREEN;
    else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)    color = FOREGROUND_GREEN;
    else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) color = FOREGROUND_BLUE;

    SetConsoleTextAttribute(hConsole, color);
    fprintf(stderr, "%s: ", severityStr);
    SetConsoleTextAttribute(hConsole, originalAttributes);
    fprintf(stderr, "%s\n", CallBackData->pMessage);
#else
    // Linux / macOS: ANSI escape codes (works in every terminal)
    fprintf(stderr, "%s%s: \033[0m%s\n", colorCode, severityStr, CallBackData->pMessage);
#endif
    LogVulkanMessage(CallBackData->pMessage, static_cast<int>(MessageSeverity));
    return VK_FALSE;
}

void VulkanDebugger::CreateLogMessageCallback(LogVulkanMessageCallback callback)
{
    g_logVulkanMessageCallback = callback;
}

void VulkanDebugger::LogVulkanMessage(const char* message, int severity)
{
    if (g_logVulkanMessageCallback)
    {
        g_logVulkanMessageCallback(message, severity);
    }
}