//=====================================================================
//
// Model.h - 该文件改写自 tinyrender 的 model.h
//
// Created by skywind on 2020/08/11
// Last Modified: 2020/08/11 19:22:13
//
//=====================================================================
#ifndef _MODEL_H_
#define _MODEL_H_

#include <fstream>
#include <sstream>
#include <iostream>

#include "RenderHelp.h"


//---------------------------------------------------------------------
// model
//---------------------------------------------------------------------
class Model {
public:
	inline virtual ~Model() {
		if (_diffusemap) delete _diffusemap;
		if (_normalmap) delete _normalmap;
		if (_specularmap) delete _specularmap;
	}

	inline Model(const char *filename) {
		_diffusemap = NULL;
		_normalmap = NULL;
		_specularmap = NULL;
		std::ifstream in;
		in.open(filename, std::ifstream::in);
		if (in.fail()) return;
		std::string line;
		while (!in.eof()) {
			std::getline(in, line);
			std::istringstream iss(line.c_str());
			char trash;
			if (line.compare(0, 2, "v ") == 0) {
				iss >> trash;
				Vec3f v;
				for (int i = 0; i < 3; i++) iss >> v[i];
				_verts.push_back(v);
			}
			else if (line.compare(0, 3, "vn ") == 0) {
				iss >> trash >> trash;
				Vec3f n;
				for (int i = 0; i < 3; i++) iss >> n[i];
				_norms.push_back(n);
			}
			else if (line.compare(0, 3, "vt ") == 0) {
				iss >> trash >> trash;
				Vec2f uv;
				iss >> uv[0] >> uv[1];
				_uv.push_back(uv);
			}
			else if (line.compare(0, 2, "f ") == 0) {
				std::vector<Vec3i> f;
				Vec3i tmp;
				iss >> trash;
				while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
					for (int i = 0; i < 3; i++) tmp[i]--;
					f.push_back(tmp);
				}
				_faces.push_back(f);
			}
		}
		std::cout << "# v# " << _verts.size() << " f# " << _faces.size() << "\n";
		_diffusemap = load_texture(filename, "_diffuse.bmp");
		_normalmap = load_texture(filename, "_nm.bmp");
		_specularmap = load_texture(filename, "_spec.bmp");
	}

public:

	inline int nverts() const { return (int)_verts.size(); }
	inline int nfaces() const { return (int)_faces.size(); }

	inline std::vector<int> face(int idx) const {
		std::vector<int> face;
		for (int i = 0; i < (int)_faces[idx].size(); i++) 
			face.push_back(_faces[idx][i][0]);
		return face;
	}

	inline Vec3f vert(int i) const { return _verts[i]; }
	inline Vec3f vert(int iface, int nthvert) { return _verts[_faces[iface][nthvert][0]]; }

	inline Vec2f uv(int iface, int nthvert) const {
		return _uv[_faces[iface][nthvert][1]];
	}

	inline Vec3f normal(int iface, int nthvert) const {
		int idx = _faces[iface][nthvert][2];
		return vector_normalize(_norms[idx]);
	}

	inline Vec4f diffuse(Vec2f uv) const {
		assert(_diffusemap);
		return _diffusemap->Sample2D(uv);
	}

	inline Vec3f normal(Vec2f uv) const {
		assert(_normalmap);
		Vec4f color = _normalmap->Sample2D(uv);
		for (int i = 0; i < 3; i++) color[i] = color[i] * 2.0f - 1.0f;
		return {color[0], color[1], color[2]};
	}

	inline float Specular(Vec2f uv) {
		Vec4f color = _specularmap->Sample2D(uv);
		return color.b;
	}

protected:
	Bitmap *load_texture(std::string filename, const char *suffix) {
		std::string texfile(filename);
		size_t dot = texfile.find_last_of(".");
		if (dot == std::string::npos) return NULL;
		texfile = texfile.substr(0, dot) + std::string(suffix);
		Bitmap *texture = Bitmap::LoadFile(texfile.c_str());
		std::cout << "loading: " << texfile << ((texture)? " OK" : " failed") << "\n";
		texture->FlipVertical();
		return texture;
	}

protected:
	std::vector<Vec3f> _verts;
	std::vector<std::vector<Vec3i> > _faces;
	std::vector<Vec3f> _norms;
	std::vector<Vec2f> _uv;
	Bitmap *_diffusemap;
	Bitmap *_normalmap;
	Bitmap *_specularmap;
};


#endif


