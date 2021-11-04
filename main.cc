#include <iostream>
#include "AveragedFlowField.h"

int main(int argc, char** argv)
{
	std::string avg   = "data/flow_mean_planar_avg_xy_nt00241000.dat";
	std::string avg2  = "data/flow_square_mean_planar_avg_xy_nt00241000.dat";
	std::string cross = "data/cross_terms_mean_planar_avg_xy_nt00241000.dat";
	AveragedFlowField mean(avg, avg2, cross);
	mean.OutputVtk("output.vtk");
	return 0;
}
