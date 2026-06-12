#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_SIZE 100

/* 1. ОПРЕДЕЛЕНИЕ ТИПОВ И ПОЛНОГО ИНТЕРФЕЙСА АТД СОГЛАСНО СХЕМЕ */

typedef int key_type;
typedef float value_type;

typedef struct {
    key_type key;
    value_type value;
} data_type;

typedef struct {
    data_type data[MAX_SIZE];
    int top_index;
} udt;

void udt_create(udt *s);
bool udt_is_empty(const udt *s);
bool udt_is_full(const udt *s);
void udt_print(const udt *s);
size_t udt_size(const udt *s);

/* Сигнатуры из методички */
void udt_push_front(udt *s);
void udt_push_back(udt *s);
void udt_pop_front(udt *s);
void udt_pop_back(udt *s);
void udt_insert(udt *s, const data_type item);
void udt_erase(udt *s, const key_type key);

/* Дополнительная интерфейсная функция для доступа к вершине */
data_type udt_back(const udt *s);
/* Вспомогательная функция для прямой вставки элемента в коде */
void udt_push_back_item(udt *s, const data_type item);

/* 2. РЕАЛИЗАЦИЯ ВСЕХ ФУНКЦИЙ ИНТЕРФЕЙСА АТД */

void udt_create(udt *s) {
    s->top_index = -1;
}

bool udt_is_empty(const udt *s) {
    return s->top_index == -1;
}

bool udt_is_full(const udt *s) {
    return s->top_index >= MAX_SIZE - 1;
}

size_t udt_size(const udt *s) {
    return (size_t)(s->top_index + 1);
}

data_type udt_back(const udt *s) {
    if (!udt_is_empty(s)) {
        return s->data[s->top_index];
    }
    data_type empty = {0, 0.0f};
    return empty;
}

void udt_push_back_item(udt *s, const data_type item) {
    if (!udt_is_full(s)) {
        s->top_index++;
        s->data[s->top_index] = item;
    }
}

/* Реализация push_back без параметров согласно схеме с чтением из потока ввода */
void udt_push_back(udt *s) {
    if (udt_is_full(s)) {
        return;
    }
    data_type item;
    if (scanf("%d %f", &item.key, &item.value) == 2) {
        udt_push_back_item(s, item);
    }
}

void udt_pop_back(udt *s) {
    if (!udt_is_empty(s)) {
        s->top_index--;
    }
}

/* Реализация функций линейных структур, адаптированная под ограничения стека */
void udt_push_front(udt *s) {
    /* Для стека добавление в начало нетипично, дублирует push_back при пустом или сдвигает массив */
    if (udt_is_full(s)) {
        return;
    }
    data_type item;
    if (scanf("%d %f", &item.key, &item.value) == 2) {
        for (int i = s->top_index; i >= 0; i--) {
            s->data[i + 1] = s->data[i];
        }
        s->data[0] = item;
        s->top_index++;
    }
}

void udt_pop_front(udt *s) {
    if (udt_is_empty(s)) {
        return;
    }
    for (int i = 0; i < s->top_index; i++) {
        s->data[i] = s->data[i + 1];
    }
    s->top_index--;
}

void udt_insert(udt *s, const data_type item) {
    if (!udt_is_full(s)) {
        udt_push_back_item(s, item);
    }
}

void udt_erase(udt *s, const key_type key) {
    if (udt_is_empty(s)) {
        return;
    }
    if (s->data[s->top_index].key == key) {
        udt_pop_back(s);
    }
}

void udt_print(const udt *s) {
    if (udt_is_empty(s)) {
        printf("Стек пуст\n");
        return;
    }
    for (int i = 0; i <= s->top_index; i++) {
        printf("[%d: %.2f] ", s->data[i].key, s->data[i].value);
    }
    printf("(вершина)\n");
}

/* 3. РЕАЛИЗАЦИЯ АЛГОРИТМА СОРТИРОВКИ СЛИЯНИЕМ (РЕКУРСИЯ) */

void udt_merge(udt *h1, udt *h2, udt *src) {
    if (udt_is_empty(h1) && udt_is_empty(h2)) {
        return;
    }

    data_type selected;

    if (udt_is_empty(h1)) {
        selected = udt_back(h2);
        udt_pop_back(h2);
        udt_merge(h1, h2, src);
        udt_push_back_item(src, selected);
    } else if (udt_is_empty(h2)) {
        selected = udt_back(h1);
        udt_pop_back(h1);
        udt_merge(h1, h2, src);
        udt_push_back_item(src, selected);
    } else {
        data_type t1 = udt_back(h1);
        data_type t2 = udt_back(h2);

        if (t1.key < t2.key) {
            selected = t2;
            udt_pop_back(h2);
        } else {
            selected = t1;
            udt_pop_back(h1);
        }
        
        udt_merge(h1, h2, src);
        udt_push_back_item(src, selected);
    }
}

void udt_merge_sort(udt *src) {
    size_t size = udt_size(src);
    if (size <= 1) {
        return;
    }

    udt h1, h2;
    udt_create(&h1);
    udt_create(&h2);

    size_t half = size / 2;
    for (size_t i = 0; i < half; i++) {
        udt_push_back_item(&h1, udt_back(src));
        udt_pop_back(src);
    }
    while (!udt_is_empty(src)) {
        udt_push_back_item(&h2, udt_back(src));
        udt_pop_back(src);
    }

    udt_merge_sort(&h1);
    udt_merge_sort(&h2);

    udt_merge(&h1, &h2, src);
}

/* 4. ТОЧКА ВХОДА И ВЕРИФИКАЦИЯ */

int main(void) {
    udt my_stack;
    udt_create(&my_stack);

    data_type items[] = {
        {14, 1.4f},
        {3,  0.3f},
        {8,  0.8f},
        {25, 2.5f},
        {1,  0.1f},
        {12, 1.2f}
    };
    size_t items_count = sizeof(items) / sizeof(items[0]);

    printf("Заполнение стека элементами:\n");
    for (size_t i = 0; i < items_count; i++) {
        udt_push_back_item(&my_stack, items[i]);
    }
    udt_print(&my_stack);

    printf("\nВыполнение сортировки слиянием...\n");
    udt_merge_sort(&my_stack);

    printf("Результат (сортировка по возрастанию от дна к вершине):\n");
    udt_print(&my_stack);

    return 0;
}