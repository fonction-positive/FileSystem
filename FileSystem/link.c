/**
* @file link.c
* @brief Fonctions pour la création et la suppression des liens durs et symboliques.
* @author jzy
* @date 2025-3-28
*/

#include "filesystem.h"

extern SuperBlock fs;

/**
 * @brief Résoudre un lien symbolique et retourner l'inode de la cible
 * @param inode_num Le numéro d'inode du lien symbolique à résoudre
 * @return Le numéro d'inode de la cible du lien, ou -1 en cas d'erreur
 */
int resolve_symlink(int inode_num) {
    int max_links = 10;  // Prévenir les boucles de liens / 防止循环链接
    int current = inode_num;
    
    while (max_links > 0 && fs.inodes[current].file_type == FILE_TYPE_SYMLINK) {
        current = get_inode_from_path(fs.inodes[current].data.symlink_path);
        if (current == -1) return -1;
        max_links--;
    }
    
    if (max_links == 0) {
        printf("Too many levels of symbolic links\n");
        return -1;
    }
    
    return current;
}

/**
 * @brief Créer un lien dur vers un fichier existant
 * @param source Le chemin du fichier source
 * @param link_name Le chemin du nouveau lien à créer
 * @return Aucun
 */
void link_file(const char *source, const char *link_name) {
    load_superblock();
    
    // Obtenir l'inode du fichier source / 获取源文件的 inode
    int src_inode = get_inode_from_path(source);
    if (src_inode == -1) {
        printf("Source file not found\n");
        return;
    }

    // Vérifier s'il s'agit d'un fichier régulier (les liens durs ne peuvent être créés que pour des fichiers réguliers) / 检查是否为普通文件（硬链接只能用于普通文件）
    if (fs.inodes[src_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Hard links can only be created for regular files\n");
        return;
    }

    // Obtenir l'inode du répertoire parent de destination / 获取目标父目录的 inode
    int dest_parent_inode = get_parent_directory_inode(link_name);
    if (dest_parent_inode == -1) {
        printf("Destination parent directory not found\n");
        return;
    }

    // Vérifier les permissions d'écriture du répertoire parent / 检查父目录的写权限
    if (!(fs.inodes[dest_parent_inode].permissions & PERM_WRITE)) {
        printf("Permission denied: parent directory is read-only\n");
        return;
    }

    // Vérifier si le nom de destination existe déjà / 检查目标名称是否已存在
    char dest_name[MAX_FILENAME_LENGTH];
    extract_last_path_component(link_name, dest_name);
    if (find_in_directory(dest_parent_inode, dest_name) != -1) {
        printf("Link name already exists\n");
        return;
    }

    // Créer une nouvelle entrée de répertoire pointant vers l'inode du fichier source / 创建新的目录项，指向源文件的 inode
    add_directory_entry(dest_parent_inode, dest_name, src_inode);

    // Incrémenter le compteur de liens de l'inode / 增加 inode 的链接计数
    fs.inodes[src_inode].link_count++;
    fs.inodes[src_inode].mtime = time(NULL);

    save_superblock();
    printf("Hard link created successfully\n");
}

/**
 * @brief Créer un lien symbolique vers un fichier ou un répertoire
 * @param target Le chemin de la cible (fichier ou répertoire)
 * @param linkpath Le chemin où créer le lien symbolique
 * @return Aucun
 */
void create_symlink(const char *target, const char *linkpath) {
    load_superblock();
    
    // Obtenir l'inode du fichier cible (vérifier si la cible existe) / 获取目标文件的 inode（检查目标是否存在）
    int target_inode = get_inode_from_path(target);
    if (target_inode == -1) {
        printf("Target file not found\n");
        return;
    }

    // Obtenir l'inode du répertoire parent du lien symbolique / 获取符号链接的父目录
    int parent_inode = get_parent_directory_inode(linkpath);
    if (parent_inode == -1) {
        printf("Parent directory not found\n");
        return;
    }

    // Vérifier si le nom du lien existe déjà / 检查链接名是否已存在
    char link_name[MAX_FILENAME_LENGTH];
    extract_last_path_component(linkpath, link_name);
    if (find_in_directory(parent_inode, link_name) != -1) {
        printf("Link name already exists\n");
        return;
    }

    // Allouer un nouvel inode / 分配新的 inode
    int new_inode = allocate_inode();
    if (new_inode == -1) {
        printf("No free inodes available\n");
        return;
    }

    // Initialiser l'inode du lien symbolique / 初始化符号链接的 inode
    Inode *symlink = &fs.inodes[new_inode];
    symlink->file_type = FILE_TYPE_SYMLINK;
    symlink->permissions = PERM_READ | PERM_WRITE;  // Permissions de lecture et d'écriture par défaut / 默认读写权限
    symlink->size = strlen(target);
    symlink->link_count = 1;
    symlink->page_count = 0;
    symlink->ctime = time(NULL);
    symlink->mtime = symlink->ctime;
    symlink->atime = symlink->ctime;

    // Construire le chemin absolu du fichier cible / 构建目标文件的绝对路径
    char abs_target_path[MAX_PATH_LENGTH];
    if (target[0] == '/') {
        // Si c'est déjà un chemin absolu, l'utiliser directement / 如果已经是绝对路径，直接使用
        strncpy(abs_target_path, target, MAX_PATH_LENGTH - 1);
    } else {
        // Si c'est un chemin relatif, le convertir en chemin absolu / 如果是相对路径，将其转换为绝对路径
        strcpy(abs_target_path, current_path);
        if (strcmp(current_path, "/") != 0) {
            strcat(abs_target_path, "/");
        }
        strcat(abs_target_path, target);
    }
    abs_target_path[MAX_PATH_LENGTH - 1] = '\0';

    // Stocker le chemin absolu / 存储绝对路径
    strncpy(symlink->data.symlink_path, abs_target_path, MAX_PATH_LENGTH - 1);
    symlink->data.symlink_path[MAX_PATH_LENGTH - 1] = '\0';
    symlink->size = strlen(symlink->data.symlink_path);

    // Ajouter l'entrée de répertoire / 添加目录项
    add_directory_entry(parent_inode, link_name, new_inode);

    save_superblock();
    printf("Symbolic link created successfully\n");
}

/**
 * @brief Supprimer un lien symbolique
 * @param path Le chemin du lien symbolique à supprimer
 * @return Aucun
 */
void delete_symlink(const char *path) {
    load_superblock();
    
    // Obtenir l'inode du lien symbolique / 获取符号链接的 inode
    int link_inode = get_inode_from_path(path);
    if (link_inode == -1) {
        printf("Symbolic link not found\n");
        return;
    }

    // Vérifier s'il s'agit d'un lien symbolique / 检查是否为符号链接
    if (fs.inodes[link_inode].file_type != FILE_TYPE_SYMLINK) {
        printf("Not a symbolic link\n");
        return;
    }

    // Obtenir l'inode du répertoire parent / 获取父目录 inode
    int parent_inode = get_parent_directory_inode(path);
    if (parent_inode == -1) {
        printf("Parent directory not found\n");
        return;
    }

    // Supprimer l'entrée du lien symbolique du répertoire parent / 从父目录中删除符号链接条目
    char link_name[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, link_name);
    remove_directory_entry(link_name, parent_inode);

    // Libérer l'inode du lien symbolique / 释放符号链接的 inode
    free_inode(link_inode);

    save_superblock();
    printf("Symbolic link deleted successfully\n");
}