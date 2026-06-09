/*
 * 双向循环链表单元测试 (Google Test)
 *
 * 测试环境：Visual Studio 2022 + Google Test Adapter
 *
 * 用法：
 *   1. 在解决方案中新建一个 "Google Test" 项目
 *   2. 将该测试项目属性 → C/C++ → 预编译头 → 选"不使用预编译头"
 *   3. 将 src/list.c、src/list.h 和本文件添加到测试项目
 *   4. 生成并运行测试
 */

#include <gtest/gtest.h>
#include "../src/list.h"

/* ==================== 测试套件：创建与销毁 ==================== */

/* 创建链表后不为 NULL */
TEST(ListCreateDestroy, CreatesNonNull)
{
    List *list = list_create();
    EXPECT_NE(list, nullptr);
    list_destroy(list);
}

/* 创建后链表为空 */
TEST(ListCreateDestroy, InitiallyEmpty)
{
    List *list = list_create();
    EXPECT_EQ(list_size(list), 0);
    EXPECT_TRUE(list_empty(list));
    list_destroy(list);
}

/* 销毁 NULL 应不崩溃 */
TEST(ListCreateDestroy, DestroyNullIsSafe)
{
    list_destroy(nullptr);
}

/* ==================== 测试套件：插入 ==================== */

/* push_front 后 size 正确，front 返回正确数据 */
TEST(ListPushFront, SinglePush)
{
    List *list = list_create();
    ASSERT_EQ(list_push_front(list, 5, 10), 0);
    EXPECT_EQ(list_size(list), 1);

    ListNode *node = list_front(list);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->x, 5);
    EXPECT_EQ(node->y, 10);

    list_destroy(list);
}

/* push_front 多次，顺序应为后进先出 */
TEST(ListPushFront, MultiplePushOrder)
{
    List *list = list_create();
    list_push_front(list, 1, 1);
    list_push_front(list, 2, 2);
    list_push_front(list, 3, 3);

    EXPECT_EQ(list_size(list), 3);
    EXPECT_EQ(list_front(list)->x, 3);
    EXPECT_EQ(list_front(list)->y, 3);

    list_destroy(list);
}

/* push_back 后 size 正确，back 返回正确数据 */
TEST(ListPushBack, SinglePush)
{
    List *list = list_create();
    ASSERT_EQ(list_push_back(list, 5, 10), 0);
    EXPECT_EQ(list_size(list), 1);

    ListNode *node = list_back(list);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->x, 5);
    EXPECT_EQ(node->y, 10);

    list_destroy(list);
}

/* push_back 多次，顺序应为先进先出 */
TEST(ListPushBack, MultiplePushOrder)
{
    List *list = list_create();
    list_push_back(list, 1, 1);
    list_push_back(list, 2, 2);
    list_push_back(list, 3, 3);

    EXPECT_EQ(list_size(list), 3);
    EXPECT_EQ(list_front(list)->x, 1);
    EXPECT_EQ(list_back(list)->x, 3);

    list_destroy(list);
}

/* push_front 和 push_back 混用 */
TEST(ListPushBack, MixedPushOrder)
{
    List *list = list_create();
    list_push_front(list, 2, 0);
    list_push_back(list, 3, 0);
    list_push_front(list, 1, 0);
    list_push_back(list, 4, 0);

    EXPECT_EQ(list_size(list), 4);
    EXPECT_EQ(list_front(list)->x, 1);
    EXPECT_EQ(list_back(list)->x, 4);

    /* 验证完整顺序：1 -> 2 -> 3 -> 4 */
    ListNode *p = list_front(list);
    EXPECT_EQ(p->x, 1); p = p->next;
    EXPECT_EQ(p->x, 2); p = p->next;
    EXPECT_EQ(p->x, 3); p = p->next;
    EXPECT_EQ(p->x, 4);

    list_destroy(list);
}

/* ==================== 测试套件：删除 ==================== */

/* pop_front 删除头部，size 递减 */
TEST(ListPopFront, SinglePop)
{
    List *list = list_create();
    list_push_back(list, 1, 0);
    list_push_back(list, 2, 0);
    list_push_back(list, 3, 0);

    ASSERT_EQ(list_pop_front(list), 0);
    EXPECT_EQ(list_size(list), 2);
    EXPECT_EQ(list_front(list)->x, 2);

    list_pop_front(list);
    EXPECT_EQ(list_front(list)->x, 3);

    list_pop_front(list);
    EXPECT_TRUE(list_empty(list));

    list_destroy(list);
}

/* pop_front 空链表返回 -1 */
TEST(ListPopFront, EmptyList)
{
    List *list = list_create();
    EXPECT_EQ(list_pop_front(list), -1);
    list_destroy(list);
}

/* pop_back 删除尾部，size 递减 */
TEST(ListPopBack, SinglePop)
{
    List *list = list_create();
    list_push_back(list, 1, 0);
    list_push_back(list, 2, 0);

    ASSERT_EQ(list_pop_back(list), 0);
    EXPECT_EQ(list_size(list), 1);
    EXPECT_EQ(list_back(list)->x, 1);
    EXPECT_EQ(list_front(list)->x, 1);

    list_pop_back(list);
    EXPECT_TRUE(list_empty(list));

    list_destroy(list);
}

/* pop_back 空链表返回 -1 */
TEST(ListPopBack, EmptyList)
{
    List *list = list_create();
    EXPECT_EQ(list_pop_back(list), -1);
    list_destroy(list);
}

/* ==================== 测试套件：访问 ==================== */

/* list_at 索引访问 */
TEST(ListAccess, AtIndex)
{
    List *list = list_create();
    list_push_back(list, 10, 20);
    list_push_back(list, 30, 40);
    list_push_back(list, 50, 60);

    ListNode *n0 = list_at(list, 0);
    ASSERT_NE(n0, nullptr);
    EXPECT_EQ(n0->x, 10);
    EXPECT_EQ(n0->y, 20);

    ListNode *n1 = list_at(list, 1);
    ASSERT_NE(n1, nullptr);
    EXPECT_EQ(n1->x, 30);

    ListNode *n2 = list_at(list, 2);
    ASSERT_NE(n2, nullptr);
    EXPECT_EQ(n2->x, 50);

    /* 越界返回 NULL */
    EXPECT_EQ(list_at(list, -1), nullptr);
    EXPECT_EQ(list_at(list, 3), nullptr);

    list_destroy(list);
}

/* 空链表 front / back 返回 NULL */
TEST(ListAccess, EmptyListReturnsNull)
{
    List *list = list_create();
    EXPECT_EQ(list_front(list), nullptr);
    EXPECT_EQ(list_back(list), nullptr);
    list_destroy(list);
}

/* ==================== 测试套件：查询 ==================== */

TEST(ListQuery, Contains)
{
    List *list = list_create();
    list_push_back(list, 3, 7);
    list_push_back(list, 5, 9);

    EXPECT_TRUE(list_contains(list, 3, 7));
    EXPECT_TRUE(list_contains(list, 5, 9));
    EXPECT_FALSE(list_contains(list, 0, 0));
    EXPECT_FALSE(list_contains(list, 3, 8));

    list_destroy(list);
}

TEST(ListQuery, EmptyListContainsNothing)
{
    List *list = list_create();
    EXPECT_FALSE(list_contains(list, 1, 1));
    list_destroy(list);
}

/* ==================== 测试套件：清空 ==================== */

TEST(ListClear, AfterClearIsEmpty)
{
    List *list = list_create();
    list_push_back(list, 1, 1);
    list_push_back(list, 2, 2);
    list_push_back(list, 3, 3);

    list_clear(list);
    EXPECT_EQ(list_size(list), 0);
    EXPECT_TRUE(list_empty(list));
    EXPECT_EQ(list_front(list), nullptr);
    EXPECT_EQ(list_back(list), nullptr);

    list_destroy(list);
}

TEST(ListClear, ClearEmptyList)
{
    List *list = list_create();
    list_clear(list);  /* 应不崩溃 */
    EXPECT_TRUE(list_empty(list));
    list_destroy(list);
}

/* ==================== 测试套件：双向循环特性 ==================== */

/* 验证链表确实是双向循环的 */
TEST(ListCircular, DoublyLinkedAndCircular)
{
    List *list = list_create();
    list_push_back(list, 1, 0);
    list_push_back(list, 2, 0);
    list_push_back(list, 3, 0);

    /* 从头正向遍历到哨兵 */
    ListNode *p = list_front(list);
    int count = 0;
    while (p != &list->sentinel) {
        count++;
        p = p->next;
    }
    EXPECT_EQ(count, 3);

    /* 从尾反向遍历到哨兵 */
    p = list_back(list);
    count = 0;
    while (p != &list->sentinel) {
        count++;
        p = p->prev;
    }
    EXPECT_EQ(count, 3);

    list_destroy(list);
}

/* ==================== 测试套件：压力测试 ==================== */

TEST(ListStress, PushPopMany)
{
    List *list = list_create();

    /* 插入 1000 个节点 */
    for (int i = 0; i < 1000; i++) {
        ASSERT_EQ(list_push_back(list, i, i), 0);
    }
    EXPECT_EQ(list_size(list), 1000);

    /* 全部弹出 */
    for (int i = 0; i < 1000; i++) {
        ASSERT_EQ(list_pop_front(list), 0);
    }
    EXPECT_TRUE(list_empty(list));

    list_destroy(list);
}

/* 交替前后插入 */
TEST(ListStress, AlternatingPush)
{
    List *list = list_create();

    for (int i = 0; i < 500; i++) {
        list_push_front(list, i, 0);
        list_push_back(list, i, 0);
    }
    EXPECT_EQ(list_size(list), 1000);

    /* 前 500 个是 499..0，后 500 个是 0..499 */
    ListNode *p = list_front(list);
    for (int i = 499; i >= 0; i--) {
        EXPECT_EQ(p->x, i);
        p = p->next;
    }
    for (int i = 0; i < 500; i++) {
        EXPECT_EQ(p->x, i);
        p = p->next;
    }

    list_destroy(list);
}
