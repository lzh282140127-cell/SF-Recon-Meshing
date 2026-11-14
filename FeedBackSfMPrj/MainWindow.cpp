#include "MainWindow.h"
#include "settings.h"

#include "meshing.h"
#include "progressbar.h"
#include <basetools.h>

MainWindow::MainWindow(QWidget* parent) :QMainWindow(parent), 
	thread_control_widget_(new ThreadControlWidget(this))
{
	resize(1000, 600);
	setWindowTitle(tr("OTF Feedback"));

	modelViewerWidget = new ModelViewerWidget(this);
	setCentralWidget(modelViewerWidget);

	QFont font;
	font.setPointSize(11);

	modelViewerWidget->statusbar_status_label = new QLabel("0 Images - 0 Points", this);
	modelViewerWidget->statusbar_status_label->setFont(font);
	modelViewerWidget->statusbar_status_label->setAlignment(Qt::AlignCenter);
	statusBar()->addWidget(modelViewerWidget->statusbar_status_label, 1);


	CreateWidgets();
	CreateActions();
	CreateToolbar();
}

void MainWindow::CreateWidgets() {
	log_widget_ = new LogWidget(this);
	dock_log_widget_ = new QDockWidget("Log", this);
	dock_log_widget_->setWidget(log_widget_);
	addDockWidget(Qt::RightDockWidgetArea, dock_log_widget_);

	settings_widget_ = new SettingsWidget(this);
	settings_widget_->setFilePath(QString("./settings.ini"));
	dock_settings_widget_ = new QDockWidget("Settings", this);
	dock_settings_widget_->setWidget(settings_widget_);
	addDockWidget(Qt::RightDockWidgetArea, dock_settings_widget_);

	tabifyDockWidget(dock_log_widget_, dock_settings_widget_);
	dock_log_widget_->raise();
}

void MainWindow::CreateToolbar() {
	reconstruction_toolbar_ = addToolBar(tr("Reconstruction"));
	reconstruction_toolbar_->addAction(action_show_model_);
	reconstruction_toolbar_->addAction(action_export_mesh_);
	reconstruction_toolbar_->addAction(action_show_mesh_);
	reconstruction_toolbar_->addAction(action_hide_model_);
	reconstruction_toolbar_->addAction(action_switch_mesh_line_);
}

void MainWindow::CreateActions() {
	action_show_model_ = new QAction("Load Model",this);
	connect(action_show_model_,
		&QAction::triggered,this,
		&MainWindow::ImportSparseModel);

	action_export_mesh_ = new QAction("Calculate Mesh", this);
	connect(action_export_mesh_,
		&QAction::triggered, this,
		&MainWindow::ExportMesh);

	action_load_reconstruction_ = new QAction("show model",this);
	connect(action_load_reconstruction_,
		&QAction::triggered,this,
		&MainWindow::ShowReconstructionModel);

	action_show_mesh_ = new QAction("Show mesh", this);
	connect(action_show_mesh_,
		&QAction::triggered, this,
		&MainWindow::ShowMesh);

	action_hide_model_ = new QAction("Hide model", this);
	connect(action_hide_model_,
		&QAction::triggered, this,
		&MainWindow::HideModel);

	action_switch_mesh_line_ = new QAction("Switch mesh line", this);
	connect(action_switch_mesh_line_,
		&QAction::triggered, this,
		&MainWindow::SwitchMeshLine);
}

void MainWindow::ShowReconstructionModel() {
	modelViewerWidget->reconstruction = reconstruction;
	modelViewerWidget->ReloadReconstruction();

	LOG(INFO) << "Import sparse model...";
}

void MainWindow::ShowMesh() {
	modelViewerWidget->mesh = mesh;
	modelViewerWidget->ReloadReconstructionWithMeshData();
}

void MainWindow::HideModel() {
	modelViewerWidget->ClearReconstruction();
}

void MainWindow::SwitchMeshLine() {
	is_draw_mesh_lines_ = !is_draw_mesh_lines_;
	modelViewerWidget->SwitchMeshLine(is_draw_mesh_lines_);
}



void MainWindow::ImportSparseModel() {
	thread_control_widget_->StartFunction(
		"Importing...", [this]() {
			//Read Model
			EasyIni* settings = EasyIni::GetPtr();
			std::string modelPath = settings->get("Base", "model_path");
			reconstruction = std::make_shared<Reconstruction>();
			reconstruction->Read(modelPath);
			//Notify the UI thread to display the model.
			action_load_reconstruction_->trigger();
		});
}

void MainWindow::AlignMeshToReconstruction() {
	std::vector<PointWithID> a;
	std::vector<PointWithID> b;
	for (const auto& v : mesh->vertices) {
		a.emplace_back(PointWithID(v.x,v.y,v.z,v.point3D_idx));
	}
	for (const auto& p : reconstruction->Points3D()) {
		b.emplace_back(PointWithID(p.second.X(), p.second.Y(), p.second.Z(), p.first));
	}
	assignIDs(a, b);
	for (size_t idx = 0; idx < mesh->vertices.size();idx++) {
		mesh->vertices[idx].point3D_idx = a[idx].id;
	}

	for (auto& f : mesh->faces) {
		TriangleTrack(f);
	}
}

double MainWindow::TriangleTrack(PlyMeshFace& face) {
	using TriangleElement = std::tuple<image_t, point2D_t, point2D_t, point2D_t>;

	PlyMeshVertex vertex1 = mesh->vertices[face.vertex_idx1];
	PlyMeshVertex vertex2 = mesh->vertices[face.vertex_idx2];
	PlyMeshVertex vertex3 = mesh->vertices[face.vertex_idx3];

	auto track1 = reconstruction->Point3D(vertex1.point3D_idx).Track().Elements();
	auto track2 = reconstruction->Point3D(vertex2.point3D_idx).Track().Elements();
	auto track3 = reconstruction->Point3D(vertex3.point3D_idx).Track().Elements();
	face.img_track = (track1.size() + track2.size() + track3.size()) / 3;
	
	std::vector<TriangleElement> result;
	std::unordered_map<image_t, point2D_t> map1, map2, map3;
	for (const auto& elem : track1) { map1[elem.image_id] = elem.point2D_idx; }
	for (const auto& elem : track2) { map2[elem.image_id] = elem.point2D_idx; }
	for (const auto& elem : track3) { map3[elem.image_id] = elem.point2D_idx; }
	for (const auto& pair1 : map1) {
		int id = pair1.first;
		if (map2.count(id) > 0 && map3.count(id) > 0) {
			result.emplace_back(TriangleElement(id, pair1.second, map2[id], map3[id]));
		}
	}
	if (result.empty()) { 
		return 0.0;
	}
	std::vector<double> areas2d;
	double max_area2d = std::numeric_limits<double>::max();
	for (const auto& tri : result) {
		auto p1 = reconstruction->Image(std::get<0>(tri)).Point2D(std::get<1>(tri));
		auto p2 = reconstruction->Image(std::get<0>(tri)).Point2D(std::get<2>(tri));
		auto p3 = reconstruction->Image(std::get<0>(tri)).Point2D(std::get<3>(tri));

		double area2d = 0.5 * std::abs(p1.xy[0] * (p2.xy[1] - p3.xy[1]) +
			p2.xy[0] * (p3.xy[1] - p1.xy[1]) + p3.xy[0] * (p1.xy[1] - p2.xy[1]));
		areas2d.push_back(area2d);
	}
	if (areas2d.empty()) { return 0.0; }
	auto maxit = std::max_element(areas2d.begin(), areas2d.end());
	if (maxit != areas2d.end()) { max_area2d = *maxit; }
	if (max_area2d <= 0.0){ max_area2d = std::numeric_limits<double>::max(); }

	double v1x = vertex2.x - vertex1.x, v1y = vertex2.y - vertex1.y, v1z = vertex2.z - vertex1.z;
	double v2x = vertex3.x - vertex1.x, v2y = vertex3.y - vertex1.y, v2z = vertex3.z - vertex1.z;
	double crossX = v1y * v2z - v1z * v2y;
	double crossY = v1z * v2x - v1x * v2z;
	double crossZ = v1x * v2y - v1y * v2x;
	double area3d = 0.5 * std::sqrt(crossX * crossX + crossY * crossY + crossZ * crossZ);
	double GSD = std::sqrt(area3d / max_area2d);

	face.gsd = GSD;
	//face.img_track = count_if(areas2d.begin(), areas2d.end(), [](double value) { return value > 0.0; });
	
	return GSD;
}

void MainWindow::ExportMesh() {
	thread_control_widget_->StartFunction(
		"Exporting mesh...", [this]() {
			EasyIni* settings = EasyIni::GetPtr();
			std::string modelPath = settings->get("Base", "model_path");
			std::string output_path = settings->get("Base", "output_path");
			auto options = DelaunayMeshingOptions();
			auto meshobj = SparseDelaunayMeshing(options, modelPath, output_path);
			mesh = std::make_shared<PlyMesh>(meshobj);
			AlignMeshToReconstruction();
	});
}

