#pragma once
#include <windows.h>
#include <inttypes.h>
#include <utility>

extern std::pair<int, const char *> ASCII_TABLES[];

class AsciiDisplay
{
private:
    bool bInit;
    HANDLE hConsole;
    PCHAR screen;
    size_t scrSize;
    int16_t scrW;
    int16_t scrH;
    const char *ascii_table;
    const size_t table_size;
    const char *media_name;
    static void GetLargestResolution(int16_t &width, int16_t &height);
    static void ScaleToFitScreen(int16_t &width, int16_t &height, int16_t maxW, int16_t maxH);
    void Initialize();
public:
    AsciiDisplay(int16_t expectW, int16_t expectH, std::pair<int, const char *> *table, const char *media_name);
    ~AsciiDisplay();
    template <typename type>
    int ToTableIndex(type value) const;
    int16_t getScreenWidth() const;
    int16_t getScreenHeight() const;
    // Main display function
    template <typename type>
    void Display(void *source, size_t linesize) const
    {
        if (!bInit)
            return;
        type *buffer = (type *)source;
        DWORD written = 0;
        size_t padding = linesize / sizeof(type) - scrW;
        for (size_t height = 0, bindex = 0, sindex = 0; height < scrH; height++, bindex += padding)
        {
            for (size_t width = 0; width < scrW; width++, sindex++, bindex++)
            {
                int index = ToTableIndex(buffer[bindex]);
                screen[sindex] = ascii_table[index];
            }
        }
        WriteConsoleOutputCharacterA(hConsole, screen, scrSize, {0, 0}, &written);
    }
};