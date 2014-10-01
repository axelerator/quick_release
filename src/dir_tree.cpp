#include "dir_tree.h"

#include <ftw.h>
#include <sys/stat.h>
#include <limits.h>     /* for PATH_MAX */
#include <unistd.h>	/* for getdtablesize(), getcwd() declarations */
#include "../include/imgui/imgui.h"
#include <iostream>

DirTree *currentDirTree;
extern int ftwCallback(const char *file, const struct stat *sb, int flag, struct FTW *s);

DirTreeNode::DirTreeNode(const std::string &name, DirTreeNode *parent, int level) : name(name), parent(parent), level(level) {}

DirTreeNode *DirTreeNode::add(const char *name, int level) {
  std::string fileName = name;
  DirTreeNode *child = new DirTreeNode(fileName, this, level);
  children.push_back(child);
  return child;
}

const std::string DirTreeNode::fullPath() {
  auto prefix = level > 1 ? parent->fullPath() : "./";
  return  prefix + name + "/";
}

void DirTreeNode::inspect() {
  printf("%*s", level , "");	/* indent over */
  std::cout << fullPath() << "(" << children.size() << ")" << std::endl;
  for (auto child : children) {
    child->inspect();
  }
}

void DirTreeNode::mkImgui(DirTreeNode **addedDir) {
  if (name.c_str()[0] == '.' && level > 0) return;
  ImGui::PushID(fullPath().c_str());
  if (level == 0) {
      if (ImGui::SmallButton("..")) {
        *addedDir = (DirTreeNode *)-1;
      }
      for (auto child : children) {
        child->mkImgui(addedDir);
      }
  } else if (children.size()) {
    const bool open = ImGui::TreeNode(name.c_str());
    ImGui::SameLine();
    if ( ImGui::SmallButton("add")) {
        *addedDir = this;
    }
    if (open) {
      for (auto child : children) {
        child->mkImgui(addedDir);
      }
      ImGui::TreePop();
    }
  } else {
      ImGui::Text(name.c_str());
      ImGui::SameLine();
      if (ImGui::SmallButton("add")) {
        *addedDir = this;
      }
  }
  ImGui::PopID();

}

DirTree::DirTree(const std::string &rootName) {
  currentDirTree = this;
  int flags = FTW_PHYS;
  currentLvl = 0;
  root = new DirTreeNode(rootName, 0, 0);
  currentParent = root;
  lastNode = root;
  int res = nftw(rootName.c_str(), ftwCallback,4, flags);
}

void DirTree::inspect() {
  root->inspect();
}

void DirTree::add(const char* name, int level) {
  if (level == 0) return;
  if (level == lastNode->level) {
    lastNode = lastNode->parent->add(name, level);
  } else if (level > lastNode->level) {
    lastNode = lastNode->add(name, level);
  } else if (level < lastNode->level) {
    int steps = lastNode->level - level + 1;
    DirTreeNode *p = lastNode;
    for (int i = 0; i < steps; ++i) {
      p = p->parent;
    }

    if (!p) {std::cout << "current parent nil" << std::endl; exit(0);}
    lastNode = p->add(name, level);
  }
};

void DirTree::imgui(DirTreeNode **addedDir) {
  root->mkImgui(addedDir);
}

const std::string &DirTree::rootName() {
  return root->name;
}

int ftwCallback(const char *file, const struct stat *sb, int flag, struct FTW *s) {
	int retval = 0;
	const char *name = file + s->base;

  if (flag == FTW_D) {
    currentDirTree->add(name, s->level);
  }
  return 0;
}

