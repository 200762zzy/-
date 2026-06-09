#ifndef GAME_H
#define GAME_H

#include "snake.h"
#include "food.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 游戏核心状态机
 *
 * 状态流转：
 *   PLAYING ──(死亡)──→ GAME_OVER
 *   PLAYING ──(P键)──→ PAUSED
 *   PAUSED  ──(P键)──→ PLAYING
 *   GAME_OVER ──(任意键)──→ PLAYING (重新开始)
 *   (START 状态由界面层管理)
 */

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} GameState;

/* 初始移动间隔（秒） */
#define INITIAL_MOVE_INTERVAL 0.25f

/* 每吃 5 个食物减少的间隔 */
#define SPEED_STEP  0.008f

/* 最小移动间隔 */
#define MIN_MOVE_INTERVAL 0.08f

/* 每隔多少个食物加速一次 */
#define SPEED_EVERY_N 5

typedef struct Game {
    Snake *snake;
    int food_x, food_y;
    int food_type;
    int score;
    int high_score;
    int total_eaten;
    float move_interval;
    GameState state;
} Game;

/*
 * 创建游戏实例。
 * 初始状态：PLAYING，蛇位于地图中央，长度 3，方向右。
 * 返回 Game*，失败返回 NULL。
 */
Game *game_create(void);

/*
 * 销毁游戏实例，释放所有内存。
 */
void game_destroy(Game *game);

/*
 * 更新一帧游戏逻辑。
 * 如果状态为 PLAYING：移动蛇 → 检测碰撞/吃食物 → 更新分数。
 */
void game_update(Game *game);

/*
 * 蛇转向（代理给 snake_turn）。
 */
void game_turn(Game *game, Direction dir);

/*
 * 设置游戏状态。
 */
void game_set_state(Game *game, GameState state);

/*
 * 重置游戏（重新开始），保留 high_score。
 */
void game_reset(Game *game);

/* ---------- 查询 ---------- */

GameState game_get_state(const Game *game);
int game_get_score(const Game *game);
int game_get_high_score(const Game *game);
int game_get_total_eaten(const Game *game);
float game_get_interval(const Game *game);
int game_get_food_x(const Game *game);
int game_get_food_y(const Game *game);
int game_get_food_type(const Game *game);
const Snake *game_get_snake(const Game *game);

#ifdef __cplusplus
}
#endif

#endif /* GAME_H */
