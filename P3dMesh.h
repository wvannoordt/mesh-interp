#pragma once
#include <vector>
#include <string>
#include "MdArray.h"
#include "v3.h"
#include "print.h"
#include <fstream>
#include <sstream>
struct P3dScalar
{
    P3dScalar(std::string name_in, v3<int> nxCell_in)
    {
        name = name_in;
        data.resize(nxCell_in[0]*nxCell_in[1]*nxCell_in[2]);
        nxCell = nxCell_in;
    }
    v3<int> nxCell;
    std::string name;
    std::vector<double> data;
    MdArray<double, 3> GetArray(void)
    {
        MdArray<double, 3> output(nxCell[0], nxCell[1], nxCell[2]);
        output.data = &data[0];
        return output;
    }
    void ReadCsv(std::string filename)
    {
        print("Read", filename);
        std::string line;
        std::ifstream infile(filename.c_str());
        size_t numRead = 0;
        while (numRead < data.size())
        {
            std::getline(infile, line);
            std::istringstream iss2(line);
            iss2>>data[numRead++];
        }
        infile.close();
    }
};
struct P3dZone
{
    std::vector<double> rawData;
    ~P3dZone(void) {for (auto v:vars) delete v;}
    v3<int> nx;
    bool is3d;
    std::vector<P3dScalar*> vars;
    MdArray<double, 4> GetArray(void)
    {
        MdArray<double, 4> output(nx[0], nx[1], nx[2], 3);
        output.data = &rawData[0];
        return output;
    }
    P3dScalar* AddScalar(std::string name)
    {
        v3<int> cell = nx;
        cell[0] -= 1;
        cell[1] -= 1;
        if (is3d) cell[2] -= 1;
        auto ns = new P3dScalar(name, cell);
        vars.push_back(ns);
        return ns;
    }
    P3dScalar* GetScalar(std::string name)
    {
        for (auto s:vars)
        {
            if (s->name == name) return s;
        }
        return NULL;
    }
};
class P3dMesh
{
    public:
        P3dMesh(std::string filename);
        ~P3dMesh(void)
        {
            for (auto z:zones)
            {
                if (z!=NULL) delete z;
            }
        }
        int NumZones(void) {return zones.size();}
        P3dZone* GetZone(int i) {return zones[i];}
        void OutputVtk(std::string filename);
        void OutputCellCentersCsv(std::string filename);
    private:
        void ReadFile(std::string filename);
        std::vector<P3dZone*> zones;
};