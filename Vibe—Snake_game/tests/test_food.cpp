#include <gtest/gtest.h>
#include "../src/food.h"
#include <cstdlib>   /* 用于 srand */

/* ==================== 食物定义完整性 ==================== */

TEST(FoodDef, AllTypesAccessible)
{
    /* 遍历所有 20 种定义，检查字段合法 */
    for (int i = 0; i < FOOD_COUNT; i++)
    {
        const FoodDef *f = food_get_def(i);
        ASSERT_NE(f, nullptr);
        EXPECT_EQ(f->type, i);
        EXPECT_NE(f->name, nullptr);
        EXPECT_GT(f->score, 0);
        EXPECT_GT(f->growth, 0);
        EXPECT_GT(f->weight, 0);
        EXPECT_GE(f->shape, FOOD_SHAPE_CIRCLE);
        EXPECT_LE(f->shape, FOOD_SHAPE_STAR);
    }
}

TEST(FoodDef, InvalidTypeReturnsNull)
{
    EXPECT_EQ(food_get_def(-1), nullptr);
    EXPECT_EQ(food_get_def(FOOD_COUNT), nullptr);
    EXPECT_EQ(food_get_def(100), nullptr);
}

/* ==================== 权重随机 ==================== */

TEST(FoodRandom, EachTypeAppearsOverManyRolls)
{
    srand(42);
    int counts[FOOD_COUNT] = {0};

    for (int i = 0; i < 10000; i++)
    {
        int type = food_random_type();
        ASSERT_GE(type, 0);
        ASSERT_LT(type, FOOD_COUNT);
        counts[type]++;
    }

    /* 每种类型至少出现 1 次 */
    for (int i = 0; i < FOOD_COUNT; i++)
    {
        EXPECT_GT(counts[i], 0)
            << "Food type " << i << " never appeared in 10000 rolls";
    }
}

TEST(FoodRandom, LegendaryRarerThanCommon)
{
    srand(123);
    int common_count = 0;
    int legendary_count = 0;

    for (int i = 0; i < 5000; i++)
    {
        int type = food_random_type();
        if (type <= 7)  common_count++;        /* Common: 0~7 */
        if (type >= 18) legendary_count++;      /* Legendary: 18~19 */
    }

    /* Common 概率 ~40/68 ≈ 59%，Legendary ~2/68 ≈ 3% */
    EXPECT_GT(common_count, legendary_count * 5);
}

/* ==================== 食物生成 ==================== */

TEST(FoodSpawn, EmptyGridSpawnsSuccessfully)
{
    int x, y, type;
    int result = food_spawn(nullptr, &x, &y, &type);

    ASSERT_EQ(result, 0);
    EXPECT_GE(x, 0);  EXPECT_LT(x, GRID_COLS);
    EXPECT_GE(y, 0);  EXPECT_LT(y, GRID_ROWS);
    EXPECT_GE(type, 0);  EXPECT_LT(type, FOOD_COUNT);
}

TEST(FoodSpawn, NotOnOccupiedCells)
{
    /* 创建一个占用了部分格子的蛇身链表 */
    List *body = list_create();
    list_push_back(body, 3, 3);
    list_push_back(body, 4, 3);
    list_push_back(body, 5, 3);

    int x, y, type;
    int result = food_spawn(body, &x, &y, &type);

    ASSERT_EQ(result, 0);
    EXPECT_FALSE(list_contains(body, x, y));

    list_destroy(body);
}

TEST(FoodSpawn, FailsWhenGridFull)
{
    /* 占满整个 20×20 网格 */
    List *body = list_create();
    for (int x = 0; x < GRID_COLS; x++)
    {
        for (int y = 0; y < GRID_ROWS; y++)
        {
            list_push_back(body, x, y);
        }
    }

    int x, y, type;
    int result = food_spawn(body, &x, &y, &type);
    EXPECT_EQ(result, -1);

    list_destroy(body);
}

/* ==================== 可重复性 ==================== */

TEST(FoodRandom, SameSeedGivesSameSequence)
{
    srand(999);
    int first[5];
    for (int i = 0; i < 5; i++) first[i] = food_random_type();

    srand(999);
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(food_random_type(), first[i])
            << "Mismatch at position " << i;
    }
}
