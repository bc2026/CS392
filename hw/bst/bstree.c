#ifndef BSTREE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./bstree.h"
#include "./utils.h"

void print_tree(node_t* node, void (*print_func)(void*)) {
    if(node == NULL)
        return;
    print_tree(node->left, print_func); // Always try to print left subtree
    print_func(node->data); // Print current node
    print_tree(node->right, print_func); // Always try to print right subtree
}

void insert_node(node_t **node, node_t *new_node, int (*cmp)(void*, void*)) {
    if (*node == NULL) {
        *node = new_node;
    } else if (cmp(new_node->data, (*node)->data) < 0) {
        insert_node(&(*node)->left, new_node, cmp);
    } else {
        insert_node(&(*node)->right, new_node, cmp);
    }
}

void add_node(void* data, size_t dataSize, tree_t* t, int (*cmp)(void*, void*)) {
    if (t == NULL || data == NULL || cmp == NULL) {
        printf("Invalid argument\n");
        return;
    }

    node_t* newNode = (node_t*)malloc(sizeof(node_t));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    newNode->data = malloc(dataSize);
    if (newNode->data == NULL) {
        printf("Memory allocation for data failed\n");
        free(newNode);
        return;
    }

    memcpy(newNode->data, data, dataSize);
    newNode->left = NULL;
    newNode->right = NULL;

    if (t->root == NULL) {
        t->root = newNode;
    } else {
        insert_node(&t->root, newNode, cmp);
    }
}


void destroy_node(node_t* node) {
    if (node == NULL) return;
    destroy_node(node->left); // t1
    destroy_node(node->right); // t2
    free(node->data); // Free the data
    free(node); // Free the node
}

void destroy(tree_t* t) {
    destroy_node(t->root); 
    t->root = NULL; }

#endif // BSTREE_H
