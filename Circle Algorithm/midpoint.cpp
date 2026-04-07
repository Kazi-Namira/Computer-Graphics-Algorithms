#include <graphics.h>
#include <dos.h>
#include <iostream>
using namespace std;

void drawCircle(int xc, int yc, int r)
{
    int x = 0, y = r;
    int p = 1 - r;

    while (x <= y)
    {
        putpixel(xc + x, yc + y, WHITE);
        putpixel(xc - x, yc + y, WHITE);
        putpixel(xc + x, yc - y, WHITE);
        putpixel(xc - x, yc - y, WHITE);
        putpixel(xc + y, yc + x, WHITE);
        putpixel(xc - y, yc + x, WHITE);
        putpixel(xc + y, yc - x, WHITE);
        putpixel(xc - y, yc - x, WHITE);

        x++;

        if (p < 0)
            p += 2 * x + 3;
        else
        {
            y--;
            p += 2 * (x - y) + 5;
        }
    }
}

int main()
{
    initwindow(500, 500);
    drawCircle(250, 250, 100);
    getch();
}