#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "config.h"

/*
 * 渲染器模块 —— 负责所有 raylib 绘制操作
 */

#define GRID_WIDTH     (GRID_COLS * CELL_SIZE)   /* 600 */
#define GRID_HEIGHT    (GRID_ROWS * CELL_SIZE)   /* 600 */

/* 窗口常量 */
#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  650
#define WINDOW_TITLE   "Vibe Snake"
#define TARGET_FPS     60

/* 前向声明（避免渲染器依赖游戏逻辑头文件） */
typedef struct Snake Snake;
typedef struct FoodDef FoodDef;

/* ---------- 窗口生命周期 ---------- */
int renderer_init(void);
void renderer_close(void);
int renderer_should_close(void);

/* ---------- 绘制生命周期 ---------- */
void renderer_begin_draw(void);
void renderer_end_draw(void);

/* ---------- 游戏元素绘制 ---------- */
void renderer_draw_grid(void);
void renderer_draw_snake(const Snake *snake);
void renderer_draw_food(int grid_x, int grid_y, const FoodDef *food);

/* ---------- UI 界面绘制 ---------- */
void renderer_draw_start_screen(void);
void renderer_draw_hud(int score, int high_score, int length, int level);
void renderer_draw_pause_overlay(void);
void renderer_draw_game_over_screen(int score, int high_score);

#endif /* RENDERER_H */
