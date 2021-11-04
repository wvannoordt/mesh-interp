#include "P3dMesh.h"
#include "print.h"
#include <fstream>
#include <sstream>
P3dMesh::P3dMesh(std::string filename)
{
    ReadFile(filename);
}

void P3dMesh::ReadFile(std::string filename)
{
    print("Reading", filename);
    std::string line;
    std::ifstream infile(filename.c_str());
    int numZones;
    std::getline(infile, line);
    std::istringstream iss1(line);
    iss1 >> numZones;
    zones.resize(numZones, NULL);
    print("Found",  numZones, ((numZones==1)?("zone"):("zones")));
    for (int n = 0; n < numZones; n++)
    {
        zones[n] = new P3dZone();
        P3dZone* newZone = zones[n];
        std::getline(infile, line);
        std::istringstream iss2(line);
        int i = 0;
        newZone->nx = 1;
        while (iss2>>newZone->nx[i++]) ;
        size_t numPoints = newZone->nx[0]*newZone->nx[1]*newZone->nx[2];
        newZone->is3d = newZone->nx[2]>1;
        newZone->rawData.resize(numPoints*3, 0.0);

        size_t numRead = 0;
        while (numRead < 3*numPoints)
        {
            std::getline(infile, line);
            std::istringstream iss3(line);
            while (iss3 >> newZone->rawData[numRead]) numRead++;
        }
        
        print("Properties of zone", n);
        print(" -- nx:", newZone->nx[0]);
        print(" -- ny:", newZone->nx[1]);
        print(" -- nz:", newZone->nx[2]);
        print(" -- read", numRead, "points");
    }
    infile.close();
}

void P3dMesh::OutputCellCentersCsv(std::string filename)
{
    std::ofstream myfile(filename.c_str());
    for (auto zone: zones)
    {
        auto array = zone->GetArray();
        if (zone->is3d) 
        {
            CallError("csv output not ready for 3D");
        }
        for (int k = 0; k < zone->nx[2]; k++)
        {
            for (int j = 0; j < zone->nx[1]-1; j++)
            {
                for (int i = 0; i < zone->nx[0]-1; i++)
                {
                    v3<double> xyz;
                    xyz[0] = 0.25*(array(i,j,k,0)+array(i+1,j,k,0)+array(i,j+1,k,0)+array(i+1,j+1,k,0));
                    xyz[1] = 0.25*(array(i,j,k,1)+array(i+1,j,k,1)+array(i,j+1,k,1)+array(i+1,j+1,k,1));
                    xyz[2] = 0.25*(array(i,j,k,2)+array(i+1,j,k,2)+array(i,j+1,k,2)+array(i+1,j+1,k,2));
                    myfile << xyz[0] << "," << xyz[1] << "," << xyz[2] << "\n";
                }
            }
        }
    }
    myfile.close();
}

void P3dMesh::OutputVtk(std::string filename)
{
    if (zones.size()!=1) CallError("I can't output multiple/no zones to vtk yet :(");
    auto zone = zones[0];
    std::ofstream myfile(filename.c_str());
    myfile << "# vtk DataFile Version 3.0" << std::endl;
    myfile << "hello world" << std::endl;
    myfile << "ASCII" << std::endl;
    myfile << "DATASET STRUCTURED_GRID" << std::endl;
    myfile << "DIMENSIONS " << zone->nx[0] << " " << zone->nx[1] << " " << zone->nx[2] << std::endl;
    myfile << "POINTS " << zone->nx[0]*zone->nx[1]*zone->nx[2] << " float" << std::endl;
    auto array = zone->GetArray();
    for (int k = 0; k < zone->nx[2]; k++)
    {
        for (int j = 0; j < zone->nx[1]; j++)
        {
            for (int i = 0; i < zone->nx[0]; i++)
            {
                v3<double> xyz;
                xyz[0] = array(i, j, k, 0);
                xyz[1] = array(i, j, k, 1);
                xyz[2] = array(i, j, k, 2);
                myfile << xyz[0] << " " << xyz[1] << " " << xyz[2] << "\n";
            }
        }
    }
    myfile << "CELL_DATA " << (zone->nx[0]-1)*(zone->nx[1]-1)*(zone->nx[2]-(zone->is3d?1:0)) << std::endl;
    for (auto v:zone->vars)
    {
        myfile  << "SCALARS " << v->name << " float" << std::endl;
        myfile  << "LOOKUP_TABLE default" << std::endl;
        for (size_t i = 0; i < v->data.size(); i++)
        {
            myfile << v->data[i] << "\n";
        }
    }
    myfile.close();
    
}