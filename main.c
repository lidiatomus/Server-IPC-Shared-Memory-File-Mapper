#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#define REQ_PIPE "REQ_PIPE_59592"
#define RESP_PIPE "RESP_PIPE_59592"
#define VARIANT 59592

void create_and_open_pipes(int *fd_readfile, int *fd_writefile)
{
    if(mkfifo(RESP_PIPE,0600) != 0)
    {
        printf("ERROR\ncannot create the response pipe");
        exit(1);
    }
    *fd_readfile = open(REQ_PIPE, O_RDONLY);
    if(*fd_readfile == -1)
    {
        printf("ERROR\ncannot open the request pipe");
        exit(1);
    }
    *fd_writefile = open(RESP_PIPE, O_WRONLY);
    if(*fd_writefile == -1)
    {
        printf("ERROR\ncannot open the response pipe");
        exit(1);
    }
    write(*fd_writefile, "BEGIN", strlen("BEGIN"));
    write(*fd_writefile, "!", strlen("!"));
    printf("SUCCES\n");
 }
 void echo(int fd_writefile)
 {
    int variant = VARIANT;
    write(fd_writefile, "ECHO", strlen("ECHO"));
    write(fd_writefile, "!", strlen("!"));
    write(fd_writefile, "VARIANT", strlen("VARIANT"));
    write(fd_writefile, "!", strlen("!"));
    write(fd_writefile, &variant, sizeof(int));
 }
void creat_shared_memory(int fd_readfile, int fd_writefile, int *shared_memory_fd, char **shared_memory)
{
    *shared_memory_fd = shm_open("/PHguy5D", O_CREAT | O_RDWR, 0644);
    if(*shared_memory_fd < 0)
    {
        write(fd_writefile, "CREATE_SHM", strlen("CREATE_SHM"));
        write(fd_writefile, "!", strlen("!"));
        write(fd_writefile, "ERROR", strlen("ERROR"));
        write(fd_writefile, "!", strlen("!"));
    }
    else
    {
      unsigned int size = 0;
      read(fd_readfile, &size, sizeof(unsigned int));
      ftruncate(*shared_memory_fd, size);
      *shared_memory = mmap(NULL,size, PROT_READ | PROT_WRITE, MAP_SHARED, *shared_memory_fd, 0);
      write(fd_writefile, "CREATE_SHM", strlen("CREATE_SHM"));
      write(fd_writefile, "!", strlen("!"));
      write(fd_writefile, "SUCCESS", strlen("SUCCESS"));
      write(fd_writefile, "!", strlen("!"));
    }
}

void write_to_shared_memory(int fd_readfile, int fd_writefile, char*shared_memory)
{
    if(shared_memory == NULL)
    {
      write(fd_writefile, "WRITE_TO_SHM", strlen("WRITE_TO_SHM"));
      write(fd_writefile, "!", strlen("!"));
      write(fd_writefile, "ERROR", strlen("ERROR"));
      write(fd_writefile, "!", strlen("!"));
    }
    else
    {
      unsigned int offset = 0;
      unsigned int value = 0;
      read(fd_readfile, &offset, 4);
      read(fd_readfile, &value, 4);
      if(offset >= 0 && offset <= 1825015 - 4)
        {
          *(unsigned int*)(shared_memory + offset) = value;
    		write(fd_writefile, "WRITE_TO_SHM", strlen("WRITE_TO_SHM"));
        write(fd_writefile, "!", strlen("!"));
		    write(fd_writefile, "SUCCESS", strlen("SUCCESS"));
        write(fd_writefile, "!", strlen("!"));
        }
        else
        {
          write(fd_writefile, "WRITE_TO_SHM", strlen("WRITE_TO_SHM"));
          write(fd_writefile, "!", strlen("!"));
          write(fd_writefile, "ERROR", strlen("ERROR"));
          write(fd_writefile, "!", strlen("!"));
        }
    }
}
void map_file(int writefile_fd, int readfile_fd,char **file_map, int *shared_memory_fd)
{
    int position =0;
    char nume_file[256] = "";
    size_t size_file;
    while(1)
    {
        read(readfile_fd, &nume_file[position], 1); //citim un byte
        if(nume_file[position] == '!')
          {
            nume_file[position]='\0';
            break;
          }
        position++;

    }

    *shared_memory_fd=open(nume_file,O_RDONLY);
    if(*shared_memory_fd == -1)
    {
      write(writefile_fd, "MAP_FILE", strlen("MAP_FILE"));
      write(writefile_fd, "!", strlen("!"));
      write(writefile_fd, "ERROR", strlen("ERROR"));
      write(writefile_fd, "!", strlen("!"));
    }
    else
    {
      struct stat st;
        if (fstat(*shared_memory_fd, &st) == -1) {
            write(writefile_fd, "MAP_FILE", strlen("MAP_FILE"));
            write(writefile_fd, "!", strlen("!"));
            write(writefile_fd, "ERROR", strlen("ERROR"));
            write(writefile_fd, "!", strlen("!"));
            return;
        }
      size_file = st.st_size;
      *file_map=mmap(NULL,size_file,PROT_READ, MAP_PRIVATE, *shared_memory_fd,0);
      if(*file_map == NULL)
      {
          write(writefile_fd, "MAP_FILE", strlen("MAP_FILE"));
          write(writefile_fd, "!", strlen("!"));
          write(writefile_fd, "ERROR", strlen("ERROR"));
          write(writefile_fd, "!", strlen("!"));
      }
      else{
          write(writefile_fd, "MAP_FILE", strlen("MAP_FILE"));
          write(writefile_fd, "!", strlen("!"));
          write(writefile_fd, "SUCCESS", strlen("SUCCESS"));
          write(writefile_fd, "!", strlen("!"));
      }
    }
}
void read_from_file_offset(int fd_read, int fd_write, int fd_shared_memory, char* shared_memory,char *file_map)
{
    if (file_map == NULL || shared_memory == NULL)
    {
        write(fd_write, "READ_FROM_FILE_OFFSET", strlen("READ_FROM_FILE_OFFSET"));
        write(fd_write, "!", strlen("!"));
        write(fd_write, "ERROR", strlen("ERROR"));
        write(fd_write, "!", strlen("!"));
    }
  else
  {
        unsigned int numar_bytes=0;
        unsigned int offset=0;
        read(fd_read, &offset,sizeof(unsigned int) );
        read(fd_read, &numar_bytes,sizeof(unsigned int));
        unsigned int pozitie = lseek(fd_shared_memory, 0, SEEK_SET); // POZITIA CURENTA -> OFFSET UL DE UNDE CITIM
        unsigned int size_file = lseek(fd_shared_memory, 0, SEEK_END); // NE MUTAM LA FINAL CA SA STIM MARIMEA intreaga
        lseek(fd_shared_memory, pozitie, SEEK_SET); // ne mutam de la final inapoi la offset, ala e pastrat in pozitie
        if(offset>=0 && offset+numar_bytes<size_file)
        {
             int nr_de_bytes_cititi = 0; // also pozitie in "sirul" de bytes ce trebuie citit si salvat in shared memmory
             while(nr_de_bytes_cititi < numar_bytes)
             {
               char byte_din_map = *(file_map + offset + nr_de_bytes_cititi);
               *(shared_memory+nr_de_bytes_cititi) = byte_din_map;
               nr_de_bytes_cititi++;
             }
            write(fd_write, "READ_FROM_FILE_OFFSET", strlen("READ_FROM_FILE_OFFSET"));
            write(fd_write, "!", strlen("!"));
            write(fd_write, "SUCCESS", strlen("SUCCESS"));
            write(fd_write, "!", strlen("!"));
        }
        else{
            write(fd_write, "READ_FROM_FILE_OFFSET", strlen("READ_FROM_FILE_OFFSET"));
            write(fd_write, "!", strlen("!"));
            write(fd_write, "ERROR", strlen("ERROR"));
            write(fd_write, "!", strlen("!"));
          }
  }
}

void read_from_file_section(int fd_read, int fd_write, int fd_shared_memory, char* shared_memory, char *file_map){
    if (file_map == NULL || shared_memory == NULL)
    {
        write(fd_write, "READ_FROM_FILE_SECTION", strlen("READ_FROM_FILE_SECTION"));
        write(fd_write, "!", strlen("!"));
        write(fd_write, "ERROR", strlen("ERROR"));
        write(fd_write, "!", strlen("!"));
        return;
    }
    else
    {
        unsigned int numar_bytes=0;
        unsigned int offset=0;
        unsigned int section_nr=0;
        read(fd_read,&section_nr,sizeof(unsigned int));
        read(fd_read, &offset,sizeof(unsigned int) );
        read(fd_read, &numar_bytes,sizeof(unsigned int));

       unsigned int size_file = lseek(fd_shared_memory, 0, SEEK_END); // NE MUTAM LA FINAL CA SA STIM MARIMEA intreaga
        unsigned short header_size = *(unsigned short*)(file_map + size_file - 3); //un byte e magicul pe inca 2 e scris header size
        unsigned int pozitie_header = size_file-header_size; //pozitia headerului e final fisier - cat e headerul
        unsigned char numar_sectiuni = *(unsigned char*)((char*)file_map + pozitie_header + 4); // cu +4 dam skip la version
        if(section_nr < 1 || section_nr > numar_sectiuni)
        {
            write(fd_write, "READ_FROM_FILE_SECTION", strlen("READ_FROM_FILE_SECTION"));
            write(fd_write, "!", strlen("!"));
            write(fd_write, "ERROR", strlen("ERROR"));
            write(fd_write, "!", strlen("!"));
            return;
        }
        else
        {
            char *section_start = file_map + pozitie_header + 5 + (section_nr - 1) * 17; //start header+inca 5 poziti(bytes) peste version si no of sections si dupa pozitia sectiunii * 17(17 bytes are sectiune)
            unsigned int section_offset = *(unsigned int*)(section_start + 9); //skip la 9 bytes: 7 pt name si 2 pt type ca sa ajungem la offset
            unsigned int section_size   = *(unsigned int*)(section_start + 13); //skip la cei 9 de dinainte si inca 4 de la offset

          if(offset + numar_bytes > section_size)
          {
              write(fd_write, "READ_FROM_FILE_SECTION", strlen("READ_FROM_FILE_SECTION"));
              write(fd_write, "!", strlen("!"));
              write(fd_write, "ERROR", strlen("ERROR"));
              write(fd_write, "!", strlen("!"));
                return;

          }
          else
          {
            int i=0;
            while(i<numar_bytes)
            {
                char x = *(file_map + section_offset + offset + i);
                *(shared_memory + i) = x;
                i++;
            }
            write(fd_write, "READ_FROM_FILE_SECTION", strlen("READ_FROM_FILE_SECTION"));
            write(fd_write, "!", strlen("!"));
            write(fd_write, "SUCCESS", strlen("SUCCESS"));
            write(fd_write, "!", strlen("!"));

          }
        }


    }
}

int main() {
    int fd_read = -1;
    int fd_write = -1;
    int shared_memory_fd= -1;
    char *shared_mem = NULL;
    char *file_map=NULL;

    create_and_open_pipes(&fd_read, &fd_write);

    char buffer[256] = {0};
    int i = 0;

    while (1) {
        char c;
        if (read(fd_read, &c, 1) != 1)
            break;

        buffer[i++] = c;
        buffer[i] = '\0';

        if (c == '!') {

            if (strcmp(buffer, "ECHO!") == 0) {
                echo(fd_write);
            } else if (strcmp(buffer, "CREATE_SHM!") == 0) {
                creat_shared_memory(fd_read, fd_write, &shared_memory_fd, &shared_mem);
            } else if(strcmp(buffer, "WRITE_TO_SHM!") == 0)
            {
                write_to_shared_memory(fd_read, fd_write, shared_mem);
            }
            else if(strcmp(buffer, "MAP_FILE!")==0)
              {
              map_file(fd_write,fd_read, &file_map, &shared_memory_fd);
              }
            else if(strcmp(buffer, "READ_FROM_FILE_OFFSET!")==0)
            {
                read_from_file_offset(fd_read, fd_write, shared_memory_fd, shared_mem, file_map);
            }
            else if(strcmp(buffer, "READ_FROM_FILE_SECTION!")==0)
            {
                read_from_file_section(fd_read, fd_write, shared_memory_fd, shared_mem, file_map);
            }
            else if (strcmp(buffer, "EXIT!") == 0) {
                break;
            }


            memset(buffer, 0, sizeof(buffer));
            i = 0;
        }
    }

    if (shared_memory_fd != -1)
      close(shared_memory_fd);
    if (fd_read != -1)
      close(fd_read);
    if (fd_write != -1)
      close(fd_write);
    unlink(RESP_PIPE);

    return 0;
}
