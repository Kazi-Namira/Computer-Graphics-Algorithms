#include <graphics.h>
#include <conio.h>
#include <cmath>

#define PI 3.14159265

void drawCircle(int xc, int yc, int r)
{
    for (int angle = 0; angle < 360; angle++)
    {
        double rad = angle * PI / 180.0;
        int x = xc + r * cos(rad);
        int y = yc + r * sin(rad);
        putpixel(x, y, WHITE);
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
