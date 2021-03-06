/**********************************************************************
 *
 * CS542 Database Management Systems
 *
 * Written by: Tyler Carroll, James Silvia, Tom Strott
 * In completion of: CS557 Project 3
 *
 * bpt.hpp
 *
 **********************************************************************/

#ifndef BPT_H
#define BPT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Default order is 4.
#define DEFAULT_ORDER 100

// Minimum order is necessarily 3.  We set the maximum
// order arbitrarily.  You may change the maximum order.
#define MIN_ORDER 3
#define MAX_ORDER 20

// TYPES.

/* Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */
typedef struct record {
	int value;
	record *next;
} record;

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
typedef struct node {
	void ** pointers;
	int * keys;
	struct node * parent;
	bool is_leaf;
	int num_keys;
	struct node * next; // Used for queue.
} node;


class Bpt {
public:
    /* functions */
    Bpt();

    // Output and utility.

    void enqueue( node * new_node );
    node * dequeue( void );
    void print_leaves( void );
    int height( void );
    int path_to_root( node * child );
    void print_tree( void );
    record * find( int key );
    void find_and_print( int key ); 
    void find_and_print_range( int range1, int range2 ); 
    int find_range( int key_start, int key_end,
            int returned_keys[], void * returned_pointers[] ); 
    node * find_leaf( int key );
    int cut( int length );

    // Insertion.

    record * make_record(int value);
    node * make_node( void );
    node * make_leaf( void );
    int get_left_index(node * parent, node * left);
    node * insert_into_leaf( node * leaf, int key, record * pointer );
    node * try_insert_into_full_leaf( node * leaf, int key, record * pointer );
    node * insert_into_leaf_after_splitting( node * leaf, int key, record * pointer );
    node * insert_into_node(node * n, 
            int left_index, int key, node * right);
    node * insert_into_node_after_splitting(node * old_node, int left_index, 
            int key, node * right);
    node * insert_into_parent(node * left, int key, node * right);
    node * insert_into_new_root(node * left, int key, node * right);
    node * start_new_tree(int key, record * pointer);
    node * insert( int key, int value );

    // Deletion.

    int get_neighbor_index( node * n );
    node * remove_entry_from_node(node * n, int key, node * pointer);
    node * adjust_root( void );
    node * coalesce_nodes( node * n, node * neighbor, int neighbor_index, int k_prime );
    node * redistribute_nodes( node * n, node * neighbor, int neighbor_index, 
            int k_prime_index, int k_prime );
    node * delete_entry( node * n, int key, void * pointer );
    node * delete_node( int key, int value );
    void destroy_tree_nodes( node * n );
    node * destroy_tree( void );

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
    node * queue;

    /* The user can toggle on and off the "verbose"
     * property, which causes the pointer addresses
     * to be printed out in hexadecimal notation
     * next to their corresponding keys.
     */
    bool verbose_output;

    /* root of the b+ tree */
    node * root;
};

#endif
