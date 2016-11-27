#include <sys/times.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

double start, stop, max,used, mf;
volatile double *mem_a, *mem_b, *mem_c;
sem_t *mem_sem;
double ftime(void);
void multiply (double **a, double **b, double **c, int n);
double m_time;

double ftime (void)
{
    struct tms t;
    
    times ( &t );
 
    return (t.tms_utime + t.tms_stime) / 100.0;
}

void print(double **d,int n) {
    int i = 0;
    int j = 0;
    for (i=0; i<n; i++) {
        printf("\n");
        for (j=0; j<n; j++) {
          printf("%.0f ",d[i][j]);
        }
      }
    printf("\n");
}

void sharedPrint(volatile double *matrix, int n) {
  int i, j;
  int count = 0;
  for (i = 0; i<n; i++) {
    printf("\n");
    for (j = 0; j<n; j++) {
count++;
      printf("%.0f ",matrix[i*n+j]);
    }
  }
}

void fillA(double **a, int n) {
int i, j, k;
  for (i=0; i<n; i++) {
    for (j = 0; j<n;j++) {
        mem_a[i*n+j]=a[i][j];    
    }
  }
}

void fillB(double **b, int n) {
int i, j, k;
  for (i=0; i<n; i++) {
    for (j = 0; j<n;j++) {
      mem_b[i*n+j]=b[i][j];        
    }
  }
}

void fillC( int n) {
int i, j, k;
  for (i=0; i<n; i++) {
    for (j = 0; j<n;j++) {
      mem_c[i*n+j]=0;
    }
  }
}

void freeMatrix(double **a, int n) {
  int i;
  for(i=0;i<n;i++) {
   free(a[i]);
  }
  free(a);
}

void multiply (double **a, double **b, double **c, int n)
{
   int i, j, k;

   // initialize c to all 0s
   for (i=0; i<n; i++)
   {
     for (j=0; j<n; j++)
       
         c[i][j] = 0;
    }
    
    // multiplication performed, store answer in c
    for (i=0; i<n; i++)
    {
       for (j=0; j<n; j++)
       {
         for (k=0; k<n; k++)
           c[i][j]= c[i][j] + a[i][k] * b[k][j];
        }
     }
  }

//new multiplication function after b is already tranposed
void multiplyT (double **a, double **b, double **c, int n)
{
   int i, j, k;

   for (i=0; i<n; i++)
   {
     for (j=0; j<n; j++)

         c[i][j] = 0;
    }

    for (i=0; i<n; i++)
    {
       for (j=0; j<n; j++)
       {
         for (k=0; k<n; k++)
           c[i][j]= c[i][j] + a[i][k] * b[j][k];
        }
     }
  }


double **transpose(double **matrix, int n) {
int i;   
int j;
double **matrixT;
matrixT= (double**)malloc(n*sizeof(double));

     for (i=0; i<n; i++)
     {
       matrixT[i]= malloc(sizeof(double)*n);
	}

for (i=0; i<n; i++)
   {
     for (j=0; j<n; j++){
	// only swap non diagnals 
	matrixT[j][i]=matrix[i][j];
    }
}
return matrixT;
}

//find minimum number
int findMin(int a, int b) {
	if(a-b>0) {
	return b;
	}
	else {
	return a;
	}
}

int isDivisible(int blockSize, int n) {
if (n%blockSize==0)
  return (1);
else
  return (0);
}
/* get block size, n/2 or n/3 to get less than 32 processes
int getB (int n) {
// n/b must be 3 or less, pick a b
// assume n is not a prime number, else cannot divide
if (n%2 == 0) {
  //best case n/b = 3
  if(n%3 == 0) {
  return n/3;
  }
  else {
  return n/2;
  }  
}
//odd number
else {
  if(n%3 == 0) 
	return n/3;
  else {
    return 1;
    }
  }
}
*/
int getB(int BlockSize) {
return BlockSize;
}

//block multiplication
void blockMultiply (double **a, double **b, double **c, int n, int blockSize)
{
   int i0, j0, k0, j,i,k;
   // initialize c to all 0s
   for (i=0; i<n; i++) { 
     for (j=0; j<n; j++){
         c[i][j] = 0;}
   }
    // multiplication performed, store answer in c
for (i0=0; i0<n;i0+=blockSize) {
  for (j0=0; j0<n; j0+=blockSize) {
    for(k0=0; k0<n; k0+=blockSize) {
	//subblocks
      for(i = i0; i<findMin(i0+blockSize,n); i++) {
        for(j = j0; j<findMin(j0+blockSize,n); j++) {
	  for(k = k0; k<findMin(k0+blockSize,n); k++) {
	    c[i][j]= c[i][j] + a[i][k] * b[k][j];
	  }
	}
      }
    }
  }
}
}

void singleBlock(int i0, int j0,int k0, int blockSize, int n) {
int i,j,k;
double start,stop;
  for(i = i0; i<findMin(i0+blockSize,n); i++) {
    for(j = j0; j<findMin(j0+blockSize,n); j++) {
      for(k = k0; k<findMin(k0+blockSize,n); k++) {
            sem_wait(&mem_sem[i*n+j]);
            start = ftime();
            mem_c[i*n+j]= mem_c[i*n+j] + mem_a[i*n+k] * mem_b[k*n+j];
            stop = ftime();
            if(stop-start>max) {
            max = stop-start;
            }
            sem_post(&mem_sem[i*n+j]);
      }
    }
  }
}

void multiProcess(double **a, double **b, int n, int blockSize) {
  int i0,j0,k0,i,j,k;
  int pid, shmfd_a, shmfd_b, shmfd_c,shmfd_sem;
int kidsPerBlock = n/blockSize;
  shmfd_a = shm_open( "/guand2a_memory", O_RDWR | O_CREAT, 0666 );
  shmfd_b = shm_open( "/guand2b_memory", O_RDWR | O_CREAT, 0666 );
  shmfd_c = shm_open( "/guand2c_memory", O_RDWR | O_CREAT, 0666 );
  shmfd_sem = shm_open( "/guand2sem_memory", O_RDWR | O_CREAT, 0666 );
  if ( shmfd_a < 0 ) {
        fprintf(stderr,"Could not create guand2a_memory\n");
        exit(1);
    }

    if ( shmfd_b < 0 ) {
        fprintf(stderr,"Could not create guand2b_memory\n");
        exit(1);
    }

   if ( shmfd_c < 0 ) {
        fprintf(stderr,"Could not create guand2c_memory\n");
        exit(1);
    }
   if ( shmfd_sem < 0 ) {
        fprintf(stderr,"Could not create guand2sem_memory\n");
        exit(1);
    }
    ftruncate ( shmfd_a, n*n*sizeof(double) );
    ftruncate ( shmfd_b, n*n*sizeof(double) );
    ftruncate ( shmfd_c, n*n*sizeof(double) );
    ftruncate ( shmfd_sem, n*n*sizeof(sem_t) );
    //set shared memory a
  mem_a= (double *)mmap ( NULL, n*n*sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_a, 0 );
  mem_b= (double *)mmap ( NULL, n*n*sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_b, 0 );
  mem_c= (double *)mmap ( NULL, n*n*sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_c, 0 );
  mem_sem= (sem_t *)mmap ( NULL, n*n*sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd_sem, 0 );
    if (mem_a < 0) {
        fprintf(stderr,"Could not map guand2a_memory\n");
        exit(1);
    }
    if (mem_b < 0) {
        fprintf(stderr,"Could not map guand2b_memory\n");
        exit(1);
    }
    if (mem_c < 0) {
        fprintf(stderr,"Could not map guand2c_memory\n");
        exit(1);
    }
   if (mem_sem < 0) {
        fprintf(stderr,"Could not map guand2sem_memory\n");
        exit(1);
    }
    close (shmfd_a);
    close (shmfd_b);
    close (shmfd_c);  
    close (shmfd_sem);
 
  shm_unlink("/guand2a_memory");
  shm_unlink("/guand2b_memory");
  shm_unlink("/guand2c_memory");
  shm_unlink("/guand2sem_memory");
  //create sem
  for (i = 0; i < n*n; i++) {
    sem_init(&(mem_sem[i]), 1,1);
  if(&mem_sem[i] == NULL) {
       fprintf(stderr,"Could not create guand2a semaphore\n");
        exit(1);
    }  
  }

    fillA(a,n);
    freeMatrix(a,n);
    fillB(b,n);
    freeMatrix(b,n);
    fillC(n);
   for(i0=0; i0<n;i0+=blockSize) {
    for (j0=0; j0<n; j0+=blockSize) {
      for(k0=0; k0<n; k0+=blockSize) {
        pid=fork();
        if ( pid < 0 ) {
            fprintf(stderr,"fork failed at %d\n",i);
            exit(1);
        } else if ( pid > 0 ) {
        } else {
            singleBlock(i0,j0,k0,blockSize,n);      
            exit(0);
        }      
      } 
    }
  }
for (i=0;i<kidsPerBlock*kidsPerBlock*kidsPerBlock;i++) wait(NULL);
}



int main (int argc, char *argv[])
{

   printf("use command line argument, arg 1 =n arg 2 = blockSize\n");
   double **a, **b, **c;
   int i, j, n, blockSize;
 //  double **a, **b, **c;
   n = atoi(argv[1]);
   blockSize = atoi(argv[2]);
   printf("n=%d\n",n);
   printf("blockSize=%d\n",blockSize);
   
    
   //Populate arrays....
     a= (double**)malloc(n*sizeof(double));
     b= (double**)malloc(n*sizeof(double));
     c= (double**)malloc(n*sizeof(double));
     for (i=0; i<n; i++)
     {
       a[i]= malloc(sizeof(double)*n);
       b[i]= malloc(sizeof(double)*n);
       c[i]= malloc(sizeof(double)*n);
      }

     for (i=0; i<n; i++)
     {
        for (j=0; j<n; j++)
         a[i][j]=8;
      }

     for (i=0; i<n; i++)
     {
        for (j=0; j<n; j++)
          b[i][j]=7;
      }

      printf("\nregular matrix multiplication:\n");
      start = ftime();
      multiply (a,b,c,n);
      stop = ftime();
      used = stop - start;
      mf = (n*n*n *2.0) / used / 1000000.0;
      printf ( "Elapsed time:   %10.2f \n", used);
      printf ( "DP MFLOPS:       %10.2f \n", mf);
      print(c,n);	
//time transpose calculation
      printf("\nTranspose:\n");
      b = transpose(b,n);
      start = ftime();
      multiplyT (a,b,c,n);
      stop = ftime();
      used = stop - start;
      mf = (n*n*n *2.0) / used / 1000000.0;
      printf ( "Elapsed time:   %10.2f \n", used);
      printf ( "DP MFLOPS:       %10.2f \n", mf);	
      print(c,n);
//block multiplication
      printf("\nblock multiplication\n");
      start = ftime();
      blockMultiply (a,b,c,n,blockSize);
      stop = ftime();
      used = stop - start;
      mf = (n*n*n *2.0) / used / 1000000.0;
      printf ( "Elapsed time:   %10.2f \n", used);
      printf ( "DP MFLOPS:       %10.2f \n", mf);
      print(c,n);
   //
      printf("\nMulti-Process block\n");
      start = ftime();
      multiProcess(a,b,n,blockSize);      
      stop = ftime();
      used = stop - start+max;
      mf = (n*n*n *2.0) / used / 1000000.0;
      printf ( "Elapsed time:   %10.2f \n", used);
      printf ( "DP MFLOPS:       %10.2f \n", mf);
      sharedPrint(mem_c,n);
return (0);
}
