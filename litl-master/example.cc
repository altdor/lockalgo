#include <iostream>
#include <pthread.h>
                                                 

int main(int argc, char **argv)
{
        static pthread_rwlock_t rwlock, rwlock1;
        pthread_rwlock_init(&rwlock, NULL);
        pthread_rwlock_wrlock(&rwlock);

        pthread_rwlock_init(&rwlock1, NULL);
        pthread_rwlock_wrlock(&rwlock1);
       
       //segment fault at following line
        pthread_rwlock_unlock(&rwlock);
        pthread_rwlock_unlock(&rwlock1);

        pthread_rwlock_destroy(&rwlock);
        pthread_rwlock_destroy(&rwlock1);

        std::cout<<"done"<<std::endl;
        return 0;
}