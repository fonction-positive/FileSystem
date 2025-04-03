## Projet de OS

### Le fonctionnement du système de fichiers

使用`make`命令编译项目，使用`./FileSystem`运行项目。（或者直接使用`make run`命令快速编译运行项目）

同时可以使用不同的`make`命令实现不同操作：

```
  % make help
  
  Cibles make disponibles : / 可用的 make 目标：
  make        - Compiler le programme / 编译程序
  make clean  - Nettoyer les fichiers générés / 清理编译产物
  make run    - Exécuter le programme / 运行程序
  make test   - Recompiler et exécuter / 重新编译并运行
  make debug  - Compiler en mode débogage / 以调试模式编译
  make memcheck - Exécuter la vérification de la mémoire / 运行内存检查
  make help   - Afficher cette aide / 显示此帮助信息
```

```
% make
gcc -Wall -Wextra -g -c main.c -o main.o
gcc -Wall -Wextra -g -c system.c -o system.o
gcc -Wall -Wextra -g -c dir.c -o dir.o
gcc -Wall -Wextra -g -c file.c -o file.o
gcc -Wall -Wextra -g -c list.c -o list.o
gcc -Wall -Wextra -g -c perm.c -o perm.o
gcc -Wall -Wextra -g -c link.c -o link.o
gcc -Wall -Wextra -g -c help.c -o help.o
gcc -Wall -Wextra -g -o FileSystem main.o system.o dir.o file.o list.o perm.o link.o help.o

% ./FileSystem

=========================================================================
                       Welcome to the Virtual File System.                         
           Type 'exit' to quit. Type 'help' for a list of commands.                
if you are first time using the system, please type 'mkfs' to format the partition.
=========================================================================
```



### 文件系统储存机制

在使用命令`mkfs`初始化文件系统（格式化磁盘分区）后，会自动创建一个`vitrual_disk.dat`文件作为文件系统的磁盘，之后所有对文件系统的操作将会保存在这个模拟磁盘中。

- 如果没有先试用命令`mkfs`初始化磁盘，将会出现以下提示：

```
/> [commande]
=========================================================================
                    File system is not initialized.                                 
                    Please run 'mkfs' to format the disk.                          
=========================================================================
```

- 成功初始化磁盘

```
/> mkfs 
Virtual disk formatted successfully
```



### 文件系统基础操作

- 显示帮助信息（文件系统的命令文档）

```
/> help

Virtual File System Commands:
=========================================================================
File System Operations:
  mkfs                   Format the file system

Directory Operations:
  pwd                   Show current working directory
  cd <path>             Change current directory
  mkdir <name>          Create a new directory
  rmdir <name>          Remove an empty directory
  rm -rf <name>         Remove a directory and its contents recursively
  mvdir <src> <dst>     Move/rename a directory
  du <name>            Calculate disk usage of a directory

File Operations:
  touch <name>          Create a new empty file
  rm <name>             Remove a file
  mv <src> <dst>        Move/rename a file
  cp <src> <dst>        Copy a file

File Content Operations:
  cat <file>            Display file content
  head <file> <n>       Display first n lines of file
  tail <file> <n>       Display last n lines of file
  echo <text> > <file>  Write text to file
  echo <text> >> <file> Append text to file

Listing Commands:
  ls                    List files in current directory
  ls -a                 List all files (including hidden)
  ls -l                 List files in long format
  ls -l <name>          Show detailed information of specific file
  ls -la                List all files in long format
  ls -it                List files with inode and type
  ls -i <name>          Show inode of specific file
  tree                  Display directory structure
  tree -i               Display directory structure with inodes

Permission Operations:
  perm <name>           Show file permissions
  chmod <name> <perm>   Change file permissions (format: rwx)

Link Operations:
  ln <src> <dst>        Create hard link
  link <src> <dst>      Create symbolic link
  unlink <name>         Remove symbolic link

Other Commands:
  help                  Show this help message
  exit                  Exit the file system
=========================================================================
```

- 退出文件系统

```bash
/> exit
Exiting Virtual File System.
```



### 文件操作

- 创建文件

```bash
touch <name>          Create a new empty file
```

```bash
/> touch test.txt
File created successfully

/> touch /dir/test.txt
File created successfully
```

- 删除文件

```bash
rm <name>             Remove a file
```

```bash
/> rm test.txt
File deleted successfully
```

- 移动、重命名文件

```bash
 mv <src> <dst>        Move/rename a file
```

```bash
//rename
/> mv test.txt test2.txt
File moved successfully

//move
/> mv test.txt /dir/test.txt
File moved successfully
/> mv test.txt /dir    
File moved successfully
```

- 复制文件
```bash
cp <src> <dst>        Copy a file
```

```bash
/> cp test.txt test_cp.txt
File copied successfully
```


- 写入文件
```bash
echo <text> > <file>  Write text to file
echo <text> >> <file> Append text to file
```

```bash
//Remplacer
/> echo HelloWorld > test.txt
File written successfully

//Ajouter du contenu après le fichier
/> echo world >> test.txt
Content appended successfully
```

- 显示文件内容

```bash
cat <file>            Display file content
```
```bash
/> cat test.txt
HelloWorldworld
```



### 目录操作

- 显示当前路径

```bash
pwd                   Show current working directory
```

```bash
/dir1/dir2> pwd
/dir1/dir2
```

- 改变当前目录

```bash
cd <path>             Change current directory
```

```bash
/> cd dir1
Changed directory to: /dir1

/> cd /dir1/dir2
Changed directory to: /dir1/dir2

/dir1/dir2> cd ..
Changed directory to: /dir1

/dir1> cd /
Changed directory to: /
```

- 创建新目录

```bash
mkdir <name>          Create a new directory
```

```bash
/> mkdir dir
Directory created successfully
```

- 删除目录

```bash
rmdir <name>          Remove an empty directory
rm -rf <name>         Remove a directory and its contents recursively
```

```bash
//Supprimer le répertoire vide
/> rmdir dir
Directory deleted successfully

//Supprimer les répertoires non vides
/> rmdir dir
Directory not empty
/> rm -rf dir
Warning: This will recursively delete 'dir' and all its contents.
Are you sure you want to continue? (yes/no): yes
Directory and all its contents deleted successfully
```

- 移动、重命名目录

```bash
mvdir <src> <dst>     Move/rename a directory
```

```bash
//Renomer
/> mvdir dir dir1
Directory moved successfully

//Deplacer
/> mvdir dir1 /mnt/dir1
Directory moved successfully
```

- 显示目录磁盘大小

```bash
du <name>            Calculate disk usage of a directory
```

```bash
/> du dir
3696 (3.61KB)	dir
```

### ls/tree命令

- ls

```bash
ls                    List files in current directory
ls -a                 List all files (including hidden)
ls -l                 List files in long format
ls -l <name>          Show detailed information of specific file
ls -la                List all files in long format
ls -it                List files with inode and type
ls -i <name>          Show inode of specific file
```

```bash
/> ls
dir
test.txt
test_link.txt
test_link_dur.txt
test_link_symbolique.txt

/> ls -a
.
..
dir
test.txt
test_link.txt
test_link_dur.txt
test_link_symbolique.txt

/> ls -l
Type Perms      Links Size     Modified             Name
----------------------------------------------------------
d    rwx        0    1320     2025-04-01 21:28:12  dir
-    rw-        0    0        2025-04-01 21:24:49  test.txt
-    rw-        1    0        2025-04-01 21:25:15  test_link.txt
-    rw-        1    0        2025-04-01 21:25:15  test_link_dur.txt
l    rw-        1    14       2025-04-01 21:25:30  test_link_symbolique.txt

/> ls -l dir
Type Perms      Links Size     Modified             Name
----------------------------------------------------------
d    rwx        0    1320     2025-04-01 21:28:12  dir

/> ls -la
Type Perms      Links Size     Modified             Name
----------------------------------------------------------
d    rwx        0    1848     2025-04-01 21:25:30  .
d    rwx        0    1848     2025-04-01 21:25:30  ..
d    rwx        0    1320     2025-04-01 21:28:12  dir
-    rw-        0    0        2025-04-01 21:24:49  test.txt
-    rw-        1    0        2025-04-01 21:25:15  test_link.txt
-    rw-        1    0        2025-04-01 21:25:15  test_link_dur.txt
l    rw-        1    14       2025-04-01 21:25:30  test_link_symbolique.txt

/> ls -it
Inode    Type         Name    
0        DIR          .       
0        DIR          ..      
1        DIR          dir     
2        FILE         test.txt
3        FILE         test_link.txt
3        FILE         test_link_dur.txt
4        SYMLINK      test_link_symbolique.txt

/> ls -i test.txt
2        test.txt
```

- tree

```bash
tree                  Display directory structure
tree -i               Display directory structure with inodes
```

```bash
/> tree

├── dir
│   └── dir2
│       └── test.txt
├── test.txt
├── test_link.txt
├── test_link_dur.txt
└── test_link_symbolique.txt -> /test_link.txt

/> tree -i
[0] 
├── [1] dir
│   └── [5] dir2
│       └── [6] test.txt
├── [2] test.txt
├── [3] test_link.txt
├── [3] test_link_dur.txt
└── [4] test_link_symbolique.txt -> /test_link.txt
```



### 权限操作

- 查看权限

```bash
perm <name>           Show file permissions
```

```bash
/> perm test.txt
-rw- test.txt

/> perm .
drwx .

/> perm /dir
drwx dir
```

- 修改权限

```bash
chmod <name> <perm>   Change file permissions (format: rwx)
```

```bash
/> chmod test.txt r-x
Permissions changed successfully
/> perm test.txt
-r-x test.txt

/> chmod dir r--
Permissions changed successfully
/> perm dir
dr-- dir
```



### 链接

- 硬链接

```bash
ln <src> <dst>        Create hard link
rm <name>             Supprimer le lien matériel
```

```bash
/> ln test.txt test_linkdur.txt
Hard link created successfully
```



- 符号链接

```bash
link <src> <dst>      Create symbolic link
unlink <name>         Remove symbolic link
```

```bash
//ficher
/> link test.txt test_linksym.txt
Symbolic link created successfully

/> unlink test_linksym.txt
Symbolic link deleted successfully


//Répertoire
/> link dir dir_sym
Symbolic link created successfully

/> unlink dir_sym
Symbolic link deleted successfully
```

