
/*
 * ThreadDeadLock_tests.c
 *
 *  Created on: Oct 24, 2012
 *      Author: fgirmay
 */


/**
 * This code tests for potential deadlocks. The code is not yet mature. testDeadLock
 */
/**
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double testDeadLock(int iterations) {

  double pi;
  int add = 0;

  pi = 4;
  for (int ii = 0; ii < iterations; ii++) {
    if (add == 1) {
      pi = pi + (4.0/(3.0+ii*2));
      add = 0;
    } else {
      pi = pi - (4.0/(3.0+ii*2));
      add = 1;
    }
  }
  return pi;
}
**/

int main() {
	/**
  if ( argc != 2 ) {
  printf("Usage: %s <niter>",argv[0]);
   return 1;
 }
  const int iterations = atoi(argv[1]);
#pragma omp parallel
  {    double pi = testDeadLock(iterations);
    printf("Thread %d, pi = %g\n",omp_get_thread_num(),pi);
  }**/
  return 0;
}
