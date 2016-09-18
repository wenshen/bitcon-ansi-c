/*
 * AssociativeArray.c
 *
 * Created on: Nov 26, 2012
 * Created by: wenshen
 * Copyright (c) 2012 MIBitcoin Project Team
 */

#include "AssociativeArray.h"

FindResult associativeArrayFind(AssociativeArray * self, uint8_t * key){
	FindResult result;
	BTreeNode * node = self->root;
	for (;;) {
		/* Do binary search on node*/
		result = bTreeNodeBinarySearch(node, key, self->keySize);
		if (result.found){
			/* Found the data on this node.*/
			result.node = node;
			return result;
		}else{
			/* We have not found the data on this node, if there is a child go to the child.*/
			if (node->children[result.pos])
				/* Child exists, move to it*/
				node = node->children[result.pos];
			else{
				/* The child doesn't exist. Return as not found with position for insertion.*/
				result.node = node;
				return result;
			}
		}
	}
}
bool associativeArrayDelete(AssociativeArray * self, FindResult pos){
	if (! pos.node->children[0]) {
		/* Leaf*/
		BTreeNode * parent = pos.node->parent;
		if (pos.node->numElements > BTREE_HALF_ORDER || ! parent) {
			/* Can simply remove this element. Nice and easy.*/
			if (--pos.node->numElements > pos.pos){
				/* Move everything down to overwrite the removed element.*/
				memmove(pos.node->children + pos.pos, pos.node->children + pos.pos + 1, (pos.node->numElements - pos.pos + 1) * sizeof(*pos.node->children));
				memmove(pos.node->elements + pos.pos, pos.node->elements + pos.pos + 1, (pos.node->numElements - pos.pos) * sizeof(*pos.node->elements));
			}
			return TRUE;
		}else{
			/* Underflow... Where things get complicated. Loop through merges all the way to the root*/
			for (;;){
				/* We have a parent so we can check siblings.*/
				/* First find the position of this node in the parent. ??? Worth additional pointers?*/
				/* Search for second key in case the first has been taken by the parent.*/
				FindResult res = bTreeNodeBinarySearch(parent, *(pos.node->elements + 1), self->keySize);
				BTreeNode * left;
				BTreeNode * right;
				if (res.pos) {
					/* Create left sibling data*/
					left = parent->children[res.pos-1];
					/* Can check left sibling*/
					if (left->numElements > BTREE_HALF_ORDER) {
						/* Can take from left*/
						/* Move elements up to overwrite the element we are deleting*/
						if (pos.pos) { /* o 0 ii 1 iii*/
							memmove(pos.node->elements + 1, pos.node->elements, pos.pos * sizeof(*pos.node->elements));
							memmove(pos.node->children + 1, pos.node->children, (pos.pos + 1) * sizeof(*pos.node->children));
						}
						/* Lower left parent element to this node*/
						pos.node->elements[0] = parent->elements[res.pos-1];
						/* Now move left sibling's far right element to parent*/
						parent->elements[res.pos-1] = left->elements[left->numElements-1];
						/* Make left sibling's far right child the far left child of this node*/
						pos.node->children[0] = left->children[left->numElements];
						/* Make this child's parent this node*/
						if (pos.node->children[0])
							((BTreeNode *)(pos.node->children[0]))->parent = pos.node;
						/* Now remove left sibling's far right element and be done*/
						left->numElements--;
						return TRUE;
					}
				}
				if (res.pos < parent->numElements) {
					/* Create right sibling data*/
					right = parent->children[res.pos+1];
					/* Can check right sibling*/
					if (right->numElements > BTREE_HALF_ORDER) {
						/* Can take from right*/
						/* Move elements down to overwrite the element we are deleting*/
						if (pos.node->numElements > pos.pos + 1) {
							memmove(pos.node->elements + pos.pos, pos.node->elements + pos.pos + 1, (pos.node->numElements - pos.pos - 1) * sizeof(*pos.node->elements));
							memmove(pos.node->children + pos.pos, pos.node->children + (pos.pos + 1), (pos.node->numElements - pos.pos) * sizeof(*pos.node->children));
						}
						/* Lower right parent element to this node*/
						pos.node->elements[pos.node->numElements-1] = parent->elements[res.pos];
						/* Now move right sibling's far left element to parent*/
						parent->elements[res.pos] = right->elements[0];
						/* Make right sibling's far left child the far right child of this node*/
						pos.node->children[pos.node->numElements] = right->children[0];
						/* Make this child's parent this node*/
						if (pos.node->children[0])
							((BTreeNode *)(pos.node->children[pos.node->numElements]))->parent = pos.node;
						/* Now remove right sibling's far left element and be done*/
						memmove(right->children, right->children + 1, right->numElements * sizeof(*right->children));
						right->numElements--;
						memmove(right->elements, right->elements + 1, right->numElements * sizeof(*right->elements));
						return TRUE;
					}
				}
				/* Take away element num.*/
				pos.node->numElements--;
				/* Could not take from siblings... now for the merging.*/
				if (! res.pos) {
					/* We are merging right sibling into the node.*/
					/* First move data down to overwrite deleted data o 0 i 1 ii*/
					if (pos.node->numElements > pos.pos)
						memmove(pos.node->elements + pos.pos, pos.node->elements + pos.pos + 1, (pos.node->numElements - pos.pos) * sizeof(*pos.node->elements));
					memmove(pos.node->children + pos.pos, pos.node->children + pos.pos + 1, (pos.node->numElements + 1) * sizeof(*pos.node->children));
					/* Add parents mid value*/
					pos.node->elements[pos.node->numElements] = parent->elements[res.pos];
					/* Now merge right sibling*/
					pos.node->numElements++;
					memmove(pos.node->elements + pos.node->numElements, right->elements, right->numElements * sizeof(*pos.node->elements));
					memmove(pos.node->children + pos.node->numElements, right->children, (right->numElements + 1) * sizeof(*right->children));
					/* Change children parent pointers*/
					if (pos.node->children[0]){
						uint8_t x;
						for ( x = BTREE_HALF_ORDER; x < BTREE_ORDER + 1; x++) {
							((BTreeNode *)(pos.node->children[x]))->parent = pos.node;
						}
					}
					/* Adjust number of elements*/
					pos.node->numElements += right->numElements;
					/* Free right node*/
					free(right);
					/* Move node over to where the right sibling was so deleted part is on the left. ??? Change algorithm to merge differently.*/
					parent->children[res.pos+1] = parent->children[res.pos];
				}else{
					/* We are merging the node into the left sibling.*/
					/* Move over parent middle element*/
					left->elements[left->numElements] = parent->elements[res.pos-1];
					/* Move over elements before deleted element.*/
					left->numElements++;
					if (pos.pos){
						memcpy(left->elements + left->numElements, pos.node->elements, pos.pos * sizeof(*left->elements));
						memcpy(left->children + left->numElements, pos.node->children, pos.pos * sizeof(*pos.node->children));
					}
					left->numElements += pos.pos;
					/* Move over elements after the deleted element to the left node*/
					if (pos.node->numElements > pos.pos)
						memcpy(left->elements + left->numElements, pos.node->elements + pos.pos + 1, (pos.node->numElements - pos.pos) * sizeof(*left->elements));
					memcpy(left->children + left->numElements, pos.node->children + pos.pos + 1, (pos.node->numElements - pos.pos + 1) * sizeof(*pos.node->children));
					/* Change children parent pointers*/
					if (left->children[0]){
						uint8_t x;
						for ( x = BTREE_HALF_ORDER; x < BTREE_ORDER + 1; x++) {
							((BTreeNode *)(left->children[x]))->parent = left;
						}
					}
					/* Adjust number of elements*/
					left->numElements += pos.node->numElements - pos.pos;
					/* Free the node*/
					free(pos.node);
					/* Move left sibling over to where the node was so deleted part is on the left.*/
					parent->children[res.pos] = parent->children[res.pos-1];
				}
				/* If position is 0 then we are deleting to the right, else to the left.*/
				pos.pos = res.pos ? res.pos - 1 : res.pos;
				/* Now delete the parent element and continue merge upto root if neccesary...*/
				if ((! parent->parent && parent->numElements > 1)
					|| (parent->parent && parent->numElements > BTREE_HALF_ORDER)) {
					/* Either a root and with more than one element to still remove or not root and more than half the minimum elements.*/
					/* Therefore remove the element and be done.*/
					parent->numElements--;
					if (parent->numElements > pos.pos) {
						memmove(parent->elements + pos.pos, parent->elements + pos.pos + 1, (parent->numElements - pos.pos) * sizeof(*parent->elements));
						memmove(parent->children + pos.pos + 1, parent->children + pos.pos + 2, (parent->numElements - pos.pos) * sizeof(*parent->children));
					}
					return TRUE;
				}else if (! parent->parent){
					/* The parent is the root and has 1 element. The root is now empty so we make it's newly merged children the root.*/
					self->root = res.pos ? left : pos.node;
					self->root->parent = NULL;
					free(parent);
					return TRUE;
				}else{
					/* The parent is not root and has the minimum allowed elements. Therefore we need to merge again, going around the loop.*/
					pos.node = parent;
					parent = pos.node->parent;
				}
			}
		}
	}else{
		/* Not leaf data, insert successor then delete successor from/in the right child.*/
		BTreeNode * child = pos.node->children[pos.pos + 1];
		/* Now we have the right child, find left most element in the child.*/
		while (child->children[0])
			child = child->children[0];
		/* Get left-most element and overwrite the element we are deleting with this left-most element.*/
		pos.node->elements[pos.pos] = child->elements[0];
		/* Now delete the successive element.*/
		pos.node = child;
		pos.pos = 0;
		return associativeArrayDelete(self, pos);
	}
}
bool AssociativeArrayInsert(AssociativeArray * self, uint8_t * keyValue, FindResult pos, BTreeNode * right){
	/* See if we can insert data in this node*/
	if (pos.node->numElements < BTREE_ORDER) {
		/* Yes we can, do that.*/
		if (pos.node->numElements > pos.pos){
			/* Move up the keys, data and children*/
			/* . 0 . x . 1 . 2 . 3 .*/
			memmove(pos.node->elements + pos.pos + 1, pos.node->elements + pos.pos, (pos.node->numElements - pos.pos) * sizeof(*pos.node->elements));
			memmove(pos.node->children + pos.pos + 2, pos.node->children + pos.pos + 1, (pos.node->numElements - pos.pos) *  sizeof(*pos.node->children));
		}
		/* Copy over new key, data and children*/
		pos.node->elements[pos.pos] = keyValue;
		pos.node->children[pos.pos + 1] = right;
		/* Increase number of elements*/
		pos.node->numElements++;
		/* The element has been added, we can stop here.*/
		return TRUE;
	}else{
		/* Nope, we need to split this node into two.*/
		BTreeNode * new = malloc(sizeof(*new));
		if (! new)
			return FALSE;
		/* Make both sides have half the order.*/
		new->numElements = BTREE_HALF_ORDER;
		pos.node->numElements = BTREE_HALF_ORDER;
		uint8_t * midKeyValue;
		if (pos.pos >= BTREE_HALF_ORDER) {
			/* Not in first child.*/
			if (pos.pos == BTREE_HALF_ORDER) {
				/*
				// New key is median. Visualisation of median insertion:
				//
				//      . C  .  G .
				//     /     |     \
				//  .A.B.  .D.F.  .H.I.
				//
				// . C . E . G . = Too many when order is 2, therefore split again
				//    /     \
				//  .D.     .F.
				//
				//         . - E - .
				//        /         \
				//      .C.         .G.
				//     /   \       /   \
				//  .A.B.  .D.   .F.  .H.I.
				//
				 * */
				memcpy(new->elements, pos.node->elements + BTREE_HALF_ORDER, BTREE_HALF_ORDER * sizeof(*new->elements));
				/* Copy children*/
				memcpy(new->children + 1, pos.node->children + BTREE_HALF_ORDER + 1, BTREE_HALF_ORDER * sizeof(*new->children));
				/* First child in right of the split becomes the right input node*/
				new->children[0] = right;
				/* Middle value is the inserted value*/
				midKeyValue = keyValue;
			}else{
				/*
				// In new child. Visualisation of order of 4:
				//
				//        . - E - . -- J -- . -- O -- . - T - .
				//       /        |         |         |        \
				//  .A.B.C.D. .F.G.H.I. .K.L.M.N. .P.Q.R.S. .U.V.W.X.
				//
				//  Add Y
				//
				//        . - E - . -- J -- . -- O -- . - T - .
				//       /        |         |         |        \
				//  .A.B.C.D. .F.G.H.I. .K.L.M.N. .P.Q.R.S. .U.V.W.X.Y. == Too many
				//
				//        . - E - . -- J -- . -- O -- . - T - . W .     == Too many
				//       /        |         |         |       |    \
				//  .A.B.C.D. .F.G.H.I. .K.L.M.N. .P.Q.R.S. .U.V. .X.Y.
				//
				//                  . ---------- O -------- .
				//                 /                         \
				//        . - E - . -- J -- .         . - T - . W .
				//       /        |         |         |       |    \
				//  .A.B.C.D. .F.G.H.I. .K.L.M.N. .P.Q.R.S. .U.V. .X.Y.
				//
				//
				//
				// Copy over first part of the new child
				 * */

				if (pos.pos > BTREE_HALF_ORDER + 1)
					memcpy(new->elements, pos.node->elements + BTREE_HALF_ORDER + 1, (pos.pos - BTREE_HALF_ORDER - 1) * sizeof(*new->elements));
				/* Copy in data*/
				new->elements[pos.pos - BTREE_HALF_ORDER - 1] = keyValue;
				/* Copy over the last part of the new child. It seems so simple to visualise but coding this is very hard...*/
				memcpy(new->elements + pos.pos - BTREE_HALF_ORDER, pos.node->elements + pos.pos, (BTREE_ORDER - pos.pos) * sizeof(*new->elements));
				/* Copy children (Can be the confusing part) o 0 i 1 ii 3 iii 4 iv*/
				memcpy(new->children, pos.node->children + BTREE_HALF_ORDER + 1, (pos.pos - BTREE_HALF_ORDER) * sizeof(*new->children)); /* Includes left as previously*/
				new->children[pos.pos - BTREE_HALF_ORDER] = right;
				/* Rest of the children.*/
				if (BTREE_ORDER > pos.pos)
					memcpy(new->children + pos.pos - BTREE_HALF_ORDER + 1, pos.node->children + pos.pos + 1, (BTREE_ORDER - pos.pos) * sizeof(*new->children));
				/* Middle value*/
				midKeyValue = pos.node->elements[BTREE_HALF_ORDER];
			}
		}else{
			/* In first child. This is the (supposedly) easy one.*/
			/* a 0 b 1 c 2 d 3 e*/
			/* a 0 b I r  c 2 d 3 e*/
			/* Copy data to new child*/
			memcpy(new->elements, pos.node->elements + BTREE_HALF_ORDER, BTREE_HALF_ORDER * sizeof(*new->elements));
			/* Insert key and data*/
			memmove(pos.node->elements + pos.pos + 1, pos.node->elements + pos.pos, (BTREE_HALF_ORDER - pos.pos) * sizeof(*pos.node->elements));
			pos.node->elements[pos.pos] = keyValue;
			/* Children...*/
			memcpy(new->children, pos.node->children + BTREE_HALF_ORDER, (BTREE_HALF_ORDER + 1) * sizeof(*new->children)); /* OK*/
			if (BTREE_HALF_ORDER > 1 + pos.pos)
				memmove(pos.node->children + pos.pos + 2, pos.node->children + pos.pos + 1, (BTREE_HALF_ORDER - pos.pos - 1) * sizeof(*new->children));
			/* Insert right to inserted area*/
			pos.node->children[pos.pos + 1] = right; /* OK*/
			/* Middle value*/
			midKeyValue = pos.node->elements[BTREE_HALF_ORDER];
		}
		/* Make the new node's children's parents reflect this new node*/
		if (new->children[0]){
			uint8_t x;
			for ( x = 0; x < BTREE_HALF_ORDER + 1; x++) {
				BTreeNode * child = new->children[x];
				child->parent = new;
			}
		}
		/* Move middle value to parent, if parent does not exist (ie. root) create new root.*/
		if (! pos.node->parent) {
			/* Create new root*/
			self->root = malloc(sizeof(*self->root));
			if (! self->root)
				return FALSE;
			self->root->numElements = 0;
			self->root->parent = NULL;
			pos.node->parent = self->root;
			self->root->children[0] = pos.node; /* Make left the left split node.*/
		}
		/* New node requires that the parent be set*/
		new->parent = pos.node->parent;
		/* Find position of value in parent and then insert.*/
		FindResult res = bTreeNodeBinarySearch(pos.node->parent, midKeyValue, self->keySize);
		res.node = pos.node->parent;
		return AssociativeArrayInsert(self, midKeyValue, res, new);
	}
}
FindResult bTreeNodeBinarySearch(BTreeNode * self, uint8_t * key, uint8_t keySize){
	FindResult res;
	res.found = FALSE;
	if (! self->numElements) {
		res.pos = 0;
		return res;
	}
	uint8_t left = 0;
	uint8_t right = self->numElements - 1;
	int cmp;
	while (left <= right) {
		res.pos = (right+left)/2;
		cmp = memcmp(key, self->elements[res.pos], keySize);
		if (cmp == 0) {
			res.found = TRUE;
			break;
		}else if (cmp < 0){
			if (! res.pos)
				break;
			right = res.pos - 1;
		}else
			left = res.pos + 1;
	}
	if (cmp > 0)
		res.pos++;
	return res;
}
void FreeAssociativeArray(AssociativeArray * self){
	FreeBTreeNode(self->root);
}
void FreeBTreeNode(BTreeNode * self){
	uint8_t x;
	for ( x = 0; x < self->numElements + 1; x++)
		if (self->children[x])
			FreeBTreeNode(self->children[x]);
	free(self);
}
bool InitAssociativeArray(AssociativeArray * self, uint8_t keySize){
	self->keySize = keySize;
	self->root = malloc(sizeof(*self->root));
	if (! self->root)
		return FALSE;
	self->root->parent = NULL;
	self->root->numElements = 0;
	uint8_t x;
	for ( x = 0; x < BTREE_ORDER + 1; x++)
		self->root->children[x] = NULL;
	return TRUE;
}
