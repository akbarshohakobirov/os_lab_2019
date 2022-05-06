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
  int timeout = -1;
  int pnum = -1;
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
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            // error handling
            if (seed <= 0) {
                printf("seed must be a positive number\n");
                return 1;
              }
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            // error handling
             if (array_size <= 0) {
                printf("array_size must be a positive number\n");
                return 1;
              }
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            // error handling
             if (pnum <= 0) {
                printf("pnum must be a positive number\n");
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
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" --timeout \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);
 
    int pipefd[2];
  //  open pipe connection
    pipe(pipefd);
       int part= array_size/pnum;
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      //  child proccess runtime
      if (child_pid == 0) {
        // child process
        struct MinMax tmp;
        if(i!=pnum-1)
        {
        tmp=GetMinMax(array,i*part,(i+1)*part);
        // parallel somehow
        }
        else tmp=GetMinMax(array,i*part,array_size);
        printf("PIPE %d \t\t MIN %d  \t\t MAX %d \n", i, tmp.min, tmp.max);
        if (with_files) {
          // use files here
          FILE* fp = fopen("ans.txt", "a");
          fwrite(&tmp, sizeof(struct MinMax), 1, fp);
          fclose(fp);
        } else {
           write(pipefd[1],&tmp,sizeof(struct MinMax));
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
    //  close unused write end
    close(pipefd[1]);
    //  closing child proccesses
    wait(NULL);
    //  child proccess will never get to this point
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    struct MinMax mm;
    
    if (with_files) {
      // read from files
       FILE* fp = fopen("ans.txt", "rb");
        fseek(fp, i*sizeof(struct MinMax), SEEK_SET);
        fread(&mm, sizeof(struct MinMax), 1, fp);
        fclose(fp);
    } else {
        read(pipefd[0], &mm, sizeof(struct MinMax));
    }

    if (mm.min < min_max.min) min_max.min = mm.min;
    if (mm.max > min_max.max) min_max.max = mm.max;
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
  remove("ans.txt");
  return 0;
}