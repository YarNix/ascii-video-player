#include <string.h>
#include <memory>

#include <numeric>
#include <cmath>
#include <valarray>
#include <algorithm>
#include "asciiPlayer.hpp"

// Smaller table seem to produce more coherence image
std::pair<int, const char *> ASCII_TABLES[] = {
    {10, " .:-=+*#%@"},
    {14, " .:-=;+?*E#0%@"},
    {70, R"==( .'`^",:;Il!i><~+_-?][}{1)(|\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$)=="},
    {87, " .-':_,^=;><+!rc*/z?sLTv)J7(|FiCfI31tluneoZ5Yxjya2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@"},
};
const char* TABLE_SIZES_MSG = "10, 14, 70, 87";

// Get size that fit the screen base on the width & height
// If the aspect ratio is equal to the current screen size then return the curent size
// else scale it base on the aspect ratio
static void ScaleToFitScreen(int16_t &width, int16_t &height, int16_t scrW, int16_t scrH)
{
    if (width < scrW && height < scrH)
        return;

    std::valarray<double> scr{double(scrW), double(scrH)}, target{double(width), double(height)};
    auto scrRatio = scr / std::gcd(scrW, scrH), tRatio = target / std::gcd(width, height);

    std::valarray<double> ret_value;
    if (scrRatio[0] == tRatio[0] && scrRatio[1] == tRatio[1])
        ret_value = scr;
    else
        ret_value = (target / (target / scr).max() / 10).apply(floor) * 10;
    width = (int16_t)ret_value[0];
    height = (int16_t)ret_value[1];
}
// Helpers regarding screen size
static void GetLargestResolution(int16_t &width, int16_t &height)
{
    SetProcessDPIAware();
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    width = desktop.right / 2;
    height = desktop.bottom / 2;
}

// Construct
AsciiDisplay::AsciiDisplay(int16_t expectW, int16_t expectH, const char *media_name, const std::pair<int, const char *> &table, int fontSize)
    : bufferW(expectW), bufferH(expectH),
      table_size(table.first), ascii_table(table.second), media_name(media_name),
      screen(nullptr)
{
    SetProcessDPIAware();
    // Create a new console so we don't have to worry bout console settings
    hConsole = CreateConsoleScreenBuffer(GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    // Switch to the our console
    SetConsoleActiveScreenBuffer(hConsole);
    // Reset the console size
    SMALL_RECT rectWindow = {0, 0, 1, 1};
    SetConsoleWindowInfo(hConsole, TRUE, &rectWindow);

    // Guess the font size if not valid
    if (fontSize <= 0){
        int16_t fitW = bufferW, fitH = bufferH, maxW, maxH;
        GetLargestResolution(maxW, maxH);
        ScaleToFitScreen(fitW, fitH, maxW, maxH);
        fontSize = std::min(maxW / fitW, maxH / fitH);
        fontSize = fontSize < 1 ? 1 : fontSize;
    }

    // Set the console's font size
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = fontSize;
    cfi.dwFontSize.Y = fontSize;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hConsole, false, &cfi);
    // Ensure the output can fit the screen
    COORD maxSz = GetLargestConsoleWindowSize(hConsole);
    ScaleToFitScreen(bufferW, bufferH, maxSz.X, maxSz.Y);
    // Set the buffer
    SetConsoleScreenBufferSize(hConsole, {bufferW, bufferH});
    rectWindow = {0, 0, SHORT(bufferW - 1), SHORT(bufferH - 1)};
    SetConsoleWindowInfo(hConsole, TRUE, &rectWindow);
    // Set the console title
    #define TITLE_PREFIX "Playing - "
    char* title = (char*)malloc(sizeof(TITLE_PREFIX) + strlen(media_name));
    strcpy(title, TITLE_PREFIX);
    strcat(title, media_name);
    SetConsoleTitleA(title);
    free(title);
    #undef TITLE_PREFIX
    // Center the console
    SetWindowPos(GetConsoleWindow(), HWND_TOP, maxSz.X / 2 - bufferW / 2, 0, 0, 0, SWP_NOSIZE);
    
    // Create the screen buffer
    scrSize = bufferW * bufferH;
    screen = new CHAR[scrSize];
    memset(screen, ' ', scrSize);
}
// Destructor
AsciiDisplay::~AsciiDisplay()
{
    if (screen)
        delete[] screen;
}

int16_t AsciiDisplay::getBufferHeight() const { return bufferH; }
int16_t AsciiDisplay::getBufferWidth() const { return bufferW; }

// Implement for ToTableIndex
// Templating allow to define implementation for other types beside grayscale like ARBG, YUV
template <>
int AsciiDisplay::ToTableIndex<float>(float value) const
{
    return round(value * (table_size - 1));
}
