#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 双向循环链表 —— 带头哨兵节点
 *
 * 结构说明：
 *   List 中内嵌一个哨兵节点 (sentinel)，不存储有效数据。
 *   sentinel.next 指向第一个数据节点，sentinel.prev 指向最后一个数据节点。
 *   空链表时 sentinel.next == sentinel.prev == &sentinel。
 *
 *   遍历方式：
 *     for (ListNode *p = list->sentinel.next; p != &list->sentinel; p = p->next)
 *
 *   该链表将用于表示贪吃蛇的蛇身：每节蛇身 = 一个 ListNode，
 *   蛇头 = 第一个数据节点，蛇尾 = 最后一个数据节点。
 */

/* 链表节点 */
typedef struct ListNode {
    int x, y;                /* 节点坐标数据（对应蛇身格子位置） */
    struct ListNode *prev;   /* 前驱指针 */
    struct ListNode *next;   /* 后继指针 */
} ListNode;

/* 链表头 */
typedef struct {
    ListNode sentinel;       /* 哨兵节点（嵌入，不单独分配内存） */
    int size;                /* 数据节点个数（不含哨兵） */
} List;

/* ---------- 创建与销毁 ---------- */

/*
 * 创建一个空链表，返回堆分配的 List 指针。
 * 返回 NULL 表示分配失败。
 */
List *list_create(void);

/*
 * 销毁链表，释放所有数据节点和 List 本身。
 */
void list_destroy(List *list);

/* ---------- 插入 ---------- */

/*
 * 在链表头部插入一个新节点（数据 x, y）。
 * 返回 0 成功，-1 失败（内存不足）。
 */
int list_push_front(List *list, int x, int y);

/*
 * 在链表尾部插入一个新节点（数据 x, y）。
 * 返回 0 成功，-1 失败（内存不足）。
 */
int list_push_back(List *list, int x, int y);

/* ---------- 删除 ---------- */

/*
 * 删除头部第一个数据节点。
 * 返回 0 成功，-1 失败（链表为空）。
 */
int list_pop_front(List *list);

/*
 * 删除尾部最后一个数据节点。
 * 返回 0 成功，-1 失败（链表为空）。
 */
int list_pop_back(List *list);

/*
 * 清空链表，移除所有数据节点。
 */
void list_clear(List *list);

/* ---------- 访问 ---------- */

/*
 * 返回第一个数据节点的指针；链表为空时返回 NULL。
 */
ListNode *list_front(const List *list);

/*
 * 返回最后一个数据节点的指针；链表为空时返回 NULL。
 */
ListNode *list_back(const List *list);

/*
 * 返回第 index 个数据节点的指针（0-based）；越界返回 NULL。
 */
ListNode *list_at(const List *list, int index);

/* ---------- 查询 ---------- */

/*
 * 返回数据节点个数。
 */
int list_size(const List *list);

/*
 * 链表是否为空（不含数据节点）。
 * 返回 1 为空，0 非空。
 */
int list_empty(const List *list);

/*
 * 链表中是否存在坐标 (x, y) 的节点。
 * 返回 1 存在，0 不存在。
 */
int list_contains(const List *list, int x, int y);

/* ---------- 辅助 ---------- */

/*
 * 打印链表内容到 stdout（调试用）。
 */
void list_print(const List *list);

#ifdef __cplusplus
}
#endif

#endif /* LIST_H */
