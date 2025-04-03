/**
* @file dir.c
* @brief Fonctions for directory operations
* @author jzy
* @date 2025-3-26
*/

#include "filesystem.h"

extern SuperBlock fs;

/**
 * @brief Crée un nouveau répertoire
 * @param[in] path Chemin complet du répertoire à créer
 * @return void
 */
void create_directory(const char *path) {
    load_superblock();
    
    // Analyser le répertoire parent / 解析父目录
    int parent_inode = get_parent_directory_inode(path);
    if (parent_inode == -1) {
        printf("Parent directory not found\n");
        return;
    }

    // Vérifier les permissions de lecture et d'écriture / 检查是否有读写权限
    if (!check_directory_permission(parent_inode, PERM_READ | PERM_WRITE)) {
        printf("Permission denied\n");
        return;
    }

    // Vérifier si le répertoire existe déjà / 检查目录是否已存在
    char dirname[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, dirname);
    if (find_in_directory(parent_inode, dirname) != -1) {
        printf("Directory already exists\n");
        return;
    }

    // Allouer un inode / 分配inode
    int new_inode = allocate_inode();
    if (new_inode == -1) {
        printf("No free inodes\n");
        return;
    }

    // Initialiser l'inode du répertoire / 初始化目录inode
    Inode *dir_inode = &fs.inodes[new_inode];
    dir_inode->file_type = FILE_TYPE_DIR;
    dir_inode->permissions = PERM_READ | PERM_WRITE | PERM_EXECUTE;
    dir_inode->size = 2 * sizeof(DirectoryEntry); // 初始包含 . 和 .. 条目
    time_t now = time(NULL);  // 获取统一的时间戳
    dir_inode->ctime = now;    // 创建时间
    dir_inode->mtime = now;    // 内容修改时间
    dir_inode->atime = now;    // 访问时间
    
    // Créer l'entrée du répertoire / 创建目录项
    add_directory_entry(parent_inode, dirname, new_inode);
    
    // Créer les entrées par défaut '.' et '..' / 创建默认的 . 和 .. 条目
    add_directory_entry(new_inode, ".", new_inode);
    add_directory_entry(new_inode, "..", parent_inode);

    save_superblock();
    printf("Directory created successfully\n");
}




/**
 * @brief Supprime un répertoire vide
 * @param[in] path Chemin du répertoire à supprimer
 * @return void
 * @note Ne fonctionne que sur les répertoires vides
 */
void delete_directory(const char *path) {
    load_superblock();
    
    // Vérifier s'il s'agit du répertoire racine / 检查是否为根目录
    if (strcmp(path, "/") == 0 || (strcmp(path, ".") == 0 && strcmp(current_path, "/") == 0)) {
        printf("Cannot delete root directory\n");
        return;
    }

    // Obtenir l'inode du répertoire / 获取目录的 inode
    int dir_inode = get_inode_from_path(path);
    if (dir_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    // Vérifier s'il s'agit d'un répertoire / 检查是否为目录
    if (fs.inodes[dir_inode].file_type != FILE_TYPE_DIR) {
        printf("Not a directory\n");
        return;
    }

    // Vérifier si le répertoire est vide (ne contient que '.' et '..') / 检查目录是否为空（只包含 . 和 ..）
    int entry_count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == dir_inode && 
            fs.directory[i].inode_number != -1) {
            entry_count++;
            // Si le nombre d'entrées dépasse 2 ('.' et '..'), le répertoire n'est pas vide / 如果条目数超过2（. 和 ..），说明目录非空
            if (entry_count > 2) {
                printf("Directory not empty\n");
                return;
            }
        }
    }

    // Obtenir l'inode du répertoire parent / 获取父目录 inode
    int parent_inode = get_parent_directory_inode(path);
    if (parent_inode == -1) {
        printf("Parent directory not found\n");
        return;
    }

    // Vérifier les permissions de lecture et d'écriture / 检查是否有读写权限
    if (!check_directory_permission(parent_inode, PERM_READ | PERM_WRITE)) {
        printf("Permission denied\n");
        return;
    }

    // Supprimer les entrées '.' et '..' du répertoire / 删除目录中的 . 和 .. 条目
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == dir_inode && 
            fs.directory[i].inode_number != -1) {
            fs.directory[i].inode_number = -1;
            fs.directory[i].parent_inode = -1;
            fs.directory[i].name[0] = '\0';
        }
    }

    // Supprimer l'entrée du répertoire du répertoire parent / 从父目录中删除该目录的条目
    char dirname[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, dirname);
    remove_directory_entry(dirname, parent_inode);

    // Libérer l'inode du répertoire / 释放目录的 inode
    free_inode(dir_inode);

    save_superblock();
    printf("Directory deleted successfully\n");
}


/**
 * @brief Supprime récursivement un répertoire et son contenu
 * @param[in] dir_inode Inode du répertoire à supprimer
 * @return void
 */
void delete_directory_recursive(int dir_inode) {
    // 遍历目录中的所有条目
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == dir_inode && 
            fs.directory[i].inode_number != -1 &&
            strcmp(fs.directory[i].name, ".") != 0 && 
            strcmp(fs.directory[i].name, "..") != 0) {
            
            int child_inode = fs.directory[i].inode_number;
            
            // 如果是目录，递归删除
            if (fs.inodes[child_inode].file_type == FILE_TYPE_DIR) {
                delete_directory_recursive(child_inode);
            }
            
            // 如果是文件或符号链接，直接删除
            if (fs.inodes[child_inode].file_type == FILE_TYPE_REGULAR) {
                // 减少硬链接计数
                fs.inodes[child_inode].link_count--;
                if (fs.inodes[child_inode].link_count == 0) {
                    // 释放文件占用的所有数据页
                    for (int j = 0; j < fs.inodes[child_inode].page_count; j++) {
                        free_page(fs.inodes[child_inode].pages[j]);
                    }
                    free_inode(child_inode);
                }
            } else if (fs.inodes[child_inode].file_type == FILE_TYPE_SYMLINK) {
                free_inode(child_inode);
            }
            
            // 删除目录项
            fs.directory[i].inode_number = -1;
            fs.directory[i].parent_inode = -1;
            fs.directory[i].name[0] = '\0';
        }
    }
}

/**
 * @brief Supprime récursivement un répertoire et son contenu
 * @param[in] path Chemin du répertoire à supprimer
 * @return void
 */
void delete_directory_force(const char *path) {
    load_superblock();
    
    // 检查是否为根目录
    if (strcmp(path, "/") == 0 || (strcmp(path, ".") == 0 && strcmp(current_path, "/") == 0)) {
        printf("Cannot delete root directory\n");
        return;
    }

    // 获取目录的 inode
    int dir_inode = get_inode_from_path(path);
    if (dir_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    // 检查是否为目录
    if (fs.inodes[dir_inode].file_type != FILE_TYPE_DIR) {
        printf("Not a directory\n");
        return;
    }

    // 获取父目录 inode
    int parent_inode = get_parent_directory_inode(path);
    if (parent_inode == -1) {
        printf("Parent directory not found\n");
        return;
    }

    // 检查权限
    if (!check_directory_permission(parent_inode, PERM_READ | PERM_WRITE)) {
        printf("Permission denied\n");
        return;
    }

    // 提示用户确认
    printf("Warning: This will recursively delete '%s' and all its contents.\n", path);
    printf("Are you sure you want to continue? (yes/no): ");
    
    char response[10];
    fgets(response, sizeof(response), stdin);
    response[strcspn(response, "\n")] = 0;  // 删除换行符
    
    if (strcmp(response, "yes") != 0) {
        printf("Operation cancelled\n");
        return;
    }

    // 递归删除目录及其内容
    delete_directory_recursive(dir_inode);

    // 删除当前目录的 . 和 .. 条目
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == dir_inode && 
            fs.directory[i].inode_number != -1) {
            fs.directory[i].inode_number = -1;
            fs.directory[i].parent_inode = -1;
            fs.directory[i].name[0] = '\0';
        }
    }

    // 从父目录中删除该目录的条目
    char dirname[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, dirname);
    remove_directory_entry(dirname, parent_inode);

    // 释放目录的 inode
    free_inode(dir_inode);

    save_superblock();
    printf("Directory and all its contents deleted successfully\n");
}


/**
 * @brief Change le répertoire de travail courant
 * @param[in] path Chemin du répertoire destination
 * @return void
 */
void change_directory(const char *path) {
    load_superblock();
    
    // Obtenir l'inode du répertoire cible / 获取目标目录的 inode
    int dir_inode = get_inode_from_path(path);
    if (dir_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    // Résoudre le lien symbolique / 解析符号链接
    if (fs.inodes[dir_inode].file_type == FILE_TYPE_SYMLINK) {
        // Obtenir le chemin réel du répertoire / 获取实际目录的路径
        const char *real_path = fs.inodes[dir_inode].data.symlink_path;
        dir_inode = resolve_symlink(dir_inode);
        if (dir_inode == -1) {
            printf("Source directory does not exist or has been deleted\n");
            return;
        }
        // Utiliser le chemin réel du répertoire / 使用实际目录的路径
        path = real_path;
    }

    // Vérifier s'il s'agit d'un répertoire / 检查是否为目录
    if (fs.inodes[dir_inode].file_type != FILE_TYPE_DIR) {
        printf("Not a directory\n");
        return;
    }

    // Vérifier les permissions de lecture et d'exécution / 检查是否有读和执行权限
    if (!check_directory_permission(dir_inode, PERM_READ | PERM_EXECUTE)) {
        printf("Permission denied\n");
        return;
    }

    // Construire le nouveau chemin absolu / 构造新的绝对路径
    char new_path[MAX_PATH_LENGTH];
    if (path[0] == '/') {
        // Chemin absolu / 绝对路径
        strncpy(new_path, path, MAX_PATH_LENGTH);
    } else {
        // Chemin relatif / 相对路径
        if (strcmp(path, "..") == 0) {
            // Retourner au répertoire parent / 返回上级目录
            char *last_slash = strrchr(current_path, '/');
            if (last_slash == current_path) {
                // Déjà dans le répertoire racine / 已经在根目录
                strcpy(new_path, "/");
            } else {
                strncpy(new_path, current_path, last_slash - current_path);
                new_path[last_slash - current_path] = '\0';
            }
        } else if (strcmp(path, ".") == 0) {
            // Répertoire actuel, pas besoin de changer / 当前目录，不需要改变
            return;
        } else {
            // Autre chemin relatif / 其他相对路径
            if (strcmp(current_path, "/") == 0) {
                snprintf(new_path, MAX_PATH_LENGTH, "/%s", path);
            } else {
                snprintf(new_path, MAX_PATH_LENGTH, "%s/%s", current_path, path);
            }
        }
    }

    // Normaliser le chemin (supprimer la barre oblique finale, sauf pour la racine) / 规范化路径（删除末尾的斜杠，除非是根目录）
    size_t len = strlen(new_path);
    if (len > 1 && new_path[len - 1] == '/') {
        new_path[len - 1] = '\0';
    }

    // Mettre à jour le chemin actuel / 更新当前路径
    strncpy(current_path, new_path, MAX_PATH_LENGTH);

    // Mettre à jour le temps d'accès / 更新访问时间
    time_t now = time(NULL);
    fs.inodes[dir_inode].atime = now;
    
    save_superblock();
    printf("Changed directory to: %s\n", current_path);  // 添加成功提示
}

/**
 * @brief Affiche le répertoire de travail courant
 * @return void
 */
void print_working_directory() {
    printf("%s\n", current_path);
}

/**
 * @brief Déplace un répertoire vers un nouvel emplacement
 * @param[in] source Chemin source
 * @param[in] destination Chemin destination
 * @return void
 */
void move_directory(const char *source, const char *destination) {
    load_superblock();
    
    // Vérifier s'il s'agit du répertoire racine / 检查是否为根目录
    if (strcmp(source, "/") == 0 || (strcmp(source, ".") == 0 && strcmp(current_path, "/") == 0)) {
        printf("Cannot move root directory\n");
        return;
    }
    
    // Obtenir l'inode du répertoire source / 获取源目录的 inode
    int src_inode = get_inode_from_path(source);
    if (src_inode == -1) {
        printf("Source directory not found\n");
        return;
    }
    
    // Vérifier si la source est un répertoire / 检查源是否为目录
    if (fs.inodes[src_inode].file_type != FILE_TYPE_DIR) {
        printf("Source is not a directory\n");
        return;
    }
    
    // Obtenir l'inode du répertoire parent de destination / 获取目标父目录的 inode
    int dest_parent_inode = get_parent_directory_inode(destination);
    if (dest_parent_inode == -1) {
        printf("Destination parent directory not found\n");
        return;
    }
    
    // Obtenir l'inode du répertoire parent source / 获取源目录的父目录 inode
    int src_parent_inode = get_parent_directory_inode(source);
    if (src_parent_inode == -1) {
        printf("Source parent directory not found\n");
        return;
    }
    
    // Vérifier si le répertoire de destination existe déjà / 检查目标目录是否已存在
    char dest_name[MAX_FILENAME_LENGTH];
    extract_last_path_component(destination, dest_name);
    if (find_in_directory(dest_parent_inode, dest_name) != -1) {
        printf("Destination already exists\n");
        return;
    }
    
    // Vérifier si on essaie de déplacer un répertoire dans son sous-répertoire / 检查是否试图将目录移动到其子目录下
    int temp_inode = dest_parent_inode;
    while (temp_inode != 0) { // 0 est l'inode du répertoire racine / 0 是根目录的 inode
        if (temp_inode == src_inode) {
            printf("Cannot move a directory to its subdirectory\n");
            return;
        }
        // Obtenir l'inode du répertoire parent / 获取父目录的 inode
        temp_inode = find_in_directory(temp_inode, "..");
    }
    
    // Supprimer l'entrée du répertoire source / 从源父目录中删除目录项
    char src_name[MAX_FILENAME_LENGTH];
    extract_last_path_component(source, src_name);
    remove_directory_entry(src_name, src_parent_inode);

    // Ajouter l'entrée dans le répertoire parent de destination / 在目标父目录中添加目录项
    add_directory_entry(dest_parent_inode, dest_name, src_inode);

    // Mettre à jour la référence '..' du répertoire déplacé / 更新移动目录中 .. 的指向
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == src_inode && 
            strcmp(fs.directory[i].name, "..") == 0) {
            fs.directory[i].inode_number = dest_parent_inode;
            break;
        }
    }
    
    save_superblock();
    printf("Directory moved successfully\n");
}

// Rechercher dans le répertoire spécifié / 在指定目录中查找条目
/**
 * @brief Recherche un élément dans un répertoire
 * @param[in] dir_inode Inode du répertoire parent
 * @param[in] name Nom de l'élément à trouver
 * @return int Numéro d'inode si trouvé, -1 sinon
 */
int find_in_directory(int dir_inode, const char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == dir_inode &&
            strcmp(fs.directory[i].name, name) == 0) {
            return fs.directory[i].inode_number;
        }
    }
    return -1;
}


// Fonctionnalités d'analyse de chemin / 路径解析功能
// Diviser la chaîne de chemin, sauvegarder dans un tableau, retourner le nombre de composants / 分割路径字符串，保存到数组，返回组件数量
/**
 * @brief Décompose un chemin en composants
 * @param[in] path Chemin à analyser
 * @param[out] components Tableau pour stocker les composants
 * @return int Nombre de composants extraits
 */
int split_path(const char *path, char components[][MAX_FILENAME_LENGTH]) {
    char path_copy[MAX_PATH_LENGTH];
    strncpy(path_copy, path, MAX_PATH_LENGTH);
    
    int count = 0;
    char *token = strtok(path_copy, "/");
    
    while (token != NULL && count < MAX_PATH_LENGTH) {
        strncpy(components[count], token, MAX_FILENAME_LENGTH);
        count++;
        token = strtok(NULL, "/");
    }
    return count;
}


// Extraire le dernier composant du chemin / 从路径中提取最后一个组件
/**
 * @brief Extrait le dernier composant d'un chemin
 * @param[in] path Chemin source
 * @param[out] component Buffer de sortie
 * @return void
 */
void extract_last_path_component(const char *path, char *component) {
    int len = strlen(path);
    int end = len - 1;

    // Ignorer les barres obliques finales / 跳过末尾的斜杠
    while (end >= 0 && path[end] == '/') {
        end--;
    }

    if (end < 0) {
        component[0] = '\0';
        return;
    }

    int start = end;
    while (start >= 0 && path[start] != '/') {
        start--;
    }

    int length = end - start;
    strncpy(component, &path[start+1], length);
    component[length] = '\0';
}


// DU command implementation / 实现DU命令
/**
 * @brief Calcule l'espace disque utilisé par un répertoire
 * @param[in] path Chemin du répertoire
 * @return void
 */
void du_command(const char *path) {
    load_superblock();
    
    // 获取目标inode
    int target_inode = get_inode_from_path(path);
    if (target_inode == -1) {
        printf("Path not found: %s\n", path);
        return;
    }

    // 验证文件类型
    Inode *inode = &fs.inodes[target_inode];
    if (inode->file_type != FILE_TYPE_DIR) {
        printf("Not a directory: %s\n", path);
        return;
    }

    // 验证读取权限
    if (!check_directory_permission(target_inode, PERM_READ)) {
        printf("Permission denied: %s\n", path);
        return;
    }

    // 计算目录大小
    int total_size = get_dir_size(target_inode);
    if (total_size == -1) return;

    // 自动选择合适单位
    const char* units[] = {"B", "KB", "MB"};
    int unit_index = 0;
    double display_size = total_size;

    if (display_size >= 1024) {
        unit_index = 1;
        display_size /= 1024;
    }
    if (display_size >= 1024) {
        unit_index = 2;
        display_size /= 1024;
    }

    printf("%d (%.2f%s)\t%s\n", total_size, display_size, units[unit_index], path);
}

