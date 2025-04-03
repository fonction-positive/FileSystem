/**
* @file main.c
* @brief Point d'entrée principal du système de fichiers virtuel
* @author jzy
* @date 2025-3-29
*/
#include "filesystem.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/**
 * @brief Fonction principale du système de fichiers virtuel
 * @details Gère la boucle principale du shell et traite les commandes utilisateur
 * @return 0 en cas de succès
 */
int main() {
    char command[256];
    char arg1[256], arg2[256];
    int lines;

    welcome();

    while (1) {
        printf("%s> ", current_path);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        // Check if virtual_disk.dat exists
        struct stat buffer;
        int fs_initialized = (stat("virtual_disk.dat", &buffer) == 0);

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "mkfs") == 0) {
            format_partition();
        } else if (!fs_initialized) {
            NotInit();
            continue;
        } else if (strcmp(command, "ls") == 0) {
            show_ls();
        } else if (strcmp(command, "ls -a") == 0) {
            show_ls_all();
        } else if (strcmp(command, "ls -l") == 0) {
            show_list();
        } else if (strcmp(command, "ls -la") == 0) {
            show_list_all();
        } else if (sscanf(command, "ls -l %s", arg1) == 1) {
            show_list_one(arg1);
        } else if (strcmp(command, "ls -it") == 0) {
            list_file_dir();
        } else if (sscanf(command, "ls -i %s", arg1) == 1) {
            show_inode(arg1);
        } else if (strcmp(command, "tree") == 0) {
            show_tree();
        } else if (strcmp(command, "tree -i") == 0) {
            show_tree_inodes();
        } else if (strcmp(command, "pwd") == 0) {
            print_working_directory();
        } else if (sscanf(command, "cd %s", arg1) == 1) {
            change_directory(arg1);
        } else if (sscanf(command, "mkdir %s", arg1) == 1) {
            create_directory(arg1);
        } else if (sscanf(command, "rmdir %s", arg1) == 1) {
            delete_directory(arg1);
        } else if (sscanf(command, "rm -rf %s", arg1) == 1) {
            delete_directory_force(arg1);
        } else if (sscanf(command, "mvdir %s %s", arg1, arg2) == 2) {
            move_directory(arg1, arg2);
        } else if (sscanf(command, "du %s", arg1) == 1) {
            du_command(arg1);
        } else if (sscanf(command, "touch %s", arg1) == 1) {
            create_file(arg1);
        } else if (sscanf(command, "rm %s", arg1) == 1) {
            delete_file(arg1);
        } else if (sscanf(command, "cat %s", arg1) == 1) {
            open_file(arg1);
        } else if (sscanf(command, "head %s %d", arg1, &lines) == 2) {
            head_file(arg1, lines);
        } else if (sscanf(command, "tail %s %d", arg1, &lines) == 2) {
            tail_file(arg1, lines);
        } else if (sscanf(command, "echo %s >> %s", arg1, arg2) == 2) {
            append_to_file(arg2, arg1);
        } else if (sscanf(command, "echo %s > %s", arg1, arg2) == 2) {
            write_file(arg2, arg1);
        } else if (sscanf(command, "mv %s %s", arg1, arg2) == 2) {
            move_file(arg1, arg2);
        } else if (sscanf(command, "cp %s %s", arg1, arg2) == 2) {
            copy_file(arg1, arg2);
        } else if (sscanf(command, "perm %s", arg1) == 1) {
            show_permissions(arg1);
        } else if (sscanf(command, "chmod %s %s", arg1, arg2) == 2) {
            change_permissions(arg1, arg2);
        } else if (sscanf(command, "ln %s %s", arg1, arg2) == 2) {
            link_file(arg1, arg2);
        } else if (sscanf(command, "link %s %s", arg1, arg2) == 2) {
            create_symlink(arg1, arg2);
        } else if (sscanf(command, "unlink %s", arg1) == 1) {
            delete_symlink(arg1);
        } else if (strcmp(command, "help") == 0) {
            show_help();
        } else {
            printf("Invalid command.\n");
        }
    }

    printf("Exiting Virtual File System.\n");
    return 0;
}


