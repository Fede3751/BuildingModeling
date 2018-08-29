/*

BUILDING MODELLING.
Computer Graphic Project 2017-2018
Student: Federico Trombetti
ID: 1665803

See .h for documentation


*/


#include "../yocto/yocto_gl.h"
using namespace std::literals;
#include "build.h"


//instantiate Direction only once and use it as a constant value
auto Dir = Direction{};


ModResult createResult(std::vector<int> faces, Building* b) {

	ModResult res;


	for (int i : faces) {

		if (b->faces[i].orientation == Dir.UP)
			res.up = i;
		else if (b->faces[i].orientation == Dir.DOWN)
			res.down = i;
		else if (b->faces[i].orientation == Dir.NORTH)
			res.north = i;
		else if (b->faces[i].orientation == Dir.SOUTH)
			res.south = i;
		else if (b->faces[i].orientation == Dir.EAST)
			res.east = i;
		else if (b->faces[i].orientation == Dir.WEST)
			res.west = i;
	}

	return res;

}


ModResult createBuilding(Building* b, float w, float h, float d) {



	//allocate space for first faces (6 in total) and points (8 in total)
	b->points = std::vector<ygl::vec3f>(8);
	b->faces = std::vector<Face>(6);

	auto points = b->points;
	auto faces = b->faces;


	//create first points
	b->points[0] = { 0, 0, 0 };
	b->points[1] = { w, 0, 0 };
	b->points[2] = { w, d, 0 };
	b->points[3] = { 0, d, 0 };

	b->points[4] = { 0, 0, h };
	b->points[5] = { w, 0, h };
	b->points[6] = { w, d, h };
	b->points[7] = { 0, d, h };

	b->faces[0] = { { 1,0,3,2 }, Dir.DOWN };
	b->faces[1] = { { 4,5,6,7 }, Dir.UP };
	b->faces[2] = { { 0,1,5,4 }, Dir.SOUTH };
	b->faces[3] = { { 3,0,4,7 }, Dir.WEST };
	b->faces[4] = { { 1,2,6,5 }, Dir.EAST };
	b->faces[5] = { { 2,3,7,6 }, Dir.NORTH };


	return createResult({ 0,1,2,3,4,5 }, b);

};


std::vector<int> subdivideFace(Building* b, int n) {


	Face f = b->faces[n];
	auto v = f.v;
	int s = b->points.size();
	int i = b->faces.size();



	//generate new points
	ygl::vec3f c = (b->points[v.x] + b->points[v.y] + b->points[v.z] + b->points[v.w]) / 4;		//center
	ygl::vec3f xy = (b->points[v.x] + b->points[v.y]) / 2;										// x-y
	ygl::vec3f yz = (b->points[v.y] + b->points[v.z]) / 2;										// y-z
	ygl::vec3f zw = (b->points[v.z] + b->points[v.w]) / 2;										// z-w
	ygl::vec3f wx = (b->points[v.w] + b->points[v.x]) / 2;										// w-x


	b->points.push_back(c);			//s
	b->points.push_back(xy);		//s+1
	b->points.push_back(yz);		//s+2
	b->points.push_back(zw);		//s+3
	b->points.push_back(wx);		//s+4

	b->faces[n].v = { s + 4,s,s + 3,v.w };
	b->faces.push_back({ { s,s + 2,v.z,s + 3 }, f.orientation });
	b->faces.push_back({ { v.x,s + 1,s,s + 4 }, f.orientation });
	b->faces.push_back({ { s + 1, v.y, s + 2, s }, f.orientation });


	return std::vector<int>{n, i, i + 1, i + 2};

};


std::vector<int> subdivideFace(Building* b, int f, int w, int h) {


	auto v = b->faces[f].v;


	auto wAmnt = (b->points[v.y] - b->points[v.x]) / w;
	auto hAmnt = (b->points[v.x] - b->points[v.w]) / h;


	//actual size of the grid (input is in term of faces, so points between faces are more)
	int pH = h + 1;
	int pW = w + 1;

	//allocate space for grid of points (so that they are easily found when creating faces)
	std::vector<int> grid = std::vector<int>(pH*pW);

	//save the existing edges in the right position in the grid
	grid[0] = v.w;
	grid[pW - 1] = v.z;
	grid[pW*pH - 1] = v.y;
	grid[pW*(pH - 1)] = v.x;


	int s = b->points.size();
	for (int i = 0; i < pW; i++) {
		for (int j = 0; j < pH; j++) {

			//do not replicate existing points
			if ((i == 0 || i == pW - 1) && (j == 0 || j == pH - 1))
				continue;

			grid[j*pW + i] = s++;
			b->points.push_back(b->points[v.w] + wAmnt*i + hAmnt * j);
		}
	}


	b->faces[f].v = { grid[pW], grid[pW + 1], grid[1], grid[0] };

	std::vector<int> res = std::vector<int>(w*h);
	res[0] = f;

	int fs = b->faces.size();
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {

			//skip first tile (as it is generated from the previous face being resized)
			if (j == 0 && i == 0)
				continue;

			res[j*w + i] = fs++;
			b->faces.push_back({ { grid[(j + 1)*pW + i], grid[(j + 1)*pW + i + 1], grid[j*pW + i + 1], grid[j*pW + i] }, b->faces[f].orientation });

		}
	}

	return res;


};


std::vector<std::vector<int>> subdivideFace(Building* b, std::vector<int> f, int w, int h, int from, int to) {

	if (from == 0 && to == 0)
		to = f.size();

	std::vector<std::vector<int>> res = std::vector<std::vector<int>>(to - from);

	for (int i = 0; i < to - from; i++)
		res[i] = subdivideFace(b, f[i + from], w, h);


	return res;


}


std::vector<int> horizontalSlice(Building* b, int f, float h) {

	auto v = b->faces[f].v;

	auto p1 = b->points[v.w];
	auto p2 = b->points[v.z];


	auto hAmnt = (b->points[v.x] - b->points[v.w]) * (1 - h);


	p1 = p1 + hAmnt;
	p2 = p2 + hAmnt;



	int p1Ind = b->points.size();
	int p2Ind = b->points.size() + 1;


	b->points.push_back(p1);
	b->points.push_back(p2);


	b->faces.push_back({ {p1Ind, p2Ind, v.z, v.w}, b->faces[f].orientation });
	b->faces[f].v = { v.x, v.y, p2Ind, p1Ind };

	return { f, (int)b->faces.size() - 1 };


}

//void Building::subdivideFace(int n, int f) {};



ModResult pullFace(Building* b, int f, float value) {


	int s = b->points.size();
	int i = b->faces.size();

	auto verse = value / std::abs(value);


	std::vector<int> temp = std::vector<int>{ f, i, i + 1, i + 2, i + 3 };

	//duplicate points
	for (auto v : b->faces[f].v)
		b->points.push_back(b->points[v]);

	//link the face which has to be pushed to the new points and save the old points to create the faces
	//(also save the new ones on a shorter variable for easier use)
	ygl::vec4i oldP = b->faces[f].v;
	b->faces[f].v = { s, s + 1, s + 2, s + 3 };
	ygl::vec4i newP = b->faces[f].v;


	//create new faces
	ygl::vec3f middlePoint =
		(b->points[oldP.x] + b->points[oldP.y] + b->points[oldP.z] + b->points[oldP.w] +
			b->points[newP.x] + b->points[newP.y] + b->points[newP.z] + b->points[newP.w]) / 4
		;


	if (b->faces[f].orientation == Dir.SOUTH) {
		b->faces.push_back({ { oldP.x, newP.x, newP.w, oldP.w }, Dir.WEST * verse });
		b->faces.push_back({ { newP.y, newP.x, oldP.x, oldP.y }, Dir.DOWN * verse });
		b->faces.push_back({ { newP.y, oldP.y, oldP.z, newP.z }, Dir.EAST * verse });
		b->faces.push_back({ { newP.w, newP.z, oldP.z, oldP.w }, Dir.UP   * verse });
	}
	else if (b->faces[f].orientation == Dir.NORTH) {
		b->faces.push_back({ { newP.y, oldP.y, oldP.z, newP.z }, Dir.WEST * verse });
		b->faces.push_back({ { oldP.x, oldP.y, newP.y, newP.x }, Dir.DOWN * verse });
		b->faces.push_back({ { oldP.x, newP.x, newP.w, oldP.w }, Dir.EAST * verse });
		b->faces.push_back({ { oldP.z, oldP.w, newP.w, newP.z }, Dir.UP   * verse });
	}
	else if (b->faces[f].orientation == Dir.EAST) {


		b->faces.push_back({ { newP.y, oldP.y, oldP.z, newP.z }, Dir.NORTH * verse });
		b->faces.push_back({ { newP.x, oldP.x, oldP.y, newP.y }, Dir.DOWN  * verse });
		b->faces.push_back({ { oldP.x, newP.x, newP.w, oldP.w }, Dir.SOUTH * verse });
		b->faces.push_back({ { oldP.w, newP.w, newP.z, oldP.z }, Dir.UP    * verse });


		
	}
	else if (b->faces[f].orientation == Dir.WEST) {
		b->faces.push_back({ { oldP.x, newP.x, newP.w, oldP.w }, Dir.NORTH * verse });
		b->faces.push_back({ { oldP.y, newP.y, newP.x, oldP.x }, Dir.DOWN  * verse });
		b->faces.push_back({ { newP.y, oldP.y, oldP.z, newP.z }, Dir.SOUTH * verse });
		b->faces.push_back({ { newP.z, oldP.z, oldP.w, newP.w }, Dir.UP    * verse });
	}
	else if (b->faces[f].orientation == Dir.UP) {
		b->faces.push_back({ { oldP.w, oldP.x, newP.x, newP.w }, Dir.WEST  * verse });
		b->faces.push_back({ { oldP.x, oldP.y, newP.y, newP.x }, Dir.SOUTH * verse });
		b->faces.push_back({ { oldP.y, oldP.z, newP.z, newP.y }, Dir.EAST  * verse });
		b->faces.push_back({ { oldP.z, oldP.w, newP.w, newP.z }, Dir.NORTH * verse });
	}
	else if (b->faces[f].orientation == Dir.DOWN) {
		b->faces.push_back({ { newP.z, newP.y, oldP.y, oldP.z }, Dir.WEST  * verse });
		b->faces.push_back({ { newP.y, newP.x, oldP.x, oldP.y }, Dir.SOUTH * verse });
		b->faces.push_back({ { newP.x, newP.w, oldP.w, oldP.x }, Dir.EAST  * verse });
		b->faces.push_back({ { newP.w, newP.z, oldP.z, oldP.w }, Dir.NORTH * verse });
	}


	//pull face
	for (auto v : b->faces[f].v) {
		b->points[v] += b->faces[f].orientation * value;
	}


	//link new faces
	return createResult(temp, b);

};

std::vector<ModResult> pullFace(Building* b, std::vector<int> f, float value, int from, int to) {

	

	if (from == 0 && to == 0)
		to = f.size();

	std::vector<ModResult> res = std::vector<ModResult>(to - from);

	for (int i = 0; i < to - from; i++)
		res[i] = pullFace(b, f[i + from], value);


	return res;


}


std::vector<int> subFace(Building* b, int f, float xSub, float ySub, float offsetX, float offsetY) {


	auto v = b->faces[f].v;

	auto wOuter = (b->points[v.y] - b->points[v.x]);
	auto hOuter = (b->points[v.x] - b->points[v.w]);

	auto wInner = wOuter * xSub;
	auto hInner = hOuter * ySub;

	auto wOffset = wOuter * offsetX;
	auto hOffset = hOuter * offsetY;


	auto grid = subdivideFace(b, f, 3, 3);

	b->points[b->faces[grid[0]].v.z] = b->points[v.w] + (wOuter - wInner) / 2;
	b->points[b->faces[grid[0]].v.x] = b->points[v.w] + (hOuter - hInner) / 2;
	b->points[b->faces[grid[0]].v.y] = b->points[v.w] + (hOuter - hInner) / 2 + (wOuter - wInner) / 2;

	b->points[b->faces[grid[2]].v.w] = b->points[v.z] - (wOuter - wInner) / 2;
	b->points[b->faces[grid[2]].v.y] = b->points[v.z] + (hOuter - hInner) / 2;
	b->points[b->faces[grid[2]].v.x] = b->points[v.z] + (hOuter - hInner) / 2  - (wOuter - wInner) / 2;

	b->points[b->faces[grid[6]].v.y] = b->points[v.x] + (wOuter - wInner) / 2;
	b->points[b->faces[grid[6]].v.w] = b->points[v.x] - (hOuter - hInner) / 2;
	b->points[b->faces[grid[6]].v.z] = b->points[v.x] - (hOuter - hInner) / 2 + (wOuter - wInner) / 2;


	b->points[b->faces[grid[8]].v.x] = b->points[v.y] - (wOuter - wInner) / 2;
	b->points[b->faces[grid[8]].v.z] = b->points[v.y] - (hOuter - hInner) / 2;
	b->points[b->faces[grid[8]].v.w] = b->points[v.y] - (hOuter - hInner) / 2 - (wOuter - wInner) / 2;



	return grid;





	
}

std::vector<int> subFace(Building* b, std::vector<int> f, float xSub, float ySub, int from, int to, float offsetX, float offsetY) {
	
	if (from == 0 && to == 0)
		to = f.size();

	std::vector<int> res = std::vector<int>(to - from);

	for (int i = 0; i < to - from; i++)
		res[i] = subFace(b, f[i + from], xSub, ySub, offsetX, offsetY)[4];


	return res;
}


void makeRoofs(Building* b, ygl::vec3f axis, int amnt) {

	int i = 0;
	for (Face f : b->faces) {

		if (f.orientation == Dir.UP) {
			pointFace(b, i, axis, amnt);
		}
		i++;
	}

}


void pointFace(Building* b, int f, ygl::vec3f axis, int amnt) {


	auto v = b->faces[f].v;
	int s = b->points.size();

	ygl::vec3f p1, p2;

	if (axis == Dir.EAST || axis == Dir.WEST) {
		p1 = (b->points[v.x] + b->points[v.w]) / 2;  //s
		p2 = (b->points[v.y] + b->points[v.z]) / 2;  //s+1

		b->faces[f].v = { v.x, v.y, s + 1, s };
		b->faces.push_back({ { s, s + 1, v.z, v.w }, Dir.UP });
		
		b->faces.push_back({ { v.w, v.x, s, s }, Dir.EAST });
		b->faces.push_back({ { v.y, v.z, s + 1, s + 1 }, Dir.WEST });

	}
	else if (axis == Dir.NORTH || axis == Dir.SOUTH) {
		p1 = (b->points[v.x] + b->points[v.y]) / 2;
		p2 = (b->points[v.z] + b->points[v.w]) / 2;

		b->faces[f].v = { v.x, s, s + 1, v.w };
		b->faces.push_back({ { s + 1, s, v.y, v.z }, Dir.UP });


		b->faces.push_back({ { v.x, v.y, s, s }, Dir.SOUTH });
		b->faces.push_back({ { v.w, v.z, s + 1, s + 1 }, Dir.NORTH });

	}

	b->points.push_back(p1);
	b->points.push_back(p2);


	b->points[s] += Dir.UP * amnt;
	b->points[s + 1] += Dir.UP * amnt;


};


ygl::shape* buildingToShape(Building* b, ygl::material* mat) {



	ygl::shape* res = new ygl::shape{};

	res->mat = mat;
	res->pos = b->points;
	res->quads = std::vector<ygl::vec4i>();
	res->texcoord = std::vector<ygl::vec2f>();
	res->quads_texcoord = std::vector<ygl::vec4i>();
	res->texcoord1 = std::vector<ygl::vec2f>();
	res->quads_pos = std::vector<ygl::vec4i>();

	for (auto f : b->faces) {
		res->quads.push_back(f.v);
	}
	for (int i = 0; i < b->faces.size(); i++) {
		res->texcoord.push_back(ygl::vec2f{ 100,100 });
		res->texcoord1.push_back(ygl::vec2f{ 100,100 });

		//res->quads_pos.push_back({ 4,5,6,7 });
		//res->quads_texcoord.push_back({ 4,5,6,7 })
	}

	return res;

};


ygl::instance* buildingToInstance(Building* b, ygl::material* mat) {

	ygl::instance* res = new ygl::instance{};

	res->shp = new ygl::shape_group{};
	res->shp->shapes = std::vector<ygl::shape *>(1);

	res->shp->shapes[0] = buildingToShape(b, mat);

	return res;

}


void addBuildingToScene(Building* b, ygl::scene* scn, ygl::material* mat) {

	auto inst = buildingToInstance(b, mat);

	scn->instances.push_back(inst);
	scn->shapes.push_back(inst->shp);

}