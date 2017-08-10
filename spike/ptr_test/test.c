#include <stdio.h> 
#include <stdlib.h>
typedef struct {
   time_t time_stamp;
   double data;
} sensor_data;
void Get_Data(sensor_data **raw_data,int *size){
    
    (*size) = 0;
    *raw_data = malloc(sizeof(sensor_data) * ((*size) + 
1));
    
    (*raw_data)[(*size)].data = 1.5;
    
    (*size)++;
    *raw_data = realloc(*raw_data, sizeof(sensor_data) * 
((*size) + 1));
    
    (*raw_data)[(*size)].data = 2.5;
    
    (*size)++;
    
}
int main() {
    
    sensor_data *raw_data;
    int size = 0;
    
    Get_Data(&raw_data,&size);
    
    printf("size=%d\n",size);
    int i = 0;
    for( i = 0; i < size; i++)
    {
        printf("data=%f\n",raw_data[i].data);
    }
    
    free(raw_data);
    
    return 0;
}
