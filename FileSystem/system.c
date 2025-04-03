/**
* @file system.c
* @brief Fonctions de base du système de fichiers, y compris la gestion des inodes et des pages
* @author jzy
* @date 2025-3-26
*/

#include "filesystem.h"

SuperBlock fs;
#define DISK_FILE "virtual_disk.dat"
char current_path[MAX_PATH_LENGTH] = "/";

// Initialisation du système de fichiers / 文件系统初始化
// Fonction auxiliaire pour écrire la structure dans le fichier / 将结构体写入文件的辅助函数
/**
 * @brief Écrire le superbloc dans le fichier disque
 * @param disk Le pointeur vers le fichier disque
 * @return Le nombre d'éléments écrits
 */
size_t write_superblock(FILE* disk) {
    fseek(disk, 0, SEEK_SET);
    return fwrite(&fs, sizeof(SuperBlock), 1, disk);
}

/**
 * @brief Formater la partition du système de fichiers
 * @details Initialise le système de fichiers avec un nouveau superbloc et un répertoire racine
 * @return Aucun
 */
void format_partition() {
    // Créer ou écraser le fichier disque / 创建或覆盖磁盘文件
    FILE* disk = fopen(DISK_FILE, "wb+");
    if (!disk) {
        perror("Failed to create virtual disk");
        exit(EXIT_FAILURE);
    }

    // Initialiser le superbloc en mémoire / 初始化内存中的超级块
    memset(&fs, 0, sizeof(SuperBlock));

    // Initialiser la liste chaînée des inodes libres / 初始化 inode 空闲链表
    for (int i = 0; i < MAX_FILES; i++) {
        fs.inodes[i].inode_number = -1;
        if (i < MAX_FILES-1) fs.inodes[i].link_count = i+1;
        else fs.inodes[i].link_count = -1;
    }
    fs.free_inode_head = 0;

    // Initialiser la liste chaînée des pages libres / 初始化页面空闲链表
    for (int i = 0; i < MAX_FILES*MAX_FILE_PAGES; i++) {
        fs.page_table[i].is_used = 0;
        if (i < MAX_FILES*MAX_FILE_PAGES-1) {
            *((int*)fs.page_table[i].data) = i+1;
        } else {
            *((int*)fs.page_table[i].data) = -1;
        }
    }
    fs.free_page_head = 0;

    // Créer le répertoire racine / 创建根目录
    int root_inode = allocate_inode();
    fs.inodes[root_inode].file_type = FILE_TYPE_DIR;
    fs.inodes[root_inode].permissions = PERM_READ | PERM_WRITE | PERM_EXECUTE;
    fs.inodes[root_inode].ctime = time(NULL);

    // Créer les entrées du répertoire racine / 创建根目录项
    strcpy(fs.directory[0].name, ".");
    fs.directory[0].inode_number = root_inode;
    fs.directory[0].parent_inode = root_inode;

    strcpy(fs.directory[1].name, "..");
    fs.directory[1].inode_number = root_inode;
    fs.directory[1].parent_inode = root_inode;

    // Initialiser les entrées de répertoire restantes comme invalides / 初始化剩余目录项为无效状态
    for (int i = 2; i < MAX_FILES; i++) {
        fs.directory[i].inode_number = -1;
        fs.directory[i].parent_inode = -1;
        fs.directory[i].name[0] = '\0';
    }

    // Écrire le superbloc initialisé sur le disque / 将初始化好的超级块写入磁盘
    if (write_superblock(disk) != 1) {
        fprintf(stderr, "Failed to write superblock\n");
        fclose(disk);
        exit(EXIT_FAILURE);
    }

    fclose(disk);
    printf("Virtual disk formatted successfully\n");
    strcpy(current_path, "/");
}

// Charger le superbloc du disque en mémoire / 从磁盘加载超级块到内存
/**
 * @brief Charger le superbloc depuis le disque en mémoire
 * @return Aucun
 */
void load_superblock() {
    FILE* disk = fopen(DISK_FILE, "rb+");
    if (!disk) {
        perror("Failed to open virtual disk");
        exit(EXIT_FAILURE);
    }
    
    if (fread(&fs, sizeof(SuperBlock), 1, disk) != 1) {
        fprintf(stderr, "Failed to read superblock\n");
        fclose(disk);
        exit(EXIT_FAILURE);
    }
    fclose(disk);
}

// Sauvegarder le superbloc de la mémoire sur le disque / 将内存中的超级块保存到磁盘
/**
 * @brief Sauvegarder le superbloc de la mémoire sur le disque
 * @return Aucun
 */
void save_superblock() {
    FILE* disk = fopen(DISK_FILE, "rb+");
    if (!disk) {
        perror("Failed to open virtual disk");
        exit(EXIT_FAILURE);
    }
    
    if (fwrite(&fs, sizeof(SuperBlock), 1, disk) != 1) {
        fprintf(stderr, "Failed to write superblock\n");
    }
    fclose(disk);
}

// Fonctions d'allocation des ressources / 资源分配函数
/**
 * @brief Allouer un nouvel inode
 * @return Le numéro de l'inode alloué, ou -1 en cas d'échec
 */
int allocate_inode() {
    if (fs.free_inode_head == -1) return -1; // Pas d'inode libre / 没有空闲inode
    
    int allocated = fs.free_inode_head;
    fs.free_inode_head = fs.inodes[allocated].link_count; // Mettre à jour la tête de liste / 更新链表头
    
    // Initialiser l'inode / 初始化inode
    memset(&fs.inodes[allocated], 0, sizeof(Inode));
    fs.inodes[allocated].inode_number = allocated;
    fs.inodes[allocated].ctime = time(NULL);
    return allocated;
}

/**
 * @brief Libérer un inode
 * @param inode_number Le numéro de l'inode à libérer
 * @return Aucun
 */
void free_inode(int inode_number) {
    fs.inodes[inode_number].link_count = fs.free_inode_head;
    fs.free_inode_head = inode_number;
}

/**
 * @brief Allouer une nouvelle page
 * @return Le numéro de la page allouée, ou -1 en cas d'échec
 */
int allocate_page() {
    if (fs.free_page_head == -1) return -1;
    
    int allocated = fs.free_page_head;
    fs.free_page_head = *((int*)fs.page_table[allocated].data); // Obtenir la prochaine page libre / 获取下一个空闲页
    fs.page_table[allocated].is_used = 1;
    return allocated;
}

/**
 * @brief Libérer une page
 * @param page_number Le numéro de la page à libérer
 * @return Aucun
 */
void free_page(int page_number) {
    *((int*)fs.page_table[page_number].data) = fs.free_page_head;
    fs.free_page_head = page_number;
    fs.page_table[page_number].is_used = 0;
}

// Obtenir le numéro d'inode à partir du chemin / 通过路径获取对应的inode编号
/**
 * @brief Obtenir le numéro d'inode à partir d'un chemin
 * @param path Le chemin à résoudre
 * @return Le numéro d'inode correspondant, ou -1 si non trouvé
 */
int get_inode_from_path(const char *path) {
    char components[MAX_PATH_LENGTH][MAX_FILENAME_LENGTH];
    int count = split_path(path, components);
    
    // Obtenir l'inode du répertoire de départ / 获取起始目录inode
    int current_inode;
    if (path[0] == '/') {
        current_inode = fs.directory[0].inode_number;
    } else {
        // Chemin relatif : construire le chemin absolu / 相对路径：构造绝对路径
        char abs_path[MAX_PATH_LENGTH];
        size_t current_len = strlen(current_path);
        if (current_len + strlen(path) + 1 >= MAX_PATH_LENGTH) {
            return -1; // Chemin trop long / 路径过长
        }
        if (current_path[current_len - 1] == '/') {
            snprintf(abs_path, MAX_PATH_LENGTH, "%s%s", current_path, path);
        } else {
            snprintf(abs_path, MAX_PATH_LENGTH, "%s/%s", current_path, path);
        }
    return get_inode_from_path(abs_path); // Traitement récursif / 递归处理
    }
    
    // Parcourir les composants du chemin / 逐级遍历路径组件
    for (int i = 0; i < count; i++) {
        if (strcmp(components[i], ".") == 0) continue;
        
        if (strcmp(components[i], "..") == 0) {
            // Obtenir l'inode du répertoire parent / 获取父目录inode
            int parent = find_in_directory(current_inode, "..");
            if (parent == -1) return -1;
            current_inode = parent;
            continue;
        }
        
        // Rechercher l'entrée du sous-répertoire / 查找子目录项
        int next_inode = find_in_directory(current_inode, components[i]);
        if (next_inode == -1) return -1;
        
        // Vérifier que les chemins intermédiaires sont des répertoires / 验证中间路径必须是目录
        if (fs.inodes[next_inode].file_type != FILE_TYPE_DIR && i != count-1) {
            return -1;
        }
        
        current_inode = next_inode;
    }
    
    return current_inode;
}

// Obtenir le numéro d'inode du répertoire parent à partir du chemin / 通过路径获取父目录的inode编号
/**
 * @brief Obtenir le numéro d'inode du répertoire parent à partir d'un chemin
 * @param path Le chemin à résoudre
 * @return Le numéro d'inode du répertoire parent, ou -1 si non trouvé
 */
int get_parent_directory_inode(const char *path) {
    char normalized_path[MAX_PATH_LENGTH];
    
    // Traiter le chemin relatif / 处理相对路径
    if (path[0] != '/') {
        snprintf(normalized_path, MAX_PATH_LENGTH, "%s/%s", current_path, path);
    } else {
        strncpy(normalized_path, path, MAX_PATH_LENGTH);
    }

    // Normaliser le chemin : supprimer les barres obliques finales / 规范化路径：去除末尾斜杠
    size_t len = strlen(normalized_path);
    while (len > 1 && normalized_path[len-1] == '/') {
        normalized_path[--len] = '\0';
    }
   
    // Traiter le cas spécial du répertoire racine / 处理根目录特殊情况
    if (strcmp(normalized_path, "/") == 0) {
        return fs.directory[0].inode_number;
    }
    
    char *last_slash = strrchr(normalized_path, '/');
    if (!last_slash) {
        printf("Invalid path: %s\n", path);
        return -1;
    }
    
    // Tronquer au chemin du répertoire parent / 截断到父目录路径
    char parent_path[MAX_PATH_LENGTH];
    strncpy(parent_path, normalized_path, last_slash - normalized_path + 1);
    parent_path[last_slash - normalized_path + 1] = '\0';
    int parent_inode = get_inode_from_path(parent_path);
    
    if (parent_inode == -1) {
        printf("Parent path invalid: %s\n", parent_path);
    }
    return parent_inode;
}

/**
 * @brief Créer une entrée de répertoire
 * @param name Le nom de l'entrée
 * @param parent_inode Le numéro d'inode du répertoire parent
 * @return Aucun
 */
void create_directory_entry(const char *name, int parent_inode) {
    // Rechercher une entrée de répertoire libre / 寻找空闲目录项
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].inode_number == -1) {
            strncpy(fs.directory[i].name, name, MAX_FILENAME_LENGTH);
            fs.directory[i].parent_inode = parent_inode;
            return;
        }
    }
}

// Ajouter une entrée de répertoire / 添加目录项
/**
 * @brief Ajouter une entrée de répertoire
 * @param parent_inode Le numéro d'inode du répertoire parent
 * @param name Le nom de l'entrée
 * @param target_inode Le numéro d'inode cible
 * @return Aucun
 */
void add_directory_entry(int parent_inode, const char *name, int target_inode) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].inode_number == -1) { // Entrée libre / 空闲条目
            strncpy(fs.directory[i].name, name, MAX_FILENAME_LENGTH);
            fs.directory[i].parent_inode = parent_inode;
            fs.directory[i].inode_number = target_inode;

            // 更新父目录大小及修改时间
            if (parent_inode >= 0 && parent_inode < MAX_FILES) {
                fs.inodes[parent_inode].size += sizeof(DirectoryEntry);
                fs.inodes[parent_inode].mtime = time(NULL);
            }

            return;
        }
    }
    
    printf("Directory is full\n");
}

// Supprimer l'entrée de répertoire / 删除目录项
/**
 * @brief Supprimer une entrée de répertoire
 * @param name Le nom de l'entrée à supprimer
 * @param parent_inode Le numéro d'inode du répertoire parent
 * @return Aucun
 */
void remove_directory_entry(const char *name, int parent_inode) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fs.directory[i].parent_inode == parent_inode && 
            strcmp(fs.directory[i].name, name) == 0) {

            // 更新父目录大小及修改时间
            if (parent_inode >= 0 && parent_inode < MAX_FILES) {
                fs.inodes[parent_inode].size -= sizeof(DirectoryEntry);
                fs.inodes[parent_inode].mtime = time(NULL);
            }

            // Vider l'entrée de répertoire / 清空目录项
            fs.directory[i].inode_number = -1;
            fs.directory[i].parent_inode = -1;
            fs.directory[i].name[0] = '\0';
            return;
        }
    }
}

//计算文件大小
/**
 * @brief Obtenir la taille d'un fichier
 * @param inode_number Le numéro d'inode du fichier
 * @return La taille du fichier en octets, ou -1 en cas d'erreur
 */
int get_file_size(int inode_number) {
    // 检查inode编号有效性
    if (inode_number < 0 || inode_number >= MAX_FILES) {
        printf("Invalid inode number\n");
        return -1;
    }
    
    Inode *target = &fs.inodes[inode_number];
    
    // 验证inode是否有效（通过inode编号匹配）
    if (target->inode_number != inode_number) {
        printf("File does not exist\n");
        return -1;
    }
    
    // 返回文件大小
    return (int)target->size;
}

// 计算目录大小包括元数据
/**
 * @brief Obtenir la taille d'un répertoire (y compris les métadonnées)
 * @param inode_number Le numéro d'inode du répertoire
 * @return La taille du répertoire en octets, ou -1 en cas d'erreur
 */
int get_dir_size(int inode_number) {
    // 检查inode有效性
    if (inode_number < 0 || inode_number >= MAX_FILES) {
        printf("Invalid inode number\n");
        return -1;
    }
    
    Inode *dir_inode = &fs.inodes[inode_number];
    
    // 验证是否为目录
    if (dir_inode->file_type != FILE_TYPE_DIR) {
        printf("Not a directory\n");
        return -1;
    }
    
    int total_size = 0;
    
    // 遍历所有目录项
    for (int i = 0; i < MAX_FILES; i++) {
        DirectoryEntry *entry = &fs.directory[i];
        
        // 只处理属于当前目录的条目
        if (entry->parent_inode == inode_number && entry->inode_number != -1) {
            Inode *child = &fs.inodes[entry->inode_number];
            
            // 递归处理子目录（排除.和..）
            if (child->file_type == FILE_TYPE_DIR && 
                strcmp(entry->name, ".") != 0 && 
                strcmp(entry->name, "..") != 0) {
                int subdir_size = get_dir_size(entry->inode_number);
                if (subdir_size == -1) return -1;
                total_size += subdir_size;
            }
            
            // 累加所有类型文件的大小
            total_size += child->size;
            
            // 符号链接额外计算链接路径长度
            if (child->file_type == FILE_TYPE_SYMLINK) {
                total_size += strlen(child->data.symlink_path);
            }
        }
    }
    
    // 包含目录自身元数据大小
    total_size += dir_inode->size; 
    
    return total_size;
}

