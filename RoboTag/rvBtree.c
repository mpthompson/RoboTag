/*
    Balanced Binary Tree Object

    Binary tree creation, traversal and freeing utilizing a user-supplied
    comparison routine.

    Code copied from:
    "Fundamentals of Data Structures"
    Ellis Horowitz and Sartaj Sahni
    Computer Science Press, Inc. 1976
    ISBN 0-914894-20'X
    pp. 454-455

    $Id: rvBtree.c 28 2010-03-09 23:49:39Z mike $
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "rvBtree.h"
#include "rvMemPool.h"

typedef struct _rvBtreeElement rvBtreeElement;

struct _rvBtreeElement
{
    rvBtreeElement *up;     // parent.
    rvBtreeElement *left;   // left child.
    rvBtreeElement *right;  // right child.
    int bf;                 // balance factor (0, 1, or -1).
    int data[1];
};

struct _rvBtree
{
    int used;
    int objectSize;
    int objectCount;
    int elementSize;
    rvBtreeElement *top;
    rvBtree_Compare compareFunc;
    rvMemPool *memPool;
};


static void
rvBtree_ElementFree(rvBtreeElement *element, rvBtree_DeleteFunction objfree)
// This will free the memory allocated for one element.
{
    // Make sure we have an element.
    if (element)
    {
        // Free the left and right elements.
        if (element->left) rvBtree_ElementFree(element->left, objfree);
        if (element->right) rvBtree_ElementFree(element->right, objfree);

        // Free the object data if a function was passed in.
        if (objfree) objfree(&element->data);
    }
}


rvBtree *
rvBtree_New(rvBtree_Compare compFunc, int objectCount, int objectSize, int extra)
// Creates an initializes the rvBtree object.
{
    rvBtree *self;
    int elementSize;

    // Sanity check the arguments.
    if (objectSize < 4) objectSize = sizeof(int);
    if (objectCount < 4) objectCount = 4;

    // Allocate the object.
    self = (rvBtree *) malloc(sizeof(rvBtree) + extra);

    // Determine the size of the element structure prior to variable data portion.
    elementSize = (rvInt32) (rvUint64) &((rvBtreeElement *) NULL)->data[0];

    // Create the memory pool allocator object.
    self->memPool = (rvMemPool *) rvMemPool_New(objectCount * (elementSize + objectSize));

    // Initialize the rvBtree object.
    self->top = NULL;
    self->used = 0;
    self->compareFunc = compFunc;
    self->objectSize = objectSize;
    self->objectCount = objectCount;
    self->elementSize = elementSize;

    // Return the tree object.
    return self;
}


void
rvBtree_Free(rvBtree *self, rvBtree_DeleteFunction objfree)
// Free all resources associated with the rvBtree object.
{
    // Free all elements.
    rvBtree_ElementFree(self->top, objfree);

    // Free the memory pool.
    rvMemPool_Free(self->memPool);

    // Free this object.
    free(self);
}


bool
rvBtree_IsEmpty(rvBtree *self)
// Return true if the rvBtree is currently empty.
{
    return (self->top == NULL);
}


void *
rvBtree_Extra(rvBtree *self)
// Return a pointer to the extra information in the rvBtree object.
// The extra information can be used to store additional information
// associated with the data stored in the rvBtree objects.
{
    // Return a pointer to the extra information.
    return self + 1;
}


int
rvBtree_Used(rvBtree *self)
// Returns the total number of elements in the rvBtree.
{
    // Return the used element count.
    return self->used;
}


size_t
rvBtree_Memory(rvBtree *self)
// Returns the total memory allocated to elements in the rvBtree.  This does not
// include memory in unallocated memory pool blocks.  This function is useful
// for tracking the total amount of memory allocated within the rvBtree.
{
    // Return the used count multiplied by the object size.
    return self->used * (size_t) (((rvBtreeElement *) 0)->data + self->objectSize);
}


void *
rvBtree_Add(rvBtree *self, const void *object)
// This function is the core rvBtree method. It will:
//   1. add a new element to the tree at the right place
//      so that the tree remains sorted.
//   2. balance the tree to be as fast as possible when
//      reading it.
{
    rvBtreeElement *return_element;

    // Allocate and initialize the new element.
    return_element = (rvBtreeElement *) rvMemPool_Alloc(self->memPool, sizeof(rvBtreeElement) + self->objectSize);
    return_element->up = NULL;
    return_element->left = NULL;
    return_element->right = NULL;
    return_element->bf = 0;

    // Increment the number of elements allocated in the rvBtree.
    ++self->used;

    // Copy the object data into the element.
    memcpy(return_element->data, object, self->objectSize);

    // Insert the object into the balanced binary tree.
    if (self->top == NULL)
    {
        // This element is the new root of the tree.
        self->top = return_element;
    }
    else
    {
        rvBtreeElement *element;
        rvBtreeElement *unbalancedParent = NULL;
        rvBtreeElement *unbalancedElement = NULL;
        rvBtreeElement *unbalancedChild;
        rvBtreeElement *parentElement;
        int unbalancedCompare = 0;
        int compare = 0;            // Initialize to avoid compiler warning.
        int dir;

        // Initialize for the search.
        parentElement = NULL;
        element = self->top;

        // Phase 1:
        // Locate insertion point for "object."
        // unbalancedElement keeps track of most recent node with bf != 0.
        // unbalancedParent is that node's parent.
        // parentElement follows element through the tree.
        while (element != NULL)
        {
            // Compare the element agains the object.
            compare = self->compareFunc(object, element->data);

            // Is this a node with bf != 0?
            if ((element->bf != 0) || (element == self->top))
            {
                unbalancedElement = element;
                unbalancedParent = parentElement;
                unbalancedCompare = compare;
            }

            if (compare < 0)
            {
                // Take the left branch.
                parentElement = element;
                element = element->left;
            }
            else /* if (compare >= 0) */
            {
                // Take the right branch.
                parentElement = element;
                element = element->right;
            }
        }

        // Phase 2:
        // Insert and rebalance.
        // "object" may be inserted as the appropriate child of parentElement.
        // Add the new element into the tree.
        if (compare < 0)
        {
            // Insert as left child.
            parentElement->left = return_element;
            return_element->up = parentElement;
        }
        else
        {
            // Insert as right child.
            parentElement->right = return_element;
            return_element->up = parentElement;
        }

        // Adjust balance factors fo nodes on path from unbalancedElement
        // to parentElement.  Note that by definition of unbalancedElement,
        // all nodes on this path must have balance factors of 0 and so will
        // change to +/-1.  dir==+1 implies element is inserted in left subtree
        // of unbalancedElement and dir==-1 implies element is inserted in
        // right sub-tree of unbalancedElement.
        if (unbalancedCompare < 0)
        {
            element = unbalancedElement->left;
            unbalancedChild = element;
            dir = 1;
        }
        else
        {
            element = unbalancedElement->right;
            unbalancedChild = element;
            dir = -1;
        }

        while (element != return_element)
        {
            if (self->compareFunc(object, element->data) < 0)
            {
                // Height of left increases by 1.
                element->bf = 1;
                element = element->left;
            }
            else
            {
                // Height of right increases by 1.
                element->bf = -1;
                element = element->right;
            }
        }

        // Is the tree unbalanced?
        if (unbalancedElement->bf == 0)
        {
            // Tree is still balanced.
            unbalancedElement->bf = dir;
        }
        else if ((unbalancedElement->bf + dir) == 0)
        {
            // Tree is still balanced.
            unbalancedElement->bf = 0;
        }
        else
        {
            rvBtreeElement *rotate_element;
            rvBtreeElement *rotate_left_child;
            rvBtreeElement *rotate_right_child;

            // The tree is unbalanced.
            if (dir == 1)
            {
                // Left imbalance.
                if (unbalancedChild->bf == 1)
                {
                    // Perform LL rotation.
                    unbalancedElement->left = unbalancedChild->right;
                    if (unbalancedChild->right != NULL) unbalancedChild->right->up = unbalancedElement;

                    unbalancedChild->right = unbalancedElement;
                    unbalancedElement->up = unbalancedChild;

                    unbalancedElement->bf = 0;
                    unbalancedChild->bf = 0;
                }
                else
                {
                    // Perform LR rotation.
                    rotate_element = unbalancedChild->right;
                    rotate_left_child = rotate_element->left;
                    rotate_right_child = rotate_element->right;

                    unbalancedChild->right = rotate_left_child;
                    if (rotate_left_child != NULL) rotate_left_child->up = unbalancedChild;

                    unbalancedElement->left = rotate_right_child;
                    if (rotate_right_child != NULL) rotate_right_child->up = unbalancedElement;

                    rotate_element->left = unbalancedChild;
                    unbalancedChild->up = rotate_element;

                    rotate_element->right = unbalancedElement;
                    unbalancedElement->up = rotate_element;

                    if (rotate_element->bf == 0)
                    {
                        // LR(a) rotation.
                        unbalancedElement->bf = 0;
                        unbalancedChild->bf = 0;
                    }
                    else if (rotate_element->bf == 1)
                    {
                        // LR(b) rotation.
                        unbalancedElement->bf = -1;
                        unbalancedChild->bf = 0;
                    }
                    else /* if (rotate_element->bf == -1) */
                    {
                        // LR(c) rotation.
                        unbalancedElement->bf = 0;
                        unbalancedChild->bf = 1;
                    }

                    // unbalancedChild is the new subtree root.
                    rotate_element->bf = 0;
                    unbalancedChild = rotate_element;
                }
            }
            else /* if (dir == -1) */
            {
                // Right imbalance.
                if (unbalancedChild->bf == -1)
                {
                    // Perform RR rotation.
                    unbalancedElement->right = unbalancedChild->left;
                    if (unbalancedChild->left != NULL) unbalancedChild->left->up = unbalancedElement;

                    unbalancedChild->left = unbalancedElement;
                    unbalancedElement->up = unbalancedChild;

                    unbalancedElement->bf = 0;
                    unbalancedChild->bf = 0;
                }
                else
                {
                    // Perform RL rotation.
                    rotate_element = unbalancedChild->left;
                    rotate_left_child = rotate_element->left;
                    rotate_right_child = rotate_element->right;

                    unbalancedChild->left = rotate_right_child;
                    if (rotate_right_child != NULL) rotate_right_child->up = unbalancedChild;

                    unbalancedElement->right = rotate_left_child;
                    if (rotate_left_child != NULL) rotate_left_child->up = unbalancedElement;

                    rotate_element->right = unbalancedChild;
                    unbalancedChild->up = rotate_element;

                    rotate_element->left = unbalancedElement;
                    unbalancedElement->up = rotate_element;

                    if (rotate_element->bf == 0)
                    {
                        // RL(a) rotation.
                        unbalancedElement->bf = 0;
                        unbalancedChild->bf = 0;
                    }
                    else if (rotate_element->bf == -1)
                    {
                        // RL(b) rotation.
                        unbalancedElement->bf = 1;
                        unbalancedChild->bf = 0;
                    }
                    else /* if (rotate_element->bf == 1) */
                    {
                        // RL(c) rotation.
                        unbalancedElement->bf = 0;
                        unbalancedChild->bf = -1;
                    }

                    // unbalancedChild is the new subtree root.
                    rotate_element->bf = 0;
                    unbalancedChild = rotate_element;
                }
            }

            // The subtree with root unbalancedChild has been rebalanced
            // and is the new subtree of unbalancedParent.  (The
            // original subtree of unbalancedParent had root unbalancedElement.)
            if (unbalancedParent == NULL)
            {
                self->top = unbalancedChild;
                unbalancedChild->up = NULL;
            }
            else if (unbalancedParent->left == unbalancedElement)
            {
                unbalancedParent->left = unbalancedChild;
                unbalancedChild->up = unbalancedParent;
            }
            else // if (unbalancedParent->right == unbalancedElement)
            {
                unbalancedParent->right = unbalancedChild;
                unbalancedChild->up = unbalancedParent;
            }
        }
    }

    return return_element->data;
}


void *
rvBtree_Find(rvBtree *self, const void *object)
// Find the indicated object returning the element in the tree.
// Returns NULL if the object is not found.
{
    rvBtreeElement *element = self->top;

    // Make sure an object was passed in.
    if (object)
    {
        while (element)
        {
            // Compare the object with the current object.
            int compare = self->compareFunc(object, element->data);

            // Determine what to do next.
            if (compare < 0)
            {
                // Look to the left.
                element = element->left;
            }
            else if (compare > 0)
            {
                // Look to the right.
                element = element->right;
            }
            else
            {
                // We found it.  Stop looking.
                break;
            }
        }
    }

    // Return what was found.
    return (element != NULL) ? element->data : NULL;
}


void *
rvBtree_Next(rvBtree *self, const void *object)
// This function returns a pointer to the leftmost element if
// element is NULL, and to the next object to the right otherways.
// If no elements left, returns a pointer to NULL.
{
    rvBtreeElement *element;
    rvBtreeElement *fatherOfElement;
    rvBtreeElement *fatherOfForefather;

    // Initialize the element from the object information.
    element = object ? (rvBtreeElement *) (((char *) object) - self->elementSize) : NULL;

    // Was a element passed in?
    if (!element)
    {
        // Get the top element.
        fatherOfElement = self->top;

        // Make sure there was an element at the top of the tree.
        if (fatherOfElement)
        {
            // Is there still a left element?
            while (fatherOfElement->left)
            {
                // Keep following the left element.
                fatherOfElement = fatherOfElement->left;
            }
        }
    }
    else
    {
        // The element passed in becomes the father element.
        fatherOfElement = element;

        // Is there an element to the right?
        if (fatherOfElement->right)
        {
            // Make the element to the right the fater element.
            fatherOfElement = fatherOfElement->right;

            // Is there an element to the left?
            while (fatherOfElement->left)
            {
                // Yes, seek to the element to the left.
                fatherOfElement = fatherOfElement->left;
            }
        }
        else
        {
            // Remember the parent of this element.
            fatherOfForefather = fatherOfElement->up;

            // Go up an element and to the right.
            while (fatherOfForefather && (fatherOfForefather->right == fatherOfElement))
            {
                // Adjust the current element and it's forefather.
                fatherOfElement = fatherOfForefather;
                fatherOfForefather = fatherOfElement->up;
            }

            // The forefather becomes the element to return.
            fatherOfElement = fatherOfForefather;
        }
    }

    // Return the element data.
    return fatherOfElement ? fatherOfElement->data : NULL;
}


void *
rvBtree_Prev(rvBtree *self, const void *object)
// This function returns a pointer to the rightmost element if
// element is NULL, and to the next object to the left otherwise.
// If no elements left, returns a pointer to NULL.
{
    rvBtreeElement *element;
    rvBtreeElement *fatherOfElement;
    rvBtreeElement *fatherOfForefather;

    // Initialize the element from the object information.
    element = object ? (rvBtreeElement *) (((char *) object) - self->elementSize) : NULL;

    // Was a element passed in?
    if (!element)
    {
        // Get the top element.
        fatherOfElement = self->top;

        // Make sure there was an element at the top of the tree.
        if (fatherOfElement)
        {
            // Is there still a right element?
            while (fatherOfElement->right)
            {
                // Keep following the right element.
                fatherOfElement = fatherOfElement->right;
            }
        }
    }
    else
    {
        // The element passed in becomes the father element.
        fatherOfElement = element;

        // Is there an element to the left?
        if (fatherOfElement->left)
        {
            // Make the element to the left the father element.
            fatherOfElement = fatherOfElement->left;

            // Is there an element to the right?
            while (fatherOfElement->right)
            {
                // Yes, seek to the element to the right.
                fatherOfElement = fatherOfElement->right;
            }
        }
        else
        {
            // Remember the parent of this element.
            fatherOfForefather = fatherOfElement->up;

            // Go up an element and to the left.
            while (fatherOfForefather && (fatherOfForefather->left == fatherOfElement))
            {
                // Adjust the current element and it's forefather.
                fatherOfElement = fatherOfForefather;
                fatherOfForefather = fatherOfElement->up;
            }

            // The forefather becomes the element to return.
            fatherOfElement = fatherOfForefather;
        }
    }

    // Return the element data.
    return fatherOfElement ? fatherOfElement->data : NULL;
}


void *
rvBtree_ElementUpdate(rvBtree *self, const void *object, rvBtree_DeleteFunction objfree)
// Update the rvBtree object.
{
    void *found = NULL;
    rvBtreeElement *element = NULL;

    // Find the item in the tree.
    found = rvBtree_Find(self, object);

    // Was the item found?
    if (found)
    {
        // Initialize the element from the object information.
        element = (rvBtreeElement *) (((char *) object) - self->elementSize);

        // Free the existing object in the element.
        if (objfree) objfree(element->data);

        // Copy the object data into the element.
        memcpy(element->data, object, self->objectSize);
    }

    // Return the element data.
    return (element != NULL) ? element->data : NULL;
}


#ifdef DEBUG
static bool btreeValidateUpError;
static int btreeValidateMaxDiff;
static int btreeValidateMaxDiffLDepth;
static int btreeValidateMaxDiffRDepth;
static int btreeValidateMaxDepth;

static int rvBtree_ValidateDepth(rvBtreeElement *element)
// Calculate the branch depth of the left and right children
// of this element, and then return the longest depth (including ourself).
// Note that this is recursive, so put as little information on the stack as possible.
{
    int l_depth = 0;
    int r_depth = 0;
    register int cur_diff;

    // Did we get a valid element passed in?
    if (element == NULL) return 0;

    // Validate the left subtree.
    if (element->left != NULL)
    {
        // Make sure the left sub-tree points up to us.
        if (element->left->up != element)
        {
            btreeValidateUpError = true;
        }

        // Validate the depth of the left sub-tree.
        l_depth = rvBtree_ValidateDepth(element->left);
    }

    // Validate the right subtree.
    if (element->right != NULL)
    {
        // Make sure the right sub-tree points up to us.
        if (element->right->up != element)
        {
            btreeValidateUpError = true;
        }

        // Validate the depth of the right sub-tree.
        r_depth = rvBtree_ValidateDepth(element->right);
    }

    // Calculate the absolute value of the difference between the depths.
    cur_diff = l_depth - r_depth;
    if (cur_diff < 0) cur_diff = -cur_diff;

    // Is this the biggest difference we've seen so far?
    if (cur_diff > btreeValidateMaxDiff)
    {
        // This is the biggest difference.  Save each side.
        btreeValidateMaxDiff = cur_diff;
        btreeValidateMaxDiffLDepth = l_depth;
        btreeValidateMaxDiffRDepth = r_depth;
    }

    // Return longest depth + 1.
    return MAXIMUM(l_depth, r_depth) + 1;
}


bool rvBtree_Validate(rvBtree *self, bool print_error)
// Validate that the tree is balanced.
{
#define VALID_DEPTH_DIFF    1   /* It's ok if one side is "1" longer than the other */

    bool rv = true;

    // Assume all is OK.
    btreeValidateUpError = false;
    btreeValidateMaxDiff = 0;
    btreeValidateMaxDiffLDepth = 0;
    btreeValidateMaxDiffRDepth = 0;

    // Validate the depth of the tree.
    btreeValidateMaxDepth = rvBtree_ValidateDepth(self->top);

    // Print largest discrepency, if there was a problem.
    if (btreeValidateMaxDiff > VALID_DEPTH_DIFF)
    {
        // Log the error.
        if (print_error)
        {
            printf("Bad rvBtree depth (max diff): l_depth=%d, r_depth=%d\n",
                   btreeValidateMaxDiffLDepth, btreeValidateMaxDiffRDepth);
        }

        // All is *not* OK.
        rv = false;
    }

    // Were all of the "up" pointers valid?
    if (btreeValidateUpError)
    {
        // Log the error.
        if (print_error)
        {
            printf("An 'up' pointer is incorrect in the tree.\n");
        }

        // All is *not* OK.
        rv = false;
    }

    return rv;
}
#endif
