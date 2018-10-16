/*
 * Tyrel Hiebert
 * CSC 360 - Assignment 4
 * UVICFS18
 * July 28, 2018
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <libgen.h>
#include <string.h>
#include "disk.h"

#define SUPERBLOCK_DATA_SIZE 32

/*
 * Wrapper to catch errors of malloc
 */
void *emalloc(size_t n) {
    void *p;
    p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "malloc of %zu bytes failed", n);
        exit(1);
    }
    return p;
}

/*
 * Wrapper catch erros of calloc
 */
void *ecalloc(int n, size_t size) {
    void *p;
    p = calloc(n, size);
    if (p == NULL) {
        fprintf(stderr, "calloc of %zu bytes failed", n * size);
        exit(1);
    }
    return p;
}

/*
 * Allocate space for a new superblock_entry_t
 */
superblock_entry_t *new_sb() {
    superblock_entry_t *sb_new;
    sb_new = (superblock_entry_t *) emalloc(sizeof(superblock_entry_t));
    return sb_new;
}

/*
 * Read superblock data from disk image
 */
superblock_entry_t *read_sb(FILE *f) {
    superblock_entry_t *sb;
    int *sb_data;
    char *fs_id;
    int i;

    // read superblock data from file
    fs_id = ecalloc(FILE_SYSTEM_ID_LEN, sizeof(char));
    fread(fs_id, sizeof(char), FILE_SYSTEM_ID_LEN, f);
    if (strcmp(fs_id, "uvicfs18") != 0) {
        fprintf(stderr, "disk image does not use a 'uvicfs18' file sytem.\n");
        exit(1);
    }

    sb_data = ecalloc(6, sizeof(int)); // TODO: MAKE 6 A CONSTANT.
    fread(sb_data, sizeof(int), 6, f);

    // copy data into superblock struct
    sb = new_sb();
    for (i = 0; i < sizeof(fs_id); i++) {
        sb->magic[i] = fs_id[i];
    }
    sb->block_size = ntohs(sb_data[0]);
    sb->num_blocks = ntohs(sb_data[1]);
    sb->fat_start = ntohs(sb_data[2]);
    sb->fat_blocks = ntohs(sb_data[3]);
    sb->dir_start = ntohs(sb_data[4]);
    sb->dir_blocks = ntohs(sb_data[5]);

    // clear rest of superblock
    int *garbage;
    garbage = ecalloc((sb->block_size - SUPERBLOCK_DATA_SIZE) / sizeof(int), sizeof(int));
    fread(garbage, sizeof(int), (sb->block_size - SUPERBLOCK_DATA_SIZE) / sizeof(int), f);

    free(garbage);
    free(fs_id);
    free(sb_data);
    return sb;
}

/*
 * Read FAT from disk image
 */
int *read_fat(superblock_entry_t *sb, FILE *f) {
    int *fat_data;
    int fat_size = sb->fat_blocks * sb->block_size;

    fat_data = ecalloc(fat_size / sizeof(int), sizeof(int));
    fread(fat_data, sizeof(int), fat_size / sizeof(int), f);

    return fat_data;
}

/*
 * Read directory listing from disk image
 */
directory_entry_t **read_dirs(superblock_entry_t *sb, FILE *f) {
    directory_entry_t **dir_list;
    int i;
    int num_dir_entries = sb->dir_blocks * (sb->block_size / 64);

    // allocate space for all the directory listing structs
    dir_list = ecalloc(num_dir_entries, sizeof(directory_entry_t*));
    for (i = 0; i < num_dir_entries; i++) {
        dir_list[i] = ecalloc(1, sizeof(directory_entry_t));
    }

    // read values of each 
    for (i = 0; i < num_dir_entries; i++) {
        char status;
        fread(&status, sizeof(char), 1, f);
        dir_list[i]->status = status;

        unsigned int starting_block;
        fread(&starting_block, sizeof(int), 1, f);
        dir_list[i]->start_block = ntohl(starting_block);

        unsigned int num_blocks;
        fread(&num_blocks, sizeof(int), 1, f);
        dir_list[i]->num_blocks = ntohl(num_blocks);

        unsigned int file_size;
        fread(&file_size, sizeof(int), 1, f);
        dir_list[i]->file_size = ntohl(file_size);

        char *create_time[DIR_TIME_WIDTH];
        fread(&create_time, sizeof(char), DIR_TIME_WIDTH, f);
        // strncpy(dir_list[i]->create_time, create_time, DIR_TIME_WIDTH);
        memcpy(dir_list[i]->create_time, create_time, DIR_TIME_WIDTH);

        char *modify_time[DIR_TIME_WIDTH];
        fread(&modify_time, sizeof(char), DIR_TIME_WIDTH, f);
        // strncpy(dir_list[i]->modify_time, modify_time, DIR_TIME_WIDTH);
        memcpy(dir_list[i]->modify_time, modify_time, DIR_TIME_WIDTH);

        char filename[DIR_FILENAME_MAX];
        fread(&filename, sizeof(char), DIR_FILENAME_MAX, f);
        strncpy(dir_list[i]->filename, filename, DIR_FILENAME_MAX);

        int *garbage;
        garbage = ecalloc(6, sizeof(char));
        fread(garbage, sizeof(char), 6, f);
        free(garbage);
    }
    
    return dir_list;
}

int count_file_alloc(directory_entry_t **dir_list, superblock_entry_t *sb) {
    int num_dir_entries = sb->dir_blocks * (sb->block_size / 64);
    int sum_allocs = 0;
    int i;

    // iterate over all directory entries and if they represent a file,
    // add its used blocks to the sum
    for (i = 0; i < num_dir_entries; i++) {
        if (dir_list[i]->status > 0) {
            sum_allocs += dir_list[i]->num_blocks;
        }
    }

    return sum_allocs;
}

/*
 * Print STAT output
 */
void print_output(superblock_entry_t *sb, char *filename, int file_alloc) {
    int alloc = sb->dir_blocks + file_alloc;
    int resv = sb->fat_blocks + 1;
    int free_blks = sb->num_blocks - alloc - resv;

    printf("%s (%s)\n\n", sb->magic, filename);
    printf("-------------------------------------------------\n");
    printf("  Bsz   Bcnt  FATst FATcnt  DIRst DIRcnt\n");
    printf("%5.d%7.d%7.d%7.d%7.d%7.d\n\n",
        sb->block_size,
        sb->num_blocks,
        sb->fat_start,
        sb->fat_blocks,
        sb->dir_start,
        sb->dir_blocks
    );
    printf("-------------------------------------------------\n");
    printf(" Free   Resv  Alloc\n");
    printf("%5.d%7.d%7.d\n",
        free_blks,
        resv,
        alloc
    );
}

int main(int argc, char *argv[]) {
    superblock_entry_t *sb;
    directory_entry_t **dir_list;
    int *fat_data;
    int i;
    char *imagepath = NULL;
    FILE *f;
    char *filename;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagepath = argv[i+1];
            i++;
        }
    }
    if (imagepath == NULL) {
        fprintf(stderr, "usage: statuvfs --image <imagename>\n");
        exit(1);
    }

    char* temp = strdup(imagepath);
    filename = basename(temp);

    f = fopen(imagepath, "r");
    sb = read_sb(f);

    fat_data = read_fat(sb, f);

    dir_list = read_dirs(sb, f);

    int file_alloc = count_file_alloc(dir_list, sb);

    print_output(sb, filename, file_alloc);

    // TODO: FREE LIST OF DIRECTORY ENTRIES!!!!!!!!!!!!!!!!!!!!!!!

    free(fat_data);
    free(sb);
    fclose(f);
    return 0; 
}
