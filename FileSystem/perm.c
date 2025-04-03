/**
* @file perm.c
* @brief Fonctions pour la gestion des permissions des fichiers
* @author jzy
* @date 2025-3-28
*/
#include "filesystem.h"

extern SuperBlock fs;

/**
 * @brief Afficher les permissions d'un fichier ou répertoire
 * @param path Le chemin du fichier ou répertoire
 * @return Aucun
 */
void show_permissions(const char *path) {
    load_superblock();
    
    // Obtenir l'inode du fichier / 获取文件的 inode
    int inode_num = get_inode_from_path(path);
    if (inode_num == -1) {
        printf("File not found\n");
        return;
    }

    Inode *inode = &fs.inodes[inode_num];
    
    // Afficher le type de fichier / 显示文件类型
    char type;
    switch (inode->file_type) {
        case FILE_TYPE_REGULAR:
            type = '-';
            break;
        case FILE_TYPE_DIR:
            type = 'd';
            break;
        case FILE_TYPE_SYMLINK:
            type = 'l';
            break;
        default:
            type = '?';
    }
    
    // Afficher les permissions / 显示权限
    char perms[10];
    perms[0] = (inode->permissions & PERM_READ) ? 'r' : '-';
    perms[1] = (inode->permissions & PERM_WRITE) ? 'w' : '-';
    perms[2] = (inode->permissions & PERM_EXECUTE) ? 'x' : '-';
    perms[3] = '\0';

    // Obtenir le nom du fichier / 获取文件名
    char filename[MAX_FILENAME_LENGTH];
    extract_last_path_component(path, filename);

    // Afficher les informations de permissions / 显示权限信息
    printf("%c%s %s\n", type, perms, filename);
    
    // Mettre à jour le temps d'accès / 更新访问时间
    inode->atime = time(NULL);
    save_superblock();
}


/**
 * @brief Analyser une chaîne de permissions
 * @param perm_str La chaîne de permissions à analyser (format: "rwx")
 * @return Les permissions sous forme d'octet
 */
unsigned char parse_permissions(const char *perm_str) {
    unsigned char perms = 0;
    
    // Analyser la chaîne de permissions, format comme "rwx" ou "rw-" etc. / 解析权限字符串，格式如 "rwx" 或 "rw-" 等
    for (int i = 0; perm_str[i] != '\0'; i++) {
        switch (perm_str[i]) {
            case 'r':
                perms |= PERM_READ;
                break;
            case 'w':
                perms |= PERM_WRITE;
                break;
            case 'x':
                perms |= PERM_EXECUTE;
                break;
        }
    }
    return perms;
}

/**
 * @brief Modifier les permissions d'un fichier ou répertoire
 * @param path Le chemin du fichier ou répertoire
 * @param perm_str La nouvelle chaîne de permissions (format: "rwx")
 * @return Aucun
 */
void change_permissions(const char *path, const char *perm_str) {
    load_superblock();
    
    // Obtenir l'inode du fichier / 获取文件的 inode
    int inode_num = get_inode_from_path(path);
    if (inode_num == -1) {
        printf("File or directory not found\n");
        return;
    }

    // Vérifier le format de la chaîne de permissions / 检查权限字符串格式
    if (strlen(perm_str) != 3) {
        printf("Invalid permission format. Please use format 'rwx' (e.g., 'rw-', 'r--', etc.)\n");
        return;
    }

    // Valider chaque caractère de la chaîne de permissions / 验证权限字符串的每个字符
    for (int i = 0; i < 3; i++) {
        char c = perm_str[i];
        switch (i) {
            case 0: // Permission de lecture / 读权限
                if (c != 'r' && c != '-') {
                    printf("Invalid read permission. Use 'r' or '-'\n");
                    return;
                }
                break;
            case 1: // Permission d'écriture / 写权限
                if (c != 'w' && c != '-') {
                    printf("Invalid write permission. Use 'w' or '-'\n");
                    return;
                }
                break;
            case 2: // Permission d'exécution / 执行权限
                if (c != 'x' && c != '-') {
                    printf("Invalid execute permission. Use 'x' or '-'\n");
                    return;
                }
                break;
        }
    }

    // Analyser la chaîne de permissions / 解析权限字符串
    unsigned char new_perms = parse_permissions(perm_str);
    
    // Mettre à jour les permissions / 更新权限
    Inode *inode = &fs.inodes[inode_num];
    inode->permissions = new_perms;
    inode->mtime = time(NULL);
    
    save_superblock();
    printf("Permissions changed successfully\n");
}

/**
 * @brief Vérifier les permissions d'opération sur un répertoire
 * @param dir_inode Le numéro d'inode du répertoire
 * @param required_perm Les permissions requises
 * @return 1 si les permissions sont suffisantes, 0 sinon
 */
int check_directory_permission(int dir_inode, unsigned char required_perm) {
    if (dir_inode == -1) {
        return 0;
    }

    Inode *dir = &fs.inodes[dir_inode];
    if (dir->file_type != FILE_TYPE_DIR) {
        return 0;
    }

    unsigned char perms = dir->permissions;
    
    switch (required_perm) {
        case PERM_READ:  // Permission de lecture uniquement, peut seulement ls / 只读权限，只能 ls
            return (perms & PERM_READ);
            
        case (PERM_READ | PERM_EXECUTE):  // Permissions de lecture et d'exécution, peut ls et cd / 读+执行权限，可以 ls 和 cd
            return ((perms & PERM_READ) && (perms & PERM_EXECUTE));
            
        case PERM_WRITE:  // 只写权限
            return (perms & PERM_WRITE);

        case (PERM_READ | PERM_WRITE):  // Permissions de lecture et d'écriture, peut ls et créer/supprimer / 读+写权限，可以 ls 和创建/删除
            return ((perms & PERM_READ) && (perms & PERM_WRITE));
            
        case (PERM_READ | PERM_WRITE | PERM_EXECUTE):  // Permissions complètes / 完全权限
            return ((perms & PERM_READ) && (perms & PERM_WRITE) && (perms & PERM_EXECUTE));
            
        default:
            return 0;
    }
}

/**
 * @brief Vérifier les permissions d'opération sur un fichier
 * @param file_inode Le numéro d'inode du fichier
 * @param required_perm Les permissions requises
 * @return 1 si les permissions sont suffisantes, 0 sinon
 */
int check_file_permission(int file_inode, unsigned char required_perm) {
    if (file_inode == -1) {
        return 0;
    }

    Inode *file = &fs.inodes[file_inode];
    if (file->file_type != FILE_TYPE_REGULAR) {
        return 0;
    }

    unsigned char perms = file->permissions;
    
    switch (required_perm) {
        case PERM_READ:  // Permission de lecture uniquement (r--) / 只读权限 (r--)
            return (perms & PERM_READ);

        case PERM_WRITE:  // 只写
            return (perms & PERM_WRITE);
            
        case (PERM_READ | PERM_WRITE):  // Permissions de lecture et d'écriture (rw-) / 读写权限 (rw-)
            return ((perms & PERM_READ) && (perms & PERM_WRITE));
            
        case (PERM_READ | PERM_WRITE | PERM_EXECUTE):  // Permissions complètes (rwx) / 完全权限 (rwx)
            return ((perms & PERM_READ) && (perms & PERM_WRITE) && (perms & PERM_EXECUTE));
            
        case PERM_EXECUTE:  // Permission d'exécution uniquement (--x) / 仅执行权限 (--x)
            return (perms & PERM_EXECUTE);
            
        default:
            return 0;
    }
}