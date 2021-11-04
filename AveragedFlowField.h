#pragma once
#include <string>
#include <vector>
#include "print.h"
#include "v3.h"
#include "MdArray.h"

namespace varid
{
    enum varid
    {
        x=0,
        y=1,
        p=2,
        u=3,
        v=4,
        w=5,
        T=6,
        p2=7,
        u2=8,
        v2=9,
        w2=10,
        T2=11,
        uv=12,//get sparsh to confirm
        uw=13,
        vw=14
    };
}

static std::string VaridStr(varid::varid id)
{
    switch (id)
    {
        case varid::x: return "x";
        case varid::y: return "y";
        case varid::p: return "p";
        case varid::u: return "u";
        case varid::v: return "v";
        case varid::w: return "w";
        case varid::T: return "T";
        case varid::p2: return "p2";
        case varid::u2: return "u2";
        case varid::v2: return "v2";
        case varid::w2: return "w2";
        case varid::T2: return "T2";
        case varid::uv: return "uv";
        case varid::uw: return "uw";
        case varid::vw: return "vw";
    }
    return "[none]";
}
class AveragedFlowField
{
    public:
        AveragedFlowField(std::string avgFile, std::string avgSquareFile, std::string crossTermsFile);
        static void InferPropertiesFromFlowAverageFile(std::string filename, v3<int>& nxb, int& nblocks, int& nvars);
        MdArray<double, 4> GetArray(void);
        MdArray<double, 2> GetBlockBoundingBoxes(void);
        MdArray<double, 2> GetBlockSpacing(void);
        void OutputVtk(std::string filename);
    private:
        void Build(std::string avgFile, std::string avgSquareFile, std::string crossTermsFile);
        void ReadFile(MdArray<double, 4>& array, std::string file, std::vector<int>& srcIdx, std::vector<int>& destIdx);
        void CreateBoundingBoxes(void);
        v3<int> numPoints;
        int numBlocks;
        int numVars;
        std::vector<double> rawData;
        std::vector<double> boundBoxRawData;
        std::vector<double> dxRawData;
};