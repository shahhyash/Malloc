/************************************************
 *      memgrind.c                              *
 *      Authors:  Seth Karten, Yash Shah        *
 *      2019                                    *
 ************************************************/

#include "mymalloc.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TEST_MSG 0
#define COLLECT_DATA 0

/*
 *      Returns time of day in seconds with precision to microseconds
 */
double get_time()
{
        struct timeval t_val;
        gettimeofday(&t_val, NULL);
        return t_val.tv_sec + t_val.tv_usec * 1e-6;
}

/*
 *      Collect data from benchmarking test double array into a text file for further
 *      processing.
 */
void write_data(double * x, int num_tests)
{
        int i;
        for (i = 0; i < num_tests; i++)
        {
                char output[50];
                sprintf(output, "%lf", x[i]);
                FILE * fp = fopen("data.txt", "a");
                fwrite(output, 1, strlen(output), fp);
                fwrite("\n", sizeof(char), sizeof(char), fp);
                fclose(fp);
        }
}

/*
 *      mallocs 1 byte and immediately frees it (size times)
 */
int test_A(int size, int num_times)
{
        int i = 0;
        while(i < num_times)
        {
                char * test = (char *) malloc(size);
                if (test == NULL)
                {
                        fprintf(stderr, "TEST A: Error in malloc.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                        return -1;
                }
                *test = '1';
                if (DEBUG) printf("\nTEST %d PTR: \t %p\n\n", i+1, test);
                free(test);
                if (*test == '1')
                {
                        fprintf(stderr, "TEST A: Error in free.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                        return -1;
                }
                test = NULL;
                if (DEBUG) printf("\n");
                ++i;
        }
        return 0;
}

/*
 *      mallocs 1 byte, stores the pointer in an array (150 times). Once
 *      50 byte chunks have been malloced, it frees the 50 1 byte pointers one
 *      by one.
 */
int test_B()
{
        int k = 0;
        for (; k < 3; k++)
        {
                char *arr[50];
                int i = 0;
                for (; i < 50; i++)
                {
                        arr[i] = (char *) malloc(1);
                        if (arr[i] == NULL)
                        {
                                fprintf(stderr, "TEST B: Error in malloc.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                                return -1;
                        }
                        arr[i][0] = '1';
                        if (DEBUG) printf("\nTEST %d PTR: \t %p\n\n", k*50 + (i+1), arr[i]);
                }
                int j = 0;
                for (; j < 50; j++)
                {
                        if (DEBUG) printf("TEST %d PTR: %p\n", k*50 + (j+1), arr[j]);
                        free(arr[j]);
                        if (arr[j][0] == '1')
                        {
                                fprintf(stderr, "TEST B: Error in free.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                                return -1;
                        }
                        arr[j] = NULL;
                        if (DEBUG) printf("\n");
                }
        }

        return 0;
}

/*
 *      Randomly chooses between a 1 byte malloc or freeing a 1 byte pointer until
 *      50 allocations have occurred. Then all the allocations are freed.
 */
int test_C()
{
        int allocated = 0;
        char *arr[50];
        time_t t;
        srand((unsigned)time(&t));
        while (allocated != 50)
        {

                if ((rand() % 2 == 0))
                {
                        /* Allocate */
                        arr[allocated] = (char *) malloc(1);
                        if (arr[allocated] == NULL)
                        {
                                fprintf(stderr,"TEST C: Error in malloc.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                                return -1;
                        }
                        arr[allocated][0] = '1';
                        if (DEBUG) printf("\nMALLOC TEST %d PTR: \t %p\n\n", allocated+1, arr[allocated]);
                        allocated++;
                }
                else
                {
                        /* Free */
                        if (allocated > 0)
                        {
                                if (DEBUG) printf("FREE TEST %d PTR: %p\n", (allocated+1), arr[allocated]);
                                free(arr[allocated-1]);
                                if (arr[allocated-1][0] == '1')
                                {
                                        fprintf(stderr, "TEST C: Error in free.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                                        return -1;
                                }
                                arr[allocated-1] = NULL;
                                if (DEBUG) printf("\n");
                                allocated--;
                        }
                }
        }
        int j = 0;
        for (; j < 50; j++)
        {
                if (DEBUG) printf("FREE TEST %d PTR: %p\n", (j+1), arr[j]);
                free(arr[j]);
                if (arr[j][0] == '1')
                {
                        fprintf(stderr, "TEST C: Error in free.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                        return -1;
                }
                arr[j] = NULL;
                if (DEBUG) printf("\n");
        }
        return 0;
}

/*
 *      Randomly chooses between a randomly sized malloc (1-64 bytes) or freeing a pointer
 *      until 50 malloc operations have occurred. Then frees all the pointers.
 */
int test_D()
{
        int allocated = 0;
        char *arr[50];
        time_t t;
        srand((unsigned)time(&t));
        while (allocated != 50)
        {

                if (DEBUG) printf("allocated: %d\n", allocated);
                if ((rand() % 2 == 0))
                {
                        /* Allocate */
                        int size = 0;
                        int free_space = base64_to_dec(2);
                        if (!heap_initialized())
                                free_space = 4088;
                        if (free_space <= 4 && DEBUG)
                        {
                                fprintf(stderr, "Error no free space: %d\n", free_space);
                                if (DEBUG) print_heap(20);
                                return -1;
                        }
                        if (DEBUG) printf("free space: %d\tInitialized? %d\n", free_space, heap_initialized());
                        while (size == 0)
                        {

                                int temp = (rand() % 64) + 1;
                                if (DEBUG) printf("stuck in loop: temp: %d\t free space: %d\n", temp, free_space);
                                if (temp < free_space)
                                        size = temp;
                        }
                        if (DEBUG) printf("exited\n");
                        arr[allocated] = (char *) malloc(size);
                        if (arr[allocated] == NULL)
                        {
                                fprintf(stderr, "TEST D: Error in malloc.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                                return -1;
                        }
                        arr[allocated][0] = '1';
                        if (DEBUG) printf("\nMALLOC TEST %d PTR: \t %p\n\n", allocated+1, arr[allocated]);
                        allocated++;
                }
                else
                {
                        /* Free */
                        if (allocated > 0)
                        {
                                if (DEBUG) printf("FREE TEST %d PTR: %p\n", (allocated+1), arr[allocated-1]);
                                free(arr[allocated-1]);
                                if (arr[allocated-1][0] == '1')
                                {
                                        fprintf(stderr, "TEST D: Error in free.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                                        return -1;
                                }
                                arr[allocated-1] = NULL;
                                if (DEBUG) printf("\n");
                                allocated--;
                        }
                }
        }
        int j = 0;
        for (; j < 50; j++)
        {
                if (DEBUG) printf("FREE TEST %d PTR: %p\n", (j+1), arr[j]);
                free(arr[j]);
                if (arr[j][0] == '1')
                {
                        fprintf(stderr, "TEST D: Error in free.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                        return -1;
                }
                arr[j] = NULL;
                if (DEBUG) printf("\n");
        }
        return 0;
}

/*
 *      Allocates as many of every possible size (0-4095 bytes) in an array.
 *      After allocating as many as possible of one size in an array, it will
 *      free all the byte chunks in the array.
 */
int test_E()
{
        if (heap_initialized())
                clean();
        int i = 0;
        for (; i < 4088; i++)
        {
                int allocated = 0;
                int iter = 4088 / (3+i);
                char * arr[iter];
                int size = i+1;
                int j;
                for (j = 0; j < iter-3; j++)
                {
                        /* Allocate */
                        if (size > base64_to_dec(2) && heap_initialized())
                                break;
                        allocated++;
                        arr[j] = (char *) malloc(size);
                        if (arr[j] == NULL)
                        {
                                printf("iter %d %d\n", i, allocated);
                                fprintf(stderr,"TEST E: Error in malloc. Size: %d\tFile: %s\tLine: %d\n", size, __FILE__, __LINE__);
                                return -1;
                        }
                        arr[j][0] = '1';
                }
                for (j = 0; j < allocated; j++)
                {
                        /* Free */
                        free(arr[j]);
                        if (arr[j][0] == '1')
                        {
                                fprintf(stderr, "TEST E: Error in free.\tFile: %s\tLine: %d\n", __FILE__, __LINE__);
                                return -1;
                        }
                        arr[j] = NULL;
                }
                if (heap_initialized())
                        clean();
        }
        return 0;
}

/*
 *      Error tests
 */
int test_F()
{
        /* Freeing addresses that are not pointers */
        int x;
        free((int *)x);
        free(&x);
        /* Freeing pointers that were not allocated by malloc */
        char * p = (char *) malloc(200);
        free (p+10);

        free (p+4096);

        int * y;
        free(y);
        /* Redundant freeing of the same pointer */
        free(p);
        free(p);

        /* Mallocing invalid sizes */
        p = (char *) malloc(0);
        p = (char *) malloc(-1);

        /* Freeing a NULL pointer */
        free(NULL);

        /* Saturation of dynamic memory */
        p = (char *) malloc(4097);

        p = (char *) malloc(4096 - 7);


        return 0;
}

/*
 *      Outputs times of each benchmarking test to stdout.
 */
int grind(int num_tests)
{
        double start;
        int ret;
        /* these values can be written to a text file for future analysis / easy graphing in python with matplotlib */
        double A[num_tests], B[num_tests], C[num_tests], D[num_tests], E[num_tests], F[num_tests];
        double A_avg = 0, B_avg = 0, C_avg = 0, D_avg = 0, E_avg = 0, F_avg = 0;
        int i = 0;
        for (; i < num_tests; i++)
        {
                start = get_time();
                ret = test_A(1, 150);
                A[i] = get_time() - start;
                if (ret == -1)
                {
                        fprintf(stderr, "ERROR: TEST A.\tFILE: %s\tLINE: %d\n", __FILE__, __LINE__);
                        if (DEBUG) print_heap(150);
                        return 1;
                }
                if (TEST_MSG) printf("TEST A PASSED SUCCESSFULLY\n");
                A_avg += A[i];
                if (TEST_MSG) printf("TEST A TIME: %lf\n", A[i]);

                start = get_time();
                ret = test_B();
                B[i] = get_time() - start;
                if (ret == -1)
                {
                        fprintf(stderr, "ERROR: TEST B\n");
                        if (DEBUG) if (DEBUG) print_heap(150);
                        return 1;
                }
                if (TEST_MSG) printf("TEST B PASSED SUCCESSFULLY\n");
                B_avg += B[i];
                if (TEST_MSG) printf("TEST B TIME: %lf\n", B[i]);
                start = get_time();
                ret = test_C();
                C[i] = get_time() - start;
                if (ret == -1)
                {
                        fprintf(stderr, "ERROR: TEST C\n");
                        if (DEBUG) print_heap(150);
                        return 1;
                }
                if (TEST_MSG) printf("TEST C PASSED SUCCESSFULLY\n");
                C_avg += C[i];
                if (TEST_MSG) printf("TEST C TIME: %lf\n", C[i]);
                start = get_time();
                ret = test_D();
                D[i] = get_time() - start;
                if (ret == -1)
                {
                        fprintf(stderr, "ERROR: TEST D\n");
                        if (DEBUG) print_heap(150);
                        return 1;
                }
                if (TEST_MSG) printf("TEST D PASSED SUCCESSFULLY\n");
                D_avg += D[i];
                if (TEST_MSG) printf("TEST D TIME: %lf\n", D[i]);
                start = get_time();
                ret = test_E();
                E[i] = get_time() - start;
                if (ret == -1)
                {
                        fprintf(stderr, "Error: TEST E.\tFILE: %s.\tLINE %d.\n", __FILE__, __LINE__);
                        return 1;
                }
                E_avg += E[i];
                start = get_time();
                ret = test_F();
                F[i] = get_time() - start;
                if (ret == -1)
                {
                        fprintf(stderr, "Error: TEST F.\tFILE: %s.\tLINE %d.\n", __FILE__, __LINE__);
                        return 1;
                }
                F_avg += F[i];
        }
        if (COLLECT_DATA)
        {
                write_data(A, num_tests);
                write_data(B, num_tests);
                write_data(C, num_tests);
                write_data(D, num_tests);
                write_data(E, num_tests);
                write_data(F, num_tests);
        }
        A_avg /= (double) num_tests;
        B_avg /= (double) num_tests;
        C_avg /= (double) num_tests;
        D_avg /= (double) num_tests;
        E_avg /= (double) num_tests;
        F_avg /= (double) num_tests;
        printf("\nTEST A AVERAGE: %lf\nTEST B AVERAGE: %lf\nTEST C AVERAGE: %lf\nTEST D AVERAGE: %lf\nTEST E AVERAGE: %lf\nTEST F AVERAGE: %lf\n\n", A_avg, B_avg, C_avg, D_avg, E_avg, F_avg);
        return 0;
}



int main(int argc, char * argv[])
{
        return grind(100);
}
