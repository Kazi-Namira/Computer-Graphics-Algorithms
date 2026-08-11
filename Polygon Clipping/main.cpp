/*
SUTHERLAND-HODGMAN POLYGON CLIPPING ALGORITHM
White Background with Black Clipping Window
*/

#include <windows.h>
#include <GL/glut.h>
#include <cstdio>
#include <cmath>
#include <vector>

using namespace std;

// Clipping window boundaries
float xmin = -0.5f, xmax = 0.5f;
float ymin = -0.5f, ymax = 0.5f;

// Point structure
struct MyPoint {
    float x, y;
    MyPoint(float x = 0, float y = 0) : x(x), y(y) {}
};

// Polygon structure
struct MyPolygon {
    vector<MyPoint> vertices;
};

// Global variables
vector<MyPolygon> originalPolygons;
vector<MyPolygon> clippedPolygons;
vector<MyPoint> tempPoints;
bool showOriginal = true;
bool showClipped = false;
bool isClipped = false;

// ==================== SUTHERLAND-HODGMAN ALGORITHM ====================

// Check if point is inside the clipping boundary
bool inside(MyPoint p, int edge) {
    switch(edge) {
        case 0: return p.x >= xmin;  // LEFT
        case 1: return p.x <= xmax;  // RIGHT
        case 2: return p.y >= ymin;  // BOTTOM
        case 3: return p.y <= ymax;  // TOP
    }
    return false;
}

// Find intersection point of line with clipping boundary
MyPoint intersection(MyPoint p1, MyPoint p2, int edge) {
    MyPoint intersect;
    float m;

    // Calculate slope
    if (p2.x != p1.x)
        m = (p2.y - p1.y) / (p2.x - p1.x);

    switch(edge) {
        case 0: // LEFT (x = xmin)
            intersect.x = xmin;
            intersect.y = p1.y + m * (xmin - p1.x);
            break;
        case 1: // RIGHT (x = xmax)
            intersect.x = xmax;
            intersect.y = p1.y + m * (xmax - p1.x);
            break;
        case 2: // BOTTOM (y = ymin)
            intersect.y = ymin;
            if (p2.x != p1.x)
                intersect.x = p1.x + (ymin - p1.y) / m;
            else
                intersect.x = p1.x;
            break;
        case 3: // TOP (y = ymax)
            intersect.y = ymax;
            if (p2.x != p1.x)
                intersect.x = p1.x + (ymax - p1.y) / m;
            else
                intersect.x = p1.x;
            break;
    }
    return intersect;
}

// Sutherland-Hodgman Polygon Clipping
MyPolygon sutherlandHodgmanClip(MyPolygon polygon) {
    MyPolygon result = polygon;

    // Clip against each of the 4 boundaries
    for (int edge = 0; edge < 4; edge++) {
        MyPolygon input = result;
        result.vertices.clear();

        if (input.vertices.empty()) break;

        MyPoint S = input.vertices.back(); // Last point

        for (size_t i = 0; i < input.vertices.size(); i++) {
            MyPoint E = input.vertices[i]; // Current point

            // Case 1: Both inside - add E
            if (inside(E, edge) && inside(S, edge)) {
                result.vertices.push_back(E);
            }
            // Case 2: Inside to outside - add intersection
            else if (inside(S, edge) && !inside(E, edge)) {
                result.vertices.push_back(intersection(S, E, edge));
            }
            // Case 3: Outside to inside - add intersection then E
            else if (!inside(S, edge) && inside(E, edge)) {
                result.vertices.push_back(intersection(S, E, edge));
                result.vertices.push_back(E);
            }
            // Case 4: Both outside - add nothing

            S = E;
        }
    }
    return result;
}

// ==================== DRAWING FUNCTIONS ====================

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

    // Light gray fill
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

void drawOriginalPolygons() {
    if (!showOriginal) return;

    // DARK GRAY outline
    glLineWidth(2.0f);
    glColor3f(0.2f, 0.2f, 0.3f);
    for (size_t p = 0; p < originalPolygons.size(); p++) {
        if (originalPolygons[p].vertices.size() < 3) continue;
        glBegin(GL_LINE_LOOP);
        for (size_t i = 0; i < originalPolygons[p].vertices.size(); i++) {
            glVertex2f(originalPolygons[p].vertices[i].x,
                       originalPolygons[p].vertices[i].y);
        }
        glEnd();
    }
}

void drawClippedPolygons() {
    if (!showClipped || clippedPolygons.empty()) return;

    // THICK RED DASHED outline
    glLineWidth(4.0f);
    glColor3f(1.0f, 0.0f, 0.0f);

    glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0xAAAA);

    for (size_t p = 0; p < clippedPolygons.size(); p++) {
        if (clippedPolygons[p].vertices.size() < 3) continue;
        glBegin(GL_LINE_LOOP);
        for (size_t i = 0; i < clippedPolygons[p].vertices.size(); i++) {
            glVertex2f(clippedPolygons[p].vertices[i].x,
                       clippedPolygons[p].vertices[i].y);
        }
        glEnd();
    }

    glDisable(GL_LINE_STIPPLE);
    glLineWidth(1.0f);

    // Fill clipped polygon with translucent red
    glEnable(GL_BLEND);
    glColor4f(1.0f, 0.2f, 0.2f, 0.3f);
    for (size_t p = 0; p < clippedPolygons.size(); p++) {
        if (clippedPolygons[p].vertices.size() < 3) continue;
        glBegin(GL_POLYGON);
        for (size_t i = 0; i < clippedPolygons[p].vertices.size(); i++) {
            glVertex2f(clippedPolygons[p].vertices[i].x,
                       clippedPolygons[p].vertices[i].y);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
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

        // Draw lines between temp points
        if (tempPoints.size() > 1) {
            glLineWidth(1.5f);
            glColor3f(0.0f, 0.5f, 0.8f);
            glBegin(GL_LINE_STRIP);
            for (size_t i = 0; i < tempPoints.size(); i++) {
                glVertex2f(tempPoints[i].x, tempPoints[i].y);
            }
            glEnd();
        }
    }
}

// ==================== CLIPPING FUNCTIONS ====================

void clipAllPolygons() {
    clippedPolygons.clear();
    for (size_t i = 0; i < originalPolygons.size(); i++) {
        MyPolygon clipped = sutherlandHodgmanClip(originalPolygons[i]);
        if (clipped.vertices.size() >= 3) {
            clippedPolygons.push_back(clipped);
        }
    }
    isClipped = true;
    showOriginal = true;
    showClipped = true;

    printf("\n========================================\n");
    printf("SUTHERLAND-HODGMAN CLIPPING COMPLETE!\n");
    printf("Original polygons: %d (DARK GRAY)\n", (int)originalPolygons.size());
    printf("Clipped polygons: %d (THICK RED DASHED)\n", (int)clippedPolygons.size());
    printf("\nLOOK FOR THICK RED DASHED polygons INSIDE the BLACK box!\n");
    printf("========================================\n\n");
}

// ==================== TEST POLYGONS ====================

void setupTestPolygon1() {
    originalPolygons.clear();
    clippedPolygons.clear();
    tempPoints.clear();
    isClipped = false;
    showOriginal = true;
    showClipped = false;

    MyPolygon poly;
    // Large square that crosses all boundaries
    poly.vertices.push_back(MyPoint(-0.8f, -0.8f));
    poly.vertices.push_back(MyPoint(0.8f, -0.8f));
    poly.vertices.push_back(MyPoint(0.8f, 0.8f));
    poly.vertices.push_back(MyPoint(-0.8f, 0.8f));
    originalPolygons.push_back(poly);

    printf("\n=== TEST POLYGON 1 ADDED (Large Square) ===\n");
    printf("Polygon crosses all clipping window boundaries\n");
    printf("Press 'c' to clip it!\n");
}

void setupTestPolygon2() {
    originalPolygons.clear();
    clippedPolygons.clear();
    tempPoints.clear();
    isClipped = false;
    showOriginal = true;
    showClipped = false;

    MyPolygon poly;
    // Pentagon/Star shape
    poly.vertices.push_back(MyPoint(-0.7f, -0.6f));
    poly.vertices.push_back(MyPoint(0.6f, -0.7f));
    poly.vertices.push_back(MyPoint(0.7f, 0.2f));
    poly.vertices.push_back(MyPoint(0.2f, 0.8f));
    poly.vertices.push_back(MyPoint(-0.6f, 0.5f));
    poly.vertices.push_back(MyPoint(-0.7f, -0.2f));
    originalPolygons.push_back(poly);

    printf("\n=== TEST POLYGON 2 ADDED (Complex Pentagon) ===\n");
    printf("Press 'c' to clip it!\n");
}

void setupTestPolygon3() {
    originalPolygons.clear();
    clippedPolygons.clear();
    tempPoints.clear();
    isClipped = false;
    showOriginal = true;
    showClipped = false;

    // Triangle
    MyPolygon poly;
    poly.vertices.push_back(MyPoint(-0.6f, -0.7f));
    poly.vertices.push_back(MyPoint(0.0f, 0.7f));
    poly.vertices.push_back(MyPoint(0.6f, -0.6f));
    originalPolygons.push_back(poly);

    printf("\n=== TEST POLYGON 3 ADDED (Triangle) ===\n");
    printf("Press 'c' to clip it!\n");
}

void setupAllTestPolygons() {
    originalPolygons.clear();
    clippedPolygons.clear();
    tempPoints.clear();
    isClipped = false;
    showOriginal = true;
    showClipped = false;

    // Polygon 1: Large Square
    MyPolygon poly1;
    poly1.vertices.push_back(MyPoint(-0.8f, -0.8f));
    poly1.vertices.push_back(MyPoint(0.8f, -0.8f));
    poly1.vertices.push_back(MyPoint(0.8f, 0.8f));
    poly1.vertices.push_back(MyPoint(-0.8f, 0.8f));
    originalPolygons.push_back(poly1);

    // Polygon 2: Pentagon
    MyPolygon poly2;
    poly2.vertices.push_back(MyPoint(-0.7f, -0.6f));
    poly2.vertices.push_back(MyPoint(0.6f, -0.7f));
    poly2.vertices.push_back(MyPoint(0.7f, 0.2f));
    poly2.vertices.push_back(MyPoint(0.2f, 0.8f));
    poly2.vertices.push_back(MyPoint(-0.6f, 0.5f));
    originalPolygons.push_back(poly2);

    // Polygon 3: Triangle
    MyPolygon poly3;
    poly3.vertices.push_back(MyPoint(-0.6f, -0.7f));
    poly3.vertices.push_back(MyPoint(0.0f, 0.7f));
    poly3.vertices.push_back(MyPoint(0.6f, -0.6f));
    originalPolygons.push_back(poly3);

    printf("\n=== ALL TEST POLYGONS ADDED (3 polygons) ===\n");
    printf("1. Large Square (crosses all boundaries)\n");
    printf("2. Complex Pentagon\n");
    printf("3. Triangle\n");
    printf("Press 'c' to clip them!\n");
}

// ==================== MOUSE HANDLER ====================

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float worldX = (x / 450.0f) * 0.95f - 0.95f;
        float worldY = ((900 - y) / 450.0f) * 0.95f - 0.95f;

        tempPoints.push_back(MyPoint(worldX, worldY));
        printf("Point %d: (%.2f, %.2f)\n", (int)tempPoints.size(), worldX, worldY);
        glutPostRedisplay();
    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        if (tempPoints.size() >= 3) {
            // Complete polygon
            MyPolygon newPoly;
            newPoly.vertices = tempPoints;
            originalPolygons.push_back(newPoly);
            tempPoints.clear();
            isClipped = false;
            showOriginal = true;
            showClipped = false;
            clippedPolygons.clear();
            printf("\nPolygon added with %d vertices! Press 'c' to clip\n", (int)newPoly.vertices.size());
        } else {
            // Clear temp points
            tempPoints.clear();
            printf("Temp points cleared. Need at least 3 points for a polygon.\n");
        }
        glutPostRedisplay();
    }
}

// ==================== KEYBOARD HANDLER ====================

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case '1':
            setupTestPolygon1();
            glutPostRedisplay();
            break;
        case '2':
            setupTestPolygon2();
            glutPostRedisplay();
            break;
        case '3':
            setupTestPolygon3();
            glutPostRedisplay();
            break;
        case '4':
            setupAllTestPolygons();
            glutPostRedisplay();
            break;
        case 'c':
        case 'C':
            if (!originalPolygons.empty()) {
                clipAllPolygons();
            } else {
                printf("No polygons to clip. Press '1', '2', '3', or '4' for test polygons!\n");
            }
            glutPostRedisplay();
            break;
        case 'o':
        case 'O':
            showOriginal = true;
            showClipped = false;
            printf("Showing ORIGINAL polygons only\n");
            glutPostRedisplay();
            break;
        case 'v':
        case 'V':
            if (!clippedPolygons.empty()) {
                showOriginal = false;
                showClipped = true;
                printf("Showing CLIPPED polygons only\n");
            } else {
                printf("No clipped result. Press 'c' first!\n");
            }
            glutPostRedisplay();
            break;
        case 'a':
        case 'A':
            showOriginal = true;
            showClipped = true;
            printf("Showing BOTH original and clipped\n");
            glutPostRedisplay();
            break;
        case 'r':
        case 'R':
            originalPolygons.clear();
            clippedPolygons.clear();
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

// ==================== DISPLAY FUNCTION ====================

void display() {
    drawBackground();
    drawGrid();
    drawAxes();
    drawClippingWindow();
    drawOriginalPolygons();
    drawClippedPolygons();
    drawTempPoints();

    // Text overlay
    glDisable(GL_LIGHTING);

    glColor3f(0.0f, 0.0f, 0.0f);
    char title[] = "Sutherland-Hodgman Polygon Clipping Algorithm";
    glRasterPos2f(-0.88f, 0.92f);
    for (int i = 0; title[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, title[i]);

    glColor3f(0.2f, 0.2f, 0.3f);
    char info[200];
    if (isClipped) {
        sprintf(info, "STATUS: CLIPPED | Original: %d | Clipped (RED inside box): %d | 'a'=Both | 'o'=Original | 'v'=Clipped",
                (int)originalPolygons.size(), (int)clippedPolygons.size());
    } else {
        sprintf(info, "STATUS: NOT CLIPPED | Polygons: %d | Press 'c' to clip | Draw with LEFT CLICK (3+ points, then RIGHT CLICK)",
                (int)originalPolygons.size());
    }
    glRasterPos2f(-0.88f, 0.86f);
    for (int i = 0; info[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[i]);

    glColor3f(0.3f, 0.3f, 0.4f);
    glRasterPos2f(-0.88f, -0.92f);
    char help[] = "1=Square | 2=Pentagon | 3=Triangle | 4=All | c=CLIP | a=Both | o=Original | v=Clipped | r=Reset | ESC=Exit";
    for (int i = 0; help[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, help[i]);

    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2f(-0.88f, -0.86f);
    char legend[] = "BLACK Box = Clipping Window | DARK GRAY = Original | THICK RED DASHED = Clipped Polygon (inside box)";
    for (int i = 0; legend[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, legend[i]);

    if (tempPoints.size() > 0) {
        glColor3f(0.0f, 0.3f, 0.6f);
        glRasterPos2f(-0.88f, 0.80f);
        char prompt[100];
        sprintf(prompt, "Points: %d | RIGHT CLICK to complete polygon (need 3+ points)", (int)tempPoints.size());
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
    glutCreateWindow("Sutherland-Hodgman Polygon Clipping Algorithm");

    initGL();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    printf("\n==========================================================\n");
    printf("   SUTHERLAND-HODGMAN POLYGON CLIPPING ALGORITHM\n");
    printf("==========================================================\n");
    printf("\nINSTRUCTIONS:\n");
    printf("  TEST POLYGONS:\n");
    printf("    1 - Large Square (crosses all boundaries)\n");
    printf("    2 - Complex Pentagon\n");
    printf("    3 - Triangle\n");
    printf("    4 - All three polygons\n");
    printf("\n  DRAW YOUR OWN:\n");
    printf("    LEFT CLICK  - Add vertices (click 3+ points)\n");
    printf("    RIGHT CLICK - Complete and add polygon\n");
    printf("\n  CLIPPING:\n");
    printf("    c - Clip polygons against BLACK box\n");
    printf("    a - Show BOTH original and clipped\n");
    printf("    o - Show only ORIGINAL\n");
    printf("    v - Show only CLIPPED\n");
    printf("    r - Reset/Clear all\n");
    printf("\n  The THICK RED DASHED polygon is the CLIPPED result!\n");
    printf("==========================================================\n\n");

    glutMainLoop();
    return 0;
}
