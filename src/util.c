#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

enum { BUFFER_GROWTH_FACTOR = 2 };

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

uint32_t util_uint32_sat_sub(uint32_t x, uint32_t y) {
    uint32_t ret = x - y;
    ret &= -(ret <= x);
    return ret;
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

struct util_Buffer {
    uint32_t cap;
    uint32_t count;
    char *data;
};

int util_buffer_init(struct util_Buffer **buff, uint32_t cap) {
    struct util_Buffer *buff_h = NULL;
    char *data_h = NULL;

    buff_h = malloc(sizeof(struct util_Buffer));
    if (buff_h == NULL) {
        return 1;
    }
    *buff = buff_h;

    data_h = calloc(cap, sizeof(char));
    if (data_h == NULL) {
        return 1;
    }

    (*buff)->data = data_h;
    (*buff)->cap = cap;
    (*buff)->count = 0;

    return 0;
}

int util_buffer_deinit(struct util_Buffer **buff) {
    if (*buff == NULL) {
        return 0;
    }

    if ((*buff)->data != NULL) {
        free((*buff)->data);
    }

    (*buff)->cap = 0;
    (*buff)->count = 0;
    (*buff)->data = NULL;
    free(*buff);
    *buff = NULL;
    return 0;
}

uint32_t util_buffer_cap(struct util_Buffer *buff) {
    if (buff == NULL) {
        return 0;
    }
    return buff->cap;
}

uint32_t util_buffer_count(struct util_Buffer *buff) {
    if (buff == NULL) {
        return 0;
    }
    return buff->count;
}

static int util_buffer_grow(struct util_Buffer *buff, uint32_t cap_hint, uint32_t growth) {
    char *data_h = NULL;
    const uint32_t cap = buff->cap;
    uint64_t new_cap = (uint64_t)cap;

    if (cap_hint <= cap) {
        return 0;
    }

    while (cap_hint > new_cap) {
        new_cap *= growth;
    }
    if (new_cap > UINT32_MAX) {
        new_cap = UINT32_MAX;
    }

    data_h = realloc(buff->data, (size_t)new_cap * sizeof(char));
    if (data_h == NULL) {
        return 1;
    }

    memset(data_h + cap, 0, (size_t)(new_cap - cap) * sizeof(char));

    buff->data = data_h;
    buff->cap = (uint32_t)new_cap;

    return 0;
}

int util_buffer_push(struct util_Buffer *buff, char item) {
    if (buff == NULL || buff->data == NULL) {
        return 1;
    }
    if (buff->count == UINT32_MAX) {
        return 1;
    }
    const uint32_t new_count = buff->count + 1;
    if (util_buffer_grow(buff, new_count, BUFFER_GROWTH_FACTOR) == 1) {
        return 1;
    }
    *(buff->data + buff->count) = item;
    buff->count = new_count;
    return 0;
}

int util_buffer_set(struct util_Buffer *buff, uint32_t index, char item) {
    if (buff == NULL || buff->data == NULL) {
        return 1;
    }
    const uint32_t new_count = index + 1;
    if (util_buffer_grow(buff, new_count, BUFFER_GROWTH_FACTOR) == 1) {
        return 1;
    }
    *(buff->data + index) = item;
    if (index >= buff->count) {
        buff->count = new_count;
    }
    return 0;
}

int util_buffer_read(struct util_Buffer *buff, uint32_t index, char *out) {
    if (buff == NULL || buff->data == NULL) {
        return 1;
    }
    if (index >= buff->count) {
        return 1;
    }
    *out = *(buff->data + index);
    return 0;
}

struct util_Buffer2d {
    uint32_t x_cap;
    uint32_t y_cap;
    char **data;
};

int util_buffer2d_init(struct util_Buffer2d **buff, uint32_t x_cap, uint32_t y_cap) {
    struct util_Buffer2d *buff_h = NULL;
    char **data_h = NULL;
    char *row_h = NULL;

    buff_h = malloc(sizeof(struct util_Buffer2d));
    if (buff_h == NULL) {
        return 1;
    }
    *buff = buff_h;

    data_h = calloc(x_cap, sizeof(char *));
    if (data_h == NULL) {
        return 1;
    }

    for (uint32_t i = 0; i < x_cap; i++) {
        row_h = calloc(y_cap, sizeof(char));
        if (row_h == NULL) {
            return 1;
        }
        *(data_h + i) = row_h;
    }

    (*buff)->data = data_h;
    (*buff)->x_cap = x_cap;
    (*buff)->y_cap = y_cap;

    return 0;
}

int util_buffer2d_deinit(struct util_Buffer2d **buff) {
    if (*buff == NULL) {
        return 0;
    }

    for (uint32_t i = 0; i < (*buff)->x_cap; i++) {
        free(*((*buff)->data + i));
    }
    free((*buff)->data);

    (*buff)->x_cap = 0;
    (*buff)->y_cap = 0;
    (*buff)->data = NULL;
    free(*buff);
    *buff = NULL;
    return 0;
}

uint32_t util_buffer2d_x_cap(struct util_Buffer2d *buff) {
    if (buff == NULL) {
        return 0;
    }
    return buff->x_cap;
}

uint32_t util_buffer2d_y_cap(struct util_Buffer2d *buff) {
    if (buff == NULL) {
        return 0;
    }
    return buff->y_cap;
}

static int util_buffer2d_grow(struct util_Buffer2d *buff,
                              uint32_t x_cap_hint,
                              uint32_t y_cap_hint,
                              uint32_t growth) {
    char **data_h = NULL;
    char *sub_h = NULL;
    const uint32_t x_cap = buff->x_cap;
    const uint32_t y_cap = buff->y_cap;
    uint64_t new_x_cap = (uint64_t)x_cap;
    uint64_t new_y_cap = (uint64_t)y_cap;

    if (x_cap_hint <= x_cap || y_cap_hint <= y_cap) {
        return 0;
    }

    while (x_cap_hint > new_x_cap) {
        new_x_cap *= growth;
    }
    while (y_cap_hint > new_y_cap) {
        new_y_cap *= growth;
    }
    if (new_x_cap > UINT32_MAX) {
        new_x_cap = UINT32_MAX;
    }
    if (new_y_cap > UINT32_MAX) {
        new_y_cap = UINT32_MAX;
    }

    data_h = realloc(buff->data, (size_t)new_x_cap * sizeof(char *));
    if (data_h == NULL) {
        return 1;
    }

    memset(data_h + x_cap, 0, (size_t)(new_x_cap - x_cap) * sizeof(char *));

    for (uint32_t i = 0; i < new_x_cap; i++) {
        sub_h = realloc(*(data_h + i), (size_t)new_y_cap * sizeof(char));
        if (sub_h == NULL) {
            return 1;
        }
        *(data_h + i) = sub_h;

        memset(*(data_h + i) + y_cap, 0, (size_t)(new_y_cap - y_cap) * sizeof(char));

        if (i >= x_cap) {
            memset(*(data_h + i), 0, (size_t)new_y_cap * sizeof(char));
        }
    }

    buff->data = data_h;
    buff->x_cap = (uint32_t)new_x_cap;
    buff->y_cap = (uint32_t)new_y_cap;

    return 0;
}

int util_buffer2d_set(struct util_Buffer2d *buff, uint32_t x_index, uint32_t y_index, char item) {
    if (buff == NULL || buff->data == NULL) {
        return 1;
    }
    const uint32_t x_cap_hint = x_index + 1;
    const uint32_t y_cap_hint = y_index + 1;
    if (util_buffer2d_grow(buff, x_cap_hint, y_cap_hint, BUFFER_GROWTH_FACTOR) == 1) {
        return 1;
    }
    *(*(buff->data + x_index) + y_index) = item;
    return 0;
}

int util_buffer2d_read(struct util_Buffer2d *buff, uint32_t x_index, uint32_t y_index, char *out) {
    if (buff == NULL || buff->data == NULL) {
        return 1;
    }
    if (x_index >= buff->x_cap || y_index >= buff->y_cap) {
        return 1;
    }
    *out = *(*(buff->data + x_index) + y_index);
    return 0;
}
