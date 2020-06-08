#include <unistd.h>
#include <stdlib.h>



int main(int argc, char *argv[]) {

    int *data = malloc (100 * sizeof(int));
    data[100] = 0;

    free(data + 40);
    
    return 0;
}