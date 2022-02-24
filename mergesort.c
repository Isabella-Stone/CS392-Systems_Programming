#include <stdio.h>
#include<stdlib.h>

#include "mergesort.h"

//I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone

int int_cmp(const void* a, const void* b) {
    int* x = (int*) a;
    int* y = (int*) b;
    if (*x == *y) {
        return 0;
    }
    else if (*x > *y) {
        return 1;
    }
    else {
        return -1;
    }
}

int dbl_cmp(const void* a, const void* b) {
    double* x = (double*) a;
    double* y = (double*) b;
    if (*x == *y) {
        return 0;
    }
    else if (*x > *y) {
        return 1;
    }
    else {
        return -1;
    }
}

void mergesort_helper(int i, int j, void* a, void* aux, size_t elem_sz, int (*comp)(const void*, const void*)) {
    if (j <= i) {
        return;     
    }
    int mid = (i + j) / 2;
    
    mergesort_helper(i, mid, a, aux, elem_sz, comp);          // sort the left sub a
    mergesort_helper(mid + 1, j, a, aux, elem_sz, comp);      // sort the right sub a

    int lptr = i;       
    int rptr = mid + 1;       
    int k;

    for (k = i; k <= j; k++) {
        //if left ptr is at end
        if (lptr == mid + 1) { 
            for (size_t i = 0; i < elem_sz; i++) {
                //loop through to ensure all bytes are copied over 
                (*((char*)aux + (k * elem_sz + i))) = (*((char*)a + (rptr * elem_sz + i)));
            }
            rptr++;
        } 
        //if right ptr is at end
        else if (rptr == j + 1) {        
            for (size_t i = 0; i < elem_sz; i++) {
                (*((char*)aux + (k * elem_sz + i))) = (*((char*)a + (lptr * elem_sz + i)));
            }    
            lptr++;
        } 
        //if left ptr points to smaller than elem than right ptr ('comp' returns -1)
        else if ( comp (( (char*)a + lptr * elem_sz),  ( (char*)a + rptr * elem_sz)) == -1) {
            for (size_t i = 0; i < elem_sz; i++) {
                (*((char*)aux + (k * elem_sz + i))) = (*((char*)a + (lptr * elem_sz + i)));
            }  
            lptr++;
        } 
        //else right ptr points to smaller elem
        else {
            for (size_t i = 0; i < elem_sz; i++) {
                (*((char*)aux + (k * elem_sz + i))) = (*((char*)a + (rptr * elem_sz + i)));
            } 
            rptr++;
        }
    }

    for (k = i; k <= j; k++) {      
        //copy the elems from aux[] back to a[]
        for (size_t i = 0; i < elem_sz; i++) {
                (*(((char*)a) + k * elem_sz + i)) = (*(((char*)aux) + k * elem_sz + i));
            } 
    }
}


void mergesort(void* array, size_t len, size_t elem_sz, int (*comp)(const void*, const void*)) {
    int left = 0;
    int right = ((int)len)-1;
    //create empty aux array to temporarily store results 
    void *aux[len];
    //call helper function
    mergesort_helper(left, right, array, aux, elem_sz, comp);
}

/*
//testing:
int main() {
    double a = 1.579;
    double b = 20.3333;
    double *pa, *pb;
    pa = &a;
    pb = &b;

    //int comp = int_cmp(pa, pb);
    int comp = dbl_cmp(pa, pb);
    printf("%d\n", comp);

}
*/