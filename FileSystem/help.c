/**
* @file help.c
* @brief Fonctions pour afficher les informations d'aide.
* @author jzy
* @date 2025-3-26
*/
#include "filesystem.h"
#include <stdio.h>

/**
 * @brief Afficher toutes les commandes disponibles du système de fichiers
 * @return Aucun
 */
void show_help() {
    printf("\nVirtual File System Commands:\n");
    printf("===================================================================================\n");
    
    // 基本文件系统操作
    printf("File System Operations:\n");
    printf("  mkfs                   Format the file system\n");
    
    // 目录操作
    printf("\nDirectory Operations:\n");
    printf("  pwd                   Show current working directory\n");
    printf("  cd <path>             Change current directory\n");
    printf("  mkdir <name>          Create a new directory\n");
    printf("  rmdir <name>          Remove an empty directory\n");
    printf("  rm -rf <name>         Remove a directory and its contents recursively\n");
    printf("  mvdir <src> <dst>     Move/rename a directory\n");
    printf("  du <name>            Calculate disk usage of a directory\n");
    
    // 文件操作
    printf("\nFile Operations:\n");
    printf("  touch <name>          Create a new empty file\n");
    printf("  rm <name>             Remove a file\n");
    printf("  mv <src> <dst>        Move/rename a file\n");
    printf("  cp <src> <dst>        Copy a file\n");
    
    // 文件内容操作
    printf("\nFile Content Operations:\n");
    printf("  cat <file>            Display file content\n");
    printf("  head <file> <n>       Display first n lines of file\n");
    printf("  tail <file> <n>       Display last n lines of file\n");
    printf("  echo <text> > <file>  Write text to file\n");
    printf("  echo <text> >> <file> Append text to file\n");
    
    // 列表和树形显示
    printf("\nListing Commands:\n");
    printf("  ls                    List files in current directory\n");
    printf("  ls -a                 List all files (including hidden)\n");
    printf("  ls -l                 List files in long format\n");
    printf("  ls -l <name>          Show detailed information of specific file\n");
    printf("  ls -la                List all files in long format\n");
    printf("  ls -it                List files with inode and type\n");
    printf("  ls -i <name>          Show inode of specific file\n");
    printf("  tree                  Display directory structure\n");
    printf("  tree -i               Display directory structure with inodes\n");
    
    // 权限操作
    printf("\nPermission Operations:\n");
    printf("  perm <name>           Show file permissions\n");
    printf("  chmod <name> <perm>   Change file permissions (format: rwx)\n");
    
    // 链接操作
    printf("\nLink Operations:\n");
    printf("  ln <src> <dst>        Create hard link\n");
    printf("  link <src> <dst>      Create symbolic link\n");
    printf("  unlink <name>         Remove symbolic link\n");
    
    // 其他命令
    printf("\nOther Commands:\n");
    printf("  help                  Show this help message\n");
    printf("  exit                  Exit the file system\n");
    
    printf("===================================================================================\n");
}

/**
 * @brief Afficher le message de bienvenue du système de fichiers
 * @return Aucun
 */
void welcome(){
    printf("===================================================================================\n");
    printf("                       Welcome to the Virtual File System.                         \n");
    printf("           Type 'exit' to quit. Type 'help' for a list of commands.                \n");
    printf("if you are first time using the system, please type 'mkfs' to format the partition.\n");
    printf("===================================================================================\n\n");
}

/**
 * @brief Afficher le message d'erreur lorsque le système de fichiers n'est pas initialisé
 * @return Aucun
 */
void NotInit(){
    printf("===================================================================================\n");
    printf("                    File system is not initialized.                                 \n");
    printf("                    Please run 'mkfs' to format the disk.                          \n");
    printf("===================================================================================\n");
}