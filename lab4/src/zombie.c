#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(){
    pid_t child_pid;
    /* Создание дочернего процесса. */
    child_pid = fork();
    if (child_pid > 0) {
        /* Это родительский процесс, делаем минутную паузу. */
        sleep(60);
        } else {
        /* Это дочерний процесс, немедленно завершаем работу. */
        exit(0);
    }
    return 0;
}

//После вводим ps -al, чтобы убедиться в наличии зомби-процесса