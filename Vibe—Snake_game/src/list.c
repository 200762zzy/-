#include "list.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * 双向循环链表实现
 *
 * 核心约定：
 *   空链表状态：sentinel.next = sentinel.prev = &sentinel
 *   非空链表：sentinel.next = 第一个数据节点，sentinel.prev = 最后一个数据节点
 *   最后一个节点.next = &sentinel，第一个节点.prev = &sentinel
 *            ┌────────────────────────────────────────────┐
 *            │                                            ▼
 *   sentinel ◄──► node1 ◄──► node2 ◄──► ... ◄──► nodeN ◄─┘
 *            ▲                                            │
 *            └────────────────────────────────────────────┘
 */

/* ---------- 创建与销毁 ---------- */

List *list_create(void)
{
    List *list = (List *)malloc(sizeof(List));
    if (!list) return NULL;

    /* 初始化哨兵节点：prev 和 next 都指向自己（空链表状态） */
    list->sentinel.prev = &list->sentinel;
    list->sentinel.next = &list->sentinel;
    list->size = 0;

    return list;
}

void list_destroy(List *list)
{
    if (!list) return;

    /* 释放所有数据节点 */
    list_clear(list);

    /* 释放链表头 */
    free(list);
}

/* ---------- 内部辅助：在指定节点后插入 ---------- */

/*
 * 在节点 after 之后插入一个新节点（数据 x, y）。
 * 用于统一实现 push_front 和 push_back。
 * 返回 0 成功，-1 失败。
 */
static int list_insert_after(List *list, ListNode *after, int x, int y)
{
    /* 分配新节点 */
    ListNode *node = (ListNode *)malloc(sizeof(ListNode));
    if (!node) return -1;

    node->x = x;
    node->y = y;

    /* 在 after 和 after->next 之间插入 */
    node->prev = after;
    node->next = after->next;

    after->next->prev = node;
    after->next = node;

    list->size++;
    return 0;
}

/* ---------- 插入 ---------- */

int list_push_front(List *list, int x, int y)
{
    /* 在哨兵之后插入 = 头部插入 */
    return list_insert_after(list, &list->sentinel, x, y);
}

int list_push_back(List *list, int x, int y)
{
    /* 在最后一个节点（sentinel.prev）之后插入 = 尾部插入 */
    return list_insert_after(list, list->sentinel.prev, x, y);
}

/* ---------- 删除 ---------- */

int list_pop_front(List *list)
{
    if (list_empty(list)) return -1;

    ListNode *target = list->sentinel.next;

    /* 将 target 从链中摘除 */
    target->next->prev = &list->sentinel;
    list->sentinel.next = target->next;

    free(target);
    list->size--;
    return 0;
}

int list_pop_back(List *list)
{
    if (list_empty(list)) return -1;

    ListNode *target = list->sentinel.prev;

    /* 将 target 从链中摘除 */
    target->prev->next = &list->sentinel;
    list->sentinel.prev = target->prev;

    free(target);
    list->size--;
    return 0;
}

void list_clear(List *list)
{
    if (!list) return;

    /* 逐个删除头部节点，直到为空 */
    while (!list_empty(list)) {
        list_pop_front(list);
    }
}

/* ---------- 访问 ---------- */

ListNode *list_front(const List *list)
{
    if (list_empty(list)) return NULL;
    return list->sentinel.next;
}

ListNode *list_back(const List *list)
{
    if (list_empty(list)) return NULL;
    return list->sentinel.prev;
}

ListNode *list_at(const List *list, int index)
{
    if (index < 0 || index >= list->size) return NULL;

    ListNode *p = list->sentinel.next;
    for (int i = 0; i < index; i++) {
        p = p->next;
    }
    return p;
}

/* ---------- 查询 ---------- */

int list_size(const List *list)
{
    return list->size;
}

int list_empty(const List *list)
{
    /* 空链表哨兵自循环 */
    return list->sentinel.next == &list->sentinel;
}

int list_contains(const List *list, int x, int y)
{
    /* 遍历所有数据节点查找坐标 */
    for (ListNode *p = list->sentinel.next; p != &list->sentinel; p = p->next) {
        if (p->x == x && p->y == y) {
            return 1;
        }
    }
    return 0;
}

/* ---------- 辅助 ---------- */

void list_print(const List *list)
{
    printf("List(size=%d): ", list->size);
    for (ListNode *p = list->sentinel.next; p != &list->sentinel; p = p->next) {
        printf("(%d,%d) ", p->x, p->y);
    }
    printf("\n");
}
