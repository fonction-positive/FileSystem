/**
* @file list.c
* @brief Fonctions pour afficher la liste des fichiers et répertoires.
* @author jzy
* @date 2025-3-27
*/
#include "filesystem.h"

extern SuperBlock fs;

/**
 * @brief Afficher la liste des fichiers du répertoire courant (sans les fichiers cachés)
 * @return Aucun
 */
void show_ls() {
    load_superblock();
    
    int current_inode = get_inode_from_path(current_path);
    if (current_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    // Vérifier les permissions de lecture du répertoire / 检查目录读权限
    if (!check_directory_permission(current_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    // Collecter tous les noms de fichiers / 收集所有文件名
    char *filenames[MAX_FILES];
    int count = 0;

    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == current_inode && 
            fs.directory[i].name[0] != '\0' && 
            fs.directory[i].name[0] != '.' &&    // Ignorer les fichiers cachés / 跳过隐藏文件
            fs.directory[i].inode_number != -1) {
            filenames[count] = fs.directory[i].name;
            count++;
        }
    }

    // Trier par ordre alphabétique / 按字母顺序排序
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(filenames[i], filenames[j]) > 0) {
                char *temp = filenames[i];
                filenames[i] = filenames[j];
                filenames[j] = temp;
            }
        }
    }

    // Afficher les noms de fichiers / 打印文件名
    for (int i = 0; i < count; i++) {
        printf("%s\n", filenames[i]);
    }

    save_superblock();
}

/**
 * @brief Afficher la liste de tous les fichiers du répertoire courant (y compris les fichiers cachés)
 * @return Aucun
 */
void show_ls_all() {
    load_superblock();
    
    int current_inode = get_inode_from_path(current_path);
    if (current_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    // Vérifier les permissions de lecture du répertoire / 检查目录读权限
    if (!check_directory_permission(current_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    // Collecter tous les noms de fichiers (y compris les fichiers cachés) / 收集所有文件名（包括隐藏文件）
    char *filenames[MAX_FILES];
    int count = 0;

    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == current_inode && 
            fs.directory[i].name[0] != '\0' && 
            fs.directory[i].inode_number != -1) {
            filenames[count] = fs.directory[i].name;
            count++;
        }
    }

    // Trier par ordre alphabétique / 按字母顺序排序
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(filenames[i], filenames[j]) > 0) {
                char *temp = filenames[i];
                filenames[i] = filenames[j];
                filenames[j] = temp;
            }
        }
    }

    // Afficher les noms de fichiers / 打印文件名
    for (int i = 0; i < count; i++) {
        printf("%s\n", filenames[i]);
    }

    save_superblock();
}

/**
 * @brief Afficher la liste détaillée de tous les fichiers du répertoire courant (y compris les fichiers cachés)
 * @return Aucun
 */
void show_list_all() {
    load_superblock();
    
    int current_inode = get_inode_from_path(current_path);
    if (current_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    printf("%-4s %-10s %-4s %-8s %-20s %-s\n", 
           "Type", "Perms", "Links", "Size", "Modified", "Name");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == current_inode && 
            fs.directory[i].name[0] != '\0' && 
            fs.directory[i].inode_number != -1) {
            
            Inode *inode = &fs.inodes[fs.directory[i].inode_number];
            
            // Type de fichier / 文件类型
            char type;
            switch (inode->file_type) {
                case FILE_TYPE_REGULAR: type = '-'; break;
                case FILE_TYPE_DIR: type = 'd'; break;
                case FILE_TYPE_SYMLINK: type = 'l'; break;
                default: type = '?';
            }
            
            // Permissions / 权限
            char perms[4];
            perms[0] = (inode->permissions & PERM_READ) ? 'r' : '-';
            perms[1] = (inode->permissions & PERM_WRITE) ? 'w' : '-';
            perms[2] = (inode->permissions & PERM_EXECUTE) ? 'x' : '-';
            perms[3] = '\0';
            
            // Heure de modification / 修改时间
            char mtime_str[20];
            strftime(mtime_str, sizeof(mtime_str), "%Y-%m-%d %H:%M:%S", 
                    localtime(&inode->mtime));
            
            printf("%-4c %-10s %-4d %-8zu %-20s %s\n",
                   type,
                   perms,
                   inode->link_count,
                   inode->size,
                   mtime_str,
                   fs.directory[i].name);
        }
    }
    
    save_superblock();
}

/**
 * @brief Afficher la liste détaillée des fichiers du répertoire courant (sans les fichiers cachés)
 * @return Aucun
 */
void show_list() {
    load_superblock();
    
    int current_inode = get_inode_from_path(current_path);
    if (current_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    // Vérifier les permissions de lecture / 检查是否有读权限
    if (!check_directory_permission(current_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    printf("%-4s %-10s %-4s %-8s %-20s %-s\n", 
           "Type", "Perms", "Links", "Size", "Modified", "Name");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == current_inode && 
            fs.directory[i].name[0] != '\0' && 
            fs.directory[i].name[0] != '.' &&    // Ignorer les fichiers cachés / 跳过隐藏文件
            fs.directory[i].inode_number != -1) {
            
            Inode *inode = &fs.inodes[fs.directory[i].inode_number];
            
            // Type de fichier / 文件类型
            char type;
            switch (inode->file_type) {
                case FILE_TYPE_REGULAR: type = '-'; break;
                case FILE_TYPE_DIR: type = 'd'; break;
                case FILE_TYPE_SYMLINK: type = 'l'; break;
                default: type = '?';
            }
            
            // Permissions / 权限
            char perms[4];
            perms[0] = (inode->permissions & PERM_READ) ? 'r' : '-';
            perms[1] = (inode->permissions & PERM_WRITE) ? 'w' : '-';
            perms[2] = (inode->permissions & PERM_EXECUTE) ? 'x' : '-';
            perms[3] = '\0';
            
            // Heure de modification / 修改时间
            char mtime_str[20];
            strftime(mtime_str, sizeof(mtime_str), "%Y-%m-%d %H:%M:%S", 
                    localtime(&inode->mtime));
            
            printf("%-4c %-10s %-4d %-8zu %-20s %s\n",
                   type,
                   perms,
                   inode->link_count,
                   inode->size,
                   mtime_str,
                   fs.directory[i].name);
        }
    }
    
    save_superblock();
}

/**
 * @brief Afficher les informations détaillées d'un fichier ou répertoire spécifique
 * @param path Le chemin du fichier ou répertoire à afficher
 * @return Aucun
 */
void show_list_one(const char *path) {
    load_superblock();
    
    int inode_num = get_inode_from_path(path);
    if (inode_num == -1) {
        printf("File or directory not found\n");
        return;
    }

    Inode *inode = &fs.inodes[inode_num];
    
    // Vérifier les permissions de lecture / 检查读权限
    if(inode->file_type == FILE_TYPE_DIR) {
        if (!check_directory_permission(inode_num, PERM_READ)) {
            printf("Permission denied\n");
            return;
        }
    }else{
        if (!check_file_permission(inode_num, PERM_READ)) {
            printf("Permission denied\n");
            return;
        }
    }
    

    // Type de fichier / 文件类型
    char type;
    switch (inode->file_type) {
        case FILE_TYPE_REGULAR: type = '-'; break;
        case FILE_TYPE_DIR: type = 'd'; break;
        case FILE_TYPE_SYMLINK: type = 'l'; break;
        default: type = '?';
    }
    
    // Permissions / 权限
    char perms[4];
    perms[0] = (inode->permissions & PERM_READ) ? 'r' : '-';
    perms[1] = (inode->permissions & PERM_WRITE) ? 'w' : '-';
    perms[2] = (inode->permissions & PERM_EXECUTE) ? 'x' : '-';
    perms[3] = '\0';
    
    // Heure de modification / 修改时间
    char mtime_str[20];
    strftime(mtime_str, sizeof(mtime_str), "%Y-%m-%d %H:%M:%S", 
            localtime(&inode->mtime));
    
    // Obtenir le nom du fichier / 获取文件名
    char filename[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, filename);

    // Afficher l'en-tête / 打印表头
    printf("%-4s %-10s %-4s %-8s %-20s %-s\n", 
           "Type", "Perms", "Links", "Size", "Modified", "Name");
    printf("----------------------------------------------------------\n");

    // Afficher les informations du fichier / 打印文件信息
    printf("%-4c %-10s %-4d %-8zu %-20s %s\n",
           type,
           perms,
           inode->link_count,
           inode->size,
           mtime_str,
           filename);

    // Si c'est un lien symbolique, afficher la cible / 如果是符号链接，显示链接目标
    if (inode->file_type == FILE_TYPE_SYMLINK) {
        printf(" -> %s\n", inode->data.symlink_path);
    }
    
    save_superblock();
}

/**
 * @brief Afficher la liste des fichiers avec leurs numéros d'inode et types
 * @return Aucun
 */
void list_file_dir() {
    load_superblock();
    
    int current_inode = get_inode_from_path(current_path);
    if (current_inode == -1 || fs.inodes[current_inode].file_type != FILE_TYPE_DIR) {
        printf("Not a directory\n");
        return;
    }

    // Vérifier les permissions de lecture / 检查是否有读权限
    if (!check_directory_permission(current_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    printf("%-8s %-12s %-8s\n", "Inode", "Type", "Name");
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == current_inode && fs.directory[i].name[0] != '\0' && fs.directory[i].inode_number != -1) {
            Inode *inode = &fs.inodes[fs.directory[i].inode_number];
            char *type;
            switch (inode->file_type) {
                case 1:
                    type = "FILE";
                    break;
                case 2:
                    type = "DIR";
                    break;
                case 3:
                    type = "SYMLINK";
                    break;
                default:
                    type = "UNKNOWN";
                    break;
            }
            printf("%-8d %-12s %-8s\n", 
                   fs.directory[i].inode_number,
                   type,
                   fs.directory[i].name);
        }
    }
    
    save_superblock();
}

/**
 * @brief Afficher le numéro d'inode d'un fichier ou répertoire
 * @param path Le chemin du fichier ou répertoire
 * @return Aucun
 */
void show_inode(const char *path) {
    load_superblock();
    
    int inode_num = get_inode_from_path(path);
    if (inode_num == -1) {
        printf("Path not found\n");
        return;
    }

    char filename[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, filename);
    
    printf("%-8d %s\n", inode_num, filename);
    
    save_superblock();
}

/**
 * @brief Fonction récursive pour afficher l'arborescence des répertoires
 * @param dir_inode Le numéro d'inode du répertoire à afficher
 * @param level Le niveau de profondeur dans l'arborescence
 * @param prefix Le préfixe pour l'affichage de l'arborescence
 * @param visited_inodes Tableau des inodes déjà visités
 * @param visited_count Nombre d'inodes visités
 * @return Aucun
 */
void print_tree_recursive(int dir_inode, int level, char *prefix, int *visited_inodes, int visited_count) {
    // Vérifier si le répertoire a déjà été visité (éviter les références circulaires) / 检查是否已经访问过该目录（避免循环引用）
    for (int i = 0; i < visited_count; i++) {
        if (visited_inodes[i] == dir_inode) {
            return;
        }
    }
    
    // Enregistrer le répertoire actuel comme visité / 记录当前目录已被访问
    visited_inodes[visited_count++] = dir_inode;
    
    // Collecter tous les éléments du répertoire actuel / 收集当前目录下的所有项目
    DirectoryEntry *entries[MAX_FILES];
    int entry_count = 0;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == dir_inode && 
            fs.directory[i].name[0] != '\0' && 
            fs.directory[i].name[0] != '.' &&  // Ignorer les fichiers cachés / 跳过隐藏文件
            fs.directory[i].inode_number != -1) {
            entries[entry_count++] = &fs.directory[i];
        }
    }
    
    // Trier les entrées du répertoire (par nom) / 对目录项进行排序（按名称）
    for (int i = 0; i < entry_count - 1; i++) {
        for (int j = i + 1; j < entry_count; j++) {
            if (strcmp(entries[i]->name, entries[j]->name) > 0) {
                DirectoryEntry *temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }
    
    // Afficher tous les éléments / 打印所有项目
    for (int i = 0; i < entry_count; i++) {
        printf("%s", prefix);
        
        // Vérifier s'il s'agit du dernier élément / 是否为最后一项
        int is_last = (i == entry_count - 1);
        printf("%s", is_last ? "└── " : "├── ");
        
        Inode *inode = &fs.inodes[entries[i]->inode_number];
        printf("%s", entries[i]->name);
        
        if (inode->file_type == FILE_TYPE_SYMLINK) {
            printf(" -> %s", inode->data.symlink_path);
        }
        printf("\n");
        
        if (inode->file_type == FILE_TYPE_DIR) {
            char new_prefix[1024];
            strcpy(new_prefix, prefix);
            strcat(new_prefix, is_last ? "    " : "│   ");
            print_tree_recursive(entries[i]->inode_number, level + 1, new_prefix, visited_inodes, visited_count);
        }
    }
}

/**
 * @brief Afficher l'arborescence du répertoire courant
 * @return Aucun
 */
void show_tree() {
    load_superblock();
    
    int current_inode = get_inode_from_path(current_path);
    if (current_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    if (!check_directory_permission(current_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    char dirname[MAX_FILENAME_LENGTH];
    extract_last_path_component(current_path, dirname);
    printf("%s\n", dirname);
    
    char prefix[1024] = "";
    int visited_inodes[MAX_FILES] = {0};  // Enregistrer les répertoires visités / 记录已访问的目录
    print_tree_recursive(current_inode, 0, prefix, visited_inodes, 0);
    
    save_superblock();
}

/**
 * @brief Fonction récursive pour afficher l'arborescence des répertoires avec les numéros d'inode
 * @param dir_inode Le numéro d'inode du répertoire à afficher
 * @param level Le niveau de profondeur dans l'arborescence
 * @param prefix Le préfixe pour l'affichage de l'arborescence
 * @param visited_inodes Tableau des inodes déjà visités
 * @param visited_count Nombre d'inodes visités
 * @return Aucun
 */
void print_tree_inodes_recursive(int dir_inode, int level, char *prefix, int *visited_inodes, int visited_count) {
    // Vérifier si le répertoire a déjà été visité (éviter les références circulaires) / 检查是否已经访问过该目录（避免循环引用）
    for (int i = 0; i < visited_count; i++) {
        if (visited_inodes[i] == dir_inode) {
            return;
        }
    }
    
    // Enregistrer le répertoire actuel comme visité / 记录当前目录已被访问
    visited_inodes[visited_count++] = dir_inode;
    
    // Collecter tous les éléments du répertoire actuel / 收集当前目录下的所有项目
    DirectoryEntry *entries[MAX_FILES];
    int entry_count = 0;
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == dir_inode && 
            fs.directory[i].name[0] != '\0' && 
            fs.directory[i].name[0] != '.' &&  // Ignorer les fichiers cachés / 跳过隐藏文件
            fs.directory[i].inode_number != -1) {
            entries[entry_count++] = &fs.directory[i];
        }
    }
    
    // Trier les entrées du répertoire (par nom) / 对目录项进行排序（按名称）
    for (int i = 0; i < entry_count - 1; i++) {
        for (int j = i + 1; j < entry_count; j++) {
            if (strcmp(entries[i]->name, entries[j]->name) > 0) {
                DirectoryEntry *temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }
    
    // Afficher tous les éléments / 打印所有项目
    for (int i = 0; i < entry_count; i++) {
        printf("%s", prefix);
        
        // Vérifier s'il s'agit du dernier élément / 是否为最后一项
        int is_last = (i == entry_count - 1);
        printf("%s", is_last ? "└── " : "├── ");
        
        Inode *inode = &fs.inodes[entries[i]->inode_number];
        // Afficher le numéro d'inode et le nom du fichier / 打印 inode 编号和文件名
        printf("[%d] %s", entries[i]->inode_number, entries[i]->name);
        
        if (inode->file_type == FILE_TYPE_SYMLINK) {
            printf(" -> %s", inode->data.symlink_path);
        }
        printf("\n");
        
        if (inode->file_type == FILE_TYPE_DIR) {
            char new_prefix[1024];
            strcpy(new_prefix, prefix);
            strcat(new_prefix, is_last ? "    " : "│   ");
            print_tree_inodes_recursive(entries[i]->inode_number, level + 1, new_prefix, visited_inodes, visited_count);
        }
    }
}

/**
 * @brief Afficher l'arborescence du répertoire courant avec les numéros d'inode
 * @return Aucun
 */
void show_tree_inodes() {
    load_superblock();
    
    int current_inode = get_inode_from_path(current_path);
    if (current_inode == -1) {
        printf("Directory not found\n");
        return;
    }

    if (!check_directory_permission(current_inode, PERM_READ)) {
        printf("Permission denied\n");
        return;
    }

    char dirname[MAX_FILENAME_LENGTH];
    extract_last_path_component(current_path, dirname);
    printf("[%d] %s\n", current_inode, dirname);
    
    char prefix[1024] = "";
    int visited_inodes[MAX_FILES] = {0};  // Enregistrer les répertoires visités / 记录已访问的目录
    print_tree_inodes_recursive(current_inode, 0, prefix, visited_inodes, 0);
    
    save_superblock();
}