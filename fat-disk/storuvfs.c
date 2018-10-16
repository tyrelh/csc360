/*
 * Tyrel Hiebert
 * CSC 360 - Assignment 4
 * UVICFS18
 * July 28, 2018
 */

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>
#include <time.h>
#include <math.h>
#include "disk.h"

#define SUPERBLOCK_DATA_SIZE 32
#define DELAY_TIME 0

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

void delay(int val) {
    int c, d;
    for (c = 1; c <= val; c++) {
        for (d = 1; d <= val; d++){}
    }
}

/*
 * Based on http://bit.ly/2vniWNb
 */
void pack_current_datetime(unsigned char *entry) {
    assert(entry);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    unsigned short year   = tm.tm_year + 1900;
    unsigned char  month  = (unsigned char)(tm.tm_mon + 1);
    unsigned char  day    = (unsigned char)(tm.tm_mday);
    unsigned char  hour   = (unsigned char)(tm.tm_hour);
    unsigned char  minute = (unsigned char)(tm.tm_min);
    unsigned char  second = (unsigned char)(tm.tm_sec);

    year = htons(year);

    memcpy(entry, &year, 2);
    entry[2] = month;
    entry[3] = day;
    entry[4] = hour;
    entry[5] = minute;
    entry[6] = second; 
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
    fread(fat_data, sizeof(int), fat_size / (sizeof(int)), f);

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

/*
 * Returns index into FAT of the next available FAT entry
 */
int next_free_block(int *fat_data, int fat_entries) {
    assert(fat_data != NULL);
    int i;

    for (i = 0; i < fat_entries; i++) {
        if (ntohl(fat_data[i]) == FAT_AVAILABLE) {
            return i;
        }
    }
    return -1;
}

/*
 * Return the size of a file in bytes.
 */
int calc_file_size(FILE *f) {
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    rewind(f);
    return size;
}

/*
 * Return the number of free blocks.
 */
int calc_free_blocks(int *fat_data, superblock_entry_t *sb) {
    int free_blocks = 0;
    int i;
    int fat_entries = sb->fat_blocks * sb->block_size / sizeof(int);

    for (i = 0; i < fat_entries; i++) {
        if (fat_data[i] == FAT_AVAILABLE) free_blocks++;
    }
    return free_blocks;
}

/*
 * Will return the directory of the given file, or NULL if not found.
 */
directory_entry_t *find_file(superblock_entry_t *sb, directory_entry_t **dir_list, char *filename) {
    int num_dir_entries = sb->dir_blocks * (sb->block_size / 64);
    int i;

    for (i = 0; i < num_dir_entries; i++) {
        if (dir_list[i]->status & 0x00000001) {
            if (!strcmp(dir_list[i]->filename, filename)) {
                return dir_list[i];
            }
        }
    }
    return NULL;
}

/*
 * Open the disk image for writing and write the required number
 * of fat entries for the file. Return the index of the first
 * entry written.
 */
int write_fat_entries(superblock_entry_t *sb, int *fat_data, int blocks_needed, FILE *f, int next_block) {
    // fat_data is BACKWARDS!
    int fat_entries = sb->fat_blocks * (sb->block_size / sizeof(int));
    int current_fat_entry_byte;
    int current_block;
    
    if (next_block == -1) {
        current_block = next_free_block(fat_data, fat_entries);
    }
    else {
        current_block = next_block;
    }
    fat_data[current_block] = htonl(FAT_RESERVED); //write junk to current block so next_free_block wont grab it.

    if (blocks_needed > 1) {
        next_block = next_free_block(fat_data, fat_entries);
    }
    else {
        next_block = FAT_LASTBLOCK;
    }

    fat_data[current_block] = htonl(next_block);
    current_fat_entry_byte = sb->block_size + current_block * sizeof(int);

    rewind(f);
    fseek(f, current_fat_entry_byte, SEEK_SET);
    next_block = htonl(next_block);
    fwrite(&next_block, sizeof(int), 1, f);
    next_block = ntohl(next_block);

    if (blocks_needed > 1) {
        write_fat_entries(sb, fat_data, --blocks_needed, f, next_block);
    }

    if (current_block == 0xFFFFFFFF) exit(1);
    
    return current_block;
}

/*
 * Write data from file to data section on disk image.
 * Use FAT to determine which blocks to write in.
 */
int write_data_blocks(superblock_entry_t *sb, int *fat_data, int block,
        int blocks_needed, int bytes_remaining, FILE *img, FILE *f) {
    int byte_current_block = block * sb->block_size;
    int i;
    int data;

    rewind(img);
    fseek(img, byte_current_block, SEEK_SET);
    for (i = 0; i < sb->block_size / sizeof(char); i++) {
        bytes_remaining -= sizeof(char);
        if (bytes_remaining >= 0) {
            fread(&data, sizeof(char), 1, f);
        }
        else if (bytes_remaining < 0) {
            data = 0x00000000;
        }
        fwrite(&data, sizeof(char), 1, img);
    }
    blocks_needed--;

    block = ntohl(fat_data[block]);

    if (blocks_needed > 0) {
        write_data_blocks(sb, fat_data, block, blocks_needed, bytes_remaining, img, f);
    }

    return 0;
}

/*
 * Write the directory entry for the file in the root
 * directory listing.
 */
int write_directory_entry(superblock_entry_t *sb, directory_entry_t **dir_list, int first_block,
        int blocks_needed, int size, char *name, FILE *img) {
    int i; int j;
    int dir_list_byte_position;
    unsigned char status;
    unsigned int  start_block;
    unsigned int  num_blocks;
    unsigned int  file_size;
    unsigned char create_time[DIR_TIME_WIDTH];
    unsigned char modify_time[DIR_TIME_WIDTH];
    char filename[DIR_FILENAME_MAX];    

    status = 0b00000011;
    start_block = htonl(first_block);
    num_blocks = htonl(blocks_needed);
    file_size = htonl(size);
    pack_current_datetime(create_time);
    pack_current_datetime(modify_time);
    strncpy(filename, name, DIR_FILENAME_MAX);

    int num_dir_entries = (sb->dir_blocks * sb->block_size) / 64;
    for (i = 0; i < num_dir_entries; i++) {
        if (dir_list[i]->status == 0x00000000) {
            break;
        }
    }

    dir_list[i]->status = status;
    dir_list[i]->start_block = ntohl(start_block);
    dir_list[i]->num_blocks = ntohl(num_blocks);
    dir_list[i]->file_size = ntohl(file_size);
    // strncpy(&dir_list[i]->create_time, &create_time, DIR_TIME_WIDTH);
    memcpy(dir_list[i]->create_time, create_time, DIR_TIME_WIDTH);
    // strncpy(&dir_list[i]->modify_time, &modify_time, DIR_TIME_WIDTH);
    memcpy(dir_list[i]->modify_time, modify_time, DIR_TIME_WIDTH);
    strncpy(dir_list[i]->filename, filename, DIR_FILENAME_MAX);

    dir_list_byte_position = sb->block_size + (sb->block_size * sb->fat_blocks) + (64 * i);

    rewind(img);
    fseek(img, dir_list_byte_position, SEEK_SET);
    
    fwrite(&status, sizeof(char), 1, img);
    fwrite(&start_block, sizeof(int), 1, img);
    fwrite(&num_blocks, sizeof(int), 1, img);
    fwrite(&file_size, sizeof(int), 1, img);
    for (j = 0; j < DIR_TIME_WIDTH; j++) {
        fwrite(&create_time[j], sizeof(char), 1, img);
    }
    for (j = 0; j < DIR_TIME_WIDTH; j++) {
        fwrite(&modify_time[j], sizeof(char), 1, img);
    }
    for (j = 0; j < DIR_FILENAME_MAX; j++) {
        fwrite(&filename[j], sizeof(char), 1, img);
    }
    for (j = 0; j < 6; j++) {
        char temp = 0xFF;
        fwrite(&temp, sizeof(char), 1, img);
    }

    return 0;
}


int main(int argc, char *argv[]) {
    superblock_entry_t *sb;
    directory_entry_t **dir_list;
    int *fat_data;
    int  i;
    char *imagepath  = NULL;
    char *imagename  = NULL;
    char *filename   = NULL;
    char *sourcename = NULL;
    FILE *img;
    FILE *f;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i+1 < argc) {
            imagepath = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--file") == 0 && i+1 < argc) {
            filename = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--source") == 0 && i+1 < argc) {
            sourcename = argv[i+1];
            i++;
        }
    }
    if (imagepath == NULL || filename == NULL || sourcename == NULL) {
        fprintf(stderr, "Usage: storuvfs --image <imagename> " \
            "--file <filename in image> " \
            "--source <filename on host>\n");
        exit(1);
    }

    char* temp = strdup(imagepath);
    imagename = basename(temp);

    // collect image fs info
    img = fopen(imagepath, "r+");
    if (img == NULL) {
        fprintf(stderr, "Unable to open the disk image %s.\nProvide a valid uvfs18 image file.", imagepath);
        exit(1);
    }
    sb = read_sb(img);
    fat_data = read_fat(sb, img); // LITTLE ENDIAN
    dir_list = read_dirs(sb, img);

    if (find_file(sb, dir_list, filename) != NULL) {
        fprintf(stderr, "File already exists on the disk.\nChoose a different filename or choose a different disk.\n");
        exit(1);
    }

    f = fopen(sourcename, "r");
    if (f == NULL) {
        fprintf(stderr, "Unable to read the file %s.\nCheck if this is file you intended.", sourcename);
        exit(1);
    }

    int file_size = calc_file_size(f);
    int blocks_needed = file_size / sb->block_size;
    if (file_size % sb->block_size > 0) blocks_needed++;
    if (blocks_needed > calc_free_blocks(fat_data, sb)) {
        fprintf(stderr, "Not enough free space on disk %s.", imagename);
        exit(1);
    }

    int first_block = write_fat_entries(sb, fat_data,
            blocks_needed, img, -1);

    write_data_blocks(sb, fat_data, first_block, blocks_needed, file_size, img, f);
    write_directory_entry(sb, dir_list, first_block, blocks_needed, file_size, filename, img);

    free(fat_data);
    free(sb);
    fclose(f);
    fclose(img);
    return 0; 
}
