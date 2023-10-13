#pragma once
#include <windows.h>
#include <inttypes.h>
#include <utility>

extern std::pair<int, const char *> ASCII_TABLES[];

class AsciiDisplay
{
private:
    HANDLE hConsole;
    PCHAR screen;
    size_t scrSize;
    int16_t bufferW;
    int16_t bufferH;
    const char *ascii_table;
    const size_t table_size;
    const char *media_name;
public:
    AsciiDisplay(int16_t expectW, int16_t expectH, const char *media_name, const std::pair<int, const char *> &table, int fontSize);
    ~AsciiDisplay();
    template <typename type>
    int ToTableIndex(type value) const;
    int16_t getBufferWidth() const;
    int16_t getBufferHeight() const;
    // Main display function
    template <typename type>
    void Display(void *source, size_t linesize) const
    {
        type *buffer = (type *)source;
        DWORD written = 0;
        size_t padding = linesize / sizeof(type) - bufferW;
        for (size_t height = 0, bindex = 0, sindex = 0; height < bufferH; height++, bindex += padding)
        {
            for (size_t width = 0; width < bufferW; width++, sindex++, bindex++)
            {
                int index = ToTableIndex(buffer[bindex]);
                screen[sindex] = ascii_table[index];
            }
        }
        WriteConsoleOutputCharacterA(hConsole, screen, scrSize, {0, 0}, &written);
    }
};