#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<stdint.h>

const int FRAMES = 256;
const int TABLE_PAGES_INPUTS = 256;
const int SIZE_PAGE = 256;
const int MASK = 255;
const int SIZE_PHISIC_MEMORY = 256 * 256;
const int FRAMES_ENTER = 256;
const char *BACKING = "BACKING_STORE.bin";

int main(int argc, char* argv[]){

  errno = 0;

	// Validation of number of arguments
  if(argc < 3){
    errno = EINVAL;
    perror("Arguments");
    printf("Use of the program: %s <file path logical addresses> <output file path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

	// Files
	FILE *logic_directions, *output_file;
	int backing_store;
	
	//Variables 
	uint32_t logical_address, physical_address;
	uint8_t no_page, offset;
  char value;

	// To read the file
	char *line = NULL;
	size_t len = 0;
	ssize_t read_bytes;

	// Table pages
	int table_pages[TABLE_PAGES_INPUTS];
	memset(table_pages, -1, TABLE_PAGES_INPUTS * sizeof(int));

	int free_frames[FRAMES_ENTER];
	memset(free_frames, -1, FRAMES_ENTER * sizeof(int));

	// Physical memory of 256x256 bytes
	char physical_memory[SIZE_PHISIC_MEMORY]; 

	// Open files
	output_file = fopen(argv[2], "w");

	logic_directions = fopen(argv[1], "r");
  if(logic_directions == NULL){
    perror(argv[1]);
    exit(EXIT_FAILURE);
  }

	backing_store = open(BACKING, O_RDONLY);
  if(backing_store == -1){
    perror(BACKING);
    exit(EXIT_FAILURE);
  }

	// Read address file line by line
	while((read_bytes = getline(&line, &len, logic_directions)) != -1){ 

    logical_address = (uint32_t)atoi(line);
    offset = logical_address & MASK;
    no_page = (logical_address >> 8) & MASK;


    if(table_pages[no_page] != -1){

      // Se encuentra la pagina cargada
      physical_address = (table_pages[no_page] * SIZE_PAGE) + offset;
      value = physical_memory[physical_address];

    }else{

      // Fallo de pagina
      int i;
      for(i = 0; i < FRAMES_ENTER; i++){
        if(free_frames[i] == -1) break;
      }

      table_pages[no_page] = i;
      free_frames[i] = 1;

      if(lseek(backing_store, no_page * SIZE_PAGE, SEEK_SET) == -1){
        perror("moving cursor");
        exit(EXIT_FAILURE);
      }

      for(int j = 0; j < SIZE_PAGE; j++)
        read(backing_store, &physical_memory[(i * SIZE_PAGE) + j], sizeof(char));

      physical_address = (i * SIZE_PAGE) + offset;
      value = physical_memory[physical_address];

    }

		printf("Virtual address: %u Physical address: %u Value: %d\n", logical_address, physical_address, value);
    fprintf(output_file, "Virtual address: %u Physical address: %u Value: %d\n", logical_address, physical_address, value);

	}

	fclose(output_file);
	fclose(logic_directions);

	close(backing_store);

	return EXIT_SUCCESS;
}

