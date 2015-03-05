/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 2
 *
 * bpt_string.hpp
 *
 **********************************************************************/

#ifndef BPT_STRING_H
#define BPT_STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bpt.hpp"

/* Type representing a node in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */
typedef struct string_node {
	void ** pointers;
    list<string> keys;
	struct string_node * parent;
	bool is_leaf;
	int num_keys;
	struct string_node * next; // Used for queue.
} string_node;


class Bpt_string {
public:
    /* functions */
    Bpt_string();

    // Output and utility.

    void enqueue( string_node * new_node );
    string_node * dequeue( void );
    void print_leaves( void );
    int height( void );
    int path_to_root( string_node * child );
    void print_tree( void );
    record * find( string key );
    void find_and_print( string key ); 
    void find_and_print_range( string key_start, string key_end ); 
    int find_range( string key_start, string key_end,
            list<string>& returned_keys, void * returned_pointers[] ); 
    string_node * find_leaf( string key );
    int cut( int length );

    // Insertion.

    record * make_record(int value);
    string_node * make_node( void );
    string_node * make_leaf( void );
    int get_left_index(string_node * parent, string_node * left);
    string_node * insert_into_leaf( string_node * leaf, string key, record * pointer );
    string_node * try_insert_into_full_leaf( string_node * leaf, string key, record * pointer );
    string_node * insert_into_leaf_after_splitting( string_node * leaf, string key, record * pointer );
    string_node * insert_into_node(string_node * n, 
            int left_index, string key, string_node * right);
    string_node * insert_into_node_after_splitting(string_node * old_node, int left_index, 
            string key, string_node * right);
    string_node * insert_into_parent(string_node * left, string key, string_node * right);
    string_node * insert_into_new_root(string_node * left, string key, string_node * right);
    string_node * start_new_tree(string key, record * pointer);
    string_node * insert( string key, int value );

    // Deletion.

    int get_neighbor_index( string_node * n );
    string_node * remove_entry_from_node(string_node * n, string key, string_node * pointer);
    string_node * adjust_root( void );
    string_node * coalesce_nodes( string_node * n, string_node * neighbor, int neighbor_index, string k_prime );
    string_node * redistribute_nodes( string_node * n, string_node * neighbor, int neighbor_index, 
            int k_prime_index, string k_prime );
    string_node * delete_entry( string_node * n, string key, void * pointer );
    string_node * delete_node( string key, int value );
    void destroy_tree_nodes( string_node * n );
    string_node * destroy_tree( void );

private:
    /* variables */

    /* The order determines the maximum and minimum
     * number of entries (keys and pointers) in any
     * node.  Every node has at most order - 1 keys and
     * at least (roughly speaking) half that number.
     * Every leaf has as many pointers to data as keys,
     * and every internal node has one more pointer
     * to a subtree than the number of keys.
     * This global variable is initialized to the
     * default value.
     */
    int order;

    /* The queue is used to print the tree in
     * level order, starting from the root
     * printing each entire rank on a separate
     * line, finishing with the leaves.
     */
    string_node * queue;

    /* The user can toggle on and off the "verbose"
     * property, which causes the pointer addresses
     * to be printed out in hexadecimal notation
     * next to their corresponding keys.
     */
    bool verbose_output;

    /* root of the b+ tree */
    string_node * root;

    /* iterator for list of strings */
    list<string>::iterator iter;
};

#endif
