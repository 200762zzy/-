#include "renderer.h"
#include "raylib.h"
#include "snake.h"
#include "food.h"
#include <math.h>

/*
 * 渲染器实现
 */

/* ---------- 颜色常量 ---------- */

/* 网格 */
static const Color COLOR_GRID_BG      = { 15,  15,  20,  255 };
static const Color COLOR_GRID_LINE    = { 30,  30,  45,  255 };
static const Color COLOR_GRID_BORDER  = { 60,  60,  80,  255 };
static const Color COLOR_PANEL_BG     = { 20,  20,  30,  255 };

/* 蛇 */
static const Color COLOR_SNAKE_BODY   = { 0,   228, 48,  255 };
static const Color COLOR_SNAKE_HEAD   = { 0,   255, 128, 255 };
static const Color COLOR_SNAKE_EYE    = { 255, 255, 255, 255 };
static const Color COLOR_SNAKE_PUPIL  = { 20,  20,  20,  255 };

#define SNAKE_PADDING    2   /* 蛇身与网格边界的间距 */
#define EYE_RADIUS       3   /* 眼白半径 */
#define PUPIL_RADIUS    1   /* 瞳孔半径 */
#define EYE_OFFSET_X     7   /* 眼睛相对蛇头中心的水平偏移 */
#define EYE_OFFSET_Y     5   /* 眼睛相对蛇头中心的垂直偏移 */

/* ---------- 窗口生命周期 ---------- */

int renderer_init(void)
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    if (!IsWindowReady()) return -1;

    SetTargetFPS(TARGET_FPS);
    return 0;
}

void renderer_close(void)
{
    CloseWindow();
}

int renderer_should_close(void)
{
    return WindowShouldClose();
}

/* ---------- 绘制生命周期 ---------- */

void renderer_begin_draw(void)
{
    BeginDrawing();
}

void renderer_end_draw(void)
{
    EndDrawing();
}

/* ---------- 食物尺寸常量 ---------- */

#define FOOD_CIRCLE_RADIUS  9
#define FOOD_SQUARE_SIZE    18
#define FOOD_DIAMOND_RADIUS 11
#define FOOD_STAR_OUTER     11
#define FOOD_STAR_INNER      5
#define FOOD_POINTS          5

/* ---------- 辅助：绘制星形 ---------- */

static void draw_star(Vector2 center, float outer, float inner, Color color)
{
    for (int i = 0; i < FOOD_POINTS; i++)
    {
        float a1 = (i * 2 * PI / FOOD_POINTS) - PI / 2;
        float a2 = ((i + 1) * 2 * PI / FOOD_POINTS) - PI / 2;
        float ai = ((i + 0.5f) * 2 * PI / FOOD_POINTS) - PI / 2;

        Vector2 p1 = { center.x + outer * cosf(a1), center.y + outer * sinf(a1) };
        Vector2 p2 = { center.x + outer * cosf(a2), center.y + outer * sinf(a2) };
        Vector2 ip = { center.x + inner * cosf(ai), center.y + inner * sinf(ai) };

        DrawTriangle(p1, ip, p2, color);
    }
}

/* ---------- 辅助：绘制菱形 ---------- */

static void draw_diamond(Vector2 center, float radius, Color color)
{
    Vector2 top    = { center.x,         center.y - radius };
    Vector2 right  = { center.x + radius, center.y };
    Vector2 bottom = { center.x,         center.y + radius };
    Vector2 left   = { center.x - radius, center.y };

    DrawTriangle(top, right, bottom, color);
    DrawTriangle(top, left, bottom, color);
}

/* ---------- 辅助：绘制单节蛇身 ---------- */

static void draw_segment(int col, int row, Color color)
{
    DrawRectangleRounded(
        (Rectangle){
            col * CELL_SIZE + SNAKE_PADDING,
            row * CELL_SIZE + SNAKE_PADDING,
            CELL_SIZE - SNAKE_PADDING * 2,
            CELL_SIZE - SNAKE_PADDING * 2
        },
        0.3f,       /* 圆角程度 */
        8,          /* 圆角分段数 */
        color
    );
}

/* ---------- 辅助：绘制蛇头眼睛 ---------- */

static void draw_head_eyes(int col, int row, Direction dir)
{
    int cx = col * CELL_SIZE + CELL_SIZE / 2;
    int cy = row * CELL_SIZE + CELL_SIZE / 2;

    /* 根据方向计算左右眼的位置 */
    int eye1_x, eye1_y, eye2_x, eye2_y;

    switch (dir)
    {
    case DIR_UP:
        eye1_x = cx - EYE_OFFSET_Y;
        eye1_y = cy - EYE_OFFSET_X;
        eye2_x = cx + EYE_OFFSET_Y;
        eye2_y = cy - EYE_OFFSET_X;
        break;
    case DIR_DOWN:
        eye1_x = cx - EYE_OFFSET_Y;
        eye1_y = cy + EYE_OFFSET_X;
        eye2_x = cx + EYE_OFFSET_Y;
        eye2_y = cy + EYE_OFFSET_X;
        break;
    case DIR_LEFT:
        eye1_x = cx - EYE_OFFSET_X;
        eye1_y = cy - EYE_OFFSET_Y;
        eye2_x = cx - EYE_OFFSET_X;
        eye2_y = cy + EYE_OFFSET_Y;
        break;
    case DIR_RIGHT:
    default:
        eye1_x = cx + EYE_OFFSET_X;
        eye1_y = cy - EYE_OFFSET_Y;
        eye2_x = cx + EYE_OFFSET_X;
        eye2_y = cy + EYE_OFFSET_Y;
        break;
    }

    /* 眼白 */
    DrawCircle(eye1_x, eye1_y, EYE_RADIUS, COLOR_SNAKE_EYE);
    DrawCircle(eye2_x, eye2_y, EYE_RADIUS, COLOR_SNAKE_EYE);

    /* 瞳孔 */
    DrawCircle(eye1_x, eye1_y, PUPIL_RADIUS, COLOR_SNAKE_PUPIL);
    DrawCircle(eye2_x, eye2_y, PUPIL_RADIUS, COLOR_SNAKE_PUPIL);
}

/* ---------- 游戏元素绘制 ---------- */

void renderer_draw_grid(void)
{
    DrawRectangle(0, 0, GRID_WIDTH, GRID_HEIGHT, COLOR_GRID_BG);

    for (int col = 0; col <= GRID_COLS; col++)
    {
        int x = col * CELL_SIZE;
        DrawLine(x, 0, x, GRID_HEIGHT, COLOR_GRID_LINE);
    }
    for (int row = 0; row <= GRID_ROWS; row++)
    {
        int y = row * CELL_SIZE;
        DrawLine(0, y, GRID_WIDTH, y, COLOR_GRID_LINE);
    }

    DrawRectangleLinesEx(
        (Rectangle){ 0, 0, (float)GRID_WIDTH, (float)GRID_HEIGHT },
        2.0f,
        COLOR_GRID_BORDER
    );

    DrawRectangle(GRID_WIDTH, 0,
                  WINDOW_WIDTH - GRID_WIDTH, WINDOW_HEIGHT,
                  COLOR_PANEL_BG);
}

void renderer_draw_snake(const Snake *snake)
{
    /* 遍历蛇身链表，第一节点为蛇头 */
    ListNode *p = list_front(snake->body);
    if (!p) return;

    /* 画蛇头（用较亮的颜色 + 眼睛） */
    draw_segment(p->x, p->y, COLOR_SNAKE_HEAD);
    draw_head_eyes(p->x, p->y, snake->dir);

    /* 统计蛇身段数（不含头）以实现渐变色 */
    int body_count = 0;
    ListNode *q = p->next;
    while (q != &snake->body->sentinel)
    {
        body_count++;
        q = q->next;
    }

    /* 画身体（头→尾渐变：从亮绿渐变为暗绿） */
    p = p->next;
    int idx = 0;
    while (p != &snake->body->sentinel)
    {
        float t = (body_count > 1) ? (float)idx / (body_count - 1) : 0.0f;
        Color c = {
            (unsigned char)(COLOR_SNAKE_BODY.r * (1.0f - t) + 20.0f * t),
            (unsigned char)(COLOR_SNAKE_BODY.g * (1.0f - t) + 60.0f * t),
            (unsigned char)(COLOR_SNAKE_BODY.b * (1.0f - t) + 10.0f * t),
            255
        };
        draw_segment(p->x, p->y, c);
        p = p->next;
        idx++;
    }
}

void renderer_draw_food(int grid_x, int grid_y, const FoodDef *food)
{
    Vector2 center = {
        grid_x * CELL_SIZE + CELL_SIZE / 2.0f,
        grid_y * CELL_SIZE + CELL_SIZE / 2.0f
    };
    Color color = { food->r, food->g, food->b, 255 };

    switch (food->shape)
    {
    case FOOD_SHAPE_CIRCLE:
        DrawCircleV(center, FOOD_CIRCLE_RADIUS, color);
        break;

    case FOOD_SHAPE_SQUARE:
        DrawRectangleRounded(
            (Rectangle){
                center.x - FOOD_SQUARE_SIZE / 2.0f,
                center.y - FOOD_SQUARE_SIZE / 2.0f,
                FOOD_SQUARE_SIZE,
                FOOD_SQUARE_SIZE
            },
            0.15f,
            6,
            color
        );
        break;

    case FOOD_SHAPE_DIAMOND:
        draw_diamond(center, FOOD_DIAMOND_RADIUS, color);
        break;

    case FOOD_SHAPE_STAR:
        draw_star(center, FOOD_STAR_OUTER, FOOD_STAR_INNER, color);
        break;
    }
}

/* ---------- UI 颜色 ---------- */

static const Color COLOR_UI_ACCENT    = { 0,   255, 128, 255 };
static const Color COLOR_UI_LABEL     = { 160, 160, 180, 255 };
static const Color COLOR_UI_TITLE     = { 0,   255, 128, 255 };
static const Color COLOR_OVERLAY_BG   = { 0,   0,   0,   180 };
static const Color COLOR_GAME_OVER_TITLE = { 255, 60, 60, 255 };

/* ---------- UI 界面绘制 ---------- */

void renderer_draw_start_screen(void)
{
    int w = WINDOW_WIDTH;
    int h = WINDOW_HEIGHT;

    DrawText("Vibe Snake",
             w / 2 - MeasureText("Vibe Snake", 60) / 2,
             h / 2 - 80,
             60, COLOR_UI_ACCENT);

    DrawText("Press any key to start",
             w / 2 - MeasureText("Press any key to start", 20) / 2,
             h / 2 + 20,
             20, COLOR_UI_LABEL);
}

void renderer_draw_hud(int score, int high_score, int length, int level)
{
    int px = GRID_WIDTH + 15;
    int py = 20;

    DrawText("VIBE SNAKE",
             GRID_WIDTH + (WINDOW_WIDTH - GRID_WIDTH) / 2
                 - MeasureText("VIBE SNAKE", 24) / 2,
             py, 24, COLOR_UI_ACCENT);

    py += 45;
    DrawText("SCORE", px, py, 18, COLOR_UI_LABEL);
    DrawText(TextFormat("%d", score), px + 100, py, 22, WHITE);

    py += 35;
    DrawText("HIGH", px, py, 18, COLOR_UI_LABEL);
    DrawText(TextFormat("%d", high_score), px + 100, py, 22, WHITE);

    py += 35;
    DrawText("LENGTH", px, py, 18, COLOR_UI_LABEL);
    DrawText(TextFormat("%d", length), px + 100, py, 22, WHITE);

    py += 35;
    DrawText("LEVEL", px, py, 18, COLOR_UI_LABEL);
    DrawText(TextFormat("%d", level), px + 100, py, 22, WHITE);
}

void renderer_draw_pause_overlay(void)
{
    DrawRectangle(0, 0, GRID_WIDTH, GRID_HEIGHT, COLOR_OVERLAY_BG);

    DrawText("PAUSED",
             GRID_WIDTH / 2 - MeasureText("PAUSED", 40) / 2,
             GRID_HEIGHT / 2 - 30,
             40, WHITE);

    DrawText("Press P to continue",
             GRID_WIDTH / 2 - MeasureText("Press P to continue", 18) / 2,
             GRID_HEIGHT / 2 + 20,
             18, COLOR_UI_LABEL);
}

void renderer_draw_game_over_screen(int score, int high_score)
{
    int w = WINDOW_WIDTH;
    int h = WINDOW_HEIGHT;

    DrawText("GAME OVER",
             w / 2 - MeasureText("GAME OVER", 50) / 2,
             h / 2 - 90,
             50, COLOR_GAME_OVER_TITLE);

    DrawText(TextFormat("Score: %d", score),
             w / 2 - MeasureText(TextFormat("Score: %d", score), 24) / 2,
             h / 2 - 20,
             24, WHITE);

    DrawText(TextFormat("Best: %d", high_score),
             w / 2 - MeasureText(TextFormat("Best: %d", high_score), 20) / 2,
             h / 2 + 15,
             20, COLOR_UI_LABEL);

    DrawText("Press SPACE to restart",
             w / 2 - MeasureText("Press SPACE to restart", 18) / 2,
             h / 2 + 65,
             18, COLOR_UI_LABEL);
}
