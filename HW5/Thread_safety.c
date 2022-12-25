void Tokenize( 
    char* lines[]    /* in/out */,
    int line_count   /* in     */,
    int thread_count /* in     */){
  
  int my_rank,i,j;
  char * my_token;
# pragma omp parallel num_threads(thread_count) default(none) private(my_rank, i, j, my_token) shared (lines, line_count)
  {
    my_rank = omp_get_thread_num();
#   pragma omp for schedule(static , 1)
    for(i = 0; i < line_count;i++){
      printf("Thread %d > line %d = %s", my_rank, i, lines[i]);
      j = 0;
      my_token = strtok(lines[i], " \t\n");
      while(my_token != NULL){
        printf("Thread %d > token %d = %s\n", my_rank, j, my_token);
        my_token = strtok(NULL, " \t\n");
        j++;
      }
    }
  }

}
