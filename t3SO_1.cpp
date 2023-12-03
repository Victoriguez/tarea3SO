#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

// Definición de la estructura del i-nodo
struct Inode {
    std::string name;
    int size;
    std::string permissions;
    int unique_id;
    bool canHaveChildren;
    Inode* parent; 
    std::vector<Inode*> children;

};

// Función para crear un nuevo i-nodo
Inode* createInode(std::string name, std::string permissions, bool canHaveChildren, Inode* parent = nullptr) {
    static int id = 0;
    Inode* newInode = new Inode;
    newInode->name = name;
    newInode->size = 0;
    newInode->permissions = permissions;
    newInode->unique_id = id++;
    newInode->canHaveChildren = canHaveChildren;
    newInode->parent = parent;
    return newInode;
}

// Función para crear un archivo
Inode* createFile(std::string name, std::string permissions) {
    return createInode(name, permissions, false);
}

// Función para crear un directorio
Inode* createDirectory(std::string name, std::string permissions) {
    return createInode(name, permissions, true);
}

// Función para renombrar un i-nodo
void renameInode(Inode* inode, std::string newName) {
    inode->name = newName;
}



// Función para cambiar los permisos de un i-nodo
void changePermissions(Inode* inode, std::string newPermissions) {
    inode->permissions = newPermissions;
}

// Función para obtener los metadatos de un i-nodo
void getMetadata(Inode* inode) {
    std::cout << "Name: " << inode->name << "\n";
    std::cout << "Size: " << inode->size << "\n";
    std::cout << "Permissions: " << inode->permissions << "\n";
    std::cout << "Unique ID: " << inode->unique_id << "\n";
}

// Función para listar los i-nodos en el directorio actual
void listInodes(Inode* inode) {
    for (Inode* child : inode->children) {
        std::cout << child->name << "\n";
    }
}

// Función para añadir un i-nodo hijo a un i-nodo padre
void addChild(Inode* parent, Inode* child) {
    if (parent->canHaveChildren){
        child->parent = parent;
        parent->children.push_back(child);
    } else{
        std::cout << "This inode can't have children.\n";
    }
}

// Función para moverse entre directorios (asumiendo que el directorio es un i-nodo)
Inode* changeDirectory(Inode* currentDirectory, Inode* newDirectory) {
    return newDirectory;
}

// Función para encontrar un i-nodo por su nombre
Inode* findInode(Inode* root, std::string name) {
    if (root->name == name) {
        return root;
    }
    for (Inode* child : root->children) {
        Inode* found = findInode(child, name);
        if (found != nullptr) {
            return found;
        }
    }
    return nullptr;
}

// Función para eliminar un i-nodo
void deleteInode(Inode* &root, std::string name) {
    Inode* toDelete = findInode(root, name);
    if (toDelete == nullptr) {
        std::cout << "Inode not found.\n";
        return;
    }
    for (auto it = root->children.begin(); it != root->children.end(); ++it) {
        if (*it == toDelete) {
            root->children.erase(it);
            delete toDelete;
            return;
        }
    }
}

// Función para mover un i-nodo
void moveInode(Inode* &root, std::string name, Inode* newParent) {
    Inode* toMove = findInode(root, name);
    if (toMove == nullptr) {
        std::cout << "Inode not found.\n";
        return;
    }
    for (auto it = root->children.begin(); it != root->children.end(); ++it) {
        if (*it == toMove) {
            root->children.erase(it);
            newParent->children.push_back(toMove);
            return;
        }
    }
}

void saveInode(std::ofstream& file, Inode* inode, int level = 0) {
    if (file.is_open()) {
        for(int i = 0; i < level; i++) {
            file << "    ";
        }
        file << inode->name;
        if(inode->canHaveChildren) {
            file << "{\n";
            for (Inode* child : inode->children) {
                saveInode(file, child, level + 1);
            }
            for(int i = 0; i < level; i++) {
                file << "    ";
            }
            file << "}\n";
        } else {
            file << "\n";
        }
    } else {
        std::cerr << "Unable to open file for writing.\n";
    }
}

void saveInodes(Inode* root) {
    std::ofstream file("inodes.txt");
    saveInode(file, root);
}

Inode* loadInodes() {
    std::ifstream file("inodes.txt");
    if (!file) {
        return createDirectory("/", "rwx");
    }

    Inode* root = nullptr;
    Inode* current = nullptr;
    std::string line;
    while (std::getline(file, line)) {
        int level = 0;
        while (line[level] == ' ') {
            level++;
        }
        line = line.substr(level);
        if (line == "{") {
            continue;
        } else if (line == "}") {
            if (current) {
                current = current->parent;
            } else {
                std::cerr << "Error: Attempt to move up from root directory.\n";
            }
        } else {
            Inode* node = new Inode;
            node->name = line;
            node->canHaveChildren = line.back() == '{';
            if (node->canHaveChildren) {
                node->name.pop_back();
            }
            if (current) {
                addChild(current, node);
            } else {
                root = node;
            }
            current = node;
        }
    }

    return root;
}

int main() {
    Inode* root = loadInodes();
    Inode* currentDirectory = root;
    
    
    

    while (true) {
        if (currentDirectory) {
            std::cout << "Current directory: " << currentDirectory->name << "\n";
        } else {
            std::cout << "Current directory is not set.\n";
        }
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "mkdir") {
            std::string name, permissions;
            iss >> name >> permissions;
            Inode* newNode = createDirectory(name, permissions);
            addChild(currentDirectory, newNode);
        } else if (command == "touch") {
            std::string name, permissions;
            iss >> name >> permissions;
            Inode* newNode = createFile(name, permissions);
            addChild(currentDirectory, newNode);
        } else if (command == "rename") {
            std::string oldName, newName;
            iss >> oldName >> newName;
            Inode* inode = findInode(root, oldName);
            if (inode != nullptr) {
                renameInode(inode, newName);
            }
            saveInodes(root);
        } else if (command == "move") {
            std::string name, newParentName;
            iss >> name >> newParentName;
            Inode* newParent = findInode(root, newParentName);
            if (newParent != nullptr) {
                moveInode(root, name, newParent);
            }
            saveInodes(root);
        } else if (command == "changePermissions") {
            std::string name, permissions;
            iss >> name >> permissions;
            Inode* inode = findInode(root, name);
            if (inode != nullptr) {
                changePermissions(inode, permissions);
            }
            saveInodes(root);
        } else if (command == "getMetadata") {
            std::string name;
            iss >> name;
            Inode* inode = findInode(root, name);
            if (inode != nullptr) {
                getMetadata(inode);
            }
        } else if (command == "ls") {
            listInodes(currentDirectory);
        } else if (command == "del") {
            std::string name;
            iss >> name;
            deleteInode(root, name);
        } else if (command == "cd") {
            std::string name;
            iss >> name;
            Inode* newDirectory = findInode(root, name);
            if (newDirectory != nullptr) {
                currentDirectory = changeDirectory(currentDirectory, newDirectory);
            }
        } else if (command == "exit") {
            saveInodes(root);
            break;
        } else {
            std::cout << "Unknown command: " << command << "\n";
        }
    }

    return 0;
}