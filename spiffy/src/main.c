#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "spiffs.h"

#define BLOCK_SIZE 4096
#define PAGE_SIZE 128

static FILE *flash_file;
static spiffs fs;
static u8_t spiffs_work_buf[PAGE_SIZE * 2];
static u8_t spiffs_fds[32 * 4];
static u8_t spiffs_cache_buf[(PAGE_SIZE + 32) * 4];

static s32_t flash_read(u32_t addr, u32_t size, u8_t *dst)
{
  if (fseek(flash_file, addr, SEEK_SET))
  {
    printf("Unable to seek to %d", addr);
    return SPIFFS_ERR_NOT_READABLE;
  }
  if (fread(dst, size, 1, flash_file) != 1)
  {
    printf("Unable to read\n");
    return SPIFFS_ERR_NOT_READABLE;
  }
  return SPIFFS_OK;
}

static s32_t flash_write(u32_t addr, u32_t size, u8_t *src)
{
  u8_t *buff = alloca(size);
  flash_read(addr, size, buff);
  for (int i = 0; i < size; i++)
  {
    buff[i] &= src[i];
  }
  if (fseek(flash_file, addr, SEEK_SET))
  {
    printf("Unable to seek to %d", addr);
    return SPIFFS_ERR_NOT_WRITABLE;
  }
  if (fwrite(buff, size, 1, flash_file) != 1){
    printf("Unable to write\n");
    return SPIFFS_ERR_NOT_WRITABLE;
  }
  fflush(flash_file);
  return SPIFFS_OK;
}

static s32_t flash_erase(u32_t addr, u32_t size)
{
  if (fseek(flash_file, addr, SEEK_SET))
  {
    printf("Unable to seek to %d", addr);
    return SPIFFS_ERR_NOT_WRITABLE;
  }
  for(int i = 0; i < size; i++)
  {
    fputc(0xFF, flash_file);
  }
  fflush(flash_file);
  return SPIFFS_OK;
}

void spiffs_mount(int flash_size)
{
  spiffs_config cfg;
  cfg.phys_size = flash_size;
  cfg.phys_addr = 0;
  cfg.phys_erase_block = BLOCK_SIZE;
  cfg.log_block_size = BLOCK_SIZE;
  cfg.log_page_size = PAGE_SIZE;
  cfg.hal_read_f = flash_read;
  cfg.hal_write_f = flash_write;
  cfg.hal_erase_f = flash_erase;
  SPIFFS_mount(&fs, &cfg, spiffs_work_buf, spiffs_fds, sizeof(spiffs_fds), spiffs_cache_buf, sizeof(spiffs_cache_buf), 0);
}

int write_file_to_spiffs(char *file_name, u8_t *data, int size)
{
  spiffs_file fd = SPIFFS_open(&fs, file_name, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
  if (SPIFFS_write(&fs, fd, data, size) < 0)
  {
    printf("Unable to write file %s - errno %i\n", file_name, SPIFFS_errno(&fs));
    SPIFFS_close(&fs, fd);
    return 1;
  }
  SPIFFS_close(&fs, fd);
  return 0;
}

void add_path_to_spiffs(char* source, char *path)
{
  char source_path[1024];
  snprintf(source_path, sizeof(source_path), "%s/%s", source, path);
  
  struct stat file_stat;
  if (stat(source_path, &file_stat) < 0)
  {
    printf("Could not stat %s\n", source_path);
    return;
  }
  
  if (S_ISREG(file_stat.st_mode))
  {
    printf("Adding file %s\n", path);
    FILE *fp = fopen(source_path,"r");
    if (fp == NULL)
    {
      printf("Could not open %s\n", source_path);
      return;
    }
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    u8_t *buff = alloca(size);
    if (buff == NULL)
    {
      printf("Could not alocate memory\n");
      return;
    }
    int count = fread(buff, size, 1, fp);
    if (count != 1)
    {
      printf("Could not read %s\n", source_path);
      fclose(fp);
      return;
    }
    write_file_to_spiffs(path, buff, size);
    fclose(fp);
  }
  
  if (S_ISDIR(file_stat.st_mode))
  {
    struct dirent **namelist;
    int count = scandir(source_path, &namelist, 0, alphasort);
    if (count < 0)
    {
      printf("Could not list %s\n", source_path);
      return;
    }
    for (int i = 0; i < count; i++)
    {
      if (strcmp(namelist[i]->d_name, ".") != 0 && strcmp(namelist[i]->d_name, "..") != 0)
      {
        if (strlen(path) == 0)
        {
          add_path_to_spiffs(source, namelist[i]->d_name);
        }
        else
        {
          char sub_path[1024];
          snprintf(sub_path, sizeof(sub_path), "%s/%s", path, namelist[i]->d_name);
          add_path_to_spiffs(source, sub_path);
        }
      }
      free(namelist[i]);
    }
    free(namelist);
  }
}

void print_help(char *name)
{
  printf("\nUsage:\n");
  printf("  %s dir file size\n\n", name);
  printf("    'dir' is the directory to create the file system from.\n");
  printf("    'file' is the name of the file system image to create.\n");
  printf("    'size' is the size (in bytes) of the final image.\n\n");
}    

int main(int argc, char **args)
{
  if (argc != 4)
  {
    printf("Could not parse command line\n");
    print_help(args[0]);
    exit(EXIT_FAILURE);
  }
  char *source = args[1];
  char *filename = args[2];
  int flash_size;
  if (sscanf(args[3], "%d", &flash_size) != 1)
  { 
    printf("Could not parse command line\n");
    print_help(args[0]);
    exit(EXIT_FAILURE);
  }
  
  printf("Creating rom %s of size %d bytes\n", filename, flash_size);
  printf("Adding files in directory %s\n", source);
  
  flash_file = fopen(filename,"w+");
  if (flash_file == NULL)
  {
    printf("Could not open output file\n");
    exit(EXIT_FAILURE);
  }
  
  for (int i = 0; i < flash_size; i++)
  {
    fputc(0xFF, flash_file);
  }
  fflush(flash_file);
  
  spiffs_mount(flash_size);
  add_path_to_spiffs(source, "");
  fclose(flash_file);
  
  exit(EXIT_SUCCESS);
}
