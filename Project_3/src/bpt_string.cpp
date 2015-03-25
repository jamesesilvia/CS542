/*********************************************************************
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 3
 *
 * Adapted from: http://www.amittai.com/prose/bpt.c
 * License from source:
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * bpt_string.cpp
 *
 **********************************************************************/

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

using namespace std;

#include "bpt_string.hpp"

/* constructor */
Bpt_string::Bpt_string() {
    root = NULL;
    verbose_output = false;
    queue = NULL;
    order = DEFAULT_ORDER;
}

/* Helper function for printing the
 * tree out.  See print_tree.
 */
void Bpt_string::enqueue( string_node * new_node ) {
    string_node * c;
    if (queue == NULL) {
        queue = new_node;
        queue->next = NULL;
    }
    else {
        c = queue;
        while(c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
        new_node->next = NULL;
    }
}


/* Helper function for printing the
 * tree out.  See print_tree.
 */
string_node * Bpt_string::dequeue( void ) {
    string_node * n = queue;
    queue = queue->next;
    n->next = NULL;
    return n;
}


/* Prints the bottom row of keys
 * of the tree (with their respective
 * pointers, if the verbose_output flag is set.
 */
void Bpt_string::print_leaves( void ) {
    int i = 0;
    string_node * c = root;
    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    while (!c->is_leaf)
        c = (string_node *)c->pointers[0];
    while (true) {
        //for (i = 0; i < c->num_keys; i++) {
        for (iter = c->keys.begin(); iter != c->keys.end(); ++iter, i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)c->pointers[i]);
            //printf("%d ", c->keys[i]);
            printf("%s ", (*iter).c_str());
        }
        if (verbose_output)
            printf("%lx ", (unsigned long)c->pointers[order - 1]);
        if (c->pointers[order - 1] != NULL) {
            printf(" | ");
            c = (string_node *)c->pointers[order - 1];
        }
        else
            break;
    }
    printf("\n");
}


/* Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 */
int Bpt_string::height( void ) {
    int h = 0;
    string_node * c = root;
    while (!c->is_leaf) {
        c = (string_node *)c->pointers[0];
        h++;
    }
    return h;
}


/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
int Bpt_string::path_to_root( string_node * child ) {
    int length = 0;
    string_node * c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
}


/* Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 */
void Bpt_string::print_tree( void ) {

    string_node * n = NULL;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    queue = NULL;
    enqueue(root);
    while( queue != NULL ) {
        n = dequeue();
        if (n->parent != NULL && n == n->parent->pointers[0]) {
            new_rank = path_to_root( n );
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        if (verbose_output) 
            printf("(%lx)", (unsigned long)n);
        //for (i = 0; i < n->num_keys; i++) {
        i = 0;
        for (iter = n->keys.begin(); iter != n->keys.end(); ++iter, i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long)n->pointers[i]);
            //printf("%d ", n->keys[i]);
            printf("%s ", (*iter).c_str());
        }
        if (!n->is_leaf)
            //i = 0;
            //for (iter = n->keys.begin(); iter != n->keys.end(); ++iter, i++)
            for (i = 0; i <= n->num_keys; i++)
                enqueue((string_node *)n->pointers[i]);
        if (verbose_output) {
            if (n->is_leaf) 
                printf("%lx ", (unsigned long)n->pointers[order - 1]);
            else
                printf("%lx ", (unsigned long)n->pointers[n->num_keys]);
        }
        printf("| ");
    }
    printf("\n");
}


/* Finds and returns the record to which
 * a key refers.
 */
record * Bpt_string::find( string key ) {
    int i = 0;
    string_node * c = find_leaf( key );
    if (c == NULL) return NULL;
    for (iter = c->keys.begin(); iter != c->keys.end(); ++iter) {
        if (*iter == key) break;
        i++;
    }
    if (i == c->num_keys) 
        return NULL;
    else
        return (record *)c->pointers[i];
}


/* Finds the record under a given key and prints an
 * appropriate message to stdout.
 */
void Bpt_string::find_and_print( string key ) {
    record * r = find(key);
    if (r == NULL)
        printf("Record not found under key %s.\n", key.c_str());
    else 
        printf("Record at %lx -- key %s, value %d.\n",
                (unsigned long)r, key.c_str(), r->value);
}


/* Finds and prints the keys, pointers, and values within a range
 * of keys between key_start and key_end, including both bounds.
 */
void Bpt_string::find_and_print_range( string key_start, string key_end ) {
    
    //we dont even use this function yet, ill return to it next project
    return;

    int i = 0;
    //int array_size = key_end - key_start + 1;
    int array_size = 1;
    list<string> returned_keys;
    void * returned_pointers[array_size];
    int num_found = find_range( key_start, key_end,
            returned_keys, returned_pointers );
    if (!num_found)
        printf("None found.\n");
    else {
        for (iter = returned_keys.begin(); iter != returned_keys.end(); ++iter)
            printf("Key: %s   Location: %lx  Value: %d\n",
                    (*iter).c_str(),
                    (unsigned long)returned_pointers[i],
                    ((record *)
                     returned_pointers[i])->value);
    }
}


/* Finds keys and their pointers, if present, in the range specified
 * by key_start and key_end, inclusive.  Places these in the arrays
 * returned_keys and returned_pointers, and returns the number of
 * entries found.
 */
int Bpt_string::find_range( string key_start, string key_end,
        list<string>& returned_keys, void * returned_pointers[] ) {
    int i = 0, num_found;
    num_found = 0;
    string_node * n = find_leaf( key_start );
    if (n == NULL) return 0;
    //for (i = 0; i < n->num_keys && n->keys[i] < key_start; i++) ;
    for (iter = n->keys.begin(); iter != n->keys.end(); ++iter) {
        if ((*iter).compare(key_start) >= 0) {
            break;
        }
        i++;
    }
    if (i == n->num_keys) return 0;
    while (n != NULL) {
        //for ( ; i < n->num_keys && n->keys[i] <= key_end; i++) {
        for ( ; iter != n->keys.end(); ++iter) {
            if ((*iter).compare(key_end) < 0) {
                break;
            }
            //returned_keys[num_found] = n->keys[i];
            returned_keys.push_back(*iter);
            returned_pointers[num_found] = n->pointers[i];
            num_found++;
        }
        n = (string_node *)n->pointers[order - 1];
        i = 0;
    }
    return num_found;
}


/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
string_node * Bpt_string::find_leaf( string key ) {
    int i = 0;
    string_node * c = root;
    if (c == NULL) {
        if (verbose_output) 
            printf("Empty tree.\n");
        return c;
    }
    while (!c->is_leaf) {
        if (verbose_output) {
            printf("[");
            for (i = 0; i < c->num_keys - 1; i++)
                printf("%s ", (*iter).c_str());
            printf("%s] ->\n", (*iter).c_str());
        }
        i = 0;
        /*while (i < c->num_keys) {
            if (key >= c->keys[i]) i++;
            else break;
        }*/
        iter = c->keys.begin();
        while (iter != c->keys.end()) {
            if (key.compare(*iter) >= 0) {
                ++iter;
                i++;
            } else {
                break;
            }
        }
        if (verbose_output)
            printf("%d ->\n", i);
        c = (string_node *)c->pointers[i];
    }
    if (verbose_output) {
        printf("Leaf [");
        //for (i = 0; i < c->num_keys - 1; i++)
        list<string>::iterator end_iter = c->keys.end();
        --end_iter;
        for (iter = c->keys.begin(); iter != end_iter; ++iter)
            printf("%s ", (*iter).c_str());
        printf("%s] ->\n", (*iter).c_str());
    }
    return c;
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int Bpt_string::cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}


// INSERTION

/* Creates a new record to hold the value
 * to which a key refers.
 */
record * Bpt_string::make_record(int value) {
    record * new_record = (record *)malloc(sizeof(record));
    if (new_record == NULL) {
        perror("Record creation.");
        exit(EXIT_FAILURE);
    }
    else {
        new_record->value = value;
        new_record->next = NULL;
    }
    return new_record;
}


/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
string_node * Bpt_string::make_node( void ) {
    string_node * new_node;
    /*new_node = (string_node *)malloc(sizeof(string_node));
    if (new_node == NULL) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }*/
    new_node = new string_node;
    /*new_node->keys = (int *)malloc( (order - 1) * sizeof(int) );
    if (new_node->keys == NULL) {
        perror("New node keys array.");
        exit(EXIT_FAILURE);
    }*/
    //new_node->keys = list<string>();

    new_node->pointers =(void **) malloc( order * sizeof(void *) );
    if (new_node->pointers == NULL) {
        perror("New node pointers array.");
        exit(EXIT_FAILURE);
    }
    new_node->is_leaf = false;
    new_node->num_keys = 0;
    new_node->parent = NULL;
    new_node->next = NULL;
    return new_node;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
string_node * Bpt_string::make_leaf( void ) {
    string_node * leaf = make_node();
    leaf->is_leaf = true;
    return leaf;
}


/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int Bpt_string::get_left_index(string_node * parent, string_node * left) {

    int left_index = 0;
    while (left_index <= parent->num_keys && 
            parent->pointers[left_index] != left)
        left_index++;
    return left_index;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
string_node * Bpt_string::insert_into_leaf( string_node * leaf, string key, record * pointer ) {

    int i, insertion_point;
    bool key_exists = false;
    record * ptr = NULL;

    insertion_point = 0;
    /*while (insertion_point < leaf->num_keys && leaf->keys[insertion_point] <= key) {
        if (leaf->keys[insertion_point] == key) {
            // our key already exists, add to the bucket
            key_exists = true;
            break;
        }
        insertion_point++;
    }*/

    iter = leaf->keys.begin();
    while (iter != leaf->keys.end() && (*iter).compare(key) <= 0) {
        if ((*iter).compare(key) == 0) {
            /* our key already exists, add to the bucket */
            key_exists = true;
            break;
        }
        insertion_point++;
        ++iter;
    }

    if (!key_exists) {
        /* key does not exist, make room for new key and insert it */
        for (i = leaf->num_keys; i > insertion_point; i--) {
            //leaf->keys[i] = leaf->keys[i - 1];
            leaf->pointers[i] = leaf->pointers[i - 1];
        }
        //leaf->keys[insertion_point] = key;
        leaf->keys.insert(iter, key);
        leaf->pointers[insertion_point] = pointer;
        leaf->num_keys++;
    } else {
        ptr = (record *)leaf->pointers[insertion_point];
        /* find the end of the list */
        while (ptr->next) {
            ptr = ptr->next;
        }
        /* insert at the end of the list */
        ptr->next = pointer;
    }
    return leaf;
}


/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
string_node * Bpt_string::try_insert_into_full_leaf( string_node * leaf, string key, record * pointer ) {

    int insertion_point;
    bool key_exists = false;
    record * ptr = NULL;

    insertion_point = 0;
    /*while (insertion_point < leaf->num_keys && leaf->keys[insertion_point] <= key) {
        if (leaf->keys[insertion_point] == key) {
            // our key already exists, add to the bucket
            key_exists = true;
            break;
        }
        insertion_point++;
    }*/

    iter = leaf->keys.begin();
    while (iter != leaf->keys.end() && (*iter).compare(key) <= 0) {
        if ((*iter).compare(key) == 0) {
            /* our key already exists, add to the bucket */
            key_exists = true;
            break;
        }
        insertion_point++;
        ++iter;
    }


    if (key_exists) {
        ptr = (record *)leaf->pointers[insertion_point];
        /* find the end of the list */
        while (ptr->next) {
            ptr = ptr->next;
        }
        /* insert at the end of the list */
        ptr->next = pointer;
        return leaf;
    } else {
        return NULL;
    }
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
string_node * Bpt_string::insert_into_leaf_after_splitting( string_node * leaf, string key, record * pointer ) {

    string_node * new_leaf;
    //int * tiemp_keys;
    list<string> temp_keys;
    void ** temp_pointers;
    //int insertion_index, split, new_key, i, j;
    int insertion_index, split, i, j;
    string new_key, temp;

    new_leaf = make_leaf();

    /*temp_keys = (int *)malloc( order * sizeof(int) );
    if (temp_keys == NULL) {
        perror("Temporary keys array.");
        exit(EXIT_FAILURE);
    }*/

    temp_pointers = (void **)malloc( order * sizeof(void *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    /*while (insertion_index < order - 1 && leaf->keys[insertion_index] < key)
        insertion_index++;*/

    temp_keys = leaf->keys;

    iter = temp_keys.begin();
    while (insertion_index < order - 1 && (*iter).compare(key) < 0) {
        insertion_index++;
        ++iter;
    }
    
    temp_keys.insert(iter, key);
   
    /*
    cout << "temp keys: ";
    for (iter = temp_keys.begin(); iter != temp_keys.end(); ++iter) {
        cout << *iter << " ";
    }
    cout << endl;
    */

    for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index) j++;
        //temp_keys[j] = leaf->keys[i];
        temp_pointers[j] = leaf->pointers[i];
    }

    //temp_keys[insertion_index] = key;
    temp_pointers[insertion_index] = pointer;

    leaf->num_keys = 0;
    leaf->keys.erase(leaf->keys.begin(), leaf->keys.end());

    split = cut(order - 1);

    for (i = 0; i < split; i++) {
        leaf->pointers[i] = temp_pointers[i];
        //leaf->keys[i] = temp_keys[i];
        //leaf->keys.push_back(temp_keys.pop_front());
        temp = temp_keys.front();
        leaf->keys.push_back(temp);
        temp_keys.pop_front();
        leaf->num_keys++;
    }

    for (i = split, j = 0; i < order; i++, j++) {
        new_leaf->pointers[j] = temp_pointers[i];
        //new_leaf->keys[j] = temp_keys[i];
        //new_leaf->keys.push_back(temp_keys.pop_front());
        temp = temp_keys.front();
        new_leaf->keys.push_back(temp);
        temp_keys.pop_front();
        new_leaf->num_keys++;
    }

    free(temp_pointers);
    //free(temp_keys);

    new_leaf->pointers[order - 1] = leaf->pointers[order - 1];
    leaf->pointers[order - 1] = new_leaf;

    for (i = leaf->num_keys; i < order - 1; i++)
        leaf->pointers[i] = NULL;
    for (i = new_leaf->num_keys; i < order - 1; i++)
        new_leaf->pointers[i] = NULL;

    new_leaf->parent = leaf->parent;
    //new_key = new_leaf->keys[0];
    new_key = new_leaf->keys.front();

    return insert_into_parent(leaf, new_key, new_leaf);
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
string_node * Bpt_string::insert_into_node( string_node * n, 
        int left_index, string key, string_node * right ) {
    int i;

    iter = n->keys.end();
    for (i = n->num_keys; i > left_index; i--, --iter) {
        n->pointers[i + 1] = n->pointers[i];
        //n->keys[i] = n->keys[i - 1];
        //iter might need to go here
        //--iter;
    }
    n->pointers[left_index + 1] = right;
    //n->keys[left_index] = key;
    n->keys.insert(iter, key);
    n->num_keys++;
    return root;
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
string_node * Bpt_string::insert_into_node_after_splitting(string_node * old_node, int left_index, 
        string key, string_node * right) {

    //int i, j, split, k_prime;
    int i, j, split;
    string k_prime, temp;
    string_node * new_node, * child;
    //int * temp_keys;
    list<string> temp_keys;
    string_node ** temp_pointers;

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */

    temp_pointers = (string_node **)malloc( (order + 1) * sizeof(string_node *) );
    if (temp_pointers == NULL) {
        perror("Temporary pointers array for splitting nodes.");
        exit(EXIT_FAILURE);
    }
    /*temp_keys = (int *)malloc( order * sizeof(int) );
    if (temp_keys == NULL) {
        perror("Temporary keys array for splitting nodes.");
        exit(EXIT_FAILURE);
    }*/

    for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_pointers[j] = (string_node *)old_node->pointers[i];
    }

    temp_keys = old_node->keys;
    iter = temp_keys.begin(); 
    for (i = 0, j = 0; i < old_node->num_keys; i++, j++, ++iter) {
        //if (j == left_index) j++;
        if (j == left_index) {
            temp_keys.insert(iter, key);
            break;
        }
        //temp_keys[j] = old_node->keys[i];
    }

    temp_pointers[left_index + 1] = right;
    //temp_keys[left_index] = key;

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */  
    split = cut(order);
    new_node = make_node();
    old_node->num_keys = 0;
    old_node->keys.erase(old_node->keys.begin(), old_node->keys.end());
    for (i = 0; i < split - 1; i++) {
        old_node->pointers[i] = temp_pointers[i];
        //old_node->keys[i] = temp_keys[i];
        //old_node->keys.push_back(temp_keys.pop_front());
        old_node->keys.push_back(*(temp_keys.begin()));
        temp_keys.pop_front();
        old_node->num_keys++;
    }
    old_node->pointers[i] = temp_pointers[i];
    //k_prime = temp_keys[split - 1];
    k_prime = *(temp_keys.begin());
    for (++i, j = 0; i < order; i++, j++) {
        new_node->pointers[j] = temp_pointers[i];
        //new_node->keys[j] = temp_keys[i];
        //new_node->keys.push_back(temp_keys.pop_front());
        temp = temp_keys.front();
        new_node->keys.push_back(temp);
        temp_keys.pop_front();
        new_node->num_keys++;
    }
    new_node->pointers[j] = temp_pointers[i];
    free(temp_pointers);
    //free(temp_keys);
    new_node->parent = old_node->parent;
    for (i = 0; i <= new_node->num_keys; i++) {
        child = (string_node *)new_node->pointers[i];
        child->parent = new_node;
    }

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */

    return insert_into_parent(old_node, k_prime, new_node);
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
string_node * Bpt_string::insert_into_parent(string_node * left, string key, string_node * right) {

    int left_index;
    string_node * parent;

    parent = left->parent;

    /* Case: new root. */

    if (parent == NULL)
        return insert_into_new_root(left, key, right);

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */

    left_index = get_left_index(parent, left);


    /* Simple case: the new key fits into the node. 
     */

    if (parent->num_keys < order - 1)
        return insert_into_node(parent, left_index, key, right);

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    return insert_into_node_after_splitting(parent, left_index, key, right);
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
string_node * Bpt_string::insert_into_new_root(string_node * left, string key, string_node * right) {

    root = make_node();
    //root->keys[0] = key;
    root->keys.push_back(key);
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->num_keys++;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    return root;
}



/* First insertion:
 * start a new tree.
 */
string_node * Bpt_string::start_new_tree(string key, record * pointer) {

    root = make_leaf();
    //root->keys[0] = key;
    root->keys.push_back(key);
    root->pointers[0] = pointer;
    root->pointers[order - 1] = NULL;
    root->parent = NULL;
    root->num_keys++;
    return root;
}



/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
string_node * Bpt_string::insert( string key, int value ) {

    record * pointer;
    string_node * leaf;

    /* Create a new record for the
     * value.
     */
    pointer = make_record(value);


    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    if (root == NULL) {
        return start_new_tree(key, pointer);
    }


    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    leaf = find_leaf(key);

    /* Case: leaf has room for key and pointer.
     */

    if (leaf->num_keys < order - 1) {
        leaf = insert_into_leaf(leaf, key, pointer);
        return root;
    } else {
        /* Case: leaf is full, check if key already exists.
         */
        if (try_insert_into_full_leaf(leaf, key, pointer)) {
            return root;
        }
    }

    /* Case:  leaf must be split.
     */
    return insert_into_leaf_after_splitting(leaf, key, pointer);
}




// DELETION.

/* Utility function for deletion.  Retrieves
 * the index of a node's nearest neighbor (sibling)
 * to the left if one exists.  If not (the node
 * is the leftmost child), returns -1 to signify
 * this special case.
 */
int Bpt_string::get_neighbor_index( string_node * n ) {

    int i;

    /* Return the index of the key to the left
     * of the pointer in the parent pointing
     * to n.  
     * If n is the leftmost child, this means
     * return -1.
     */
    for (i = 0; i <= n->parent->num_keys; i++)
        if (n->parent->pointers[i] == n)
            return i - 1;

    // Error state.
    printf("Search for nonexistent pointer to node in parent.\n");
    printf("Node:  %#lx\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}


string_node * Bpt_string::remove_entry_from_node(string_node * n, string key, string_node * pointer) {

    int i, num_pointers;

    // Remove the key and shift other keys accordingly.
    /*i = 0;
    while (n->keys[i] != key)
        i++;
    for (++i; i < n->num_keys; i++)
        n->keys[i - 1] = n->keys[i];
    */
    n->keys.remove(key);

    // Remove the pointer and shift other pointers accordingly.
    // First determine number of pointers.
    num_pointers = n->is_leaf ? n->num_keys : n->num_keys + 1;
    i = 0;
    while (n->pointers[i] != pointer)
        i++;
    for (++i; i < num_pointers; i++)
        n->pointers[i - 1] = n->pointers[i];


    // One key fewer.
    n->num_keys--;

    // Set the other pointers to NULL for tidiness.
    // A leaf uses the last pointer to point to the next leaf.
    if (n->is_leaf)
        for (i = n->num_keys; i < order - 1; i++)
            n->pointers[i] = NULL;
    else
        for (i = n->num_keys + 1; i < order; i++)
            n->pointers[i] = NULL;

    return n;
}


string_node * Bpt_string::adjust_root( void ) {

    string_node * new_root;

    /* Case: nonempty root.
     * Key and pointer have already been deleted,
     * so nothing to be done.
     */

    if (root->num_keys > 0)
        return root;

    /* Case: empty root. 
     */

    // If it has a child, promote 
    // the first (only) child
    // as the new root.

    if (!root->is_leaf) {
        new_root = (string_node *)root->pointers[0];
        new_root->parent = NULL;
    }

    // If it is a leaf (has no children),
    // then the whole tree is empty.

    else
        new_root = NULL;

    //free(root->keys);
    free(root->pointers);
    delete root;

    return new_root;
}


/* Coalesces a node that has become
 * too small after deletion
 * with a neighboring node that
 * can accept the additional entries
 * without exceeding the maximum.
 */
string_node * Bpt_string::coalesce_nodes( string_node * n, string_node * neighbor, int neighbor_index, string k_prime) {

    int i, j, neighbor_insertion_index, n_end;
    string_node * tmp;

    /* Swap neighbor with node if node is on the
     * extreme left and neighbor is to its right.
     */

    if (neighbor_index == -1) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }

    /* Starting point in the neighbor for copying
     * keys and pointers from n.
     * Recall that n and neighbor have swapped places
     * in the special case of n being a leftmost child.
     */

    neighbor_insertion_index = neighbor->num_keys;

    /* Case:  nonleaf node.
     * Append k_prime and the following pointer.
     * Append all pointers and keys from the neighbor.
     */

    if (!n->is_leaf) {

        /* Append k_prime.
         */

        //neighbor->keys[neighbor_insertion_index] = k_prime;
        neighbor->keys.push_back(k_prime);
        neighbor->num_keys++;


        n_end = n->num_keys;

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            //neighbor->keys[i] = n->keys[j];
            //neighbor->keys.push_back(n->keys.pop_front());
            neighbor->keys.push_back(*(n->keys.begin()));
            n->keys.pop_front();
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
            n->num_keys--;
        }

        /* The number of pointers is always
         * one more than the number of keys.
         */

        neighbor->pointers[i] = n->pointers[j];

        /* All children must now point up to the same parent.
         */

        for (i = 0; i < neighbor->num_keys + 1; i++) {
            tmp = (string_node *)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }

    /* In a leaf, append the keys and pointers of
     * n to the neighbor.
     * Set the neighbor's last pointer to point to
     * what had been n's right neighbor.
     */

    else {
        for (i = neighbor_insertion_index, j = 0; j < n->num_keys; i++, j++) {
            //neighbor->keys[i] = n->keys[j];
            //neighbor->keys.push_back(n->keys.pop_front());
            neighbor->keys.push_back(*(n->keys.begin()));
            n->keys.pop_front();
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
        }
        neighbor->pointers[order - 1] = n->pointers[order - 1];
    }

    root = delete_entry(n->parent, k_prime, n);
    //free(n->keys);
    free(n->pointers);
    delete n; 
    return root;
}


/* Redistributes entries between two nodes when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small node's entries without exceeding the
 * maximum
 */
string_node * Bpt_string::redistribute_nodes( string_node * n, string_node * neighbor, int neighbor_index, 
        int k_prime_index, string k_prime ) {  

    int i;
    string_node * tmp;

    /* Case: n has a neighbor to the left. 
     * Pull the neighbor's last key-pointer pair over
     * from the neighbor's right end to n's left end.
     */

    if (neighbor_index != -1) {
        if (!n->is_leaf)
            n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
        for (i = n->num_keys; i > 0; i--) {
            //n->keys[i] = n->keys[i - 1];
            n->pointers[i] = n->pointers[i - 1];
        }
        if (!n->is_leaf) {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (string_node *)n->pointers[0];
            tmp->parent = n;
            neighbor->pointers[neighbor->num_keys] = NULL;
            //n->keys[0] = k_prime;
            n->keys.push_front(k_prime);
            //n->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
            //n->parent->keys.insert(k_prime_index, neighbor->keys.pop_back());
            i = 0;
            for (iter = n->parent->keys.begin(); iter != n->parent->keys.end(); ++iter, i++) {
                if (i == k_prime_index) {
                    //n->parent->keys.insert(iter, neighbor->keys.pop_back());
                    n->parent->keys.insert(iter, *(neighbor->keys.end()));
                    break;
                }
            }
        }
        else {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = NULL;
            //n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            //n->keys.push_front(neighbor->keys.pop_back());
            n->keys.push_front(*(neighbor->keys.end()));
            //n->parent->keys[k_prime_index] = n->keys[0];
            //n->parent->keys.insert(k_prime_index, *(n->keys.begin()))
            i = 0;
            for (iter = n->parent->keys.begin(); iter != n->parent->keys.end(); ++iter, i++) {
                if (i == k_prime_index) {
                    n->parent->keys.insert(iter, *(n->keys.begin()));
                    break;
                }
            }
        }
    }

    /* Case: n is the leftmost child.
     * Take a key-pointer pair from the neighbor to the right.
     * Move the neighbor's leftmost key-pointer pair
     * to n's rightmost position.
     */

    else {  
        if (n->is_leaf) {
            //n->keys[n->num_keys] = neighbor->keys[0];
            //n->keys.push_back(neighbor->keys.pop_front());
            n->keys.push_back(*(neighbor->keys.begin()));
            neighbor->keys.pop_front();
            n->pointers[n->num_keys] = neighbor->pointers[0];
            //n->parent->keys[k_prime_index] = neighbor->keys[1];
            //n->parent->keys.insert(k_prime_index, *(neighbor->keys.begin()+1))
            i = 0;
            list<string>::iterator iter_insert = n->keys.begin();
            ++iter_insert;
            for (iter = n->parent->keys.begin(); iter != n->parent->keys.end(); ++iter, i++) {
                if (i == k_prime_index) {
                    n->parent->keys.insert(iter, *(iter_insert));
                    break;
                }
            }
        }
        else {
            //n->keys[n->num_keys] = k_prime;
            n->keys.push_back(k_prime);
            n->pointers[n->num_keys + 1] = neighbor->pointers[0];
            tmp = (string_node *)n->pointers[n->num_keys + 1];
            tmp->parent = n;
            //n->parent->keys[k_prime_index] = neighbor->keys[0];
            //n->parent->keys.insert(k_prime_index, *(n->keys.begin()))
            i = 0;
            for (iter = n->parent->keys.begin(); iter != n->parent->keys.end(); ++iter, i++) {
                if (i == k_prime_index) {
                    n->parent->keys.insert(iter, *(n->keys.begin()));
                    break;
                }
            }
        }
        for (i = 0; i < neighbor->num_keys - 1; i++) {
            //neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!n->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
    }

    /* n now has one more key and one more pointer;
     * the neighbor has one fewer of each.
     */

    n->num_keys++;
    neighbor->num_keys--;

    return root;
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer
 * from the leaf, and then makes all appropriate
 * changes to preserve the B+ tree properties.
 */
string_node * Bpt_string::delete_entry( string_node * n, string key, void * pointer ) {

    int min_keys;
    string_node * neighbor;
    int neighbor_index;
    //int k_prime_index, k_prime;
    int k_prime_index, i;
    string k_prime;
    int capacity;

    // Remove key and pointer from node.

    n = remove_entry_from_node(n, key, (string_node *)pointer);

    /* Case:  deletion from the root. 
     */

    if (n == root) 
        return adjust_root();


    /* Case:  deletion from a node below the root.
     * (Rest of function body.)
     */

    /* Determine minimum allowable size of node,
     * to be preserved after deletion.
     */

    min_keys = n->is_leaf ? cut(order - 1) : cut(order) - 1;

    /* Case:  node stays at or above minimum.
     * (The simple case.)
     */

    if (n->num_keys >= min_keys)
        return root;

    /* Case:  node falls below minimum.
     * Either coalescence or redistribution
     * is needed.
     */

    /* Find the appropriate neighbor node with which
     * to coalesce.
     * Also find the key (k_prime) in the parent
     * between the pointer to node n and the pointer
     * to the neighbor.
     */

    neighbor_index = get_neighbor_index( n );
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    //k_prime = n->parent->keys[k_prime_index];
    for (iter = n->parent->keys.begin(); iter != n->parent->keys.end(); ++iter, i++) {
        if (i == k_prime_index) {
            k_prime = *iter;
            break;
        }
    }
    neighbor = (string_node *)(neighbor_index == -1 ? n->parent->pointers[1] : 
        n->parent->pointers[neighbor_index]);

    capacity = n->is_leaf ? order : order - 1;

    /* Coalescence. */

    if (neighbor->num_keys + n->num_keys < capacity)
        return coalesce_nodes(n, neighbor, neighbor_index, k_prime);

    /* Redistribution. */

    else
        return redistribute_nodes(n, neighbor, neighbor_index, k_prime_index, k_prime);
}



/* Master deletion function.
 */
string_node * Bpt_string::delete_node( string key, int value ) {
    
    string_node * key_leaf;
    record * key_record, * temp, *temp2;
    bool delete_from_tree = false;
    int i;

    key_record = find(key);
    key_leaf = find_leaf(key);
    temp = key_record;
    /* only delete the record we want */
    /* case 1: only 1 record */
    if (temp->next == NULL && temp->value == value) {
        delete_from_tree = true;
    }
    /* case 2: mult records, first one is one we want */
    else if (temp->next != NULL && temp->value == value) {
        delete_from_tree = false;
        //for (i = 0; i < key_leaf->num_keys; i++)
        //    if (key_leaf->keys[i] == key) break;
        i = 0;
        for (iter = key_leaf->keys.begin(); iter != key_leaf->keys.end(); ++iter, i++)
            if (*iter == key) break;
        
        key_leaf->pointers[i] = temp->next;
        free(temp);

    }
    /* case 3: mult records, first one is not what we want */
    else {
        delete_from_tree = false;
        while (temp->next) {
            temp2 = temp;
            temp = temp->next;
            if (temp->value == value) {
                temp2->next = temp->next;
                free(temp);
                break;
            }
        }
    }

    if (delete_from_tree) {
        if (key_record != NULL && key_leaf != NULL) {
            root = delete_entry(key_leaf, key, key_record);
            free(key_record);
        }
    }
    return root;
}


void Bpt_string::destroy_tree_nodes( string_node * n ) {
    int i;
    if (n->is_leaf)
        for (i = 0; i < n->num_keys; i++)
            free(n->pointers[i]);
    else
        for (i = 0; i < n->num_keys + 1; i++)
            destroy_tree_nodes((string_node *)n->pointers[i]);
    free(n->pointers);
    //free(n->keys);
    delete n;
}


string_node * Bpt_string::destroy_tree( void ) {
    destroy_tree_nodes(root);
    return NULL;
}

