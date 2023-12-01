
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define Name_Size 50
sem_t mutex;


typedef struct Resource {
    char type[10];
    char name[Name_Size];
    int amount;
}Resource;
Resource* resources;
int res_length = 0;

typedef struct Service {
    char type[10];
    char name[Name_Size];
    int time;
    int listLength;
    int* list;
}Service;
Service* services;
int serv_length = 0;

typedef struct Request {
    int waiting_for_service; // we will use this later
    char number[10];
    int hour;
    int listLength;
    int* list;
}Request;
Request* requests;
int req_length = 0;

void get_resources(char* filename); //* get all the input from the files using those 3 functions **/
void get_services(char* filename);
void get_requests(char* filename);

int timer = 0; /// a global timer 
void* start_time();

void start_garage();

void* car_request(void* i);
int get_service_index(int id);
int isAvailable(int id);
void start_resource(int id);
void give_resource(int id);

void free_requests();
void free_sources();
void free_services();


int main(int argc, char* argv[]) {
    /// get all the inputs from the files.
    get_resources("resources.txt");
    get_services("services.txt");
    get_requests("requests.txt");
    sem_init(&mutex, 0, 1); //** this mutex will manage the resources **/
    // create+ start the timer thread.
    pthread_t timer_thread;
    pthread_create(&timer_thread, NULL, start_time, NULL);
    //** after we got all the information and we started the timer , now we can start the garaage **/
    start_garage();

    free_services();
    free_requests();
    free(resources);
    return 0;

}

void get_resources(char* filename) {
    char buffer[1024];
    char s[3] = "\t";
    FILE* f;
    int i;
    f = fopen(filename, "rt");
    if (f == NULL) {
        printf("failed opening file\n");
        exit(1);
    }

    while ((fgets(buffer, 1024, f)) != NULL) {
        res_length++;
        resources = realloc(resources, res_length * sizeof(Resource));
        if (resources == NULL) {
            for (i = 0;i < res_length;i++)
                free(&(resources[i]));
            printf("Failed allocating memory.\n");
            exit(1);
        }
        char* token;
        token = strtok(buffer, s);
        strcpy(resources[res_length - 1].type, token);
        token = strtok(NULL, s);
        strcpy(resources[res_length - 1].name, token);
        token = strtok(NULL, s);
        resources[res_length - 1].amount = atoi(token);
    }

    fclose(f);
}
void get_services(char* filename) {
    char buffer[1024], * token;
    char s[3] = "\t";
    FILE* f;
    int i;
    f = fopen(filename, "rt");
    if (f == NULL) {
        printf("failed opening file\n");
        exit(1);
    }
    while ((fgets(buffer, 1024, f)) != NULL) {
        serv_length++;
        services = realloc(services, serv_length * sizeof(Service));
        if (services == NULL) {
            for (i = 0;i < serv_length;i++)
                free(&(services[i]));
            printf("Failed allocating memory.\n");
            exit(1);
        }
        token = strtok(buffer, s);
        strcpy(services[serv_length - 1].type, token);
        token = strtok(NULL, s);
        strcpy(services[serv_length - 1].name, token);
        token = strtok(NULL, s);
        services[serv_length - 1].time = atoi(token);
        token = strtok(NULL, s);
        services[serv_length - 1].listLength = atoi(token);
        if (services[serv_length - 1].listLength > 0) {
            if ((services[serv_length - 1].list = (int*)malloc(sizeof(int) * services[serv_length - 1].listLength)) == NULL) //*  allocating memory to save the list of resources **/
            {
                printf("Failed allocating memory.\n");
                exit(1);
            }
            for (i = 0;i < services[serv_length - 1].listLength;i++) {
                token = strtok(NULL, s);
                services[serv_length - 1].list[i] = atoi(token);
            }
        }
    }
    fclose(f);
}
void get_requests(char* filename) {
    char buffer[1024], * token;
    char s[3] = "\t";
    FILE* f;
    int i;
    f = fopen(filename, "rt");
    if (f == NULL) {
        printf("failed opening file\n");
        exit(1);
    }
    while ((fgets(buffer, 1024, f)) != NULL) {
        req_length++;
        requests = realloc(requests, req_length * sizeof(Request));
        if (requests == NULL) {
            for (i = 0;i < req_length;i++)
                free(&(requests[i]));
            printf("Failed allocating memory.\n");
            exit(1);
        }
        requests[req_length - 1].waiting_for_service = 0; // for all cars we init to 0.
        token = strtok(buffer, s);
        strcpy(requests[req_length - 1].number, token);
        token = strtok(NULL, s);
        requests[req_length - 1].hour = atoi(token);
        token = strtok(NULL, s);
        requests[req_length - 1].listLength = atoi(token);
        if (requests[req_length - 1].listLength > 0) {
            if ((requests[req_length - 1].list = (int*)malloc(sizeof(int) * requests[req_length - 1].listLength)) == NULL) {  // allocating memory to save the list of services //
                printf("Failed allocating memory.\n");
                exit(1);
            }
            for (i = 0;i < requests[req_length - 1].listLength;i++) {
                token = strtok(NULL, s);
                requests[req_length - 1].list[i] = atoi(token);
            }
        }
    }
    fclose(f);
}

void* start_time() { //we have to increase the timer by 1 hour every 1 second..
    while (1) {
        sleep(1);
        timer++;
    }
}

void start_garage() {
    // ** creating threads for every request 
   //* so they can run all in the same timee
    pthread_t* request_threads = (pthread_t*)malloc(sizeof(pthread_t) * req_length);
    int i;
    if (request_threads == NULL) {
        printf("Error allocating memory.\n");
        exit(1);
    }
    //** creating array of integers to get the index of the specific thread .
    int* request_num = (int*)malloc(sizeof(int) * req_length);
    if (request_num == NULL) {
        printf("Error allocating memory.\n");
        exit(1);
    }
    for (i = 0;i < req_length;i++)
        request_num[i] = i;

    int started = 0; // this variable will tell us how many services started already.
    while (started != req_length) {
        for (i = 0;i < req_length;i++) {

            if ((timer >= requests[i].hour) && (requests[i].waiting_for_service == 0)) {
                requests[i].waiting_for_service = 1;
                printf("car: %s time: %d arrived to the garage\n", requests[i].number, timer);  /// the car arrived to our garage.
                pthread_create(&request_threads[i], NULL, car_request, (void*)&request_num[i]); /// calling car_request function for every request.
                started++;
            }
        }
    }
    // wait for all the requests to finished
    for (i = 0;i < req_length;i++) {
        pthread_join(request_threads[i], NULL);
    }
    //* free allocated memory.
    free(request_num);
    free(request_threads);
}

void* car_request(void* i) {
    int j, k;
    int car = *(int*)i;
    int services_left = requests[car].listLength;
    int index, ready = 0;

    printf("car: %s time: %d service request started.\n", requests[car].number, timer); // the car service request has started.
    while (services_left > 0) {                                /// looping untill we have done all the services for this request.
        for (j = 0;j < requests[car].listLength;j++) {
            if (requests[car].list[j] != -1) {
                index = get_service_index(requests[car].list[j]);
                if (index == -1) {
                    printf("sorry, this service (%d) is not exist.\n", requests[car].list[j]);
                    requests[car].list[j] = -1; // so we know we finished with it.
                    services_left--;
                }
                //wait untill we can use recources.
                sem_wait(&mutex);
                //count how many sources are availble.
                ready = 0; // 
                for (k = 0;k < services[index].listLength;k++) {
                    if (isAvailable(services[index].list[k]))
                        ready++;
                }
                if (ready == services[index].listLength) { // if so , so we can start the service for this car
                    printf("car: %s time: %d started %s.\n", requests[car].number, timer, services[index].name);
                    //start sources
                    for (k = 0;k < services[index].listLength;k++)
                        start_resource(services[index].list[k]);
                    sem_post(&mutex); // so other request can use recources when we are doing this service
                    sleep(services[index].time); //sleep for the time that we need for this service.
                    sem_wait(&mutex); // service is finished.
                    for (k = 0;k < services[index].listLength;k++) ///////////prooobb
                        give_resource(services[index].list[k]);

                    printf("car: %s time: %d completed %s.\n", requests[car].number, timer, services[index].name);

                    requests[car].list[j] = -1; // we finished this one
                    services_left--;
                }
                sem_post(&mutex);
            }
        }
    }
    //we finished with this car.
    printf("car: %s time: %d service completed\n", requests[car].number, timer);
    return NULL;
}

int get_service_index(int id) {
    int i;
    for (i = 0;i < serv_length;i++) {
        if (atoi(services[i].type) == id)
            return i;
    }
    return -1; // we dont have this service
}

int isAvailable(int id) {
    int i;
    for (i = 0;i < res_length;i++) {
        if (atoi(resources[i].type) == id) {
            if (resources[i].amount > 0)
                return 1;
        }
    }
    return 0;
}

void start_resource(int id) {
    int i;
    for (i = 0;i < res_length;i++) {
        if (atoi(resources[i].type) == id) {
            resources[i].amount--;
            return;
        }
    }
}

void give_resource(int id) {
    int i;
    for (i = 0;i < res_length;i++) {
        if (atoi(resources[i].type) == id) {
            resources[i].amount++;
            return;
        }
    }
}
void free_services() {
    int i;
    for (i = 0;i < serv_length;i++) {
        free(services[i].list);
    }
    free(services);
}


void free_requests() {
    int i;
    for (i = 0;i < req_length;i++) {
        free(requests[i].list);
    }
    free(requests);
}