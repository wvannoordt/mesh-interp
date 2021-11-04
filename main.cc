#include <iostream>
#include "AveragedFlowField.h"
#include "P3dMesh.h"
int main(int argc, char** argv)
{
	std::string avg   = "data/flow_mean_planar_avg_xy_nt00241000.dat";
	std::string avg2  = "data/flow_square_mean_planar_avg_xy_nt00241000.dat";
	std::string cross = "data/cross_terms_mean_planar_avg_xy_nt00241000.dat";
	AveragedFlowField mean(avg, avg2, cross);
	
	std::string p3d = "interpmesh/ramp_geom_wsm_3.p3d";
	P3dMesh grid(p3d);
	
	mean.OutputCsv("output/avg.csv");
	grid.OutputCellCentersCsv("output/p3d.csv");
	
	auto p  = grid.GetZone(0)->AddScalar( "v");
	auto u  = grid.GetZone(0)->AddScalar( "u");
	auto v  = grid.GetZone(0)->AddScalar( "v");
	auto w  = grid.GetZone(0)->AddScalar( "w");
	auto T  = grid.GetZone(0)->AddScalar( "w");
	auto p2 = grid.GetZone(0)->AddScalar("v2");
	auto u2 = grid.GetZone(0)->AddScalar("u2");
	auto v2 = grid.GetZone(0)->AddScalar("v2");
	auto w2 = grid.GetZone(0)->AddScalar("w2");
	auto T2 = grid.GetZone(0)->AddScalar("w2");
	auto uv = grid.GetZone(0)->AddScalar("uv");
	auto vw = grid.GetZone(0)->AddScalar("vw");
	auto uw = grid.GetZone(0)->AddScalar("uw");
	
	int vv = 3;
	p->ReadCsv( "output/var_"+std::to_string(vv++)+".csv");
	u->ReadCsv( "output/var_"+std::to_string(vv++)+".csv");
	v->ReadCsv( "output/var_"+std::to_string(vv++)+".csv");
	w->ReadCsv( "output/var_"+std::to_string(vv++)+".csv");
	T->ReadCsv( "output/var_"+std::to_string(vv++)+".csv");
	p2->ReadCsv("output/var_"+std::to_string(vv++)+".csv");
	u2->ReadCsv("output/var_"+std::to_string(vv++)+".csv");
	v2->ReadCsv("output/var_"+std::to_string(vv++)+".csv");
	w2->ReadCsv("output/var_"+std::to_string(vv++)+".csv");
	T2->ReadCsv("output/var_"+std::to_string(vv++)+".csv");
	uv->ReadCsv("output/var_"+std::to_string(vv++)+".csv");
	vw->ReadCsv("output/var_"+std::to_string(vv++)+".csv");
	uw->ReadCsv("output/var_"+std::to_string(vv++)+".csv");
	
	grid.OutputVtk("output/interp.vtk");
	return 0;
}
