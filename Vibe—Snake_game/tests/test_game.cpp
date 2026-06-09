#include <gtest/gtest.h>
#include "../src/game.h"
#include <cmath>

/* 方向偏移：UP, DOWN, LEFT, RIGHT */
static const int DIR_DX[] = { 0, 0, -1, 1 };
static const int DIR_DY[] = { -1, 1, 0, 0 };

/*
 * 辅助：在蛇头正前方放置食物。
 */
static void place_food_ahead(Game *game)
{
    Direction dir = snake_direction(game->snake);
    int hx, hy;
    snake_head_pos(game->snake, &hx, &hy);

    game->food_x = hx + DIR_DX[dir];
    game->food_y = hy + DIR_DY[dir];
    game->food_type = 0;  /* 最小食物，score=1, growth=1 */
}

/* ==================== 创建 ==================== */

TEST(GameCreate, InitialState)
{
    Game *game = game_create();
    ASSERT_NE(game, nullptr);

    EXPECT_EQ(game_get_state(game), GAME_STATE_PLAYING);
    EXPECT_EQ(game_get_score(game), 0);
    EXPECT_EQ(game_get_total_eaten(game), 0);
    EXPECT_EQ(game_get_interval(game), INITIAL_MOVE_INTERVAL);
    EXPECT_NE(game_get_snake(game), nullptr);
    EXPECT_EQ(snake_length(game_get_snake(game)), 3);

    game_destroy(game);
}

/* ==================== 吃食物 ==================== */

TEST(GameScore, EatOneFood)
{
    Game *game = game_create();
    ASSERT_EQ(game_get_score(game), 0);

    place_food_ahead(game);
    game_update(game);

    EXPECT_EQ(game_get_state(game), GAME_STATE_PLAYING);
    EXPECT_EQ(game_get_score(game), 1);  /* 食物 type=0, score=1 */
    EXPECT_EQ(snake_length(game_get_snake(game)), 4);  /* 增长 1 节 */

    game_destroy(game);
}

TEST(GameScore, EatMultipleFoodsAccumulates)
{
    Game *game = game_create();

    int total_score = 0;
    for (int i = 0; i < 5; i++)
    {
        place_food_ahead(game);
        game_update(game);
        ASSERT_EQ(game_get_state(game), GAME_STATE_PLAYING)
            << "Snake died unexpectedly at step " << i;
        total_score += 1;  /* 每个食物 type=0, score=1 */
    }

    EXPECT_EQ(game_get_score(game), total_score);

    game_destroy(game);
}

/* ==================== 速度 ==================== */

TEST(GameSpeed, DecreasesAfterFiveFoods)
{
    Game *game = game_create();
    float initial = game_get_interval(game);

    for (int i = 0; i < SPEED_EVERY_N; i++)
    {
        place_food_ahead(game);
        game_update(game);
        ASSERT_EQ(game_get_state(game), GAME_STATE_PLAYING);
    }

    EXPECT_LT(game_get_interval(game), initial);
    EXPECT_EQ(game_get_interval(game), initial - SPEED_STEP);

    game_destroy(game);
}

TEST(GameSpeed, ClampedToMinimum)
{
    Game *game = game_create();

    /* 把速度调到接近下限 */
    game->move_interval = MIN_MOVE_INTERVAL + SPEED_STEP * 0.5f;
    game->total_eaten = SPEED_EVERY_N - 1;  /* 下次吃食物触发加速 */

    place_food_ahead(game);
    game_update(game);

    EXPECT_GE(game_get_interval(game), MIN_MOVE_INTERVAL);

    game_destroy(game);
}

/* ==================== 死亡 ==================== */

TEST(GameDeath, WallCollision)
{
    Game *game = game_create();

    /* 一直向右走直到撞墙 */
    for (int i = 0; i < 20; i++)
    {
        game_update(game);
        if (game_get_state(game) == GAME_STATE_GAME_OVER) break;
    }

    EXPECT_EQ(game_get_state(game), GAME_STATE_GAME_OVER);

    game_destroy(game);
}

/* ==================== 分数记录 ==================== */

TEST(GameScore, HighScoreUpdatedOnDeath)
{
    Game *game = game_create();

    /* 先吃几个食物攒分 */
    for (int i = 0; i < 3; i++)
    {
        place_food_ahead(game);
        game_update(game);
        ASSERT_EQ(game_get_state(game), GAME_STATE_PLAYING);
    }

    int score_before_death = game_get_score(game);

    /* 持续移动直到撞墙死亡 */
    for (int i = 0; i < 25; i++)
    {
        game_update(game);
        if (game_get_state(game) == GAME_STATE_GAME_OVER) break;
    }

    EXPECT_EQ(game_get_state(game), GAME_STATE_GAME_OVER);
    EXPECT_GE(game_get_high_score(game), score_before_death);

    game_destroy(game);
}

/* ==================== 状态机 ==================== */

TEST(GameState, PauseResume)
{
    Game *game = game_create();
    EXPECT_EQ(game_get_state(game), GAME_STATE_PLAYING);

    game_set_state(game, GAME_STATE_PAUSED);
    EXPECT_EQ(game_get_state(game), GAME_STATE_PAUSED);

    /* 暂停时 update 不应移动蛇 */
    int hx_before, hy_before;
    snake_head_pos(game->snake, &hx_before, &hy_before);

    game_update(game);

    int hx_after, hy_after;
    snake_head_pos(game->snake, &hx_after, &hy_after);
    EXPECT_EQ(hx_after, hx_before);
    EXPECT_EQ(hy_after, hy_before);

    game_set_state(game, GAME_STATE_PLAYING);
    EXPECT_EQ(game_get_state(game), GAME_STATE_PLAYING);

    game_destroy(game);
}

/* ==================== 销毁 ==================== */

TEST(GameDestroy, NullIsSafe)
{
    game_destroy(nullptr);
}
