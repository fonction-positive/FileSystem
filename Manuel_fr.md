## Projet de OS

### Le fonctionnement du système de fichiers

Utilisez la commande `make` pour compiler le projet, puis exécutez le projet avec `./FileSystem` (ou utilisez directement la commande `make run` pour compiler et exécuter rapidement le projet).

Vous pouvez également utiliser différentes commandes `make` pour effectuer différentes opérations :

```bash
  % make help
  
  Cibles make disponibles : 
  make        - Compiler le programme 
  make clean  - Nettoyer les fichiers générés 
  make run    - Exécuter le programme 
  make test   - Recompiler et exécuter 
  make debug  - Compiler en mode débogage
  make memcheck - Exécuter la vérification de la mémoire 
  make help   - Afficher cette aide 
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



### Mécanisme de stockage du système de fichiers

Après avoir initialisé le système de fichiers avec la commande `mkfs` (formatage de la partition disque), un fichier `vitrual_disk.dat` sera automatiquement créé en tant que disque du système de fichiers. Toutes les opérations effectuées sur le système de fichiers seront ensuite sauvegardées dans ce disque virtuel.

- Si vous n'avez pas d'abord initialisé le disque avec la commande `mkfs`, le message suivant apparaîtra :

```
/> [commande]
=========================================================================
                    File system is not initialized.                                 
                    Please run 'mkfs' to format the disk.                          
=========================================================================
```

- Disque virtuel formaté avec succès

```
/> mkfs 
Virtual disk formatted successfully
```



### Opérations de base du système de fichiers

- Afficher les informations d’aide (documentation des commandes du système de fichiers)

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

- Quitter le système de fichiers

```bash
/> exit
Exiting Virtual File System.
```



### Opérations sur les fichiers

- Créer un fichier

```bash
touch <name>          Create a new empty file
```

```bash
/> touch test.txt
File created successfully

/> touch /dir/test.txt
File created successfully
```

- Supprimer le fichier

```bash
rm <name>             Remove a file
```

```bash
/> rm test.txt
File deleted successfully
```

- Déplacer et renommer des fichiers

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

- Copier le fichier
```bash
cp <src> <dst>        Copy a file
```

```bash
/> cp test.txt test_cp.txt
File copied successfully
```


- Écrire dans le fichier
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

- Afficher le contenu du fichier

```bash
cat <file>            Display file content
```
```bash
/> cat test.txt
HelloWorldworld
```



### Opération de répertoire

- Afficher le chemin actuel

```bash
pwd                   Show current working directory
```

```bash
/dir1/dir2> pwd
/dir1/dir2
```

- Changer le répertoire courant

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

- Créer un nouveau répertoire

```bash
mkdir <name>          Create a new directory
```

```bash
/> mkdir dir
Directory created successfully
```

- Supprimer le répertoire

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

- Déplacer et renommer le répertoire

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

- Afficher la taille du disque du répertoire

```bash
du <name>            Calculate disk usage of a directory
```

```bash
/> du dir
3696 (3.61KB)	dir
```

### ls/tree Commande

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



### Opération d'autorisation

- Voir les autorisations

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

- Modifier les autorisations

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



### Lien

- Lien dur

```bash
ln <src> <dst>        Create hard link
rm <name>             Supprimer le lien matériel
```

```bash
/> ln test.txt test_linkdur.txt
Hard link created successfully
```



- Lien symbolique

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

