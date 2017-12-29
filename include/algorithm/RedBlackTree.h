#ifndef _ALGORITHM_REDBLACKTREE_H_
#define _ALGORITHM_REDBLACKTREE_H_

#include <stdexcept>
#include <vector>

#include "base/EventEmitter.h"

#ifndef NULL
#define NULL 0
#endif

namespace xchange {

namespace algorithm {

#define PARENT(x) (x->parent)
#define GRAND(x) (x->parent->parent)
#define UNCLE_LEFT(x) (x->parent->parent->left)
#define UNCLE_RIGHT(x) (x->parent->parent->right)
#define SIBLING_LEFT(x) (x->parent->left)
#define SIBLING_RIGHT(x) (x->parent->right)
#define SON_LEFT(x) (x->left)
#define SON_RIGHT(x) (x->right)

    template<typename Key, typename Value>
    class RedBlackTree {
        public:
            enum Color {RED = 0, BLACK};

            explicit RedBlackTree(): root_(NULL), length_(0) {}
            ~RedBlackTree() {
                eachNode_([](Node *node)->void {
                    delete node;
                });
            }

            struct Node {
                Node(Key k, Value v, Color c = RED)
                    : key(k), value(v), color(c),
                    left(NULL), right(NULL), parent(NULL) {};
                ~Node() {};

                Key key;
                Value value;
                Color color;

                // don't assign color
                Node& operator=(const Node& old) {
                    key = old.key;
                    value = old.value;

                    return *this;
                }

                Node *left, *right, *parent;
            };

            uint32_t size() const {return length_;}

            void each(void (*fn)(Value)) {
                std::vector<Node *> stack;
                Node *current = root_;

                while (stack.size() || current) {
                    if (current) {
                        stack.push_back(current);
                        current = current->left;
                    } else {
                        current = stack.back();
                        stack.pop_back();

                        fn(current->value);

                        current = current->right;
                    }
                }
            }

            int insert(Key key, Value val) {
                Node * newNode = new Node(key, val);
                // y: record last postion of x
                Node * x = root_, *y = NULL;

                while (x != NULL) {
                    y = x;
                    if (key < x->key) {
                        x = x->left;
                    } else if (key > x->key){
                        x = x->right;
                    } else {
                        // insert same key will overwrite the old value
                        x->value = val;
                        length_++;

                        return 0;
                    }
                }

                if (y == NULL) {
                    root_ = newNode;
                    root_->color = BLACK;
                    length_++;

                    return 0;
                } else {
                    if (key < y->key) {
                        // insert as left child
                        y->left = newNode;
                    } else if (key > y->key){
                        // insert as right child
                        y->right = newNode;
                    }
                }

                newNode->parent = y;

                insertFixup_(newNode);
                length_++;

                return 0;
            };

            int remove(Key key) {
                Node *target = find_(key);
                // x can be the left or right child of 'needToDelete'
                Node *needToDelete = NULL, *x = NULL;

                if (target == NULL) {
                    return -1;
                }

                // confirm which to delete
                if (target->left == NULL || target->right == NULL) {
                    // case 1: target has no child
                    needToDelete = target;
                } else {
                    needToDelete = getSuccessor(target);
                }

                if (needToDelete->left != NULL) {
                    x = needToDelete->left;
                } else {
                    x = needToDelete->right;
                }
                if (x != NULL) {
                    PARENT(x) = PARENT(needToDelete);
                }

                if (PARENT(needToDelete) == NULL) {
                    root_ = x;
                } else {
                    if (PARENT(needToDelete)->left == needToDelete) {
                        PARENT(needToDelete)->left = x;
                    } else {
                        PARENT(needToDelete)->right = x;
                    }
                }

                if (needToDelete != target) {
                    *target = *needToDelete;
                }

                if (needToDelete->color == BLACK && x != NULL) {
                    removeFixup_(x);
                }

                delete needToDelete;
                length_--;
                return 0;
            };

            Value find(Key key) {
                Node *result = find_(key);

                if (result == NULL) {
                    throw std::out_of_range("Key not found");
                }

                return result->value;
            }
        private:
            Node *root_;
            uint32_t length_;

            void eachNode_(void (*fn)(Node *)) {
                std::vector<Node *> stack;
                Node *current = root_;

                while (stack.size() || current) {
                    if (current) {
                        stack.push_back(current);
                        current = current->left;
                    } else {
                        current = stack.back();
                        stack.pop_back();

                        fn(current);

                        current = current->right;
                    }
                }
            }

            Node *find_(Key key) {
                Node *current = root_;

                while (current != NULL && current->key != key) {
                    if (key < current->key) {
                        current = current->left;
                    } else if (key > current->key) {
                        current = current->right;
                    }
                }

                return current;
            }

            Node *getSuccessor(Node *current) {
                if (current == NULL) return NULL;

                if (current->right != NULL) {
                    current = current->right;

                    while (current->left != NULL) {
                        current = current->left;
                    }

                    return current;
                } else if (PARENT(current)) {
                    if (SIBLING_LEFT(current) == current) {
                        return PARENT(current);
                    } else {
                        while (PARENT(current) && SIBLING_RIGHT(current) == current) {
                            current = PARENT(current);
                        }

                        return current;
                    }
                }

                return NULL;
            }

            void rotateLeft_(Node *center) {
                Node *rightNode = SON_RIGHT(center);

                SON_RIGHT(center) = SON_LEFT(rightNode);
                if (SON_LEFT(rightNode)) {
                    PARENT(SON_LEFT(rightNode)) = center;
                }

                PARENT(rightNode) = PARENT(center);
                if (center == root_) {
                    root_ = rightNode;
                } else {
                    if (SON_LEFT(PARENT(center)) == center) {
                        SON_LEFT(PARENT(center)) = rightNode;
                    } else {
                        SON_RIGHT(PARENT(center)) = rightNode;
                    }
                }

                SON_LEFT(rightNode) = center;
                PARENT(center) = rightNode;
            }

            void rotateRight_(Node *center) {
                Node *leftNode = SON_LEFT(center);

                SON_LEFT(center) = SON_RIGHT(leftNode);
                if (SON_RIGHT(leftNode)) {
                    PARENT(SON_RIGHT(leftNode)) = center;
                }

                PARENT(leftNode) = PARENT(center);
                if (center == root_) {
                    root_ = leftNode;
                } else {
                    if (SON_RIGHT(PARENT(center)) == center) {
                        SON_RIGHT(PARENT(center)) = leftNode;
                    } else {
                        SON_LEFT(PARENT(center)) = leftNode;
                    }
                }

                SON_RIGHT(leftNode) = center;
                PARENT(center) = leftNode;
            }

            void insertFixup_(Node *current) {
                while (current != root_ && PARENT(current)->color == RED) {
                    if (UNCLE_LEFT(current) == PARENT(current)) {
                        // parent is left child of grandparent

                        // case 1: color of uncle is red
                        if (UNCLE_RIGHT(current) && UNCLE_RIGHT(current)->color == RED) {
                            PARENT(current)->color = BLACK;
                            UNCLE_RIGHT(current)->color = BLACK;
                            GRAND(current)->color = RED;

                            current = GRAND(current);
                        } else {
                            // color of uncle is black

                            if (current == SIBLING_RIGHT(current)) {
                                // case 2: if current Node is right child of parent
                                // turn it to the case 3
                                current = PARENT(current);
                                rotateLeft_(current);
                            }

                            // case 3: current Node is right child of parent
                            PARENT(current)->color = BLACK;
                            GRAND(current)->color = RED;
                            rotateRight_(GRAND(current));
                        }
                    } else {
                        // parent is right child of grandparent
                        // mirror of upper code(right to left, left to right)
                        if (UNCLE_LEFT(current) && UNCLE_LEFT(current)->color == RED) {
                            PARENT(current)->color = BLACK;
                            UNCLE_LEFT(current)->color = BLACK;
                            GRAND(current)->color = RED;

                            current = GRAND(current);
                        } else {
                            if (current == SIBLING_LEFT(current)) {
                                current = PARENT(current);
                                rotateRight_(current);
                            }
                            PARENT(current)->color = BLACK;
                            GRAND(current)->color = RED;
                            rotateLeft_(GRAND(current));
                        }
                    }
                }

                root_->color = BLACK;
            }

            void removeFixup_(Node *current) {
                while (current != root_) {
                    if (SIBLING_LEFT(current) == current) {
                        // current is the left child of parent

                        // case 1: turn it to case 2,3,4
                        if (SIBLING_RIGHT(current)->color == RED) {
                            SIBLING_RIGHT(current)->color = BLACK;
                            PARENT(current)->color = RED;
                            rotateLeft_(PARENT(current));
                        }

                        // case 2:
                        if (SON_LEFT(SIBLING_RIGHT(current))->color == BLACK &&
                                SON_RIGHT(SIBLING_RIGHT(current))->color == BLACK) {
                            SIBLING_RIGHT(current)->color = RED;
                            current = PARENT(current);
                        } else {
                            // case 3: turn it to case 4
                            if (SON_RIGHT(SIBLING_RIGHT(current))->color != RED) {
                                SON_LEFT(SIBLING_RIGHT(current))->color = BLACK;
                                SIBLING_RIGHT(current)->color = RED;
                                rotateRight_(SIBLING_RIGHT(current));
                            }

                            // case 4:
                            SIBLING_RIGHT(current) = PARENT(current);
                            PARENT(current)->color = BLACK;
                            SON_RIGHT(SIBLING_RIGHT(current))->color = BLACK;
                            rotateLeft_(PARENT(current));

                            break;
                        }
                    } else {
                        // current is the right child of parent
                        if (SIBLING_LEFT(current)->color == RED) {
                            SIBLING_LEFT(current)->color = BLACK;
                            PARENT(current)->color = RED;
                            rotateRight_(PARENT(current));
                        }

                        if (SON_RIGHT(SIBLING_LEFT(current))->color == BLACK &&
                                SON_LEFT(SIBLING_LEFT(current))->color == BLACK) {
                            SIBLING_LEFT(current)->color = RED;
                            current = PARENT(current);
                        } else {
                            if (SON_LEFT(SIBLING_LEFT(current))->color != RED) {
                                SON_RIGHT(SIBLING_LEFT(current))->color = BLACK;
                                SIBLING_LEFT(current)->color = RED;
                                rotateLeft_(SIBLING_LEFT(current));
                            }

                            SIBLING_LEFT(current) = PARENT(current);
                            PARENT(current)->color = BLACK;
                            SON_LEFT(SIBLING_LEFT(current))->color = BLACK;
                            rotateRight_(PARENT(current));

                            break;
                        }
                    }
                }

                root_->color = BLACK;
            }

   };

#undef PARENT
#undef GRAND
#undef UNCLE_LEFT
#undef UNCLE_RIGHT
#undef SIBLING_LEFT
#undef SIBLING_RIGHT
#undef SON_LEFT
#undef SON_RIGHT
}

}

#endif
