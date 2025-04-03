#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>
#include <time.h>
#include <stdio.h>  
#include <string.h>
#include <stdlib.h>

// 大约20MB
#define MAX_FILES 500  // Nombre maximum de fichiers / 文件数量上限
#define MAX_FILENAME_LENGTH 256  // Longueur maximale du nom de fichier / 文件名最大长度
#define MAX_PATH_LENGTH 1024  // Longueur maximale d'un chemin / 路径最大长度
#define PAGE_SIZE 4096  //4KB per page / 每页4KB
#define MAX_FILE_PAGES 10  // Nombre maximum de pages par fichier / 文件最大页数

// Définition des permissions / 权限定义
#define PERM_READ    4
#define PERM_WRITE   2
#define PERM_EXECUTE 1

// Définition des types de fichiers / 文件类型定义
#define FILE_TYPE_REGULAR 1    // Fichier régulier / 普通文件
#define FILE_TYPE_DIR     2    // Répertoire / 目录
#define FILE_TYPE_SYMLINK 3    // Lien symbolique / 符号链接

// Structure d'inode / inode 结构
typedef struct {
    int inode_number;            // Numéro d'inode / inode编号
    size_t size;                 // Taille du fichier / 文件大小
    unsigned char file_type;     // Type de fichier / 文件类型
    unsigned char permissions;   // Permissions / 权限
    int link_count;              // Compteur de liens durs / 硬链接计数
    time_t atime;                // Temps d'accès / 访问时间
    time_t mtime;                // Temps de modification / 修改时间
    time_t ctime;                // Temps de création / 创建时间
    int page_count;              // Nombre de pages utilisées / 文件使用的页面数量
    int pages[MAX_FILE_PAGES];   // Pointeurs directs vers les pages / 直接页面指针
    union {
        char symlink_path[MAX_PATH_LENGTH]; // Chemin du lien symbolique / 符号链接路径
    } data;
} Inode;

// Structure d'entrée de répertoire / 目录项结构
typedef struct {
    char name[MAX_FILENAME_LENGTH];
    int inode_number;
    int parent_inode;  // Inode du répertoire parent / 父目录 inode
} DirectoryEntry;

// Structure d'entrée de table de pages / 页表项结构
typedef struct {
    int is_used;       // Indicateur d'utilisation / 是否被使用
    char data[PAGE_SIZE];  // Page de données / 数据页面
} PageTableEntry;

// Structure du superbloc / 超级块结构
typedef struct {
    Inode inodes[MAX_FILES];                      // Tableau d'inodes / inode 数组
    DirectoryEntry directory[MAX_FILES];         // Tableau d'entrées de répertoire / 目录项数组
    PageTableEntry page_table[MAX_FILES * MAX_FILE_PAGES];  // Table des pages / 页面表
    int free_inode_head;                         // Tête de liste des inodes libres / 空闲 inode 链表头
    int free_page_head;                          // Tête de liste des pages libres / 空闲页面链表头
} SuperBlock;


///system.h
// Déclarations des fonctions du système de fichiers / 文件系统操作函数声明
void format_partition(); // Initialiser la partition (créer un grand fichier "virtual_disk.dat" pour simuler le système de fichiers et initialiser le répertoire racine) / 初始化分区（创建一个大文件"virtual_disk.dat"来模拟文件系统，同时初始化根目录）
size_t write_superblock(FILE* disk); // Écrire le superbloc sur le disque / 将超级块写入磁盘
void load_superblock(); // Charger le superbloc du disque en mémoire / 从磁盘加载超级块到内存
void save_superblock(); // Sauvegarder le superbloc sur le disque / 将超级块保存到磁盘

// Déclarations des fonctions d'inode et de page / inode和page操作函数声明
int allocate_inode(); // Allouer un inode / 分配 inode
void free_inode(int inode_number); // Libérer un inode / 释放 inode
int allocate_page(); // Allouer une page / 分配页面
void free_page(int page_number); // Libérer une page / 释放页面
int get_inode_from_path(const char *path); // Obtenir l'inode à partir du chemin / 根据路径获取inode
int get_parent_directory_inode(const char *path); // Obtenir l'inode du répertoire parent / 获取父目录的inode
int get_file_size(int inode_number); // 获取文件大小
int get_dir_size(int inode_number); // 获取目录大小
void create_directory_entry(const char *name, int parent_inode); // Créer une entrée de répertoire / 创建目录项
void add_directory_entry(int parent_inode, const char *name, int target_inode); // Ajouter une entrée de répertoire / 添加目录项
void remove_directory_entry(const char *name, int parent_inode); // Supprimer une entrée de répertoire / 删除目录项
// void update_file_times(int inode_number, int update_atime, int update_mtime); // 更新文件时间
// void write_to_page(int page_number, const char *data, size_t size); // 写入数据到页面
// void read_from_page(int page_number, char *buffer, size_t size); // 从页面读取数据


///file.h
// Déclarations des fonctions de manipulation de fichiers / 文件操作函数声明
void create_file(const char *filename);
void delete_file(const char *filename);
void move_file(const char *source, const char *destination);
void copy_file(const char *source, const char *destination);
void open_file(const char *filename); // Afficher le contenu du fichier (cat) / 打印文件内容（cat）
void head_file(const char *path, int lines);  // Afficher les n premières lignes du fichier / 显示文件前 n 行
void tail_file(const char *path, int lines);  // Afficher les n dernières lignes du fichier / 显示文件后 n 行
void write_file(const char *filename, const char *content); // Écrire dans le fichier (echo) / 写入文件内容（echo）
void append_to_file(const char *path, const char *content); // Ajouter du contenu au fichier (echo >>) / 追加文件内容（echo >>）

///dir.h
// Déclarations des fonctions de manipulation de répertoires / 目录操作函数声明
void create_directory(const char *dirname);
void delete_directory(const char *dirname);
void delete_directory_recursive(int dir_inode); // Supprimer un répertoire et son contenu récursivement / 递归删除目录及其内容（辅助函数）
void delete_directory_force(const char *path); // Supprimer un répertoire et son contenu (rm -rf) / 删除目录及其内容（rm -rf）
void change_directory(const char *dirname); // Changer de répertoire courant (cd) / 切换当前目录（cd）
void print_working_directory(); // Afficher le répertoire de travail actuel (pwd) / 打印当前工作目录（pwd）
void move_directory(const char *source, const char *destination); // Déplacer un répertoire / 移动目录
void du_command(const char *path); // Afficher l'utilisation du disque (du) / 显示磁盘使用情况 (du)
// Fonctions auxiliaires de manipulation de répertoires / 目录操作函数声明(辅助函数)
int split_path(const char *path, char components[][MAX_FILENAME_LENGTH]); // Diviser le chemin / 分割路径
void extract_last_path_component(const char *path, char *component); // Extraire le dernier composant du chemin / 提取路径最后一个分量
int find_in_directory(int dir_inode, const char *name); // Rechercher dans le répertoire / 在目录中查找文件



///list.h
// Déclarations des fonctions de listage / ls 相关函数声明
void show_inode(const char *path); // Afficher les informations d'inode (ls -i) / 显示inode信息 (ls -i)
void list_file_dir();  // Afficher les fichiers et répertoires du répertoire courant (inode,type,name) (ls -it) / 打印当前目录下的文件和文件夹(inode,type,name)(ls -it)
void show_ls(); // Afficher les noms de fichiers du répertoire courant (ls) / 打印当前目录下文件名 (ls)
void show_ls_all(); // Afficher tous les noms de fichiers, y compris les fichiers cachés (ls -a) / 打印当前目录下文件名(包括隐藏文件) (ls -a)
void show_list(); // Afficher les informations détaillées des fichiers (ls -l) / 打印当前目录下文件的详细信息 (ls -l)
void show_list_one(const char *path); // Afficher les informations détaillées d'un fichier ou répertoire (ls -l name) / 打印单个文件或目录的详细信息 (ls -l name)
void show_list_all(); // Afficher les informations détaillées de tous les fichiers, y compris les fichiers cachés (ls -la) / 打印当前目录下所有文件的详细信息(包括隐藏文件) (ls -la)
void show_tree(); // Afficher la structure en arbre du répertoire / 以树形结构显示目录
void show_tree_inodes(); // Afficher la structure en arbre avec les inodes (tree -i) / 以树形结构显示目录(显示inodes) (tree -i)


///perm.h
// Déclarations des fonctions de gestion des permissions / 权限操作函数声明
void show_permissions(const char *path); 
void change_permissions(const char *path, const char *perm_str); // Modifier les permissions du fichier / 修改文件权限
int check_file_permission(int file_inode, unsigned char required_perm); // Vérifier les permissions du fichier / 文件权限检查函数
int check_directory_permission(int dir_inode, unsigned char required_perm); // Vérifier les permissions du répertoire / 目录权限检查函数
unsigned char parse_permissions(const char *perm_str); // Analyser la chaîne de permissions / 解析权限字符串


///link.h
// Déclarations des fonctions de liens / 硬链接和符号链接操作函数声明
void link_file(const char *source, const char *link_name);  // Créer un lien dur / 创建硬链接
int resolve_symlink(int inode_num); // Résoudre un lien symbolique / 解析符号链接
void create_symlink(const char *target, const char *linkpath); // Créer un lien symbolique / 创建符号链接
void delete_symlink(const char *path);  // Supprimer un lien symbolique / 删除符号链接
void show_symlink(const char *linkpath); // Afficher la cible du lien symbolique / 显示符号链接目标


///help.h
// Déclarations des fonctions d'aide / 帮助信息函数声明
void show_help(); // Afficher les informations d'aide / 显示帮助信息
void welcome();
void NotInit();

// Variables globales / 全局变量
extern SuperBlock superblock;  // Superbloc / 超级块
extern char current_path[MAX_PATH_LENGTH];  // Répertoire de travail courant / 当前工作目录


// 测试函数
// void test_filesystem_basic();
// void test_filesystem_dir();
// void test_file_operations();  // 新增文件操作测试


#endif // FILESYSTEM_H