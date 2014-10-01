#ifndef DIR_TREE_H
#define DIR_TREE_H

#include <string>
#include <vector>

class DirTreeNode {
  public:
  DirTreeNode(const std::string &name, DirTreeNode *parent, int level);
  DirTreeNode *add(const char *name, int level);
  const std::string fullPath();
  void inspect();
  void mkImgui(DirTreeNode **addedDir);

  std::string name;
  DirTreeNode *parent;
  int level;
  std::vector<DirTreeNode *> children;
};

class DirTree {
  public:
  DirTree(const std::string &rootName);
  void inspect();
  void add(const char* name, int level);;
  void imgui(DirTreeNode **addedDir);
  const std::string &rootName();

  private:

  int currentLvl;
  DirTreeNode *root;
  DirTreeNode *currentParent;
  DirTreeNode *lastNode;
};

#endif

