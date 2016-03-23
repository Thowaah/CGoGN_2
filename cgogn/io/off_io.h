/*******************************************************************************
* CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: http://cgogn.unistra.fr/                                           *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#ifndef IO_OFF_IO_H_
#define IO_OFF_IO_H_

#include <io/surface_import.h>

namespace cgogn
{

namespace io
{

template<typename MAP_TRAITS, typename VEC3>
class OffSurfaceImport : public SurfaceImport<MAP_TRAITS> {
public:
	using Self = OffSurfaceImport<MAP_TRAITS, VEC3>;
	using Inherit = SurfaceImport<MAP_TRAITS>;
	using Scalar = typename geometry::vector_traits<VEC3>::Scalar;
	template<typename T>
	using ChunkArray = typename Inherit::template ChunkArray<T>;

	virtual ~OffSurfaceImport() override {}
protected:
	virtual bool import_file_impl(const std::string& filename) override
	{
		std::ifstream fp(filename.c_str(), std::ios::in);

		std::string line;
		line.reserve(512);

		// read OFF header
		std::getline(fp, line);
		if (line.rfind("OFF") == std::string::npos)
		{
			std::cerr << "Problem reading off file: not an off file" << std::endl;
			std::cerr << line << std::endl;
			return false;
		}

		// check if binary file
		if (line.rfind("BINARY") != std::string::npos)
		{
			return this->import_off_bin(fp);
		}


		// read number of vertices, edges, faces
		this->nb_vertices_ = this->read_uint(fp,line);
		this->nb_faces_ = this->read_uint(fp,line);
		this->nb_edges_ = this->read_uint(fp,line);

		ChunkArray<VEC3>* position = this->vertex_attributes_.template add_attribute<VEC3>("position");

		// read vertices position
		std::vector<uint32> vertices_id;
		vertices_id.reserve(this->nb_vertices_);

		for (uint32 i = 0; i < this->nb_vertices_; ++i)
		{

			float64 x = this->read_double(fp,line);
			float64 y = this->read_double(fp,line);
			float64 z = this->read_double(fp,line);

			VEC3 pos{Scalar(x), Scalar(y), Scalar(z)};

			uint32 vertex_id = this->vertex_attributes_.template insert_lines<1>();
			(*position)[vertex_id] = pos;

			vertices_id.push_back(vertex_id);
		}

		// read faces (vertex indices)
		this->faces_nb_edges_.reserve(this->nb_faces_);
		this->faces_vertex_indices_.reserve(this->nb_vertices_ * 8);
		for (uint32 i = 0; i < this->nb_faces_; ++i)
		{
			uint32 n = this->read_uint(fp,line);
			this->faces_nb_edges_.push_back(n);
			for (uint32 j = 0; j < n; ++j)
			{
				uint32 index = this->read_uint(fp,line);
				this->faces_vertex_indices_.push_back(vertices_id[index]);
			}

		}

		return true;
	}

	inline bool import_off_bin(std::istream& fp)
	{
		char buffer1[12];
		fp.read(buffer1,12);

		this->nb_vertices_= swap_endianness_native_big(*(reinterpret_cast<uint32*>(buffer1)));
		this->nb_faces_= swap_endianness_native_big(*(reinterpret_cast<uint32*>(buffer1+4)));
		this->nb_edges_= swap_endianness_native_big(*(reinterpret_cast<uint32*>(buffer1+8)));


		ChunkArray<VEC3>* position = this->vertex_attributes_.template add_attribute<VEC3>("position");


		static const uint32 BUFFER_SZ = 1024*1024;
		float32* buff_pos = new float32[3*BUFFER_SZ];
		std::vector<uint32> vertices_id;
		vertices_id.reserve(this->nb_vertices_);

		{ // limit j scope
			unsigned j = BUFFER_SZ;
			for (uint32 i = 0; i < this->nb_vertices_; ++i, ++j)
			{
				if (j == BUFFER_SZ)
				{
					j = 0;
					// read from file into buffer
					if (i + BUFFER_SZ < this->nb_vertices_)
						fp.read(reinterpret_cast<char*>(buff_pos), 3 * sizeof(float32)*BUFFER_SZ);
					else
						fp.read(reinterpret_cast<char*>(buff_pos), 3 * sizeof(float32)*(this->nb_vertices_ - i));

					//endian
					uint32* ptr = reinterpret_cast<uint32*>(buff_pos);
					for (uint32 k = 0; k < 3 * BUFFER_SZ; ++k)
					{
						*ptr = swap_endianness_native_big(*ptr);
						++ptr;
					}
				}

				VEC3 pos{ buff_pos[3 * j], buff_pos[3 * j + 1], buff_pos[3 * j + 2] };

				uint32 vertex_id = this->vertex_attributes_.template insert_lines<1>();
				(*position)[vertex_id] = pos;

				vertices_id.push_back(vertex_id);
			}
		}

		delete[] buff_pos;

		// read faces (vertex indices)

		uint32* buff_ind = new uint32[BUFFER_SZ];
		this->faces_nb_edges_.reserve(this->nb_faces_);
		this->faces_vertex_indices_.reserve(this->nb_vertices_ * 8);

		uint32* ptr = buff_ind;
		uint32 nb_read = BUFFER_SZ;
		for (uint32 i = 0; i < this->nb_faces_; ++i)
		{
			if (nb_read == BUFFER_SZ)
			{
				fp.read(reinterpret_cast<char*>(buff_ind),BUFFER_SZ*sizeof(uint32));
				ptr = buff_ind;
				for (uint32 k=0; k< BUFFER_SZ;++k)
				{
					*ptr = swap_endianness_native_big(*ptr);
					++ptr;
				}
				ptr = buff_ind;
				nb_read =0;
			}

			uint32 n = *ptr++;
			nb_read++;

			this->faces_nb_edges_.push_back(n);
			for (uint32 j = 0; j < n; ++j)
			{
				if (nb_read == BUFFER_SZ)
				{
					fp.read(reinterpret_cast<char*>(buff_ind),BUFFER_SZ*sizeof(uint32));
					ptr = buff_ind;
					for (uint32 k=0; k< BUFFER_SZ;++k)
					{
						*ptr = swap_endianness_native_big(*ptr);
						++ptr;
					}
					ptr = buff_ind;
					nb_read=0;
				}
				uint32 index = *ptr++;
				nb_read++;
				this->faces_vertex_indices_.push_back(vertices_id[index]);
			}
		}

		delete[] buff_ind;

		return true;
	}
private:
	static inline float64 read_double(std::istream& fp, std::string& line)
	{
		fp >> line;
		while (line[0]=='#')
		{
			fp.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			fp >> line;
		}
		return std::stod(line);
	}

	static inline uint32 read_uint(std::istream& fp, std::string& line)
	{
		fp >> line;
		while (line[0]=='#')
		{
			fp.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			fp >> line;
		}
		return uint32((std::stoul(line)));
	}
};

} // namespace io
} // namespace cgogn

#endif // IO_OFF_IO_H_
