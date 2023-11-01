#include "../../src/pch.hpp"

#include "output.hpp"

namespace con
{
    WORD bgMask(BACKGROUND_BLUE |
        BACKGROUND_GREEN |
        BACKGROUND_RED |
        BACKGROUND_INTENSITY);
    WORD fgMask(FOREGROUND_BLUE |
        FOREGROUND_GREEN |
        FOREGROUND_RED |
        FOREGROUND_INTENSITY);

    WORD fgBlack(0);
    WORD fgLoRed(FOREGROUND_RED);
    WORD fgLoGreen(FOREGROUND_GREEN);
    WORD fgLoBlue(FOREGROUND_BLUE);
    WORD fgLoCyan(fgLoGreen | fgLoBlue);
    WORD fgLoMagenta(fgLoRed | fgLoBlue);
    WORD fgLoYellow(fgLoRed | fgLoGreen);
    WORD fgLoWhite(fgLoRed | fgLoGreen | fgLoBlue);
    WORD fgNone(FOREGROUND_INTENSITY);
    WORD fgGray(fgBlack | FOREGROUND_INTENSITY);
    WORD fgHiWhite(fgLoWhite | FOREGROUND_INTENSITY);
    WORD fgHiBlue(fgLoBlue | FOREGROUND_INTENSITY);
    WORD fgHiGreen(fgLoGreen | FOREGROUND_INTENSITY);
    WORD fgHiRed(fgLoRed | FOREGROUND_INTENSITY);
    WORD fgHiCyan(fgLoCyan | FOREGROUND_INTENSITY);
    WORD fgHiMagenta(fgLoMagenta | FOREGROUND_INTENSITY);
    WORD fgHiYellow(fgLoYellow | FOREGROUND_INTENSITY);
    WORD bgBlack(0);
    WORD bgLoRed(BACKGROUND_RED);
    WORD bgLoGreen(BACKGROUND_GREEN);
    WORD bgLoBlue(BACKGROUND_BLUE);
    WORD bgLoCyan(bgLoGreen | bgLoBlue);
    WORD bgLoMagenta(bgLoRed | bgLoBlue);
    WORD bgLoYellow(bgLoRed | bgLoGreen);
    WORD bgLoWhite(bgLoRed | bgLoGreen | bgLoBlue);
    WORD bgGray(bgBlack | BACKGROUND_INTENSITY);
    WORD bgHiWhite(bgLoWhite | BACKGROUND_INTENSITY);
    WORD bgHiBlue(bgLoBlue | BACKGROUND_INTENSITY);
    WORD bgHiGreen(bgLoGreen | BACKGROUND_INTENSITY);
    WORD bgHiRed(bgLoRed | BACKGROUND_INTENSITY);
    WORD bgHiCyan(bgLoCyan | BACKGROUND_INTENSITY);
    WORD bgHiMagenta(bgLoMagenta | BACKGROUND_INTENSITY);
    WORD bgHiYellow(bgLoYellow | BACKGROUND_INTENSITY);

    HANDLE hCon;
    DWORD                       cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO  csbi;
    DWORD                       dwConSize;

    static class con_dev
    {
    private:


    public:
        con_dev()
        {
            AllocConsole();

            SetConsoleTitleW(L"extraordinary examples of gamer vision technology");
            freopen_s(&safe_handle_stream,  ("CONIN$"),  ("r"), stdin);
            freopen_s(&safe_handle_stream, ("CONOUT$"),  ("w"), stdout);
            freopen_s(&safe_handle_stream, ("CONOUT$"),  ("w"), stderr);

            hCon = GetStdHandle(STD_OUTPUT_HANDLE);
        }
    private:
        void GetInfo()
        {
            GetConsoleScreenBufferInfo(hCon, &csbi);
            dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
        }
    public:
        void Clear()
        {
            COORD coordScreen = { 0, 0 };

            GetInfo();
            FillConsoleOutputCharacter(hCon, ' ',
                dwConSize,
                coordScreen,
                &cCharsWritten);
            GetInfo();
            FillConsoleOutputAttribute(hCon,
                csbi.wAttributes,
                dwConSize,
                coordScreen,
                &cCharsWritten);
            SetConsoleCursorPosition(hCon, coordScreen);
        }
        void SetColor(WORD wRGBI, WORD Mask)
        {
            GetInfo();
            csbi.wAttributes &= Mask;
            csbi.wAttributes |= wRGBI;
            SetConsoleTextAttribute(hCon, csbi.wAttributes);
        }
    } console;

    std::ostream& fg_none(std::ostream& os)
    {
        os.flush();
        console.SetColor(fgLoWhite, bgMask);

        return os;
    }

    std::ostream& fg_red(std::ostream& os)
    {
        os.flush();
        console.SetColor(fgHiRed, bgMask);

        return os;
    }

    std::ostream& fg_green(std::ostream& os)
    {
        os.flush();
        console.SetColor(fgHiGreen, bgMask);

        return os;
    }

    std::ostream& fg_cyan(std::ostream& os)
    {
        os.flush();
        console.SetColor(fgHiCyan, bgMask);

        return os;
    }

    std::ostream& fg_magenta(std::ostream& os)
    {
        os.flush();
        console.SetColor(fgHiMagenta, bgMask);

        return os;
    }

    std::ostream& fg_yellow(std::ostream& os)
    {
        os.flush();
        console.SetColor(fgHiYellow, bgMask);

        return os;
    }
}
