/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#include "oscl_tree.h"

OSCL_EXPORT_REF void Oscl_Rb_Tree_Base::rotate_left(base_link_type x, base_link_type& root)
{
    base_link_type y = x->right;
    x->right = y->left;
    if (y->left != 0)
        y->left->parent = x;
    y->parent = x->parent;

    if (x == root)
        root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;
}

OSCL_EXPORT_REF void Oscl_Rb_Tree_Base::rotate_right(base_link_type x, base_link_type& root)
{
    base_link_type y = x->left;
    x->left = y->right;
    if (y->right != 0)
        y->right->parent = x;
    y->parent = x->parent;

    if (x == root)
        root = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;
    y->right = x;
    x->parent = y;
}

OSCL_EXPORT_REF void Oscl_Rb_Tree_Base::rebalance(base_link_type x, base_link_type& root)
{
    x->color = Oscl_Rb_Tree_Node_Base::red;
    while (x != root && x->parent->color == Oscl_Rb_Tree_Node_Base::red)
    {
        if (x->parent == x->parent->parent->left)
        {
            base_link_type y = x->parent->parent->right;
            if (y && y->color == Oscl_Rb_Tree_Node_Base::red)
            {
                x->parent->color = Oscl_Rb_Tree_Node_Base::black;
                y->color = Oscl_Rb_Tree_Node_Base::black;
                x->parent->parent->color = Oscl_Rb_Tree_Node_Base::red;
                x = x->parent->parent;
            }
            else
            {
                if (x == x->parent->right)
                {
                    x = x->parent;
                    rotate_left(x, root);
                }
                x->parent->color = Oscl_Rb_Tree_Node_Base::black;
                x->parent->parent->color = Oscl_Rb_Tree_Node_Base::red;
                rotate_right(x->parent->parent, root);
            }
        }
        else
        {
            base_link_type y = x->parent->parent->left;
            if (y && y->color == Oscl_Rb_Tree_Node_Base::red)
            {
                x->parent->color = Oscl_Rb_Tree_Node_Base::black;
                y->color = Oscl_Rb_Tree_Node_Base::black;
                x->parent->parent->color = Oscl_Rb_Tree_Node_Base::red;
                x = x->parent->parent;
            }
            else
            {
                if (x == x->parent->left)
                {
                    x = x->parent;
                    rotate_right(x, root);
                }
                x->parent->color = Oscl_Rb_Tree_Node_Base::black;
                x->parent->parent->color = Oscl_Rb_Tree_Node_Base::red;
                rotate_left(x->parent->parent, root);
            }
        }
    }
    root->color = Oscl_Rb_Tree_Node_Base::black;
}

OSCL_EXPORT_REF Oscl_Rb_Tree_Base::base_link_type Oscl_Rb_Tree_Base::rebalance_for_erase(base_link_type z,
        base_link_type& root,
        base_link_type& leftmost,
        base_link_type& rightmost)
{
    base_link_type y = z;
    base_link_type x = 0;
    base_link_type x_parent = 0;

    if (y->left == 0)             // z has at most one non-null child. y == z.
        x = y->right;               // x might be null.
    else
    {
        if (y->right == 0)          // z has exactly one non-null child.  y == z.
            x = y->left;              // x is not null.
        else                        // z has two non-null children.  Set y to
        {
            y = y->right;             //   z's successor.  x might be null.
            while (y->left != 0)
                y = y->left;
            x = y->right;
        }
    }
    if (y != z)                   // relink y in place of z.  y is z's successor
    {
        z->left->parent = y;
        y->left = z->left;
        if (y != z->right)
        {
            x_parent = y->parent;
            if (x) x->parent = y->parent;
            y->parent->left = x;      // y must be a left child
            y->right = z->right;
            z->right->parent = y;
        }
        else
            x_parent = y;
        if (root == z)
            root = y;
        else if (z->parent->left == z)
            z->parent->left = y;
        else
            z->parent->right = y;
        y->parent = z->parent;
        //            swap(y->color, z->color);
        Oscl_Rb_Tree_Node_Base::color_type tmp_color = y->color;
        y->color = z->color;
        z->color = tmp_color;
        y = z;
        // y now points to node to be actually deleted
    }
    else                          // y == z
    {
        x_parent = y->parent;
        if (x) x->parent = y->parent;
        if (root == z)
            root = x;
        else
        {
            if (z->parent->left == z)
                z->parent->left = x;
            else
                z->parent->right = x;
        }
        if (leftmost == z)
        {
            if (z->right == 0)        // z->left must be null also
                leftmost = z->parent;
            // makes leftmost == header if z == root
            else
                leftmost = Oscl_Rb_Tree_Node_Base::minimum(x);
        }
        if (rightmost == z)
        {
            if (z->left == 0)         // z->right must be null also
                rightmost = z->parent;
            // makes rightmost == header if z == root
            else                      // x == z->left
                rightmost = Oscl_Rb_Tree_Node_Base::maximum(x);
        }
    }
    if (y->color != Oscl_Rb_Tree_Node_Base::red)
    {
        while (x != root && (x == 0 || x->color == Oscl_Rb_Tree_Node_Base::black))
        {
            if (x == x_parent->left)
            {
                base_link_type w = x_parent->right;
                if (w->color == Oscl_Rb_Tree_Node_Base::red)
                {
                    w->color = Oscl_Rb_Tree_Node_Base::black;
                    x_parent->color = Oscl_Rb_Tree_Node_Base::red;
                    rotate_left(x_parent, root);
                    w = x_parent->right;
                }
                if ((w->left == 0 || w->left->color == Oscl_Rb_Tree_Node_Base::black) &&
                        (w->right == 0 || w->right->color == Oscl_Rb_Tree_Node_Base::black))
                {
                    w->color = Oscl_Rb_Tree_Node_Base::red;
                    x = x_parent;
                    x_parent = x_parent->parent;
                }
                else
                {
                    if (w->right == 0 || w->right->color == Oscl_Rb_Tree_Node_Base::black)
                    {
                        if (w->left) w->left->color = Oscl_Rb_Tree_Node_Base::black;
                        w->color = Oscl_Rb_Tree_Node_Base::red;
                        rotate_right(w, root);
                        w = x_parent->right;
                    }
                    w->color = x_parent->color;
                    x_parent->color = Oscl_Rb_Tree_Node_Base::black;
                    if (w->right) w->right->color = Oscl_Rb_Tree_Node_Base::black;
                    rotate_left(x_parent, root);
                    break;
                }
            }
            else                    // same as above, with right <-> left.
            {
                base_link_type w = x_parent->left;
                if (w->color == Oscl_Rb_Tree_Node_Base::red)
                {
                    w->color = Oscl_Rb_Tree_Node_Base::black;
                    x_parent->color = Oscl_Rb_Tree_Node_Base::red;
                    rotate_right(x_parent, root);
                    w = x_parent->left;
                }
                if ((w->right == 0 || w->right->color == Oscl_Rb_Tree_Node_Base::black) &&
                        (w->left == 0 || w->left->color == Oscl_Rb_Tree_Node_Base::black))
                {
                    w->color = Oscl_Rb_Tree_Node_Base::red;
                    x = x_parent;
                    x_parent = x_parent->parent;
                }
                else
                {
                    if (w->left == 0 || w->left->color == Oscl_Rb_Tree_Node_Base::black)
                    {
                        if (w->right) w->right->color = Oscl_Rb_Tree_Node_Base::black;
                        w->color = Oscl_Rb_Tree_Node_Base::red;
                        rotate_left(w, root);
                        w = x_parent->left;
                    }
                    w->color = x_parent->color;
                    x_parent->color = Oscl_Rb_Tree_Node_Base::black;
                    if (w->left) w->left->color = Oscl_Rb_Tree_Node_Base::black;
                    rotate_right(x_parent, root);
                    break;
                }
            }
        }
        if (x) x->color = Oscl_Rb_Tree_Node_Base::black;
    }
    return y;
}
