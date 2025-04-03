CC = gcc
CFLAGS = -Wall -Wextra -g
# Liste des fichiers source, incluant tous les fichiers .c / 源文件列表，包含所有.c文件
SRCS = main.c system.c dir.c file.c list.c perm.c link.c help.c
OBJS = $(SRCS:.c=.o)
TARGET = FileSystem
VDISK = virtual_disk.dat

# Cible par défaut / 默认目标
all: $(TARGET)

# Générer l'exécutable / 生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Générer les fichiers objets, ajouter des dépendances explicites / 生成目标文件，添加明确的依赖关系
%.o: %.c filesystem.h
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyer les fichiers générés / 清理编译产物
clean:
	rm -f $(OBJS) $(TARGET) $(VDISK)

# Exécuter le programme / 运行程序
run: $(TARGET)
	./$(TARGET)

# Recompiler et exécuter / 重新编译并运行
test: clean all run

# Support de débogage / 调试支持
debug: CFLAGS += -DDEBUG -g3
debug: clean all

# Vérification de la mémoire / 内存检查
memcheck: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

# Informations d'aide / 帮助信息
help:
	@echo "Cibles make disponibles : / 可用的 make 目标："
	@echo "  make        - Compiler le programme / 编译程序"
	@echo "  make clean  - Nettoyer les fichiers générés / 清理编译产物"
	@echo "  make run    - Exécuter le programme / 运行程序"
	@echo "  make test   - Recompiler et exécuter / 重新编译并运行"
	@echo "  make debug  - Compiler en mode débogage / 以调试模式编译"
	@echo "  make memcheck- Exécuter la vérification de la mémoire / 运行内存检查"
	@echo "  make help   - Afficher cette aide / 显示此帮助信息"

# Déclarer les cibles factices / 声明伪目标
.PHONY: all clean run test debug memcheck help