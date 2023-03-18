#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#define PIPE_RESP "RESP_PIPE_75199"
#define PIPE_REQ "REQ_PIPE_75199"

int main(){
	int fd_write, fd_read, shmFd, fd_file;
	char *file;
	int file_size;
	char* mem;

	if(mkfifo(PIPE_RESP, 0600) != 0) {
        perror("ERROR\n");
        return 1;
    }
    
    fd_read = open(PIPE_REQ, O_RDONLY);
    if(fd_read == -1) {
        perror("ERROR\n");
        return 1;
    }
    
    fd_write = open(PIPE_RESP, O_WRONLY);
    if(fd_write == -1) {
        perror("ERROR\n");
        return 1;
    }

    char c = 7;
    write(fd_write, &c, 1);
    write(fd_write, "CONNECT",7);
    printf("SUCCESS\n");
    
    char size;
    char request[30];
    while(1){
    	read(fd_read,&size,1);
    	read(fd_read,request,size);
    	request[(int)size] = '\0';
    	if (strcmp(request,"PING") == 0){
    		unsigned int nr = 75199;
    		write(fd_write,&size,1);
    		write(fd_write,"PING",4);
    		write(fd_write,&size,1);
    		write(fd_write,"PONG",4);
    		write(fd_write,&nr,sizeof(nr));
    	}
    	if (strcmp(request,"CREATE_SHM") == 0){
    		unsigned int nr;
    		read(fd_read,&nr,sizeof(nr));
    		write(fd_write,&size,1);
    		write(fd_write,"CREATE_SHM",10);
    		shmFd = shm_open("/KJaM5t0",O_CREAT | O_RDWR, 0644);
    		if (shmFd < 0){
    			int c = 5;
    			write(fd_write,&c,1);
    			write(fd_write,"ERROR",5);
    		}
    		ftruncate(shmFd,nr);
    		int c = 7;
    		write(fd_write,&c,1);
    		write(fd_write,"SUCCESS",7);
    	}
    	if (strcmp(request, "WRITE_TO_SHM") == 0){
    		unsigned int offset,value;
    		read(fd_read,&offset,sizeof(offset));
    		read(fd_read,&value,sizeof(value));
    		write(fd_write,&size,1);
    		write(fd_write,"WRITE_TO_SHM",12);
    		if (offset >= 0 && offset <= 4827897){
    			unsigned char*buffer = (unsigned char*)&value;
    			if (offset + strlen((char*)buffer) < 4827897){
    				mem = (char*)mmap(0,4827897,PROT_READ | PROT_WRITE,MAP_SHARED,shmFd,0);
    				if (mem == (void*)-1){
    					int c = 5;
    					write(fd_write,&c,1);
    					write(fd_write,"ERROR",5);
    				}
    				for (int i = 0; i < strlen((char*)buffer) ; i++){
    					mem[offset+i] = buffer[i];
    				}
    				int c = 7;
    				write(fd_write,&c,1);
    				write(fd_write,"SUCCESS",7);
    			} else {
    				int c = 5;
    				write(fd_write,&c,1);
    				write(fd_write,"ERROR",5);
    			}
    		} else {
    			int c = 5;
    			write(fd_write,&c,1);
    			write(fd_write,"ERROR",5);
    		}
    	}
    	if (strcmp(request, "MAP_FILE") == 0){
    		char file_name_size;
    		char file_name[20];
    		read(fd_read,&file_name_size,1);
    		read(fd_read,file_name,file_name_size);
    		
    		write(fd_write,&size,1);
    		write(fd_write,"MAP_FILE",8);
    		
    		fd_file = open(file_name,O_RDONLY);
    		file_size = lseek(fd_file,0,SEEK_END);
    		lseek(fd_file,0,SEEK_SET);
    		
    		file = (char*)mmap(NULL,file_size,PROT_READ,MAP_SHARED,fd_file,0);
    		if (file == (void*)-1) {
    			int c = 5;
    			write(fd_write,&c,1);
    			write(fd_write,"ERROR",5);
    		}
    		int c = 7;
    		write(fd_write,&c,1);
    		write(fd_write,"SUCCESS",7);
    		//close(fd_file);
    	}
    	if (strcmp(request, "READ_FROM_FILE_OFFSET") == 0){
			unsigned int no_of_bytes, offset;
			read(fd_read,&offset,sizeof(offset));
    		read(fd_read,&no_of_bytes,sizeof(no_of_bytes));
    		write(fd_write,&size,1);
    		write(fd_write,"READ_FROM_FILE_OFFSET",21);
    		
    		if (offset + no_of_bytes < file_size){
    			mem = (char*)mmap(0,4827897,PROT_READ | PROT_WRITE,MAP_SHARED,shmFd,0);
    			for (int i = 0; i < no_of_bytes; i++){
    				mem[i] = file[offset + i];
    			}
    			int c = 7;
    			write(fd_write,&c,1);
    			write(fd_write,"SUCCESS",7);
			} else {
				int c = 5;
    			write(fd_write,&c,1);
    			write(fd_write,"ERROR",5);
			}
			    	
    	}
    	if (strcmp(request, "READ_FROM_FILE_SECTION") == 0){
    		unsigned int section_no, offset, no_of_bytes;
    		read(fd_read,&section_no,sizeof(section_no));
    		read(fd_read,&offset,sizeof(offset));
    		read(fd_read,&no_of_bytes,sizeof(no_of_bytes));
    		write(fd_write,&size,1);
    		write(fd_write,"READ_FROM_FILE_SECTION",22);
    		if (file[0] == 'j' && file[1] == 'M'){
    			char version_c[5];
    			version_c[0] = file[4]; version_c[1] = file[5]; version_c[2] = file[6];
    			version_c[3] = file[7]; version_c[4] = '\0';
    			int version = *((int*)version_c);
    			if (version >= 126 && version <= 239){
					if(file[8] >= 6 && file[8] <= 11){
						if (section_no <= file[8]){
							int pos = 9 + 23*(section_no - 1);
							pos = pos + 13;
							if (file[pos] == 91 || file[pos] == 24 || file[pos] == 53){
								pos += 2;
								char offset_c[5];
								offset_c[0] = file[pos]; offset_c[1] = file[pos+1];
								offset_c[2] = file[pos + 2]; offset_c[3] = file[pos+3]; 							offset_c[4] = '\n';
								int offset_file = *((int*)offset_c);
								mem = (char*)mmap(0,4827897,PROT_READ | PROT_WRITE,MAP_SHARED,shmFd,0);
								for (int i = 0; i < no_of_bytes; i++){
									mem[i] = file[offset_file + offset + i];
								}
								int c = 7;
								write(fd_write,&c,1);
								write(fd_write,"SUCCESS",7);
							} else {
								int c = 5;
								write(fd_write,&c,1);
								write(fd_write,"ERROR",5);
							}
							
						} else{
							int c = 5;
							write(fd_write,&c,1);
							write(fd_write,"ERROR",5);
						}
					} else{
						int c = 5;
						write(fd_write,&c,1);
						write(fd_write,"ERROR",5);
					}
				} else{
					int c = 5;
					write(fd_write,&c,1);
					write(fd_write,"ERROR",5);
				}
    		} else{
    			int c = 5;
    			write(fd_write,&c,1);
    			write(fd_write,"ERROR",5);
    		}
    	}
    	if (strcmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0){
    		unsigned int logical_offset, no_of_bytes;
    		read(fd_read,&logical_offset,sizeof(logical_offset));
    		read(fd_read,&no_of_bytes,sizeof(no_of_bytes));
    		write(fd_write,&size,1);
    		write(fd_write,"READ_FROM_LOGICAL_SPACE_OFFSET",30);
    		if (file[0] == 'j' && file[1] == 'M'){
    			char version_c[5];
    			version_c[0] = file[4]; version_c[1] = file[5]; version_c[2] = file[6];
    			version_c[3] = file[7]; version_c[4] = '\0';
    			int version = *((int*)version_c);
    			if (version >= 126 && version <= 239){
					if(file[8] >= 6 && file[8] <= 11){
						int section_size[file[8]];
						int section_offset[file[8]];
						int sections_needed[file[8]];
						int pos = 9;
						for (int i = 0; i < file[8]; i++){
							if (i == 0)
								pos += 15;
							else
								pos += 19;
							char offset_c[5];
    						offset_c[0] = file[pos]; offset_c[1] = file[pos+1]; 
    						offset_c[2] = file[pos+2]; offset_c[3] = file[pos+3]; 
    						offset_c[4] = '\0';
    						section_offset[i] = *((int*)offset_c);
    						pos += 4;
    						
							char size_c[5];
    						size_c[0] = file[pos]; size_c[1] = file[pos+1]; 
    						size_c[2] = file[pos+2]; size_c[3] = file[pos+3]; size_c[4] = '\0';
    						section_size[i] = *((int*)size_c);
    						sections_needed[i] = (int)(section_size[i] / 5120) + 1;
						} 
						mem = (char*)mmap(0,4827897,PROT_READ | PROT_WRITE,MAP_SHARED,shmFd,0);
						
						for (int i = 0; i < no_of_bytes; i++){
							int address;
								int j;
								int var = 0;
								for (j = 0; j < file[8]; j++){
									if (5120 * sections_needed[j] + var >= logical_offset)
										break;
									var += 5120 * sections_needed[j];
								}
								int sum = 0;
								for (int k = 0; k < j; k++){
									sum += sections_needed[k]*5120;
								}
								address = section_offset[j] + i + logical_offset - sum;
							mem[i] = file[address];
						}
					} else{
    					int c = 5;
    					write(fd_write,&c,1);
    					write(fd_write,"ERROR",5);
    				}
				} else{
    				int c = 5;
    				write(fd_write,&c,1);
    				write(fd_write,"ERROR",5);
    			}
			} else{
    			int c = 5;
    			write(fd_write,&c,1);
    			write(fd_write,"ERROR",5);
    		}
    		
    		int c = 7;
    		write(fd_write,&c,1);
    		write(fd_write,"SUCCESS",7);
    		
    	}
    	if (strcmp(request, "EXIT") == 0){
    		munmap(file,file_size);
    		munmap(mem,4827897);
    		close(shmFd);
    		close(fd_file);
    		close(fd_write);
    		close(fd_read);

    		unlink(PIPE_RESP);
    		return 0;
    	}
    }

	return 0;
}
