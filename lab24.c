#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Типы узлов дерева
typedef enum { NODE_OP, NODE_VAR, NODE_NUM } NodeType;

// Структура узла бинарного дерева выражений
typedef struct Node {
    NodeType type;
    char op;                  // Для NODE_OP (+, -, *, /)
    char var_name[32];        // Для NODE_VAR
    int num_value;            // Для NODE_NUM
    struct Node *left;
    struct Node *right;
} Node;

// Прототипы функций синтаксического анализа (Рекурсивный спуск)
Node* parse_expr(const char** src);
Node* parse_term(const char** src);
Node* parse_factor(const char** src);
void free_tree(Node* node);
Node* simplify_tree(Node* node);
void print_infix(Node* node, Node* parent);

// Вспомогательные функции создания узлов
Node* create_num(int val) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->type = NODE_NUM;
    n->num_value = val;
    n->left = n->right = NULL;
    return n;
}

Node* create_var(const char* name) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->type = NODE_VAR;
    strcpy(n->var_name, name);
    n->left = n->right = NULL;
    return n;
}

Node* create_op(char op, Node* left, Node* right) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->type = NODE_OP;
    n->op = op;
    n->left = left;
    n->right = right;
    return n;
}

void skip_spaces(const char** src) {
    while (**src && isspace(**src)) {
        (*src)++;
    }
}

// Парсинг базового элемента (число, переменная или выражение в скобках)
Node* parse_factor(const char** src) {
    skip_spaces(src);
    if (**src == '(') {
        (*src)++; // Пропускаем '('
        Node* node = parse_expr(src);
        skip_spaces(src);
        if (**src == ')') (*src)++; // Пропускаем ')'
        return node;
    }
    if (isdigit(**src)) {
        int val = 0;
        while (**src && isdigit(**src)) {
            val = val * 10 + (**src - '0');
            (*src)++;
        }
        return create_num(val);
    }
    if (isalpha(**src)) {
        char name[32];
        int i = 0;
        while (**src && (isalnum(**src) || **src == '_')) {
            if (i < 31) name[i++] = **src;
            (*src)++;
        }
        name[i] = '\0';
        return create_var(name);
    }
    return NULL;
}

// Парсинг операций умножения и деления
Node* parse_term(const char** src) {
    Node* root = parse_factor(src);
    while (1) {
        skip_spaces(src);
        if (**src == '*' || **src == '/') {
            char op = **src;
            (*src)++;
            Node* right = parse_factor(src);
            root = create_op(op, root, right);
        } else {
            break;
        }
    }
    return root;
}

// Парсинг операций сложения и вычитания
Node* parse_expr(const char** src) {
    Node* root = parse_term(src);
    while (1) {
        skip_spaces(src);
        if (**src == '+' || **src == '-') {
            char op = **src;
            (*src)++;
            Node* right = parse_term(src);
            root = create_op(op, root, right);
        } else {
            break;
        }
    }
    return root;
}

// Освобождение памяти дерева
void free_tree(Node* node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

// Глубокое копирование узла/поддерева
Node* clone_tree(Node* node) {
    if (!node) return NULL;
    Node* n = (Node*)malloc(sizeof(Node));
    memcpy(n, node, sizeof(Node));
    n->left = clone_tree(node->left);
    n->right = clone_tree(node->right);
    return n;
}

// Рекурсивный сбор всех сомножителей в изолированной ветви умножения
void collect_factors(Node* node, int* prod, Node** vars, int* var_cnt, int* num_cnt) {
    if (!node) return;
    if (node->type == NODE_OP && node->op == '*') {
        collect_factors(node->left, prod, vars, var_cnt, num_cnt);
        collect_factors(node->right, prod, vars, var_cnt, num_cnt);
    } else if (node->type == NODE_NUM) {
        *prod *= node->num_value;
        (*num_cnt)++;
    } else {
        vars[(*var_cnt)++] = clone_tree(node);
    }
}

// Функция трансформации дерева выражений (упрощение умножения)
Node* simplify_tree(Node* node) {
    if (!node) return NULL;
    if (node->type != NODE_OP) return node;

    // Сначала оптимизируем дочерние поддеревья (посточередной обход)
    node->left = simplify_tree(node->left);
    node->right = simplify_tree(node->right);

    if (node->type == NODE_OP && node->op == '*') {
        int prod = 1;
        int var_cnt = 0;
        int num_cnt = 0;
        Node* vars[128];

        collect_factors(node, &prod, vars, &var_cnt, &num_cnt);

        // Если найдено больше одной числовой константы, выполняем свертку
        if (num_cnt > 1) {
            free_tree(node); // Удаляем старое несвернутое поддерево

            if (prod == 0) {
                for (int i = 0; i < var_cnt; i++) free_tree(vars[i]);
                return create_num(0);
            }

            Node* new_root = NULL;
            int start_idx = 0;

            if (prod != 1 || var_cnt == 0) {
                new_root = create_num(prod);
            } else {
                new_root = vars[0];
                start_idx = 1;
            }

            for (int i = start_idx; i < var_cnt; i++) {
                new_root = create_op('*', new_root, vars[i]);
            }
            return new_root;
        } else {
            // Если оптимизация не требуется, очищаем временные копии
            for (int i = 0; i < var_cnt; i++) free_tree(vars[i]);
        }
    }
    return node;
}

int get_precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

// Печать дерева обратно в инфиксную строку с расстановкой скобок по приоритетам
void print_infix(Node* node, Node* parent) {
    if (!node) return;
    
    int needs_parentheses = 0;
    if (parent && node->type == NODE_OP && parent->type == NODE_OP) {
        if (get_precedence(node->op) < get_precedence(parent->op)) {
            needs_parentheses = 1;
        }
    }

    if (needs_parentheses) printf("(");

    if (node->type == NODE_NUM) {
        printf("%d", node->num_value);
    } else if (node->type == NODE_VAR) {
        printf("%s", node->var_name);
    } else if (node->type == NODE_OP) {
        print_infix(node->left, node);
        printf(" %c ", node->op);
        print_infix(node->right, node);
    }

    if (needs_parentheses) printf(")");
}

// Псевдографический вывод структуры дерева в консоль
void print_tree_structure(Node* node, int level, char* prefix) {
    if (!node) return;
    for (int i = 0; i < level; i++) printf("    ");
    printf("%s", prefix);
    if (node->type == NODE_NUM) printf("%d\n", node->num_value);
    else if (node->type == NODE_VAR) printf("%s\n", node->var_name);
    else printf("%c\n", node->op);

    print_tree_structure(node->left, level + 1, "L-> ");
    print_tree_structure(node->right, level + 1, "R-> ");
}

// Функция тестирования
void run_test(const char* expr_str) {
    printf("====================================================\n");
    printf("Исходная строка: %s\n", expr_str);
    
    const char* ptr = expr_str;
    Node* root = parse_expr(&ptr);
    
    printf("\n--- Исходное дерево ---\n");
    print_tree_structure(root, 0, "Корень: ");
    
    root = simplify_tree(root);
    
    printf("\n--- Преобразованное дерево ---\n");
    print_tree_structure(root, 0, "Корень: ");
    
    printf("\nРезультат в тексте: ");
    print_infix(root, NULL);
    printf("\n");
    
    free_tree(root);
}

int main() {
    // Тест 1: Без преобразуемых элементов
    run_test("a + b * c");
    
    // Тест 2: С ровно одним целевым элементом (базовый пример)
    run_test("2 * b * 2");
    
    // Тест 3: С несколькими преобразуемыми элементами в разных частях дерева
    run_test("2 * b * 2 + 3 * c * 3");
    
    // Тест 4: Сложная структура со скобками
    run_test("(x * 5) * (4 * y * 2)");
    
    return 0;
}