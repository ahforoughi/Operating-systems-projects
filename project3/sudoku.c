#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

struct param {
    int row;
    int col;
};

int array[9][9] = {
    {6,2,4,5,3,9,1,8,7},
    {5,1,9,7,2,8,6,3,4},
    {8,3,7,6,1,4,2,9,5},
    {1,4,3,8,6,5,7,2,9},
    {9,5,8,2,4,7,3,6,1},
    {7,6,2,3,9,1,4,5,8},
    {3,7,1,9,5,6,8,4,2},
    {4,9,6,1,8,2,5,7,3},
    {21,8,5,4,7,3,9,1,6},
};

void* column_verify(void *param) {
    struct param *p = (struct param*)param;
    int flag[9] = {0};
    
    for (int col = 0; col < p->col; col++) {
        memset(flag, -1, sizeof(flag));
        for (int row = 0; row < p->row; row++) {
            if (array[row][col] < 9) {
                flag[array[row][col]] = array[row][col];
            } else {
                return NULL;
            }
        }
        for (int span = 0; span < sizeof(flag); span++) {
            if (flag[span] != span) 
                continue;
        }
    }
    return NULL;
}

void *row_verify(void *param) {
    struct param *p = (struct param*)param;
    int flag[9] = {0};
    
    for (int row = 0; row < p->row; row++) {
        memset(flag, -1, sizeof(flag));
        for (int col = 0; col < p->col; col++) {
            if (array[row][col] < 9) {
                flag[array[row][col]] = array[row][col];
            } else {
                return NULL;
            }
        }
        for (int span = 0; span < sizeof(flag); span++) {
            if (flag[span] != span)
                continue;
        }
    }
    return NULL;
}

void* block_verify(void* idx) {
    int index = *(int*)idx;
    
    int sx = (3 * index) / 9;
    int sy = (3 * index) % 9;

    int endx = sx+2;
    int endy = sy+2;
    int flag[9] = {0};
    memset(flag, -1, sizeof(flag));
    for (int i=sx;i<endx;i++) {
        for (int j=sy;j<endy;j++) {
            if (array[i][j] < 9) {
                flag[array[i][j]] = array[i][j];
            } else {
                return NULL;
            }
        }
    }
    for (int span = 0; span < sizeof(flag); span++) {
        if (flag[span] != span) /*return "1"*/
            continue;
    }
    return NULL;
}

int main() {
    pthread_t column;
    pthread_t row;
    pthread_t th[9];

    struct param p;
    p.row = 9;
    p.col = 9;
    printf("thread1: \n");
    pthread_create(&column, NULL, column_verify, &p);
    printf("thread2: \n");
    pthread_create(&row, NULL, row_verify, &p);

    for (int idx = 0; idx < 9; idx++){
        printf("thread %d: \n", idx+2);
        pthread_create(&th[idx], NULL, block_verify, &idx);
    }

    char *res = NULL;
    bool flag = true;
    pthread_join(column, (void**)&res);
    //printf("%s %d ", res, *res == '1');
    flag &= *res == true;
    pthread_join(row, (void**)&res);
    //printf("%s", res);
    flag &= *res == true;
    for (int idx = 0; idx < 9; idx++) {
        pthread_join(th[idx], (void**)&res);
        //printf("%s", res);
        flag &= *res == true;
    }
    printf("final verification: %d\n", flag);
    return 0;
}