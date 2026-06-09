#include <gtest/gtest.h>
#include "../src/snake.h"

/* ==================== 初始状态 ==================== */

TEST(SnakeInit, CreatesWithCorrectState)
{
    Snake *snake = snake_create(10, 10, 3, DIR_RIGHT);
    ASSERT_NE(snake, nullptr);

    EXPECT_EQ(snake_length(snake), 3);
    EXPECT_TRUE(snake_is_alive(snake));
    EXPECT_EQ(snake_direction(snake), DIR_RIGHT);

    int hx, hy;
    snake_head_pos(snake, &hx, &hy);

    /* 中心在 (10,10)，长度 3，向右，头在最右 */
    EXPECT_EQ(hx, 11);
    EXPECT_EQ(hy, 10);

    snake_destroy(snake);
}

/* ==================== 前移 ==================== */

TEST(SnakeMove, MovesForwardCorrectly)
{
    /*
     * 初始蛇：中心 (5,5)，长度 3，方向右。
     *   头 (6,5) → (5,5) → 尾 (4,5)
     * 向右移动一格后：
     *   头 (7,5) → (6,5) → 尾 (5,5)
     */
    Snake *snake = snake_create(5, 5, 3, DIR_RIGHT);
    ASSERT_EQ(snake_move(snake), 0);
    ASSERT_TRUE(snake_is_alive(snake));

    int hx, hy;
    snake_head_pos(snake, &hx, &hy);

    EXPECT_EQ(hx, 7);
    EXPECT_EQ(hy, 5);
    EXPECT_EQ(snake_length(snake), 3);

    snake_destroy(snake);
}

TEST(SnakeMove, MovesMultipleSteps)
{
    /*
     * 初始蛇：中心 (5,5)，长度 3，方向右。
     * 头在 (6,5)，向右 5 步后头在 (11,5)。
     */
    Snake *snake = snake_create(5, 5, 3, DIR_RIGHT);
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(snake_move(snake), 0);
    }
    int hx, hy;
    snake_head_pos(snake, &hx, &hy);
    EXPECT_EQ(hx, 11);
    EXPECT_EQ(hy, 5);
    EXPECT_EQ(snake_length(snake), 3);

    snake_destroy(snake);
}

/* ==================== 转向 ==================== */

TEST(SnakeTurn, TurnsInAllFourDirections)
{
    Snake *snake = snake_create(10, 10, 3, DIR_RIGHT);

    snake_turn(snake, DIR_UP);
    snake_move(snake);
    int hx, hy;
    snake_head_pos(snake, &hx, &hy);
    EXPECT_EQ(hx, 11);
    EXPECT_EQ(hy, 9);
    EXPECT_EQ(snake_direction(snake), DIR_UP);

    snake_turn(snake, DIR_LEFT);
    snake_move(snake);
    snake_head_pos(snake, &hx, &hy);
    EXPECT_EQ(hx, 10);
    EXPECT_EQ(hy, 9);
    EXPECT_EQ(snake_direction(snake), DIR_LEFT);

    snake_turn(snake, DIR_DOWN);
    snake_move(snake);
    snake_head_pos(snake, &hx, &hy);
    EXPECT_EQ(hx, 10);
    EXPECT_EQ(hy, 10);
    EXPECT_EQ(snake_direction(snake), DIR_DOWN);

    snake_turn(snake, DIR_RIGHT);
    snake_move(snake);
    snake_head_pos(snake, &hx, &hy);
    EXPECT_EQ(hx, 11);
    EXPECT_EQ(hy, 10);
    EXPECT_EQ(snake_direction(snake), DIR_RIGHT);

    snake_destroy(snake);
}

/* ==================== 禁止反方向 ==================== */

TEST(SnakeTurn, NoReverseDirection)
{
    Snake *snake = snake_create(10, 10, 3, DIR_RIGHT);

    /* 尝试 180° 掉头 */
    snake_turn(snake, DIR_LEFT);
    snake_move(snake);
    int hx, hy;
    snake_head_pos(snake, &hx, &hy);
    /* 应该还是向右走 */
    EXPECT_EQ(hx, 12);
    EXPECT_EQ(hy, 10);
    EXPECT_EQ(snake_direction(snake), DIR_RIGHT);

    snake_destroy(snake);
}

TEST(SnakeTurn, ReverseBlockedAfterTurn)
{
    Snake *snake = snake_create(10, 10, 3, DIR_RIGHT);

    /* 先转到 DOWN */
    snake_turn(snake, DIR_DOWN);
    snake_move(snake);
    EXPECT_EQ(snake_direction(snake), DIR_DOWN);

    /* 尝试反向到 UP */
    snake_turn(snake, DIR_UP);
    snake_move(snake);
    /* 应该还是 DOWN */
    EXPECT_EQ(snake_direction(snake), DIR_DOWN);

    snake_destroy(snake);
}

/* ==================== 撞墙 ==================== */

TEST(SnakeCollision, WallCollisionLeft)
{
    Snake *snake = snake_create(0, 10, 1, DIR_LEFT);
    ASSERT_TRUE(snake_is_alive(snake));

    int result = snake_move(snake);
    EXPECT_EQ(result, -1);
    EXPECT_FALSE(snake_is_alive(snake));

    snake_destroy(snake);
}

TEST(SnakeCollision, WallCollisionRight)
{
    Snake *snake = snake_create(GRID_COLS - 1, 10, 1, DIR_RIGHT);
    ASSERT_TRUE(snake_is_alive(snake));

    snake_move(snake);
    EXPECT_FALSE(snake_is_alive(snake));

    snake_destroy(snake);
}

TEST(SnakeCollision, WallCollisionTop)
{
    Snake *snake = snake_create(10, 0, 1, DIR_UP);
    ASSERT_TRUE(snake_is_alive(snake));

    snake_move(snake);
    EXPECT_FALSE(snake_is_alive(snake));

    snake_destroy(snake);
}

TEST(SnakeCollision, WallCollisionBottom)
{
    Snake *snake = snake_create(10, GRID_ROWS - 1, 1, DIR_DOWN);
    ASSERT_TRUE(snake_is_alive(snake));

    snake_move(snake);
    EXPECT_FALSE(snake_is_alive(snake));

    snake_destroy(snake);
}

/* ==================== 撞自身 ==================== */

/*
 * 自碰撞场景：让蛇走一个 S 形，使蛇头绕回身体中部。
 *
 * 步骤：RIGHT×3 → DOWN×3 → LEFT×2 → DOWN×1 → RIGHT×1 → UP
 * 此时蛇头新位置与身体中部重叠，触发自碰撞。
 */
TEST(SnakeCollision, SelfCollisionDetected)
{
    Snake *snake = snake_create(10, 10, 6, DIR_RIGHT);
    ASSERT_TRUE(snake_is_alive(snake));

    /* RIGHT × 3 */
    for (int i = 0; i < 3; i++) snake_move(snake);
    ASSERT_TRUE(snake_is_alive(snake));

    /* DOWN × 3 */
    snake_turn(snake, DIR_DOWN);
    for (int i = 0; i < 3; i++) snake_move(snake);
    ASSERT_TRUE(snake_is_alive(snake));

    /* LEFT × 2 */
    snake_turn(snake, DIR_LEFT);
    for (int i = 0; i < 2; i++) snake_move(snake);
    ASSERT_TRUE(snake_is_alive(snake));

    /* DOWN × 1 */
    snake_turn(snake, DIR_DOWN);
    snake_move(snake);
    ASSERT_TRUE(snake_is_alive(snake));

    /* RIGHT × 1 */
    snake_turn(snake, DIR_RIGHT);
    snake_move(snake);
    ASSERT_TRUE(snake_is_alive(snake));

    /* UP × 1 —— 此时应撞自身 */
    snake_turn(snake, DIR_UP);
    int result = snake_move(snake);
    EXPECT_EQ(result, -1);
    EXPECT_FALSE(snake_is_alive(snake));

    snake_destroy(snake);
}

/* ==================== 增长 ==================== */

TEST(SnakeGrow, GrowAfterEating)
{
    Snake *snake = snake_create(10, 10, 3, DIR_RIGHT);
    EXPECT_EQ(snake_length(snake), 3);

    snake_grow(snake);
    snake_move(snake);
    /* 增长一节：3 + 1 = 4 */
    EXPECT_EQ(snake_length(snake), 4);

    /* 下一次不增长，长度不变 */
    snake_move(snake);
    EXPECT_EQ(snake_length(snake), 4);

    snake_destroy(snake);
}

TEST(SnakeGrow, MultipleGrowAccumulates)
{
    Snake *snake = snake_create(10, 10, 2, DIR_RIGHT);

    snake_grow(snake);
    snake_grow(snake);
    snake_grow(snake);

    snake_move(snake); /* 第 1 次增长：2 → 3 */
    EXPECT_EQ(snake_length(snake), 3);

    snake_move(snake); /* 第 2 次增长：3 → 4 */
    EXPECT_EQ(snake_length(snake), 4);

    snake_move(snake); /* 第 3 次增长：4 → 5 */
    EXPECT_EQ(snake_length(snake), 5);

    snake_move(snake); /* 不再增长：5 → 5 */
    EXPECT_EQ(snake_length(snake), 5);

    snake_destroy(snake);
}

/* ==================== 组合行为 ==================== */

TEST(SnakeIntegration, EatAndGrowThenMove)
{
    Snake *snake = snake_create(5, 5, 2, DIR_RIGHT);

    /* 向右移动 5 格（边吃边长） */
    for (int i = 0; i < 5; i++) {
        snake_grow(snake);
        snake_move(snake);
    }
    EXPECT_EQ(snake_length(snake), 7);
    EXPECT_TRUE(snake_is_alive(snake));

    /* 再正常移动 3 步，长度不变 */
    for (int i = 0; i < 3; i++) {
        snake_move(snake);
    }
    EXPECT_EQ(snake_length(snake), 7);

    snake_destroy(snake);
}

/* ==================== 销毁 ==================== */

TEST(SnakeDestroy, NullIsSafe)
{
    snake_destroy(nullptr);
}
