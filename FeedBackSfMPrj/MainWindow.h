#include <QMainWindow>
#include "ModelViewerWidget.h"
#include "thread_control_widget.h"
#include "log_widget.h"
#include "settings_widget.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget* parent = nullptr);

private:
	std::shared_ptr<Reconstruction> reconstruction;
	std::shared_ptr <PlyMesh> mesh;

	ModelViewerWidget* modelViewerWidget = nullptr;
	ThreadControlWidget* thread_control_widget_;
	LogWidget* log_widget_;
	SettingsWidget* settings_widget_;

	QToolBar* reconstruction_toolbar_;

	QDockWidget* dock_log_widget_;
	QDockWidget* dock_settings_widget_;

	QAction* action_show_model_;
	QAction* action_export_mesh_;
	QAction* action_show_mesh_;
	QAction* action_hide_model_;
	QAction* action_switch_mesh_line_;
	QAction* action_load_reconstruction_; //load reconstruction model in model view

	bool is_draw_mesh_lines_ = false;

	void CreateWidgets();
	void CreateToolbar();
	void CreateActions();

	void ImportSparseModel();
	void ExportMesh();
	void AlignMeshToReconstruction();
	double TriangleTrack(PlyMeshFace& face);

	void ShowReconstructionModel();
	void ShowMesh();
	void HideModel();
	void SwitchMeshLine();
};