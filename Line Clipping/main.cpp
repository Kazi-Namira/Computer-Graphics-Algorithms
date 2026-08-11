#include <windows.h>
#include <GL/glut.h>
#include <cstdio>
#include <cmath>
#include <vector>

using namespace std;

// Clipping window boundaries
float xmin = -0.5f, xmax = 0.5f;
float ymin = -0.5f, ymax = 0.5f;

// Region codes
const int INSIDE = 0;
const int LEFT = 1;
const int RIGHT = 2;
const int BOTTOM = 4;
const int TOP = 8;

// Point structure
struct MyPoint {
    float x, y;
    MyPoint(float x = 0, float y = 0) : x(x), y(y) {}
};

// Line structure
struct MyLine {
    MyPoint p1, p2;
    MyLine(MyPoint a = MyPoint(), MyPoint b = MyPoint()) : p1(a), p2(b) {}
};

// Global variables
vector<MyLine> originalLines;
vector<MyLine> clippedLines;
vector<MyPoint> tempPoints;
bool showOriginal = true;
bool showClipped = false;
bool isClipped = false;

// Function to compute region code
int computeCode(float x, float y) {
    int code = INSIDE;
    if (x < xmin) code |= LEFT;
    else if (x > xmax) code |= RIGHT;
    if (y < ymin) code |= BOTTOM;
    else if (y > ymax) code |= TOP;
    return code;
}

// Cohen-Sutherland line clipping algorithm
bool clipLine(float x1, float y1, float x2, float y2,
              float& cx1, float& cy1, float& cx2, float& cy2) {
    int code1 = computeCode(x1, y1);
    int code2 = computeCode(x2, y2);
    bool accept = false;

    cx1 = x1; cy1 = y1;
    cx2 = x2; cy2 = y2;

    while (true) {
        if (code1 == 0 && code2 == 0) {
            accept = true;
            break;
        } else if (code1 & code2) {
            break;
        } else {
            int codeOut;
            float x, y;

            if (code1 != 0) codeOut = code1;
            else codeOut = code2;

            if (codeOut & TOP) {
                x = cx1 + (cx2 - cx1) * (ymax - cy1) / (cy2 - cy1);
                y = ymax;
            } else if (codeOut & BOTTOM) {
                x = cx1 + (cx2 - cx1) * (ymin - cy1) / (cy2 - cy1);
                y = ymin;
            } else if (codeOut & RIGHT) {
                y = cy1 + (cy2 - cy1) * (xmax - cx1) / (cx2 - cx1);
                x = xmax;
            } else {
                y = cy1 + (cy2 - cy1) * (xmin - cx1) / (cx2 - cx1);
                x = xmin;
            }

            if (codeOut == code1) {
                cx1 = x; cy1 = y;
                code1 = computeCode(cx1, cy1);
            } else {
                cx2 = x; cy2 = y;
                code2 = computeCode(cx2, cy2);
            }
        }
    }
    return accept;
}

// Drawing functions
void drawBackground() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void drawGrid() {
    glColor3f(0.85f, 0.85f, 0.9f);
    glBegin(GL_LINES);
    for (float i = -0.9f; i <= 0.9f; i += 0.2f) {
        glVertex2f(-0.95f, i);
        glVertex2f(0.95f, i);
        glVertex2f(i, -0.95f);
        glVertex2f(i, 0.95f);
    }
    glEnd();
}

void drawAxes() {
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(-0.95f, 0);
    glVertex2f(0.95f, 0);
    glVertex2f(0, -0.95f);
    glVertex2f(0, 0.95f);
    glEnd();
}

void drawClippingWindow() {
    // BLACK border
    glLineWidth(3.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(xmin, ymin);
    glVertex2f(xmax, ymin);
    glVertex2f(xmax, ymax);
    glVertex2f(xmin, ymax);
    glEnd();

    // Very light gray fill
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.9f, 0.9f, 0.9f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(xmin, ymin);
    glVertex2f(xmax, ymin);
    glVertex2f(xmax, ymax);
    glVertex2f(xmin, ymax);
    glEnd();
    glDisable(GL_BLEND);

    glLineWidth(1.0f);
}

void drawOriginalLines() {
    // DARK GRAY for original lines
    glLineWidth(2.0f);
    glColor3f(0.2f, 0.2f, 0.3f);
    for (size_t i = 0; i < originalLines.size(); i++) {
        glBegin(GL_LINES);
        glVertex2f(originalLines[i].p1.x, originalLines[i].p1.y);
        glVertex2f(originalLines[i].p2.x, originalLines[i].p2.y);
        glEnd();
    }
}

void drawClippedLines() {
    if (clippedLines.empty()) return;

    // THICK RED for clipped lines
    glLineWidth(5.0f);
    glColor3f(1.0f, 0.0f, 0.0f);

    // Dashed pattern
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xAAAA);

    for (size_t i = 0; i < clippedLines.size(); i++) {
        glBegin(GL_LINES);
        glVertex2f(clippedLines[i].p1.x, clippedLines[i].p1.y);
        glVertex2f(clippedLines[i].p2.x, clippedLines[i].p2.y);
        glEnd();
    }

    glDisable(GL_LINE_STIPPLE);
    glLineWidth(1.0f);
}

void drawTempPoints() {
    if (tempPoints.size() > 0) {
        glPointSize(8.0f);
        glColor3f(0.0f, 0.5f, 1.0f);
        glBegin(GL_POINTS);
        for (size_t i = 0; i < tempPoints.size(); i++) {
            glVertex2f(tempPoints[i].x, tempPoints[i].y);
        }
        glEnd();
    }
}

// Clipping function - shows BOTH original AND clipped
void clipAllLines() {
    clippedLines.clear();
    for (size_t i = 0; i < originalLines.size(); i++) {
        float cx1, cy1, cx2, cy2;
        if (clipLine(originalLines[i].p1.x, originalLines[i].p1.y,
                     originalLines[i].p2.x, originalLines[i].p2.y,
                     cx1, cy1, cx2, cy2)) {
            clippedLines.push_back(MyLine(MyPoint(cx1, cy1), MyPoint(cx2, cy2)));
        }
    }
    isClipped = true;
    // Show BOTH original and clipped
    showOriginal = true;
    showClipped = true;

    printf("\n========================================\n");
    printf("CLIPPING COMPLETE!\n");
    printf("Original lines: %d (DARK GRAY)\n", (int)originalLines.size());
    printf("Clipped segments: %d (THICK RED DASHED)\n", (int)clippedLines.size());
    printf("\nLOOK FOR THICK RED DASHED lines INSIDE the BLACK box!\n");
    printf("========================================\n\n");
}

// Test lines
void setupTestLines() {
    originalLines.clear();
    clippedLines.clear();
    tempPoints.clear();
    isClipped = false;
    showOriginal = true;
    showClipped = false;

    // Add 4 test lines that cross the clipping window
    originalLines.push_back(MyLine(MyPoint(-0.9f, 0.0f), MyPoint(0.9f, 0.0f)));   // Horizontal
    originalLines.push_back(MyLine(MyPoint(0.0f, -0.9f), MyPoint(0.0f, 0.9f)));   // Vertical
    originalLines.push_back(MyLine(MyPoint(-0.9f, -0.9f), MyPoint(0.9f, 0.9f)));  // Diagonal
    originalLines.push_back(MyLine(MyPoint(-0.9f, 0.9f), MyPoint(0.9f, -0.9f)));  // Anti-diagonal

    printf("\n=== 4 TEST LINES ADDED ===\n");
    printf("Press 'c' to clip them!\n");
}

// Mouse handler
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float worldX = (x / 450.0f) * 0.95f - 0.95f;
        float worldY = ((900 - y) / 450.0f) * 0.95f - 0.95f;

        if (tempPoints.empty()) {
            tempPoints.push_back(MyPoint(worldX, worldY));
            printf("First point: (%.2f, %.2f)\n", worldX, worldY);
        } else if (tempPoints.size() == 1) {
            originalLines.push_back(MyLine(tempPoints[0], MyPoint(worldX, worldY)));
            tempPoints.clear();
            isClipped = false;
            showOriginal = true;
            showClipped = false;
            clippedLines.clear();
            printf("Line added! Total: %d lines. Press 'c' to clip\n", (int)originalLines.size());
        }
        glutPostRedisplay();
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        originalLines.clear();
        clippedLines.clear();
        tempPoints.clear();
        isClipped = false;
        showOriginal = true;
        showClipped = false;
        printf("\n=== CLEARED ===\n");
        glutPostRedisplay();
    }
}

// Keyboard handler
void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 't':
        case 'T':
            setupTestLines();
            glutPostRedisplay();
            break;
        case 'c':
        case 'C':
            if (!originalLines.empty()) {
                clipAllLines();
            } else {
                printf("No lines to clip. Press 't' for test lines!\n");
            }
            glutPostRedisplay();
            break;
        case 'r':
        case 'R':
            originalLines.clear();
            clippedLines.clear();
            tempPoints.clear();
            isClipped = false;
            showOriginal = true;
            showClipped = false;
            printf("\n=== RESET ===\n");
            glutPostRedisplay();
            break;
        case 27:
            exit(0);
            break;
    }
}

// Display function
void display() {
    drawBackground();
    drawGrid();
    drawAxes();
    drawClippingWindow();
    drawOriginalLines();
    drawClippedLines();
    drawTempPoints();

    // Text overlay
    glDisable(GL_LIGHTING);

    glColor3f(0.0f, 0.0f, 0.0f);
    char title[] = "Cohen-Sutherland Line Clipping Algorithm";
    glRasterPos2f(-0.88f, 0.92f);
    for (int i = 0; title[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, title[i]);

    glColor3f(0.2f, 0.2f, 0.3f);
    char info[150];
    if (isClipped) {
        sprintf(info, "STATUS: CLIPPED | Original (DARK GRAY): %d | Clipped (RED inside box): %d",
                (int)originalLines.size(), (int)clippedLines.size());
    } else {
        sprintf(info, "STATUS: NOT CLIPPED | Lines: %d | Press 'c' to clip",
                (int)originalLines.size());
    }
    glRasterPos2f(-0.88f, 0.86f);
    for (int i = 0; info[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[i]);

    glColor3f(0.3f, 0.3f, 0.4f);
    glRasterPos2f(-0.88f, -0.92f);
    char help[] = "t=Test | c=CLIP (RED appears inside box) | r=Reset | LEFT=Draw | RIGHT=Clear | ESC=Exit";
    for (int i = 0; help[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help[i]);

    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2f(-0.88f, -0.86f);
    char legend[] = "BLACK Box = Clipping Window | DARK GRAY = Original | THICK RED DASHED = Clipped (inside box)";
    for (int i = 0; legend[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, legend[i]);

    if (tempPoints.size() == 1) {
        glColor3f(0.0f, 0.3f, 0.6f);
        glRasterPos2f(-0.88f, 0.80f);
        char prompt[] = "Click second point to complete line...";
        for (int i = 0; prompt[i] != '\0'; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, prompt[i]);
    }

    glEnable(GL_LIGHTING);
    glutSwapBuffers();
}

void initGL() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(900, 900);
    glutCreateWindow("Cohen-Sutherland Line Clipping - See RED Inside BLACK Box");

    initGL();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    printf("\n==========================================================\n");
    printf("   COHEN-SUTHERLAND LINE CLIPPING ALGORITHM\n");
    printf("==========================================================\n");
    printf("\nINSTRUCTIONS:\n");
    printf("  1. Press 't' to add 4 test lines\n");
    printf("  2. Press 'c' to CLIP them\n");
    printf("  3. Look INSIDE the BLACK box - you will see THICK RED DASHED lines\n");
    printf("\nThe THICK RED DASHED lines are the CLIPPED portion!\n");
    printf("==========================================================\n\n");

    glutMainLoop();
    return 0;
}
