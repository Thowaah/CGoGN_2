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

#include <QApplication>
#include <QMatrix4x4>

#include <qoglviewer.h>
#include <vec.h>
#include <QMouseEvent>
#include <QVector3D>

#include <core/cmap/cmap2.h>

#include <io/map_import.h>

#include <geometry/algos/bounding_box.h>

#include <rendering/map_render.h>
#include <rendering/shaders/shader_flat.h>
#include <rendering/shaders/vbo.h>
#include <rendering/drawer.h>

#include <geometry/algos/picking.h>

#define DEFAULT_MESH_PATH CGOGN_STR(CGOGN_TEST_MESHES_PATH)

using namespace cgogn::numerics;

using Map2 = cgogn::CMap2<cgogn::DefaultMapTraits>;
//using Vec3 = Eigen::Vector3d;
using Vec3 = cgogn::geometry::Vec_T<std::array<double,3>>;

template<typename T>
using VertexAttributeHandler = Map2::VertexAttributeHandler<T>;


class Viewer : public QOGLViewer
{
public:
	Viewer();
	Viewer(const Viewer&) = delete;
	Viewer& operator=(const Viewer&) = delete;

	virtual void draw();
	virtual void init();
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void keyPressEvent(QKeyEvent *);

	void import(const std::string& surfaceMesh);

	virtual ~Viewer();



private:
	void rayClick(QMouseEvent* event, qoglviewer::Vec& P, qoglviewer::Vec& Q);


	QRect viewport_;
	QMatrix4x4 proj_;
	QMatrix4x4 view_;

	Map2 map_;
	VertexAttributeHandler<Vec3> vertex_position_;

	cgogn::geometry::BoundingBox<Vec3> bb_;

	cgogn::rendering::MapRender* render_;

	cgogn::rendering::VBO* vbo_pos_;

	cgogn::rendering::ShaderFlat* shader2_;

	cgogn::rendering::Drawer* drawer_;

	int32 cell_picking;
};


//
// IMPLEMENTATION
//


void Viewer::import(const std::string& surfaceMesh)
{
	cgogn::io::import_surface<Vec3>(map_, surfaceMesh);

	vertex_position_ = map_.get_attribute<Vec3, Map2::Vertex::ORBIT>("position");

	cgogn::geometry::compute_bounding_box(vertex_position_, bb_);

	setSceneRadius(bb_.diag_size()/2.0);
	Vec3 center = bb_.center();
	setSceneCenter(qoglviewer::Vec(center[0], center[1], center[2]));
	showEntireScene();
}

Viewer::~Viewer()
{
	delete render_;
	delete vbo_pos_;
	delete shader2_;
}

Viewer::Viewer() :
	map_(),
	vertex_position_(),
	bb_(),
	render_(nullptr),
	vbo_pos_(nullptr),
	shader2_(nullptr),
	drawer_(nullptr),
	cell_picking(0)
{}

void Viewer::draw()
{
	camera()->getProjectionMatrix(proj_);
	camera()->getModelViewMatrix(view_);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);

	shader2_->bind();
	shader2_->set_matrices(proj_,view_);
	shader2_->bind_vao(0);
	render_->draw(cgogn::rendering::TRIANGLES);
	shader2_->release_vao(0);
	shader2_->release();

	glDisable(GL_POLYGON_OFFSET_FILL);

	drawer_->call_list(proj_,view_);

}

void Viewer::init()
{
	glClearColor(0.1f,0.1f,0.3f,0.0f);

	vbo_pos_ = new cgogn::rendering::VBO(3);
	cgogn::rendering::update_vbo(vertex_position_, *vbo_pos_);
	render_ = new cgogn::rendering::MapRender();

	render_->init_primitives<Vec3>(map_, cgogn::rendering::TRIANGLES, vertex_position_);

	shader2_ = new cgogn::rendering::ShaderFlat;
	shader2_->add_vao();
	shader2_->set_vao(0, vbo_pos_);

	shader2_->bind();
	shader2_->set_front_color(QColor(0,200,0));
	shader2_->set_back_color(QColor(0,0,200));
	shader2_->set_ambiant_color(QColor(5,5,5));
	shader2_->release();

	drawer_ = new cgogn::rendering::Drawer(this);
}


void Viewer::rayClick(QMouseEvent* event, qoglviewer::Vec& P, qoglviewer::Vec& Q)
{
	P = camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(),event->y(),0.0));
	Q = camera()->unprojectedCoordinatesOf(qoglviewer::Vec(event->x(),event->y(),1.0));
}


void Viewer::keyPressEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
		case Qt::Key_0:
			cell_picking = 0;
			break;
		case Qt::Key_1:
			cell_picking = 1;
			break;
		case Qt::Key_2:
			cell_picking = 2;
			break;
		case Qt::Key_3:
			cell_picking = 3;
			break;
	}
	QOGLViewer::keyPressEvent(ev);
}


void Viewer::mousePressEvent(QMouseEvent* event)
{
	if (event->modifiers() & Qt::ShiftModifier)
	{
		qoglviewer::Vec P;
		qoglviewer::Vec Q;
		rayClick(event,P,Q);

		Vec3 A(P[0],P[1],P[2]);
		Vec3 B(Q[0],Q[1],Q[2]);

		drawer_->new_list();
		switch(cell_picking)
		{
			case 0:
			{
				std::vector<Map2::Vertex> selected;
				cgogn::geometry::picking_vertices<Vec3>(map_,vertex_position_,A,B,selected);
				std::cout<< "Selected vertices: "<< selected.size()<<std::endl;
				if (!selected.empty())
				{
					drawer_->point_size_aa(4.0);
					drawer_->begin(GL_POINTS);
					// closest point in red
					drawer_->color3f(1.0,0.0,0.0);
					drawer_->vertex3fv(vertex_position_[selected[0]]);
					// others in yellow
					drawer_->color3f(1.0,1.0,0.0);
					for(uint32 i=1u;i<selected.size();++i)
						drawer_->vertex3fv(vertex_position_[selected[i]]);
					drawer_->end();
				}
			}
			break;
			case 1:
			{
				std::vector<Map2::Edge> selected;
				cgogn::geometry::picking_edges<Vec3>(map_,vertex_position_,A,B,selected);
				std::cout<< "Selected edges: "<< selected.size()<<std::endl;
				if (!selected.empty())
				{
					drawer_->line_width(2.0);
					drawer_->begin(GL_LINES);
					// closest face in red
					drawer_->color3f(1.0,0.0,0.0);
					cgogn::rendering::add_edge_to_drawer<Vec3>(map_,selected[0],vertex_position_,drawer_);
					// others in yellow
					drawer_->color3f(1.0,1.0,0.0);
					for(uint32 i=1u;i<selected.size();++i)
						cgogn::rendering::add_edge_to_drawer<Vec3>(map_,selected[i],vertex_position_,drawer_);
					drawer_->end();
				}
			}
			break;
			case 2:
			{
				std::vector<Map2::Face> selected;
				cgogn::geometry::picking_faces<Vec3>(map_,vertex_position_,A,B,selected);
				std::cout<< "Selected faces: "<< selected.size()<<std::endl;
				if (!selected.empty())
				{
					drawer_->line_width(2.0);
					drawer_->begin(GL_LINES);
					// closest face in red
					drawer_->color3f(1.0,0.0,0.0);
					cgogn::rendering::add_face_to_drawer<Vec3>(map_,selected[0],vertex_position_,drawer_);
					// others in yellow
					drawer_->color3f(1.0,1.0,0.0);
					for(uint32 i=1u;i<selected.size();++i)
						cgogn::rendering::add_face_to_drawer<Vec3>(map_,selected[i],vertex_position_,drawer_);
					drawer_->end();
				}
			}
			break;
			case 3:
			{
				std::vector<Map2::Volume> selected;
				cgogn::geometry::picking_volumes<Vec3>(map_,vertex_position_,A,B,selected);
				std::cout<< "Selected volumes: "<< selected.size()<<std::endl;
				if (!selected.empty())
				{
					drawer_->line_width(2.0);
					drawer_->begin(GL_LINES);
					// closest face in red
					drawer_->color3f(1.0,0.0,0.0);
					cgogn::rendering::add_volume_to_drawer<Vec3>(map_,selected[0],vertex_position_,drawer_);
					// others in yellow
					drawer_->color3f(1.0,1.0,0.0);
					for(uint32 i=1u;i<selected.size();++i)
						cgogn::rendering::add_volume_to_drawer<Vec3>(map_,selected[i],vertex_position_,drawer_);
					drawer_->end();
				}
			}
			break;
		}
		drawer_->line_width(4.0);
		drawer_->begin(GL_LINES);
		drawer_->color3f(1.0, 0.0, 1.0);
		drawer_->vertex3fv(A);
		drawer_->vertex3fv(B);
		drawer_->end();
		drawer_->end_list();
	}


	QOGLViewer::mousePressEvent(event);
}


int main(int argc, char** argv)
{
	std::string surfaceMesh;
	if (argc < 2)
	{
		std::cout << "USAGE: " << argv[0] << " [filename]" << std::endl;
		surfaceMesh = std::string(DEFAULT_MESH_PATH) + std::string("aneurysm3D_1.off");
		std::cout << "Using default mesh : " << surfaceMesh << std::endl;
	}
	else
		surfaceMesh = std::string(argv[1]);

	QApplication application(argc, argv);
	qoglviewer::init_ogl_context();

	// Instantiate the viewer.
	Viewer viewer;
	viewer.setWindowTitle("simpleViewer");
	viewer.import(surfaceMesh);
	viewer.show();
	viewer.resize(800,600);

	// Run main loop.
	return application.exec();
}