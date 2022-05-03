#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

void Killing(int sig){
    kill(-1, SIGKILL);
    printf("TIMEOUT\n");
}

int main(int argc, char **argv) {
      int seed = -1;
      int array_size = -1;
      int pnum = -1;
      int timeout = -1;
      bool with_files = false;
    
      while (true) {
        int current_optind = optind ? optind : 1;
    
        static struct option options[] = {{"seed", required_argument, 0, 0},
                                          {"array_size", required_argument, 0, 0},
                                          {"pnum", required_argument, 0, 0},
                                          {"by_files", no_argument, 0, 'f'},
                                          {"timeout", required_argument, 0, 0},
                                          {0, 0, 0, 0}};
    
        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index); //обработка опций, вводимых с командой
    
        if (c == -1) break;
    
        switch (c) {
          case 0:
            switch (option_index) {
              case 0:
                seed = atoi(optarg);
                if (seed <= 0) { //проверка семени на положительность
                    printf("seed is a positive number\n");
                    return 1;
                }
                break;
              case 1:
                array_size = atoi(optarg);
                if (array_size <= 0) { //проверка размера массива на положительность
                    printf("array_size is a positive number\n");
                    return 1;
                }
                break;
              case 2:
                pnum = atoi(optarg);
                if (pnum <= 0) { //проверка размера массива на положительность
                    printf("pnum is a positive number\n");
                    return 1;
                }
                break;
              case 3:
                with_files = true;
                break;
                
              case 4:
                timeout = atoi(optarg);
                if (timeout <= 0){
                    printf("timeout must be a positive number\n");
                    return 1;
                }
                break;
    
              defalut:
                printf("Index %d is out of options\n", option_index);
            }
            break;
          case 'f':
            with_files = true;
            break;
    
          case '?':
            break;
          
            
          default:
            printf("getopt returned character code 0%o?\n", c);
        }
      }
    
      if (optind < argc) {
        printf("Has at least one no option argument\n");
        return 1;
      }
    
      if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
               argv[0]);
        return 1;
      }
    
      int *array = malloc(sizeof(int) * array_size);
      GenerateArray(array, array_size, seed);
     /* for (int i=0; i<array_size; i++)
        printf("%d\t",array[i]);
      printf("\n");*/
      int active_child_processes = 0;
    
      struct timeval start_time;
    gettimeofday(&start_time, NULL);
  
    int pipefd[2];
    pipe(pipefd);
    
    int sub_len= array_size/pnum;
    
    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork(); //создание дочернего процесса
        if (child_pid >= 0) { //проверка успешности создания дочернего процесса
          // successful fork
          active_child_processes += 1;
          if (child_pid == 0) { //потомок читает из канала
            struct MinMax min_max;
            if (i!=pnum-1){
                min_max = GetMinMax(array, i*sub_len, (i+1)*sub_len); //определение максимума 
             //и минимума подмассива
            }
            else 
                min_max=GetMinMax(array,i*sub_len,array_size);
            if (with_files) { //записываем в файл
                FILE * fp;
                fp = fopen ("file.txt", "a");
                if (fp==0){
                    printf( "Could not open file\n" );
                    return 1;
                }
                else{
                    fwrite(&min_max, sizeof(struct MinMax), 1, fp);
                }
                fclose (fp);
              
            } else { //записываем в канал
                    write(pipefd[1],&min_max,sizeof(struct MinMax));
            }
            return 0;
          }
    
        } else {
          printf("Fork failed!\n");
          return 1;
        }
    }


    if(timeout>0)
    {
        signal(SIGALRM, Killing);
        alarm(timeout);
    }
    
  while (active_child_processes > 0) {
    close(pipefd[1]); // Закрываем неиспользуемый конец для записи
    
    wait(NULL); //Ждем потомка

    active_child_processes -= 1;
  }
  


  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    struct MinMax Min_Max;

    if (with_files) { //считываем из файла
        FILE* fp = fopen("file.txt", "rb");
        if (fp==0){
            printf( "Could not open file\n" );
            return 1;
        } else{
            fseek(fp, i*sizeof(struct MinMax), SEEK_SET);
            fread(&Min_Max, sizeof(struct MinMax), 1, fp); 
        }
        fclose(fp);
        
    } else { //считываем из каналов
        read(pipefd[0], &Min_Max, sizeof(struct MinMax));
    }

    if (Min_Max.min < min_max.min) min_max.min = Min_Max.min;
    if (Min_Max.max > min_max.max) min_max.max = Min_Max.max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  remove("file.txt");
  return 0;
}