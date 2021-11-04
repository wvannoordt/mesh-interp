#include "AveragedFlowField.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "CallError.h"

AveragedFlowField::AveragedFlowField(std::string avgFile, std::string avgSquareFile, std::string crossTermsFile)
{
    Build(avgFile, avgSquareFile, crossTermsFile);
}

void AveragedFlowField::Build(std::string avgFile, std::string avgSquareFile, std::string crossTermsFile)
{
    this->numPoints = 1;
    this->numBlocks = 0;
    this->numVars = 0;
    InferPropertiesFromFlowAverageFile(avgFile, this->numPoints, this->numBlocks, this->numVars);
    size_t numElements = this->numPoints[0]*this->numPoints[1]*this->numBlocks*this->numVars;
    rawData.resize(numElements, 0.0);
    auto array = this->GetArray();
    std::vector<int> primsSrc {0, 1, 2, 3, 4, 5, 6};
    std::vector<int> primsDest{varid::x, varid::y, varid::p, varid::u, varid::v, varid::w, varid::T};
    
    std::vector<int> prims2Src {2, 3, 4, 5, 6};
    std::vector<int> prims2Dest{varid::p2, varid::u2, varid::v2, varid::w2, varid::T2};
    
    std::vector<int> crossSrc {2, 3, 4};
    std::vector<int> crossDest{varid::uv, varid::uw, varid::vw};
    
    ReadFile(array, avgFile,        primsSrc,  primsDest);
    ReadFile(array, avgSquareFile,  prims2Src, prims2Dest);
    ReadFile(array, crossTermsFile, crossSrc,  crossDest);
    
    print("Read", numElements*sizeof(double), "bytes");
    
    CreateBoundingBoxes();
}

MdArray<double, 2> AveragedFlowField::GetBlockBoundingBoxes(void)
{
    MdArray<double, 2> output(4, this->numBlocks);
    output.data = &boundBoxRawData[0];
    return output;
}

MdArray<double, 2> AveragedFlowField::GetBlockSpacing(void)
{
    MdArray<double, 2> output(2, this->numBlocks);
    output.data = &dxRawData[0];
    return output;
}

void AveragedFlowField::CreateBoundingBoxes(void)
{
    print("Building bounding boxes");
    boundBoxRawData.resize(4*numBlocks, 0.0);
    dxRawData.resize(2*numBlocks, 0.0);
    
    double xmin = 0.0;
    double ymin = 0.0;
    
    double xmax = 0.0;
    double ymax = 0.0;
    
    auto array = this->GetArray();
    auto boundingBoxes = this->GetBlockBoundingBoxes();
    auto dx = this->GetBlockSpacing();
    for (int lb = 0; lb < this->numBlocks; lb++)
    {
        dx(0, lb) = array(varid::x, 1, 0, lb)-array(varid::x, 0, 0, lb);
        dx(1, lb) = array(varid::y, 0, 1, lb)-array(varid::y, 0, 0, lb);
        xmin = array(varid::x, 0, 0, lb) - 0.5*dx(0, lb);
        xmax = array(varid::x, this->numPoints[0]-1, 0, lb) + 0.5*dx(0, lb);
        ymin = array(varid::y, 0, 0, lb) - 0.5*dx(1, lb);
        ymax = array(varid::y, 0, this->numPoints[1]-1, lb) + 0.5*dx(1, lb);
        boundingBoxes(0, lb) = xmin;
        boundingBoxes(1, lb) = xmax;
        boundingBoxes(2, lb) = ymin;
        boundingBoxes(3, lb) = ymax;
    }
}

void AveragedFlowField::OutputVtk(std::string filename)
{
    int nx = this->numPoints[0];
    int ny = this->numPoints[1];
    print("Writing", filename);
    std::ofstream myfile;
    myfile.open(filename);
    myfile << "# vtk DataFile Version 3.0" << std::endl;
    myfile << "hello world" << std::endl;
    myfile << "ASCII" << std::endl;
    myfile << "DATASET UNSTRUCTURED_GRID" << std::endl;
    myfile << "POINTS " << numBlocks*nx*ny*4 << " float" << std::endl;
    auto bbox = GetBlockBoundingBoxes();
    auto dx = this->GetBlockSpacing();
    double dxBlock[2] = {0.0, 0.0};
    double bboxCell[4] = {0.0, 0.0, 0.0, 0.0};
    print(" --> Write points");
    for (size_t lb = 0; lb < this->numBlocks; lb++)
    {
        dxBlock[0] = dx(0, lb);
        dxBlock[1] = dx(1, lb);
        for (int j = 0; j < ny; j++)
        {
            for (int i = 0; i < nx; i++)
            {
                bboxCell[0] = bbox(0, lb) + i*dxBlock[0];
                bboxCell[1] = bboxCell[0] + dxBlock[0];
                
                bboxCell[2] = bbox(2, lb) + j*dxBlock[1];
                bboxCell[3] = bboxCell[2] + dxBlock[1];
                
                for (int pt = 0; pt < 4; pt++)
                {
                    myfile << bboxCell[pt&1] << " " << bboxCell[2+((pt>>1)&1)] << " " << "0.0 " << std::endl;
                }
            }
        }
    }
    print(" --> Write cells");
    myfile << "CELLS " << numBlocks*nx*ny << " " << numBlocks*nx*ny*5 << std::endl;
    size_t idx = 0;
    for (size_t a = 0; a < numBlocks*nx*ny; a++)
    {
    	myfile  << "4 ";
    	myfile  << idx++ << " ";
    	myfile  << idx++ << " ";
    	myfile  << idx++ << " ";
    	myfile  << idx++ << std::endl;
    }
    print(" --> Write types");
    myfile << "CELL_TYPES " << numBlocks*nx*ny << std::endl;
    for (size_t a = 0; a < numBlocks*nx*ny; a++)
    {
    	myfile  << "8" << std::endl;
    }
    auto data = GetArray();
    myfile << "CELL_DATA " << numBlocks*nx*ny << std::endl;
    for (int nvar = 0; nvar < this->numVars; nvar++)
    {
        print(" --> Write", VaridStr((varid::varid)nvar));
        myfile  << "SCALARS " << VaridStr((varid::varid)nvar) << " float" << std::endl;
        myfile  << "LOOKUP_TABLE default" << std::endl;
        for (size_t lb = 0; lb < numBlocks; lb++)
        {
            for (int j = 0; j < ny; j++)
            {
                for (int i = 0; i < nx; i++)
                {
                    myfile << data(nvar, i, j, lb) << std::endl;
                }
            }
        }
    }
    myfile.close();
}

void AveragedFlowField::ReadFile(MdArray<double, 4>& array, std::string file, std::vector<int>& srcIdx, std::vector<int>& destIdx)
{
    print("Reading", file);
    if (srcIdx.size() != destIdx.size()) CallError("mismatch in index dimension");
    std::string line;
    int numVarsInFile = 0;
    {
        std::ifstream infile(file.c_str());
        std::getline(infile, line);
        std::istringstream iss(line);
        double dummy;
        while (iss>>dummy) numVarsInFile++;
        infile.close();
    }
    int maxIndex = -1;
    auto max = [](int a, int b) -> int { return a>b? a:b; };
    for (auto i: srcIdx) maxIndex = max(maxIndex, i);
    if (maxIndex>=numVarsInFile) CallError("Source index exceeds bounds");

    maxIndex = -1;
    for (auto i: destIdx) maxIndex = max(maxIndex, i);
    if (maxIndex>=this->numVars) CallError("Destination index exceeds bounds");
    std::vector<double> vals;
    vals.resize(numVarsInFile, 0.0);
    std::ifstream infile(file.c_str());
    for (int lb = 0; lb < this->numBlocks; lb++)
    {
        for (int j = 0; j < this->numPoints[1]; j++)
        {
            for (int i = 0; i < this->numPoints[0]; i++)
            {
                std::getline(infile, line);
                std::istringstream iss(line);
                for (int n = 0; n < numVarsInFile; n++) iss >> vals[n];
                for (int q = 0; q < srcIdx.size(); q++) array(destIdx[q], i, j, lb) = vals[srcIdx[q]];
            }
        }
    }
}

MdArray<double, 4> AveragedFlowField::GetArray(void)
{
    MdArray<double, 4> output(this->numVars, this->numPoints[0], this->numPoints[1], this->numBlocks);
    output.data = &rawData[0];
    return output;
}

void AveragedFlowField::InferPropertiesFromFlowAverageFile(std::string filename, v3<int>& nxb, int& nblocks, int& nvars)
{
    double dx[2] = {0.0, 0.0};
    //x, y, p, u, v, w, t
    print("Inferring properties from", filename);
    std::string line;
    
    {
        std::ifstream infile(filename.c_str());
        std::getline(infile, line);
        std::istringstream iss(line);
        nvars = 0;
        double dummy;
        while (iss>>dummy) nvars++;
        infile.close();
        nvars -= 2; // gives the number of flowfield variables
        nvars *= 2; // account for the number of square quantities
        nvars += 3; // account for the cross terms uw, vw, uv
        nvars += 2; // add back x, y
    }
    
    std::vector<double> vals;
    vals.resize(nvars, 0.0);
    {
        std::ifstream infile(filename.c_str());
        std::getline(infile, line);
        {
            std::istringstream iss(line);
            for (int i = 0; i < nvars; i++) iss >> vals[i];
        }
        double x0 = vals[0];
        
        std::getline(infile, line);
        {
            std::istringstream iss(line);
            for (int i = 0; i < nvars; i++) iss >> vals[i];
        }
        double x1 = vals[0];
        dx[0] = x1-x0;
        infile.close();
    }
    bool foundNxb = false;
    bool foundNyb = false;
    {
        std::ifstream infile(filename.c_str());
        std::getline(infile, line);
        {
            std::istringstream iss(line);
            for (int i = 0; i < nvars; i++) iss >> vals[i];
        }
        double xtarget = vals[0];
        double ytarget = vals[1];
        double y0 = vals[1];
        double tol = 1e-6;
        int nx = 0;
        int nxy = 0;
        auto fabs = [](double d) -> double {return d<0?-d:d;};
        {
            while (std::getline(infile, line) && !(foundNxb&&foundNyb))
            {
                std::istringstream iss(line);
                for (int i = 0; i < nvars; i++) iss >> vals[i];
                if (!foundNxb) nx++;
                nxy++;
                if (fabs((vals[0]-xtarget)/dx[0])<tol)
                {
                    foundNxb = true;
                }
                if (foundNxb && fabs((vals[1]-ytarget)/dx[0])<tol)
                {
                    foundNyb = true;
                }
            }
        }
        
        for (int p = 0; p < nx; p++) std::getline(infile, line);
        {
            std::istringstream iss(line);
            for (int i = 0; i < nvars; i++) iss >> vals[i];
        }
        
        double y1 = vals[1];
        dx[1] = y1 - y0;
        infile.close();
        nxb[0] = nx;
        nxb[1] = nxy/nx;
        int numLines = 0;
        std::ifstream infile2(filename.c_str());
        while(std::getline(infile2, line))
        {
            numLines++;
        }
        infile2.close();
        nblocks = numLines/(nxb[0]*nxb[1]);
    }
    print(" -- Found:");
    print(" ---- nx:         ", nxb[0]);
    print(" ---- ny:         ", nxb[1]);
    print(" ---- num. blocks:", nblocks);
    print(" ---- num. vars:  ", nvars);
}