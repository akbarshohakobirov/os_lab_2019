//Как пример дедлока рассмотрим задачу обедающих философов. Эта задача была придумана Дейкстрой 
//и сформулирована в современном виде Хоаром для описания типичных проблем многопоточной среды.

//За круглым столом сидят философы (для определённости их пять), пронумерованные против часовой 
//стрелки. На столе лежат 5 вилок. Каждый философ должен взять две вилки (левую от себя и правую 
//от себя), пообедать и положить их обратно. Проблема в том, что философы берут вилки по очереди, 
//поэтому после того, как один взял левую вилку, правая, к примеру, уже может находиться в руках 
//второго философа. Тогда первый вынужден будет ждать, когда второй пообедает.

//Пусть первый уже взял левую вилку. Второй, третий, четвёртый и пятый поступили также. Левая 
//вилка у второго философа является правой у первого, поэтому первый не может взять правую. 
//Философ будет бесконечно ожидать правую вилку, также будут ожидать и все остальные философы. 
//Мы получим дедлок, в котором пять потоков взаимно ожидают разблокировки ресурсов.

//Весь код: пусть философы едят бесконечно и ждут случайное время.
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
    #include <conio.h>
    #include <Windows.h>
    #define Sleep(X) Sleep(X)
    #define wait() _getch()
#else
    #include <unistd.h>
    #define Sleep(X) sleep(X)
    #define wait() scanf("1")
#endif
#define PHT_SIZE 5

//Объявим структуру «философ», которая будет хранить имя философа и 
//номера вилок, которые он может взять.
typedef struct philosopher_tag {
    const char *name;
    unsigned left_fork;
    unsigned right_fork;
} philosopher_t;

//Далее структура «стол», которая состоит их массива вилок. В качестве 
//вилки будет выступать мьютекс. Блокировка мьютекса означает «взять вилку», 
//а разблокировка «положить её обратно».
typedef struct table_tag {
    pthread_mutex_t forks[PHT_SIZE];
} table_t;

//Кроме того, для передачи этих параметров в поток объединим их в структуру
typedef struct philosopher_args_tag {
    const philosopher_t *philosopher;
    const table_t *table;
} philosopher_args_t;

//Две вспомогательные функции инициализации
void init_philosopher(philosopher_t *philosopher, const char *name, unsigned left_fork, unsigned right_fork) {
    philosopher->name = name;
    philosopher->left_fork = left_fork;
    philosopher->right_fork = right_fork;
}
void init_table(table_t *table) {
    size_t i;
    for (i = 0; i < PHT_SIZE; i++) {
        pthread_mutex_init(&table->forks[i], NULL);
    }
}

//И наша основная функция, eat, которая моделирует обед. Для эмуляции дедлока 
//замедлим процесс подъёма вилки со стола: добавим sleep

void* eat(void *args) {
    philosopher_args_t *arg = (philosopher_args_t*) args;
    const philosopher_t *philosopher = arg->philosopher;
    const table_t *table = arg->table;
 
    printf("%s started dinner\n", philosopher->name);
 
    pthread_mutex_lock(&table->forks[philosopher->left_fork]);
    Sleep(1000);
    pthread_mutex_lock(&table->forks[philosopher->right_fork]);   
     
    printf("%s is eating\n", philosopher->name);
     
    pthread_mutex_unlock(&table->forks[philosopher->right_fork]);
    pthread_mutex_unlock(&table->forks[philosopher->left_fork]);
 
    printf("%s finished dinner\n", philosopher->name);
}

void main() {
    pthread_t threads[PHT_SIZE];
 
    philosopher_t philosophers[PHT_SIZE];
    philosopher_args_t arguments[PHT_SIZE];
    table_t table;
 
    size_t i;
 
    init_table(&table);
 
    init_philosopher(&philosophers[0], "Alice", 0, 1);
    init_philosopher(&philosophers[1], "Bob", 1, 2);
    init_philosopher(&philosophers[2], "Clark", 2, 3);
    init_philosopher(&philosophers[3], "Denis", 3, 4);
    init_philosopher(&philosophers[4], "Eugin", 4, 0);
 
    for (i = 0; i < PHT_SIZE; i++) {
        arguments[i].philosopher = &philosophers[i];
        arguments[i].table = &table;
    }
 
    for (i = 0; i < PHT_SIZE; i++) {
        pthread_create(&threads[i], NULL, eat, &arguments[i]);
    }
 
    for (i = 0; i < PHT_SIZE; i++) {
        pthread_join(threads[i], NULL);
    }
 
    wait();
}