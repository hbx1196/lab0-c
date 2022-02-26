#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (new) {
        INIT_LIST_HEAD(new);
        return new;
    } else
        return NULL;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    while (l->next != l) {
        struct list_head *del = l->next;
        list_del(del);
        q_release_element(list_entry(del, element_t, list));
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    int length = strlen(s) + 1;
    if (new) {
        new->value = malloc(length);
        if (new->value) {
            memcpy(new->value, s, length);
            list_add(&new->list, head);
            return true;
        } else {
            q_release_element(new);
            return false;
        }
    }
    return false;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *new = malloc(sizeof(element_t));

    int length = strlen(s) + 1;
    if (new) {
        new->value = malloc(length);
        if (new->value) {
            memcpy(new->value, s, length);
            list_add_tail(&new->list, head);
            return true;
        } else {
            q_release_element(new);
            return false;
        }
    }
    return false;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *rm = list_first_entry(head, element_t, list);
    if (sp) {
        int len = strlen(rm->value);
        len = len < bufsize - 1 ? len : bufsize - 1;
        memcpy(sp, rm->value, len);
        sp[len] = '\0';
    }
    list_del(head->next);

    return rm;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || head->next == head)
        return NULL;
    element_t *rm = list_last_entry(head, element_t, list);
    if (sp) {
        int len = strlen(rm->value);
        len = len < bufsize - 1 ? len : bufsize - 1;
        memcpy(sp, rm->value, len);
        sp[len] = '\0';
    }
    list_del(head->prev);
    return rm;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || head->next == head)
        return 0;
    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || head->next == head)
        return false;

    struct list_head *fast, **low = &head->next, *del;
    for (fast = head->next; fast != head && fast->next != head;
         fast = fast->next->next)
        low = &(*low)->next;
    del = *low;
    *low = (*low)->next;
    q_release_element(list_entry(del, element_t, list));

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head || list_empty(head))
        return false;

    struct list_head *cur, *del;
    char *val = NULL;

    list_for_each (cur, head) {
        char *str = list_entry(cur, element_t, list)->value;
        if (!val || strcmp(val, str))
            val = str;
        else {
            del = cur;
            cur = cur->prev;
            list_del(del);
            q_release_element(list_entry(del, element_t, list));
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head **indir, *cur, *nxt;

    indir = &head->next;
    for (cur = head->next; cur != head && cur->next != head; cur = cur->next) {
        nxt = cur->next;
        *indir = nxt;
        cur->next = nxt->next;
        nxt->prev = cur->prev;
        cur->next->prev = cur;
        cur->prev = nxt;
        nxt->next = cur;
        indir = &cur->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *pre, *nxt, *cur, *t;
    cur = head->next;
    pre = head;
    while (cur != head) {
        nxt = cur->next;
        cur->next = pre;
        pre = cur;
        cur = nxt;
    }
    cur = head->prev;
    nxt = head;
    while (cur != head) {
        pre = cur->prev;
        cur->prev = nxt;
        nxt = cur;
        cur = pre;
    }
    t = head->next;
    head->next = head->prev;
    head->prev = t;
}

struct list_head *merge(struct list_head *l1, struct list_head *l2)
{
    struct list_head *head = NULL, **indir = &head;

    while (l1 && l2) {
        if (strcmp(list_entry(l1, element_t, list)->value,
                   list_entry(l2, element_t, list)->value) <= 0) {
            *indir = l1;
            l1 = l1->next;
        } else {
            *indir = l2;
            l2 = l2->next;
        }
        indir = &(*indir)->next;
    }
    *indir = (struct list_head *) ((uintptr_t) l1 | (uintptr_t) l2);
    return head;
}

struct list_head *merge_sort(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    struct list_head *slow, *fast, *head2;

    for (slow = head, fast = head->next; fast && fast->next;
         fast = fast->next->next)
        slow = slow->next;

    head2 = slow->next;
    slow->next = NULL;
    head = merge_sort(head);
    head2 = merge_sort(head2);
    return merge(head, head2);
}


/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    head->prev->next = NULL;
    head->next = merge_sort(head->next);

    struct list_head *cur, *pre;
    for (cur = head->next, pre = head; cur; cur = cur->next) {
        cur->prev = pre;
        pre = cur;
    }
    pre->next = head;
    head->prev = pre;
}
