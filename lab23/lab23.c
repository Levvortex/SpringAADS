#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node *left;
    struct Node *right;
} Node;

Node* insert(Node* root, int data) {
    if (root == NULL) {
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = data;
        newNode->left = NULL;
        newNode->right = NULL;
        return newNode;
    }
    if (data < root->data) {
        root->left = insert(root->left, data);
    } else if (data > root->data) {
        root->right = insert(root->right, data);
    }
    return root;
}

void printTree(Node* root, int depth) {
    if (root == NULL) return;
    printTree(root->right, depth + 1);
    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%d\n", root->data);
    printTree(root->left, depth + 1);
}

Node* findMin(Node* root) {
    while (root->left != NULL) root = root->left;
    return root;
}

Node* deleteNode(Node* root, int data) {
    if (root == NULL) return root;
    if (data < root->data) {
        root->left = deleteNode(root->left, data);
    } else if (data > root->data) {
        root->right = deleteNode(root->right, data);
    } else {
        if (root->left == NULL) {
            Node* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            Node* temp = root->left;
            free(root);
            return temp;
        }
        Node* temp = findMin(root->right);
        root->data = temp->data;
        root->right = deleteNode(root->right, temp->data);
    }
    return root;
}

void findMinDepthLeafHelper(Node* root, int depth, int* minDepth, int* minLeafValue) {
    if (root == NULL) return;
    if (root->left == NULL && root->right == NULL) {
        if (depth < *minDepth) {
            *minDepth = depth;
            *minLeafValue = root->data;
        }
        return;
    }
    findMinDepthLeafHelper(root->left, depth + 1, minDepth, minLeafValue);
    findMinDepthLeafHelper(root->right, depth + 1, minDepth, minLeafValue);
}

void findMinDepthLeaf(Node* root) {
    if (root == NULL) {
        printf("Tree is empty\n");
        return;
    }
    int minDepth = 2147483647;
    int minLeafValue = -1;
    findMinDepthLeafHelper(root, 0, &minDepth, &minLeafValue);
    printf("Значение: %d\n", minLeafValue);
}

void freeTree(Node* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main() {
    Node* root = NULL;
    int choice, val;
    while (1) {
        printf("\n1. Добавить узел\n2. Вывод дерева\n3. Удалить узел\n4. Найти лист с минимальной глубиной\n5. Выход\n> ");
        if (scanf("%d", &choice) != 1) break;
        if (choice == 1) {
            printf("Значение: ");
            scanf("%d", &val);
            root = insert(root, val);
        } else if (choice == 2) {
            printf("\n");
            printTree(root, 0);
        } else if (choice == 3) {
            printf("Значение: ");
            scanf("%d", &val);
            root = deleteNode(root, val);
        } else if (choice == 4) {
            findMinDepthLeaf(root);
        } else if (choice == 5) {
            break;
        }
    }
    freeTree(root);
    return 0;
}