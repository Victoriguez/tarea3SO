#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <thread>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <sstream>

using namespace std::chrono;

class Directory;

static int ids = 0;

class Inode
{
protected:
    std::string nombre;
    int id;
    std::time_t creation_time;
    std::string ruta;
    std::string ruta_padre;
    Inode *padre = nullptr;

public:
    Inode(std::string nombre, std::string ruta_actual)
    {
        static int count_id = 1;
        this->id = count_id;
        count_id++;
        this->nombre = nombre;
        auto time_point = system_clock::now();
        this->creation_time = system_clock::to_time_t(time_point);
        this->ruta = ruta_actual + '/' + nombre;

        if (ruta_actual == "C:/Users/victo/Desktop/Tarea3SO")
        {
            ruta_padre = ruta;
            padre = nullptr;
        }
        else
        {
            ruta_padre = ruta_actual;
        }
    }

    virtual std::string getNombre() const = 0;
    virtual void printMetadata() const = 0;

    int getId() const
    {
        return this->id;
    }

    std::time_t getCreationTime() const
    {
        return this->creation_time;
    }

    virtual std::string getRuta() const = 0;

    virtual std::string getRutaPadre() const = 0;

    virtual void setPadre(Inode *padre) = 0;

    virtual void rename(std::string new_name, std::string ruta) = 0;
    virtual Inode *getPadre() = 0;

    /*virtual void save(std::ofstream& file) const {
        file.write(reinterpret_cast<const char*>(this), sizeof(*this));
    }

    virtual void load(std::ifstream& file) {
        file.read(reinterpret_cast<char*>(this), sizeof(*this));
    }

    virtual void create() const = 0;*/
};

class File : public Inode
{
private:
public:
    File(std::string nombre, std::string ruta_actual) : Inode(nombre, ruta_actual)
    {

        std::ofstream f(ruta);
    }

    std::size_t getSize() const
    {
        const std::string arch = ruta;

        std::ifstream archivo(arch, std::ios::binary);

        if (!archivo.is_open())
        {
            std::cout << "Error";
            return 1;
        }

        archivo.seekg(0, std::ios::end);
        std::size_t filesize = archivo.tellg();
        archivo.seekg(0, std::ios::beg);

        archivo.close();

        return filesize;
    }

    std::string getRuta() const override
    {
        return this->ruta;
    }

    std::string getRutaPadre() const override
    {
        if (padre != nullptr)
        {
            return padre->getRuta();
        }
        return ruta;
    }

    std::string getNombre() const override
    {
        return this->nombre;
    }

    void setPadre(Inode *padre_aux) override
    {
        padre = padre_aux;
    }

    Inode *getPadre() override
    {
        return padre;
    }

    void printMetadata() const override
    {
        std::cout << "nombre: " << nombre << '\n';
        std::cout << "ID: " << id << '\n';
        std::cout << "Size: " << getSize() << '\n';
        std::cout << "Ubicacion: " << ruta << '\n';
        std::cout << "Ubicacion padre: " << padre->getRuta() << '\n';
        std::cout << "Fecha de creacion: " << std::ctime(&creation_time) << '\n';
    }

    void rename(std::string new_name, std::string ruta) override
    {

        std::rename(this->ruta.c_str(), (ruta + '/' + new_name).c_str());
        this->ruta = ruta + '/' + new_name;
        this->nombre = new_name;
    }
};

class Directory : public Inode
{
private:
public:
    std::vector<Inode *> hijos;
    Directory(std::string nombre, std::string ruta_actual) : Inode(nombre, ruta_actual)
    {
        
        mkdir(ruta.c_str());
    }

    std::string getRuta() const override
    {
        return this->ruta;
    }

    std::string getRutaPadre() const override
    {
        if (padre != nullptr)
        {
            return padre->getRuta();
        }
        return ruta;
    }

    std::string getNombre() const override
    {
        return this->nombre;
    }

    std::size_t getSize() const
    {
        int s{0};
        if (hijos.size() == 0)
        {
            return 0;
        }
        else
        {

            for (int i = 0; i < hijos.size(); i++)
            {
                const std::string arch = hijos[i]->getRuta();

                std::ifstream archivo(arch, std::ios::binary);

                if (!archivo.is_open())
                {
                    std::cout << "Error";
                    return 1;
                }

                archivo.seekg(0, std::ios::end);
                std::size_t filesize = archivo.tellg();
                archivo.seekg(0, std::ios::beg);

                archivo.close();

                s += filesize;
            }
        }
        return s;
    }

    void setPadre(Inode *padre_aux) override
    {
        padre = padre_aux;
    }

    Inode *getPadre() override
    {
        return padre;
    }

    void addFile(File *f)
    {
        hijos.push_back(f);
        Directory *padre_aux = this;
        hijos.back()->setPadre(padre_aux);
    }
    void addDirectory(Directory *d)
    {
        hijos.push_back(d);
        Directory *padre_aux = this;
        hijos.back()->setPadre(padre_aux);
    }

    void printMetadata() const override
    {
        std::cout << "nombre: " << nombre << '\n';
        std::cout << "ID: " << id << '\n';
        std::cout << "Size: " << getSize() << '\n';
        std::cout << "Ubicacion: " << ruta << '\n';
        std::cout << "Ubicacion padre: " << padre->getRuta() << '\n';
        std::cout << "Fecha de creacion: " << std::ctime(&creation_time) << '\n';
    }

    void rename(std::string new_name, std::string ruta) override
    {
        std::rename(this->ruta.c_str(), (ruta + '/' + new_name).c_str());
        this->ruta = ruta + '/' + new_name;
        this->nombre = new_name;
    }
};

class FileSystem
{
private:
    Directory *root;
    std::string current_ruta;
    Directory *current_directory;

public:
    FileSystem()
    {
        root = new Directory("root", "/home/lucas/workspace/universidad/sistemas-operativos/tarea3");
        current_ruta = root->getRuta();
        current_directory = root;
    }

    void addFile(std::string nombre)
    {
        std::cout << "Current ruta: " << current_ruta << '\n';
        std::vector<std::string> partes;
        std::istringstream stream(nombre);

        std::string parte;
        while (std::getline(stream, parte, '/'))
        {
            partes.push_back(parte);
        }

        if(partes.size() == 1)
        {
            File *f = new File(nombre, current_ruta);
            current_directory->addFile(f);
        }
        else
        {
            std::cout << "ELSE rename\n";
            Directory *aux = current_directory;
            std::cout << "Partes: " << partes.size() << '\n';
            std::cout << aux->getRuta() << '\n';
            bool new_name_exist = false;
            for (int i = 0; i < partes.size(); i++)
            {

                for (int j = 0; j < aux->hijos.size(); j++)
                {
                    std::cout << "for " << j << " =====" << aux->getRuta() << '\n';
                    if (partes.at(i) == aux->hijos[j]->getNombre())
                    {
                        std::cout << "Parte " << i << ": " << partes.at(i) << " existe\n";
                        if (i == partes.size() - 1)
                        {
                            std::cout << "Hola\n";
                            new_name_exist = true;
                            break;
                        }
                        else
                        {
                            std::cout << "Adios\n";
                            aux = static_cast<Directory *>(aux->hijos[j]);
                            j = -1;
                            std::cout << aux->getRuta() << '\n';
                            if(aux->hijos.size() == 0)
                            {
                                std::cout << "HOLAAAAAAAAAAAAA: " << aux->getRuta() << '\n';
                                File *f = new File(partes.back(), aux->getRuta());
                                aux->addFile(f);
                                break;

                            }

                        }
                    }

                }
                if(new_name_exist) {
                    break;
                }
            }
            if (!new_name_exist)
            {
                std::cout << "EL nombre no existe\n";
                File *f = new File(partes.back(), aux->getRuta());
                aux->addFile(f);
            }
            else {
                std::cout << "El nombre ya exite\n";
            }
           
        }
       
    }
    void addDirectory(std::string nombre)
    {
        std::cout << "Current ruta: " << current_ruta << '\n';
        std::vector<std::string> partes;
        std::istringstream stream(nombre);

        std::string parte;
        while (std::getline(stream, parte, '/'))
        {
            partes.push_back(parte);
        }

        if(partes.size() == 1)
        {
            Directory *d = new Directory(nombre, current_ruta);
            current_directory->addDirectory(d);
        }
        else
        {
            std::cout << "ELSE rename\n";
            Directory *aux = current_directory;
            std::cout << "Partes: " << partes.size() << '\n';
            std::cout << aux->getRuta() << '\n';
            bool new_name_exist = false;
            for (int i = 0; i < partes.size(); i++)
            {

                for (int j = 0; j < aux->hijos.size(); j++)
                {
                    std::cout << "for " << j << " =====" << aux->getRuta() << '\n';
                    if (partes.at(i) == aux->hijos[j]->getNombre())
                    {
                        std::cout << "Parte " << i << ": " << partes.at(i) << " existe\n";
                        if (i == partes.size() - 1)
                        {
                            std::cout << "Hola\n";
                            new_name_exist = true;
                            break;
                        }
                        else
                        {
                            std::cout << "Adios\n";
                            aux = static_cast<Directory *>(aux->hijos[j]);
                            j = -1;
                            std::cout << aux->getRuta() << '\n';
                            if(aux->hijos.size() == 0)
                            {
                                std::cout << "HOLAAAAAAAAAAAAA: " << aux->getRuta() << '\n';
                                Directory *d = new Directory(partes.back(), aux->getRuta());
                                aux->addDirectory(d);
                                break;

                            }

                        }
                    }

                }
                if(new_name_exist) {
                    break;
                }
            }
            if (!new_name_exist)
            {
                std::cout << "EL nombre no existe\n";
                Directory *d = new Directory(partes.back(), aux->getRuta());
                aux->addDirectory(d);
            }
            else {
                std::cout << "El nombre ya exite\n";
            }
           
        }
    }

    std::string getRuta()
    {
        return current_directory->getRuta();
    }

    void printData()
    {
        for (int i = 0; i < current_directory->hijos.size(); i++)
        {
            current_directory->hijos[i]->printMetadata();
            std::cout << "\n\n";
        }
    }
    void cambio_directorio(Directory *aux1)
    {
        this->current_directory = aux1;
        this->current_ruta = current_directory->getRuta();
    }

    void rename(std::string old_name, std::string new_name, std::string ruta_actual)
    {

        bool exist_name = false;
        int index{};

        std::vector<std::string> partes;
        std::istringstream stream(old_name);

        std::string parte;
        while (std::getline(stream, parte, '/'))
        {
            partes.push_back(parte);
        }

        if (partes.size() == 1)
        {
            for (int i = 0; i < current_directory->hijos.size(); i++)
            {
                if (partes.at(0) == current_directory->hijos[i]->getNombre())
                {
                    index = i;
                }
                if (new_name == current_directory->hijos[i]->getNombre())
                {
                    exist_name = true;
                }
            }
            if (exist_name == false)
            {
                current_directory->hijos[index]->rename(new_name, ruta_actual);
            }
        }
        else
        {
            Directory *aux = current_directory;
            bool new_name_exist = false;
            bool old_name_exist = false;
            int index{};
            for (int i = 0; i < partes.size(); i++)
            {

                for (int j = 0; j < aux->hijos.size(); j++)
                {
                    if (partes.at(i) == aux->hijos[j]->getNombre())
                    {

                        if (i == partes.size() - 1)
                        {
                            old_name_exist = true;

                            index = j;
                        }
                        else
                        {

                            aux = static_cast<Directory *>(aux->hijos[j]);
                            j = -1;
                        }
                    }

                    if (partes.at(i) == new_name)
                    {
                        new_name_exist = true;
                        break;
                    }
                }
            }
            if (old_name_exist == true and new_name_exist == false)
            {
                aux->hijos[index]->rename(new_name, aux->getRuta());
            }
        }
    }

    void cd_next(std::string ruta)
    {
        if (ruta == "..")
        {
            Inode *aux = current_directory->getPadre();
            current_directory = static_cast<Directory *>(aux);
            current_ruta = current_directory->getRuta();
        }
        else
        {
            std::vector<std::string> partes;
            std::istringstream stream(ruta);

            std::string parte;
            while (std::getline(stream, parte, '/'))
            {
                partes.push_back(parte);
            }

            if (partes.size() == 1)
            {

                for (int i = 0; i < current_directory->hijos.size(); i++)
                {

                    if (partes.at(0) == current_directory->hijos[i]->getNombre())
                    {
                        current_directory = static_cast<Directory *>(current_directory->hijos[i]);
                        cambio_directorio(current_directory);
                        std::cout << current_ruta << '\n';

                        break;
                    }
                }
            }
            else
            {
                bool existe_ruta = false;
                Directory *aux = current_directory;
                for (int i = 0; i < partes.size(); i++)
                {

                    for (int j = 0; j < aux->hijos.size(); j++)
                    {

                        if (partes.at(i) == aux->hijos[j]->getNombre())
                        {

                            if (i == partes.size() - 1)
                            {
                                existe_ruta = true;
                                aux = static_cast<Directory *>(aux->hijos[j]);
                                break;
                            }
                            else
                            {

                                aux = static_cast<Directory *>(aux->hijos[j]);
                                j = -1;
                            }
                        }
                    }
                }

                if (!existe_ruta)
                {
                    std::cout << "La ruta especificada no existe\n";
                }
                else
                {

                    cambio_directorio(aux);
                }
            }
        }
    }
};

int main()
{

    FileSystem *f = new FileSystem();
    std::string ruta_actual = f->getRuta();

    while (true)
    {

        std::cout << "Ruta actual: " << f->getRuta() << '\n';

        std::string comando{};
        std::cin >> comando;

        if (comando == "touch")
        {
            std::string nombre{};
            std::cin >> nombre;
            f->addFile(nombre);
        }
        else if (comando == "mkdir")
        {
            std::string nombre{};
            std::cin >> nombre;
            f->addDirectory(nombre);
        }
        else if (comando == "mv")
        {
            std::string ruta_arch{};
            std::cin >> ruta_arch;

            std::string new_name{};
            std::cin >> new_name;

            f->rename(ruta_arch, new_name, ruta_actual);
        }
        else if (comando == "cd")
        {
            std::string ruta_arch{};
            std::cin >> ruta_arch;

            f->cd_next(ruta_arch);
        }
        else
        {
            f->printData();
            std::cout << '\n';
        }
    }

    return 0;
}