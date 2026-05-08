/**
 * 实验：目录树查看器（仿 Linux tree 命令）
 * 学号：2504020103  姓名：阚紫瑄
 * 说明：请补全所有标记为 TODO 的函数体，不要修改其他代码。
 * 目录树查看器（仿 Linux tree 命令）
 * 完整实现版本（C语言，左孩子右兄弟二叉树）
 * 编译：gcc -o tree tree.c -std=c99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// ================== 二叉树结点定义 ==================
typedef struct FileNode {
    char *name;                  // 文件/目录名
    int isDir;                   // 1:目录 0:文件
    struct FileNode *firstChild; // 左孩子：第一个子项
    struct FileNode *nextSibling;// 右兄弟：下一个同层项
} FileNode;

// ================== 函数声明 ==================
FileNode* createNode(const char *name, int isDir);
int cmpNode(const void *a, const void *b);
FileNode* buildTree(const char *path);
void printTree(FileNode *node, const char *prefix, int isLast);
int countNodes(FileNode *root);
int countLeaves(FileNode *root);
int treeHeight(FileNode *root);
void countDirFile(FileNode *root, int *dirs, int *files);
void freeTree(FileNode *root);
char* getBaseName(void);

// ================== 需要补全的函数 ==================

// 创建新结点（分配内存、复制字符串、初始化指针）
FileNode* createNode(const char *name, int isDir) {
    FileNode *node = (FileNode*)malloc(sizeof(FileNode));
    if (node) {
        node->name = strdup(name);
        node->isDir = isDir;
        node->firstChild = NULL;
        node->nextSibling = NULL;
    }
    return node;
}

// 比较函数，用于 qsort 对子项按名称排序
int cmpNode(const void *a, const void *b) {
    FileNode *na=*(FileNode**)a;
    FileNode *nb=*(FileNode**)b;
    return strcmp(na->name,nb->name);
    return 0;
}

// 递归构建目录树（核心难点）
FileNode* buildTree(const char *path) {
    // 1. opendir 打开目录，失败返回 NULL
    DIR *dir=opendir(path);
    if(!dir){
        perror("opendir");
        return NULL;
    }
    // 2. 从 path 中提取最后的目录名作为当前结点名（注意处理根目录"/"）
    const char *base=strrchr(path,'/');
    base=base?base+1:path;
    FileNode *curDir=createNode(base,1);

    // 3. 创建当前目录结点
    // 4. 循环 readdir，跳过 "." 和 ".."
    struct dirent *entry;
    FileNode **children=NULL;
    int childCount=0;
    while ((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)
            continue;
        char fullPath[1024];
        snprintf(fullPath,sizeof(fullPath),"%s/%s",path,entry->d_name);
        // 5. 拼接完整路径，用 stat 判断类型
        // 6. 若是目录，递归调用 buildTree；若是普通文件，调用 createNode
        struct stat st;
        if(stat(fullPath,&st)!=0) continue;
        FileNode *child=NULL;
        if(S_ISDIR(st.st_mode)){
            child=buildTree(fullPath);
        }else if(S_ISREG(st.st_mode)){
            child=createNode(entry->d_name,0);
        }
        // 7. 将得到的子结点存入临时数组
        if(child){
            children=(FileNode**)realloc(children,(childCount+1)*sizeof(FileNode*));
            children[childCount++]=child;
        }
    }
    // 8. 关闭目录
    closedir(dir);
    
    // 9. 对子结点数组排序（调用 qsort 和 cmpNode）
    // 10. 将排序后的子结点链接成兄弟链表（firstChild 指向第一个，后续 nextSibling）
    if(childCount>0){
        qsort(children,childCount,sizeof(FileNode*),cmpNode);
        curDir->firstChild=children[0];
        for (int i = 0; i < childCount-1; i++)
        {
            children[i]->nextSibling=children[i+1];
        }
        
    }
    // 11. 释放临时数组，返回当前目录结点
    free(children);
    return curDir;
    return NULL;
}

// 树形输出（仿 tree 命令）
void printTree(FileNode *node, const char *prefix, int isLast) {
    // 1. 如果 node 为空，返回
    // 2. 输出前缀、分支符号（isLast ? "`-- " : "|-- "）、结点名
    // 3. 如果是目录，输出 "/"
    // 4. 换行
    if(!node){
        return;
    }
    printf("%s",prefix);
    printf("%s",isLast?"`-- ":"|-- ");
    if(node->isDir){
        printf("%s/\n",node->name);
    }
    // 5. 
    if(!node->firstChild){
        return;
    }
    // 6. 遍历孩子链表，对每个孩子：
    //     计算新前缀 = prefix + (isLast ? "    " : "|   ")
    //     判断是否为最后一个孩子
    //     递归调用 printTree
    FileNode *child = node->firstChild;
    int childNum = 0;
    FileNode *tmp = child;
    while (tmp) { childNum++; tmp = tmp->nextSibling; }
    int idx = 0;
    while (child) {
        int lastChild = (++idx == childNum);
        char newPrefix[1024];
        snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isLast ? "    " : "|   ");
        printTree(child, newPrefix, lastChild);
        child = child->nextSibling;
    }
}

// 统计二叉树结点总数
int countNodes(FileNode *root) {
    if(!root){
        return 0;
    }
    return 1+countNodes(root->firstChild)+countNodes(root->nextSibling);
}

// 统计叶子结点数（firstChild == NULL 的结点）
int countLeaves(FileNode *root) {
    if(!root){
        return 0;
    }
    int leaf=(root->firstChild==NULL)?1:0;
    return leaf + countLeaves(root->firstChild) + countLeaves(root->nextSibling);
}

// 计算二叉树高度（根深度为1，空树高度为0）
int treeHeight(FileNode *root) {
    if (!root) return 0;
    int childH = treeHeight(root->firstChild);
    int siblingH = treeHeight(root->nextSibling);
    int h = childH + 1;
    return (h > siblingH) ? h : siblingH;
    return 0;
}

// 统计目录数和文件数（遍历整棵树）
void countDirFile(FileNode *root, int *dirs, int *files) {
   if (!root) return;
    if (root->isDir) (*dirs)++;
    else (*files)++;
    countDirFile(root->firstChild, dirs, files);
    countDirFile(root->nextSibling, dirs, files);
}

// 释放整棵树的内存
void freeTree(FileNode *root) {
    if (!root) return;
    freeTree(root->firstChild);
    freeTree(root->nextSibling);
    free(root->name);
    free(root);
}

// 获取当前工作目录的“基本名称”（用于显示根结点名）
char* getBaseName(void) {
    char *cwd = getcwd(NULL, 0);
    if (!cwd) return strdup(".");
    char *base = strrchr(cwd, '/');
    char *res = base ? strdup(base+1) : strdup(cwd);
    free(cwd);
    return res;
    return NULL;
}

int main(int argc, char *argv[]) {
    char targetPath[1024];
    if (argc >= 2) {
        strncpy(targetPath, argv[1], sizeof(targetPath)-1);
        targetPath[sizeof(targetPath)-1] = '\0';
    } else {
        if (getcwd(targetPath, sizeof(targetPath)) == NULL) {
            perror("getcwd");
            return 1;
        }
    }

    int len = strlen(targetPath);
    if (len > 0 && targetPath[len-1] == '/')
        targetPath[len-1] = '\0';

    struct stat st;
    if (stat(targetPath, &st) != 0) {
        perror("stat");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "错误: %s 不是目录\n", targetPath);
        return 1;
    }

    FileNode *root = buildTree(targetPath);
    if (!root) {
        fprintf(stderr, "无法构建目录树\n");
        return 1;
    }

    // 输出根目录名
    char *displayName = NULL;
    if (argc >= 2) {
        displayName = root->name;
    } else {
        displayName = getBaseName();
    }
    printf("%s/\n", displayName);
    if (argc < 2) free(displayName);

    FileNode *child = root->firstChild;
    int childCount = 0;
    FileNode *tmp = child;
    while (tmp) { childCount++; tmp = tmp->nextSibling; }
    int idx = 0;
    while (child) {
        int isLast = (++idx == childCount);
        printTree(child, "", isLast);
        child = child->nextSibling;
    }

    int dirs = 0, files = 0;
    countDirFile(root, &dirs, &files);
    printf("\n%d 个目录, %d 个文件\n", dirs, files);
    printf("二叉树结点总数: %d\n", countNodes(root));
    printf("叶子结点数: %d\n", countLeaves(root));
    printf("树的高度: %d\n", treeHeight(root));

    freeTree(root);
    return 0;
}