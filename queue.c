#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    struct list_head *tmp;
    while (l->next != l) {
        tmp = l->next;
        l->next = tmp->next;
        tmp->next->prev = l;

        element_t *del = container_of(tmp, element_t, list);
        free(del->value);
        free(del);
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
    int length = strlen(s);
    if (new) {
        new->value = malloc(sizeof(char) * (length + 1));
        if (new->value) {
            strncpy(new->value, s, length + 1);
            new->value[length] = '\0';
            INIT_LIST_HEAD(&(new->list));

            list_add(&new->list, head);
            return true;
        } else {
            free(new);
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

    int length = strlen(s);
    if (new) {
        new->value = malloc(sizeof(char) * (length + 1));
        if (new->value) {
            strncpy(new->value, s, length + 1);
            new->value[length] = '\0';
            INIT_LIST_HEAD(&(new->list));

            list_add_tail(&new->list, head);
            return true;
        } else {
            free(new);
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
    if (!head || head->next == head)
        return NULL;
    element_t *rm = list_first_entry(head, element_t, list);
    if (sp) {
        int len = strlen(rm->value);
        len = len < bufsize ? len : bufsize;
        strncpy(sp, rm->value, len);
        sp[len] = '\0';
    }
    head->next = (rm->list).next;
    (rm->list).next->prev = head;
    INIT_LIST_HEAD(&rm->list);

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
        len = len < bufsize ? len : bufsize;
        strncpy(sp, rm->value, len);
        sp[len] = '\0';
    }
    rm->list.prev->next = head;
    head->prev = rm->list.prev;
    INIT_LIST_HEAD(&rm->list);

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
    element_t *del_ele = list_entry(del, element_t, list);
    free(del_ele->value);
    free(del_ele);
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
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
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
    if (!head || head->next == head)
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

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head) {}
