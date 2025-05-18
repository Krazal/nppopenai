#include "EncodingUtils.h"
#include <codecvt>
#include <locale>

// Convert std::string to std::wstring
std::wstring stringToWstring(const std::string &str)
{
    try
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(str);
    }
    catch (...)
    {
        // fallback
        std::wstring result;
        for (unsigned char c : str)
        {
            result += static_cast<wchar_t>(c);
        }
        return result;
    }
}

std::string toUTF8(const std::wstring &wide)
{
    try
    {
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(wide);
    }
    catch (...)
    {
        // fallback
        std::string result;
        for (wchar_t c : wide)
        {
            if (c <= 0x7F)
            {
                result += static_cast<char>(c);
            }
            else if (c <= 0x7FF)
            {
                result += static_cast<char>(0xC0 | ((c >> 6) & 0x1F));
                result += static_cast<char>(0x80 | (c & 0x3F));
            }
            else if (c <= 0xFFFF)
            {
                result += static_cast<char>(0xE0 | ((c >> 12) & 0x0F));
                result += static_cast<char>(0x80 | ((c >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (c & 0x3F));
            }
            else
            {
                result += '?';
            }
        }
        return result;
    }
}

TCHAR *multiByteToWideChar(const char *utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    TCHAR *wide = new TCHAR[len];
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide, len);
    return wide;
}
