#include <graphics.h>
#include <conio.h>
#include <dos.h>
#include <cmath>

void drawCircle(int xc, int yc, int r)
{
    for (int x = 0; x <= r; x++)
    {
        int y = (int)round(sqrt(r * r - x * x));
        putpixel(xc + x, yc + y, WHITE);
        putpixel(xc - x, yc + y, WHITE);
        putpixel(xc + x, yc - y, WHITE);
        putpixel(xc - x, yc - y, WHITE);
        putpixel(xc + y, yc + x, WHITE);
        putpixel(xc - y, yc + x, WHITE);
        putpixel(xc + y, yc - x, WHITE);
        putpixel(xc - y, yc - x, WHITE);
    }
}

int main()
{
    int gd = DETECT, gm;
    initgraph(&gd, &gm, "");
    drawCircle(250, 250, 100);
    getch();
    closegraph();
}