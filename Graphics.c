/*
 * 2D Graphics Editor using ASCII characters (* and _)
 * Features: Draw circle, rectangle, line, triangle
 *           Add, delete, modify objects
 *           2D character array as canvas
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ROWS 25
#define COLS 60
#define MAX_OBJECTS 50

/* ---------- Canvas ---------- */
char canvas[ROWS][COLS];

void initCanvas() {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            canvas[i][j] = ' ';
}

void displayCanvas() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++)
            putchar(canvas[i][j]);
        putchar('\n');
    }
}

/* ---------- Bounds check ---------- */
int inBounds(int x, int y) {
    return (x >= 0 && x < ROWS && y >= 0 && y < COLS);
}

/* ---------- Drawing primitives ---------- */

/* Draw a straight line (horizontal or vertical only, for simplicity) */
void drawLine(int x1, int y1, int x2, int y2, char ch) {
    if (x1 == x2) {
        /* vertical line */
        int start = (y1 < y2) ? y1 : y2;
        int end   = (y1 < y2) ? y2 : y1;
        for (int j = start; j <= end; j++)
            if (inBounds(x1, j)) canvas[x1][j] = ch;
    } else if (y1 == y2) {
        /* horizontal line */
        int start = (x1 < x2) ? x1 : x2;
        int end   = (x1 < x2) ? x2 : x1;
        for (int i = start; i <= end; i++)
            if (inBounds(i, y1)) canvas[i][y1] = ch;
    } else {
        /* diagonal line using simple DDA */
        int dx = x2 - x1, dy = y2 - y1;
        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
        if (steps == 0) steps = 1;
        float xInc = (float)dx / steps;
        float yInc = (float)dy / steps;
        float x = x1, y = y1;
        for (int i = 0; i <= steps; i++) {
            if (inBounds((int)(x + 0.5), (int)(y + 0.5)))
                canvas[(int)(x + 0.5)][(int)(y + 0.5)] = ch;
            x += xInc;
            y += yInc;
        }
    }
}

/* Draw a rectangle given top-left corner, width, height */
void drawRectangle(int x, int y, int width, int height, char ch) {
    drawLine(x, y, x + width, y, ch);                 /* top */
    drawLine(x, y + height, x + width, y + height, ch); /* bottom */
    drawLine(x, y, x, y + height, ch);                /* left */
    drawLine(x + width, y, x + width, y + height, ch);/* right */
}

/* Draw a circle using midpoint circle algorithm */
void drawCircle(int xc, int yc, int r, char ch) {
    int x = 0, y = r;
    int d = 3 - 2 * r;

    while (x <= y) {
        int points[8][2] = {
            {xc + x, yc + y}, {xc - x, yc + y},
            {xc + x, yc - y}, {xc - x, yc - y},
            {xc + y, yc + x}, {xc - y, yc + x},
            {xc + y, yc - x}, {xc - y, yc - x}
        };
        for (int i = 0; i < 8; i++)
            if (inBounds(points[i][0], points[i][1]))
                canvas[points[i][0]][points[i][1]] = ch;

        if (d < 0) {
            d += 4 * x + 6;
        } else {
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

/* Draw a triangle given three vertices */
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch) {
    drawLine(x1, y1, x2, y2, ch);
    drawLine(x2, y2, x3, y3, ch);
    drawLine(x3, y3, x1, y1, ch);
}

/* ---------- Object management ---------- */

typedef enum { LINE, RECT, CIRCLE, TRIANGLE } ShapeType;

typedef struct {
    ShapeType type;
    int x1, y1, x2, y2, x3, y3; /* generic coordinates */
    int r;                       /* radius for circle */
    char ch;
    int active;                  /* 0 = deleted, 1 = active */
} Shape;

Shape objects[MAX_OBJECTS];
int objCount = 0;

/* Add a new object */
int addObject(Shape s) {
    if (objCount >= MAX_OBJECTS) {
        printf("Object limit reached!\n");
        return -1;
    }
    s.active = 1;
    objects[objCount] = s;
    return objCount++;
}

/* Delete an object (mark inactive) */
void deleteObject(int index) {
    if (index >= 0 && index < objCount) {
        objects[index].active = 0;
        printf("Object %d deleted.\n", index);
    } else {
        printf("Invalid object index.\n");
    }
}

/* Modify an existing object */
void modifyObject(int index, Shape newShape) {
    if (index >= 0 && index < objCount) {
        newShape.active = 1;
        objects[index] = newShape;
        printf("Object %d modified.\n", index);
    } else {
        printf("Invalid object index.\n");
    }
}

/* Render all active objects onto the canvas */
void renderObjects() {
    initCanvas();
    for (int i = 0; i < objCount; i++) {
        if (!objects[i].active) continue;
        Shape s = objects[i];
        switch (s.type) {
            case LINE:
                drawLine(s.x1, s.y1, s.x2, s.y2, s.ch);
                break;
            case RECT:
                drawRectangle(s.x1, s.y1, s.x2, s.y2, s.ch);
                break;
            case CIRCLE:
                drawCircle(s.x1, s.y1, s.r, s.ch);
                break;
            case TRIANGLE:
                drawTriangle(s.x1, s.y1, s.x2, s.y2, s.x3, s.y3, s.ch);
                break;
        }
    }
}

/* List all objects with their indices */
void listObjects() {
    if (objCount == 0) {
        printf("No objects yet.\n");
        return;
    }
    printf("\n--- Object List ---\n");
    for (int i = 0; i < objCount; i++) {
        Shape s = objects[i];
        if (!s.active) {
            printf("[%d] (deleted)\n", i);
            continue;
        }
        switch (s.type) {
            case LINE:
                printf("[%d] LINE   from (%d,%d) to (%d,%d) char='%c'\n",
                       i, s.x1, s.y1, s.x2, s.y2, s.ch);
                break;
            case RECT:
                printf("[%d] RECT   top-left (%d,%d) w=%d h=%d char='%c'\n",
                       i, s.x1, s.y1, s.x2, s.y2, s.ch);
                break;
            case CIRCLE:
                printf("[%d] CIRCLE center (%d,%d) r=%d char='%c'\n",
                       i, s.x1, s.y1, s.r, s.ch);
                break;
            case TRIANGLE:
                printf("[%d] TRIANGLE (%d,%d) (%d,%d) (%d,%d) char='%c'\n",
                       i, s.x1, s.y1, s.x2, s.y2, s.x3, s.y3, s.ch);
                break;
        }
    }
}

/* ---------- Input helpers ---------- */
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ---------- Menu ---------- */
void printMenu() {
    printf("\n========== 2D Graphics Editor ==========\n");
    printf("1. Add Line\n");
    printf("2. Add Rectangle\n");
    printf("3. Add Circle\n");
    printf("4. Add Triangle\n");
    printf("5. Delete Object\n");
    printf("6. Modify Object\n");
    printf("7. List Objects\n");
    printf("8. Display Picture\n");
    printf("9. Exit\n");
    printf("==========================================\n");
    printf("Enter choice: ");
}

/* Read shape details based on type from user */
Shape inputShapeDetails(ShapeType type) {
    Shape s;
    s.type = type;

    switch (type) {
        case LINE:
            printf("Enter x1 y1 x2 y2 and character: ");
            scanf("%d %d %d %d %c", &s.x1, &s.y1, &s.x2, &s.y2, &s.ch);
            break;
        case RECT:
            printf("Enter top-left x y, width, height and character: ");
            scanf("%d %d %d %d %c", &s.x1, &s.y1, &s.x2, &s.y2, &s.ch);
            break;
        case CIRCLE:
            printf("Enter center x y, radius and character: ");
            scanf("%d %d %d %c", &s.x1, &s.y1, &s.r, &s.ch);
            break;
        case TRIANGLE:
            printf("Enter x1 y1 x2 y2 x3 y3 and character: ");
            scanf("%d %d %d %d %d %d %c", &s.x1, &s.y1, &s.x2, &s.y2,
                  &s.x3, &s.y3, &s.ch);
            break;
    }
    clearInputBuffer();
    return s;
}

/* ---------- Main ---------- */
int main() {
    int choice;
    int running = 1;

    initCanvas();

    while (running) {
        printMenu();
        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("Invalid input. Try again.\n");
            continue;
        }
        clearInputBuffer();

        Shape s;
        int idx;

        switch (choice) {
            case 1: /* Add Line */
                s = inputShapeDetails(LINE);
                addObject(s);
                printf("Line added.\n");
                break;

            case 2: /* Add Rectangle */
                s = inputShapeDetails(RECT);
                addObject(s);
                printf("Rectangle added.\n");
                break;

            case 3: /* Add Circle */
                s = inputShapeDetails(CIRCLE);
                addObject(s);
                printf("Circle added.\n");
                break;

            case 4: /* Add Triangle */
                s = inputShapeDetails(TRIANGLE);
                addObject(s);
                printf("Triangle added.\n");
                break;

            case 5: /* Delete Object */
                listObjects();
                printf("Enter object index to delete: ");
                scanf("%d", &idx);
                clearInputBuffer();
                deleteObject(idx);
                break;

            case 6: /* Modify Object */
                listObjects();
                printf("Enter object index to modify: ");
                scanf("%d", &idx);
                clearInputBuffer();
                if (idx >= 0 && idx < objCount) {
                    ShapeType t = objects[idx].type;
                    Shape newS = inputShapeDetails(t);
                    modifyObject(idx, newS);
                } else {
                    printf("Invalid index.\n");
                }
                break;

            case 7: /* List Objects */
                listObjects();
                break;

            case 8: /* Display Picture */
                renderObjects();
                displayCanvas();
                break;

            case 9: /* Exit */
                running = 0;
                printf("Exiting editor. Goodbye!\n");
                break;

            default:
                printf("Invalid choice. Please select 1-9.\n");
        }
    }

    return 0;
}
