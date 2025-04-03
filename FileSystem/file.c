/**
@file file.c
@brief Fonctions pour les opérations sur les fichiers.
@author jzy
@date 2025-3-26
*/
#include "filesystem.h"

extern SuperBlock fs;

/**
 * @brief Créer un nouveau fichier régulier dans le système de fichiers
 * @param path Le chemin où le fichier doit être créé
 * @return Aucun
 */
void create_file(const char *path) {
    load_superblock();

    // 新增：处理根目录路径的特殊情况
    if (strcmp(path, "/") == 0) {
        printf("Cannot create root directory\n");
        return;
    }
    
    // Analyser le répertoire parent / 解析父目录
    int parent_inode = get_parent_directory_inode(path);
    if (parent_inode == -1) {
        printf("Parent directory not found\n");
        return;
    }

    // Vérifier les permissions de lecture et d'écriture du répertoire parent / 检查父目录的读写权限
    if (!check_directory_permission(parent_inode, PERM_READ | PERM_WRITE)) {
        printf("Permission denied\n");
        return;
    }

    // Vérifier si le fichier existe déjà / 检查文件是否已存在
    char filename[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, filename);
    if (find_in_directory(parent_inode, filename) != -1) {
        printf("File already exists\n");
        return;
    }

    // Allouer un inode / 分配inode
    int new_inode = allocate_inode();
    if (new_inode == -1) {
        printf("No free inodes\n");
        return;
    }

    // Initialiser l'inode du fichier / 初始化文件inode
    Inode *file_inode = &fs.inodes[new_inode];
    file_inode->file_type = FILE_TYPE_REGULAR;  // Définir comme fichier régulier / 设置为普通文件
    file_inode->permissions = PERM_READ | PERM_WRITE;  // Permissions de lecture et d'écriture par défaut / 默认读写权限
    file_inode->size = 0;  // Taille initiale du fichier est 0 / 新文件大小为0
    //初始化文件时间
    time_t now = time(NULL);
    file_inode->ctime = now;
    file_inode->mtime = now;
    file_inode->atime = now;

    
    // Créer l'entrée du répertoire / 创建目录项
    add_directory_entry(parent_inode, filename, new_inode);

    //更新父目录大小（每个目录项固定为32字节）
    fs.inodes[parent_inode].size += sizeof(DirectoryEntry);

    save_superblock();
    printf("File created successfully\n");
}

/**
 * @brief Supprimer un fichier régulier du système de fichiers
 * @param path Le chemin du fichier à supprimer
 * @return Aucun
 */
void delete_file(const char *path) {
    load_superblock();
    
    // Obtenir l'inode du fichier / 获取文件的 inode
    int file_inode = get_inode_from_path(path);
    if (file_inode == -1) {
        printf("File not found\n");
        return;
    }

    // Vérifier s'il s'agit d'un fichier régulier / 检查是否为普通文件
    if (fs.inodes[file_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Not a regular file\n");
        return;
    }

    // Obtenir l'inode du répertoire parent / 获取父目录 inode
    int parent_inode = get_parent_directory_inode(path);
    if (parent_inode == -1) {
        printf("Parent directory not found\n");
        return;
    }

    // Vérifier les permissions de lecture et d'écriture du répertoire parent / 检查父目录的读写权限
    if (!check_directory_permission(parent_inode, PERM_READ | PERM_WRITE)) {
        printf("Permission denied\n");
        return;
    }

    // Libérer toutes les pages de données utilisées par le fichier / 释放文件占用的所有数据页
    Inode *inode = &fs.inodes[file_inode];
    for (int i = 0; i < inode->page_count; i++) {
        free_page(inode->pages[i]);
    }

    // Supprimer l'entrée du fichier du répertoire parent / 从父目录中删除文件条目
    char filename[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, filename);
    remove_directory_entry(filename, parent_inode);

   
    // Libérer l'inode du fichier / 释放文件的 inode
    // si le nombre de hard link est 0 /只有当硬链接数为0时才真正删除文件
    if (inode->link_count == 0) {
        // 释放文件占用的所有数据页
        for (int i = 0; i < inode->page_count; i++) {
            free_page(inode->pages[i]);
        }
        // 释放文件的 inode
        free_inode(file_inode);
        printf("File deleted successfully\n");
    } else {
        printf("File unlinked successfully (hard links remaining: %d)\n", inode->link_count);
    }
    save_superblock();
}

/**
 * @brief Écrire du contenu dans un fichier, en écrasant le contenu existant
 * @param path Le chemin du fichier à écrire
 * @param content Le contenu à écrire dans le fichier
 * @return Aucun
 */
void write_file(const char *path, const char *content) {
    load_superblock();
    
    // Obtenir l'inode du fichier / 获取文件的 inode
    int file_inode = get_inode_from_path(path);
    if (file_inode == -1) {
        printf("File not found\n");
        return;
    }

    // Résoudre le lien symbolique / 解析符号链接
    if (fs.inodes[file_inode].file_type == FILE_TYPE_SYMLINK) {
        file_inode = resolve_symlink(file_inode);
        if (file_inode == -1) {
            printf("Source file does not exist or has been deleted\n");
            return;
        }
    }

    // Vérifier s'il s'agit d'un fichier régulier / 检查是否为普通文件
    if (fs.inodes[file_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Not a regular file\n");
        return;
    }

    // Vérifier les permissions de lecture et d'écriture du fichier / 检查文件的读写权限
    if (!check_file_permission(file_inode, PERM_READ | PERM_WRITE)) {
        printf("Permission denied\n");
        return;
    }

    // Calculer le nombre de pages nécessaires / 计算需要的页面数量
    size_t content_len = strlen(content);
    int pages_needed = (content_len + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // Libérer les pages existantes / 释放原有的页面
    Inode *inode = &fs.inodes[file_inode];
    for (int i = 0; i < inode->page_count; i++) {
        free_page(inode->pages[i]);
    }
    inode->page_count = 0;

    // Allouer de nouvelles pages et écrire le contenu / 分配新的页面并写入内容
    size_t remaining = content_len;
    size_t offset = 0;
    
    for (int i = 0; i < pages_needed; i++) {
        int new_page = allocate_page();
        if (new_page == -1) {
            printf("No free pages available\n");
            save_superblock();
            return;
        }
        
        inode->pages[i] = new_page;
        inode->page_count++;
        
        // Calculer la taille des données à écrire sur la page actuelle / 计算当前页面要写入的数据大小
        size_t write_size = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;
        
        // Écrire les données / 写入数据
        memcpy(fs.page_table[new_page].data, content + offset, write_size);
        
        remaining -= write_size;
        offset += write_size;
    }

    // Mettre à jour les informations du fichier / 更新文件信息
    inode->size = content_len;
    time_t now = time(NULL);
    inode->mtime = now;
    inode->atime = now;
    int dest_parent_inode = get_parent_directory_inode(path);
    if (dest_parent_inode == -1) {
        fs.inodes[dest_parent_inode].mtime = now;
    }
    
    save_superblock();
    printf("File written successfully\n");
}

/**
 * @brief Ajouter du contenu à la fin d'un fichier
 * @param path Le chemin du fichier à modifier
 * @param content Le contenu à ajouter au fichier
 * @return Aucun
 */
void append_to_file(const char *path, const char *content) {
    load_superblock();
    
    // Get file inode / 获取文件inode
    int file_inode = get_inode_from_path(path);
    if (file_inode == -1) {
        printf("File not found\n");
        return;
    }

    // Resolve symbolic link / 解析符号链接
    if (fs.inodes[file_inode].file_type == FILE_TYPE_SYMLINK) {
        file_inode = resolve_symlink(file_inode);
        if (file_inode == -1) {
            printf("Source file does not exist or has been deleted\n");
            return;
        }
    }

    // Check file type / 检查文件类型
    if (fs.inodes[file_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Not a regular file\n");
        return;
    }

    // Verify write permission / 检查写权限
    if (!check_file_permission(file_inode, PERM_WRITE)) {
        printf("Permission denied\n");
        return;
    }

    Inode *inode = &fs.inodes[file_inode];
    size_t content_len = strlen(content);
    size_t total_needed = inode->size + content_len;
    int total_pages_needed = (total_needed + PAGE_SIZE - 1) / PAGE_SIZE;

    // Check max pages limit / 检查最大页数限制
    if (total_pages_needed > MAX_FILE_PAGES) {
        printf("File size exceeds maximum limit\n");
        return;
    }

    size_t remaining = content_len;
    size_t offset = 0;
    
    // Fill existing page space / 填充现有页面剩余空间
    if (inode->page_count > 0) {
        int last_page = inode->pages[inode->page_count - 1];
        size_t existing_used = inode->size % PAGE_SIZE;
        size_t free_space = PAGE_SIZE - existing_used;
        
        if (free_space > 0) {
            size_t write_size = (remaining < free_space) ? remaining : free_space;
            char* page_ptr = fs.page_table[last_page].data + existing_used;
            
            memcpy(page_ptr, content, write_size);
            offset += write_size;
            remaining -= write_size;
            inode->size += write_size;
        }
    }

    // Allocate new pages / 分配新页面
    while (remaining > 0) {
        int new_page = allocate_page();
        if (new_page == -1) {
            printf("No free pages available\n");
            save_superblock();
            return;
        }
        
        inode->pages[inode->page_count++] = new_page;
        size_t write_size = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;
        
        memcpy(fs.page_table[new_page].data, content + offset, write_size);
        offset += write_size;
        remaining -= write_size;
        inode->size += write_size;
    }

    // Update metadata / 更新元数据
    time_t now = time(NULL);
    inode->mtime = now;
    inode->atime = now;
    
    // Update parent directory / 更新父目录
    int parent_inode = get_parent_directory_inode(path);
    if (parent_inode != -1) {
        fs.inodes[parent_inode].mtime = now;
    }
    
    save_superblock();
    printf("Content appended successfully\n");
}

/**
 * @brief Afficher le contenu d'un fichier
 * @param path Le chemin du fichier à afficher
 * @return Aucun
 */
void open_file(const char *path) {
    load_superblock();
    
    // Obtenir l'inode du fichier / 获取文件的 inode
    int file_inode = get_inode_from_path(path);
    if (file_inode == -1) {
        printf("File not found\n");
        return;
    }

    // Résoudre le lien symbolique / 解析符号链接
    if (fs.inodes[file_inode].file_type == FILE_TYPE_SYMLINK) {
        file_inode = resolve_symlink(file_inode);
        if (file_inode == -1) {
            printf("Source file does not exist or has been deleted\n");
            return;
        }
    }

    // Vérifier les permissions de lecture / 检查文件的读权限
    if (!check_file_permission(file_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    // Vérifier s'il s'agit d'un fichier régulier / 检查是否为普通文件
    if (fs.inodes[file_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Not a regular file\n");
        return;
    }

    Inode *inode = &fs.inodes[file_inode];
    
    // Lire et afficher le contenu du fichier / 读取并打印文件内容
    char buffer[PAGE_SIZE];
    size_t remaining = inode->size;

    for (int i = 0; i < inode->page_count && remaining > 0; i++) {
        size_t read_size = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;
        
        // Lire les données de la page / 读取页面数据
        memcpy(buffer, fs.page_table[inode->pages[i]].data, read_size);
        buffer[read_size] = '\0';
        
        // Afficher le contenu / 打印内容
        printf("%s", buffer);
        
        remaining -= read_size;
    }
    printf("\n");

    // Mettre à jour le temps d'accès / 更新访问时间
    inode->atime = time(NULL);
    
    save_superblock();
}

/**
 * @brief Afficher les n premières lignes d'un fichier
 * @param path Le chemin du fichier à afficher
 * @param lines Le nombre de lignes à afficher depuis le début
 * @return Aucun
 */
void head_file(const char *path, int lines) {
    load_superblock();
    
    // Obtenir l'inode du fichier / 获取文件的 inode
    int file_inode = get_inode_from_path(path);
    if (file_inode == -1) {
        printf("File not found\n");
        return;
    }

    // Résoudre le lien symbolique / 解析符号链接
    if (fs.inodes[file_inode].file_type == FILE_TYPE_SYMLINK) {
        file_inode = resolve_symlink(file_inode);
        if (file_inode == -1) {
            printf("Source file does not exist or has been deleted\n");
            return;
        }
    }

    // Vérifier s'il s'agit d'un fichier régulier / 检查是否为普通文件
    if (fs.inodes[file_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Not a regular file\n");
        return;
    }

    // Vérifier les permissions de lecture / 检查文件的读权限
    if (!check_file_permission(file_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    Inode *inode = &fs.inodes[file_inode];
    char buffer[PAGE_SIZE];
    size_t remaining = inode->size;
    int line_count = 0;

    // Lire et afficher page par page jusqu'à atteindre le nombre de lignes spécifié / 逐页读取并打印，直到达到指定行数
    for (int i = 0; i < inode->page_count && remaining > 0 && line_count < lines; i++) {
        size_t read_size = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;
        memcpy(buffer, fs.page_table[inode->pages[i]].data, read_size);
        buffer[read_size] = '\0';
        
        // Traiter caractère par caractère et compter les lignes / 逐字符处理，计数行数
        for (size_t j = 0; j < read_size && line_count < lines; j++) {
            putchar(buffer[j]);
            if (buffer[j] == '\n') {
                line_count++;
            }
        }
        
        remaining -= read_size;
    }
    printf("\n");

    // Mettre à jour le temps d'accès / 更新访问时间
    inode->atime = time(NULL);
    save_superblock();
}

/**
 * @brief Afficher les n dernières lignes d'un fichier
 * @param path Le chemin du fichier à afficher
 * @param lines Le nombre de lignes à afficher depuis la fin
 * @return Aucun
 */
void tail_file(const char *path, int lines) {
    load_superblock();
    
    // Obtenir l'inode du fichier / 获取文件的 inode
    int file_inode = get_inode_from_path(path);
    if (file_inode == -1) {
        printf("File not found\n");
        return;
    }

    // Résoudre le lien symbolique / 解析符号链接
    if (fs.inodes[file_inode].file_type == FILE_TYPE_SYMLINK) {
        file_inode = resolve_symlink(file_inode);
        if (file_inode == -1) {
            printf("Source file does not exist or has been deleted\n");
            return;
        }
    }

    // Vérifier s'il s'agit d'un fichier régulier / 检查是否为普通文件
    if (fs.inodes[file_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Not a regular file\n");
        return;
    }

    // Vérifier les permissions de lecture / 检查文件的读权限
    if (!check_file_permission(file_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    Inode *inode = &fs.inodes[file_inode];
    char buffer[PAGE_SIZE];
    
    // Premier passage : compter le nombre total de lignes / 第一次遍历：计算总行数
    size_t remaining = inode->size;
    int total_lines = 0;
    
    for (int i = 0; i < inode->page_count && remaining > 0; i++) {
        size_t read_size = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;
        memcpy(buffer, fs.page_table[inode->pages[i]].data, read_size);
        
        for (size_t j = 0; j < read_size; j++) {
            if (buffer[j] == '\n') {
                total_lines++;
            }
        }
        remaining -= read_size;
    }
    
    // Deuxième passage : afficher les dernières lignes / 第二次遍历：打印最后几行
    remaining = inode->size;
    int current_line = 0;
    int start_line = total_lines - lines;
    if (start_line < 0) start_line = 0;
    
    for (int i = 0; i < inode->page_count && remaining > 0; i++) {
        size_t read_size = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;
        memcpy(buffer, fs.page_table[inode->pages[i]].data, read_size);
        buffer[read_size] = '\0';
        
        for (size_t j = 0; j < read_size; j++) {
            if (current_line >= start_line) {
                putchar(buffer[j]);
            }
            if (buffer[j] == '\n') {
                current_line++;
            }
        }
        remaining -= read_size;
    }
    printf("\n");

    // Mettre à jour le temps d'accès / 更新访问时间
    inode->atime = time(NULL);
    save_superblock();
}

/**
 * @brief Déplacer ou renommer un fichier
 * @param source Le chemin du fichier source
 * @param destination Le chemin de destination
 * @return Aucun
 */
void move_file(const char *source, const char *destination) {
    load_superblock();
    
    // Obtenir l'inode du fichier source / 获取源文件的 inode
    int src_inode = get_inode_from_path(source);
    if (src_inode == -1) {
        printf("Source file not found\n");
        return;
    }

    // Vérifier s'il s'agit d'un fichier régulier / 检查是否为普通文件
    if (fs.inodes[src_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Source is not a regular file\n");
        return;
    }

    // Vérifier les permissions de lecture / 检查文件的读权限
    if (!check_file_permission(src_inode, PERM_READ)) {
        printf("Permission denied: cannot read source file\n");
        return;
    }

    // Obtenir l'inode de destination / 获取目标的 inode
    int dest_inode = get_inode_from_path(destination);
    char dest_name[MAX_FILENAME_LENGTH];
    int dest_parent_inode;
    int is_rename = 1;  // 默认为重命名操作

    // 检查目标路径是否包含目录分隔符
    if (strchr(destination, '/') != NULL) {
        is_rename = 0;  // 包含目录分隔符，是移动操作
    }

    // Si la destination est un répertoire, utiliser le nom du fichier original / 如果目标是目录，使用原文件名
    if (dest_inode != -1 && fs.inodes[dest_inode].file_type == FILE_TYPE_DIR) {
        // 如果目标是目录，使用原文件名
        dest_parent_inode = dest_inode;
        extract_last_path_component(source, dest_name);
        is_rename = 0;  // 目标是目录，是移动操作
    } else {
        // 如果目标不是目录，使用指定的新文件名
        dest_parent_inode = get_parent_directory_inode(destination);
        extract_last_path_component(destination, dest_name);
    }
    

    if (dest_parent_inode == -1) {
        printf("Destination parent directory not found\n");
        return;
    }

    // Obtenir l'inode du répertoire parent source / 获取源文件的父目录 inode
    int src_parent_inode = get_parent_directory_inode(source);
    if (src_parent_inode == -1) {
        printf("Source parent directory not found\n");
        return;
    }

    
    // 在move_file函数中，让我们先打印更多调试信息
    if (is_rename) {
        // 重命名操作只需要检查源目录的写权限
        if (!check_directory_permission(src_parent_inode, PERM_WRITE)) {
            printf("Permission denied: cannot modify source directory\n");
            return;
        }
    }else {
        // 移动操作需要检查源目录和目标目录的写权限
        if (!check_directory_permission(src_parent_inode, PERM_WRITE)) {
            printf("Permission denied: cannot modify source directory\n");
            return;
        }
        if (!check_directory_permission(dest_parent_inode, PERM_WRITE)) {
            printf("Permission denied: cannot modify destination directory\n");
            return;
        }
    }



    // Vérifier si le fichier de destination existe déjà / 检查目标文件是否已存在
    if (find_in_directory(dest_parent_inode, dest_name) != -1) {
        printf("Destination already exists\n");
        return;
    }

    // Supprimer l'entrée du fichier du répertoire source / 从源目录中删除文件条目
    char src_name[MAX_FILENAME_LENGTH];
    extract_last_path_component(source, src_name);
    remove_directory_entry(src_name, src_parent_inode);

    // Ajouter l'entrée du fichier dans le répertoire de destination / 在目标目录中添加文件条目
    add_directory_entry(dest_parent_inode, dest_name, src_inode);

    // Mettre à jour le temps de modification / 更新修改时间
    fs.inodes[src_inode].mtime = time(NULL);
    

    save_superblock();
    printf("File moved successfully\n");
}

/**
 * @brief Copier un fichier vers un nouvel emplacement
 * @param source Le chemin du fichier source
 * @param destination Le chemin de destination
 * @return Aucun
 */
void copy_file(const char *source, const char *destination) {
    load_superblock();
    
    // Obtenir l'inode du fichier source / 获取源文件的 inode
    int src_inode = get_inode_from_path(source);
    if (src_inode == -1) {
        printf("Source file not found\n");
        return;
    }

    // Vérifier s'il s'agit d'un fichier régulier / 检查是否为普通文件
    if (fs.inodes[src_inode].file_type != FILE_TYPE_REGULAR) {
        printf("Source is not a regular file\n");
        return;
    }

    // Vérifier les permissions de lecture du fichier source / 检查源文件的读权限
    if (!check_file_permission(src_inode, PERM_READ)) {
        printf("Permission denied: cannot read source file\n");
        return;
    }

    // Obtenir l'inode de destination / 获取目标的 inode
    int dest_inode = get_inode_from_path(destination);
    char dest_name[MAX_FILENAME_LENGTH];
    int dest_parent_inode;

    if (dest_inode != -1 && fs.inodes[dest_inode].file_type == FILE_TYPE_DIR) {
        // Si la destination est un répertoire, utiliser le nom du fichier original / 如果目标是目录，使用原文件名
        dest_parent_inode = dest_inode;
        extract_last_path_component(source, dest_name);
    } else {
        // Si la destination n'est pas un répertoire, utiliser le nouveau nom de fichier spécifié / 如果目标不是目录，使用指定的新文件名
        dest_parent_inode = get_parent_directory_inode(destination);
        extract_last_path_component(destination, dest_name);
    }

    if (dest_parent_inode == -1) {
        printf("Destination parent directory not found\n");
        return;
    }

    // Vérifier les permissions de lecture et d'écriture du répertoire parent de destination / 检查目标父目录的读写权限
    if (!check_directory_permission(dest_parent_inode, PERM_READ | PERM_WRITE)) {
        printf("Permission denied: cannot write to destination directory\n");
        return;
    }

    // Vérifier si le fichier de destination existe déjà / 检查目标文件是否已存在
    if (find_in_directory(dest_parent_inode, dest_name) != -1) {
        printf("Destination already exists\n");
        return;
    }

    // Allouer un nouvel inode / 分配新的 inode
    int new_inode = allocate_inode();
    if (new_inode == -1) {
        printf("No free inodes available\n");
        return;
    }

    // Copier les informations de l'inode / 复制 inode 信息
    Inode *src = &fs.inodes[src_inode];
    Inode *dest = &fs.inodes[new_inode];
    dest->file_type = FILE_TYPE_REGULAR;
    dest->permissions = src->permissions;
    dest->size = src->size;
    dest->page_count = 0;
    dest->ctime = time(NULL);
    dest->mtime = dest->ctime;
    dest->atime = dest->ctime;

    // Copier le contenu du fichier / 复制文件内容
    for (int i = 0; i < src->page_count; i++) {
        // Allouer une nouvelle page / 分配新页面
        int new_page = allocate_page();
        if (new_page == -1) {
            printf("No free pages available\n");
            // Nettoyer les ressources allouées / 清理已分配的资源
            for (int j = 0; j < dest->page_count; j++) {
                free_page(dest->pages[j]);
            }
            free_inode(new_inode);
            save_superblock();
            return;
        }

        // Copier le contenu de la page / 复制页面内容
        memcpy(fs.page_table[new_page].data, 
               fs.page_table[src->pages[i]].data, 
               PAGE_SIZE);
        
        dest->pages[dest->page_count++] = new_page;
    }

    // Ajouter l'entrée dans le répertoire / 添加目录项
    add_directory_entry(dest_parent_inode, dest_name, new_inode);

    save_superblock();
    printf("File copied successfully\n");
}
