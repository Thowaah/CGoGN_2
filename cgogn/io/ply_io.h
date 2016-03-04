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

#ifndef IO_PLY_IO_H_
#define IO_PLY_IO_H_

#include <io/surface_import.h>
#include <io/import_ply_data.h>

namespace cgogn
{

namespace io
{

template<typename MAP_TRAITS, typename VEC3>
class PlySurfaceImport : public SurfaceImport<MAP_TRAITS> {
public:
	using Self = PlySurfaceImport<MAP_TRAITS, VEC3>;
	using Inherit = SurfaceImport<MAP_TRAITS>;
	using Scalar = typename geometry::vector_traits<VEC3>::Scalar;
	template<typename T>
	using ChunkArray = typename Inherit::template ChunkArray<T>;

	virtual ~PlySurfaceImport() override {}

protected:
	virtual bool import_file_impl(const std::string& filename) override
	{

		PlyImportData pid;

		if (! pid.read_file(filename) )
		{
			std::cerr << "Unable to open file " << filename << std::endl;
			return false;
		}

		ChunkArray<VEC3>* position = this->vertex_attributes_.template add_attribute<VEC3>("position");
		ChunkArray<VEC3>* color = nullptr;
		if (pid.has_colors())
		{
			color = this->vertex_attributes_.template add_attribute<VEC3>("color");
		}

		this->nb_vertices_ = pid.nb_vertices();
		this->nb_faces_ = pid.nb_faces();


		// read vertices position
		std::vector<unsigned int> vertices_id;
		vertices_id.reserve(this->nb_vertices_);

		for (unsigned int i = 0; i < this->nb_vertices_; ++i)
		{
			VEC3 pos;
			pid.vertex_position(i, pos);

			unsigned int vertex_id = this->vertex_attributes_.template insert_lines<1>();
			(*position)[vertex_id] = pos;

			vertices_id.push_back(vertex_id);

			if (pid.has_colors())
			{
				VEC3 rgb;
				pid.vertex_color_float32(i, rgb);

				(*color)[vertex_id] = pos;
			}
		}

		// read faces (vertex indices)
		this->faces_nb_edges_.reserve(this->nb_faces_);
		this->faces_vertex_indices_.reserve(this->nb_vertices_ * 8);
		for (unsigned int i = 0; i < this->nb_faces_; ++i)
		{
			unsigned int n = pid.get_face_valence(i);
			this->faces_nb_edges_.push_back(n);
			int* indices = pid.get_face_indices(i);
			for (unsigned int j = 0; j < n; ++j)
			{
				unsigned int index = (unsigned int)(indices[j]);
				this->faces_vertex_indices_.push_back(vertices_id[index]);
			}
		}

		return true;
	}
};


}// namespace io
} // namespace cgogn
#endif // IO_PLY_IO_H_
