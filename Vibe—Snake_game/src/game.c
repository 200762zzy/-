#include "game.h"
#include <stdlib.h>

/*
 * 游戏核心实现
 */

/* ---------- 创建与销毁 ---------- */

Game *game_create(void)
{
    Game *game = (Game *)malloc(sizeof(Game));
    if (!game) return NULL;

    game->snake = snake_create(GRID_COLS / 2, GRID_ROWS / 2, 3, DIR_RIGHT);
    if (!game->snake)
    {
        free(game);
        return NULL;
    }

    /* 第一个食物 */
    food_spawn(game->snake->body, &game->food_x, &game->food_y, &game->food_type);

    game->score = 0;
    game->high_score = 0;
    game->total_eaten = 0;
    game->move_interval = INITIAL_MOVE_INTERVAL;
    game->state = GAME_STATE_PLAYING;

    return game;
}

void game_destroy(Game *game)
{
    if (!game) return;
    snake_destroy(game->snake);
    free(game);
}

/* ---------- 更新 ---------- */

void game_update(Game *game)
{
    if (game->state != GAME_STATE_PLAYING) return;

    /* 用 next_dir 预测蛇头实际移动方向 */
    int hx, hy;
    snake_head_pos(game->snake, &hx, &hy);
    Direction dir = snake_next_direction(game->snake);
    int next_x = hx, next_y = hy;
    switch (dir)
    {
    case DIR_UP:    next_y--; break;
    case DIR_DOWN:  next_y++; break;
    case DIR_LEFT:  next_x--; break;
    case DIR_RIGHT: next_x++; break;
    }
    int eating = (next_x == game->food_x && next_y == game->food_y);

    /* 预置增长，让本次移动就增长 */
    if (eating)
    {
        const FoodDef *fd = food_get_def(game->food_type);
        for (int i = 0; i < fd->growth; i++)
        {
            snake_grow(game->snake);
        }
    }

    /* 移动蛇 */
    int result = snake_move(game->snake);

    /* 蛇死亡 */
    if (result != 0)
    {
        if (game->score > game->high_score)
        {
            game->high_score = game->score;
        }
        game->state = GAME_STATE_GAME_OVER;
        return;
    }

    /* 后置保护：预判失败但蛇头实际踩到了食物（极少的转向竞争条件） */
    if (!eating)
    {
        snake_head_pos(game->snake, &hx, &hy);
        if (hx == game->food_x && hy == game->food_y)
        {
            eating = 1;
            /* 尾部已移除，延迟一帧增长 */
            snake_grow(game->snake);
        }
    }

    /* 吃食物的后续处理 */
    if (eating)
    {
        const FoodDef *fd = food_get_def(game->food_type);
        game->score += fd->score;
        game->total_eaten++;

        if (game->total_eaten % SPEED_EVERY_N == 0)
        {
            game->move_interval -= SPEED_STEP;
            if (game->move_interval < MIN_MOVE_INTERVAL)
            {
                game->move_interval = MIN_MOVE_INTERVAL;
            }
        }

        food_spawn(game->snake->body, &game->food_x, &game->food_y, &game->food_type);
    }
}

/* ---------- 控制 ---------- */

void game_turn(Game *game, Direction dir)
{
    if (game->state == GAME_STATE_PLAYING)
    {
        snake_turn(game->snake, dir);
    }
}

void game_set_state(Game *game, GameState state)
{
    game->state = state;
}

/* ---------- 重置 ---------- */

void game_reset(Game *game)
{
    if (!game) return;

    int saved_high = game->high_score;

    snake_destroy(game->snake);
    game->snake = snake_create(GRID_COLS / 2, GRID_ROWS / 2, 3, DIR_RIGHT);
    if (game->snake)
    {
        food_spawn(game->snake->body, &game->food_x, &game->food_y, &game->food_type);
    }

    game->score = 0;
    game->total_eaten = 0;
    game->move_interval = INITIAL_MOVE_INTERVAL;
    game->state = GAME_STATE_PLAYING;
    game->high_score = saved_high;
}

/* ---------- 查询 ---------- */

GameState game_get_state(const Game *game) { return game->state; }
int game_get_score(const Game *game) { return game->score; }
int game_get_high_score(const Game *game) { return game->high_score; }
int game_get_total_eaten(const Game *game) { return game->total_eaten; }
float game_get_interval(const Game *game) { return game->move_interval; }
int game_get_food_x(const Game *game) { return game->food_x; }
int game_get_food_y(const Game *game) { return game->food_y; }
int game_get_food_type(const Game *game) { return game->food_type; }
const Snake *game_get_snake(const Game *game) { return game->snake; }
