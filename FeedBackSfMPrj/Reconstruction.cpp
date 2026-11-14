#include "Reconstruction.h"
#include "misc.h"

#include <fstream>


Reconstruction::Reconstruction()
    : correspondence_graph_(nullptr), num_added_points3D_(0) {}

Reconstruction::~Reconstruction()
{
    try
    {
        cameras_.clear();
        images_.clear();
        points3D_.clear();
        image_pair_stats_.clear();
    }
    catch (...)
    {
        
    }
}

void Reconstruction::ReadCamerasText(const std::string& path) {
    cameras_.clear();

    std::ifstream file(path);

    std::string line;
    std::string item;
    
    while (std::getline(file, line)) {
        StringTrim(&line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream line_stream(line);

        class Camera camera;

        // ID
        std::getline(line_stream, item, ' ');
        camera.SetCameraId(std::stoul(item));

        // MODEL
        std::getline(line_stream, item, ' ');
        camera.SetModelIdFromName(item);

        // WIDTH
        std::getline(line_stream, item, ' ');
        camera.SetWidth(std::stoll(item));

        // HEIGHT
        std::getline(line_stream, item, ' ');
        camera.SetHeight(std::stoll(item));

        // PARAMS
        camera.Params().clear();
        while (!line_stream.eof()) {
            std::getline(line_stream, item, ' ');
            camera.Params().push_back(std::stold(item));
        }


        cameras_.emplace(camera.CameraId(), camera);
    }
}

void Reconstruction::ReadImagesText(const std::string& path) {
    images_.clear();

    std::ifstream file(path);

    std::string line;
    std::string item;

    while (std::getline(file, line)) {
        StringTrim(&line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream line_stream1(line);

        // ID
        std::getline(line_stream1, item, ' ');
        const image_t image_id = std::stoul(item);

        class Image image;
        image.SetImageId(image_id);

        image.SetRegistered(true);
        reg_image_ids_.push_back(image_id);

        Rigid3d& cam_from_world = image.CamFromWorld();

        std::getline(line_stream1, item, ' ');
        cam_from_world.rotation.w() = std::stold(item);

        std::getline(line_stream1, item, ' ');
        cam_from_world.rotation.x() = std::stold(item);

        std::getline(line_stream1, item, ' ');
        cam_from_world.rotation.y() = std::stold(item);

        std::getline(line_stream1, item, ' ');
        cam_from_world.rotation.z() = std::stold(item);

        cam_from_world.rotation.normalize();

        std::getline(line_stream1, item, ' ');
        cam_from_world.translation.x() = std::stold(item);

        std::getline(line_stream1, item, ' ');
        cam_from_world.translation.y() = std::stold(item);

        std::getline(line_stream1, item, ' ');
        cam_from_world.translation.z() = std::stold(item);

        // CAMERA_ID
        std::getline(line_stream1, item, ' ');
        image.SetCameraId(std::stoul(item));

        // NAME
        std::getline(line_stream1, item, ' ');
        image.SetName(item);

        // POINTS2D
        if (!std::getline(file, line)) {
            break;
        }

        StringTrim(&line);
        std::stringstream line_stream2(line);

        std::vector<Eigen::Vector2d> points2D;
        std::vector<point3D_t> point3D_ids;

        if (!line.empty()) {
            while (!line_stream2.eof()) {
                Eigen::Vector2d point;

                std::getline(line_stream2, item, ' ');
                point.x() = std::stold(item);

                std::getline(line_stream2, item, ' ');
                point.y() = std::stold(item);

                points2D.push_back(point);

                std::getline(line_stream2, item, ' ');
                if (item == "-1") {
                    point3D_ids.push_back(kInvalidPoint3DId);
                }
                else {
                    point3D_ids.push_back(std::stoll(item));
                }
            }
        }

        image.SetUp(Camera(image.CameraId()));
        image.SetPoints2D(points2D);

        for (point2D_t point2D_idx = 0; point2D_idx < image.NumPoints2D();
            ++point2D_idx) {
            if (point3D_ids[point2D_idx] != kInvalidPoint3DId) {
                image.SetPoint3DForPoint2D(point2D_idx, point3D_ids[point2D_idx]);
            }
        }

        images_.emplace(image.ImageId(), image);
    }
}

void Reconstruction::ReadPoints3DText(const std::string& path) {
    points3D_.clear();

    std::ifstream file(path);

    std::string line;
    std::string item;

    while (std::getline(file, line)) {
        StringTrim(&line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream line_stream(line);

        // ID
        std::getline(line_stream, item, ' ');
        const point3D_t point3D_id = std::stoll(item);

        // Make sure, that we can add new 3D points after reading 3D points
        // without overwriting existing 3D points.
        num_added_points3D_ = std::max(num_added_points3D_, point3D_id);

        class Point3D point3D;

        // XYZ
        std::getline(line_stream, item, ' ');
        point3D.XYZ(0) = std::stold(item);

        std::getline(line_stream, item, ' ');
        point3D.XYZ(1) = std::stold(item);

        std::getline(line_stream, item, ' ');
        point3D.XYZ(2) = std::stold(item);

        // Color
        std::getline(line_stream, item, ' ');
        point3D.Color(0) = static_cast<uint8_t>(std::stoi(item));

        std::getline(line_stream, item, ' ');
        point3D.Color(1) = static_cast<uint8_t>(std::stoi(item));

        std::getline(line_stream, item, ' ');
        point3D.Color(2) = static_cast<uint8_t>(std::stoi(item));

        // ERROR
        std::getline(line_stream, item, ' ');
        point3D.SetError(std::stold(item));

        // TRACK
        while (!line_stream.eof()) {
            TrackElement track_el;

            std::getline(line_stream, item, ' ');
            StringTrim(&item);
            if (item.empty()) {
                break;
            }
            track_el.image_id = std::stoul(item);

            std::getline(line_stream, item, ' ');
            track_el.point2D_idx = std::stoul(item);

            point3D.Track().AddElement(track_el);
        }

        point3D.Track().Compress();

        points3D_.emplace(point3D_id, point3D);
    }
}

void Reconstruction::ReadCamerasBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    const size_t num_cameras = ReadBinaryLittleEndian<uint64_t>(&file);
    for (size_t i = 0; i < num_cameras; ++i) {
        class Camera camera;
        camera.SetCameraId(ReadBinaryLittleEndian<camera_t>(&file));
        camera.SetModelId(ReadBinaryLittleEndian<int>(&file));
        camera.SetWidth(ReadBinaryLittleEndian<uint64_t>(&file));
        camera.SetHeight(ReadBinaryLittleEndian<uint64_t>(&file));
        ReadBinaryLittleEndian<double>(&file, &camera.Params());
        cameras_.emplace(camera.CameraId(), camera);
    }
}

void Reconstruction::ReadImagesBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    const size_t num_reg_images = ReadBinaryLittleEndian<uint64_t>(&file);
    for (size_t i = 0; i < num_reg_images; ++i) {
        class Image image;

        image.SetImageId(ReadBinaryLittleEndian<image_t>(&file));

        Rigid3d& cam_from_world = image.CamFromWorld();
        cam_from_world.rotation.w() = ReadBinaryLittleEndian<double>(&file);
        cam_from_world.rotation.x() = ReadBinaryLittleEndian<double>(&file);
        cam_from_world.rotation.y() = ReadBinaryLittleEndian<double>(&file);
        cam_from_world.rotation.z() = ReadBinaryLittleEndian<double>(&file);
        cam_from_world.rotation.normalize();
        cam_from_world.translation.x() = ReadBinaryLittleEndian<double>(&file);
        cam_from_world.translation.y() = ReadBinaryLittleEndian<double>(&file);
        cam_from_world.translation.z() = ReadBinaryLittleEndian<double>(&file);

        image.SetCameraId(ReadBinaryLittleEndian<camera_t>(&file));

        char name_char;
        do {
            file.read(&name_char, 1);
            if (name_char != '\0') {
                image.Name() += name_char;
            }
        } while (name_char != '\0');

        const size_t num_points2D = ReadBinaryLittleEndian<uint64_t>(&file);

        std::vector<Eigen::Vector2d> points2D;
        points2D.reserve(num_points2D);
        std::vector<point3D_t> point3D_ids;
        point3D_ids.reserve(num_points2D);
        for (size_t j = 0; j < num_points2D; ++j) {
            const double x = ReadBinaryLittleEndian<double>(&file);
            const double y = ReadBinaryLittleEndian<double>(&file);
            points2D.emplace_back(x, y);
            point3D_ids.push_back(ReadBinaryLittleEndian<point3D_t>(&file));
        }

        image.SetUp(Camera(image.CameraId()));
        image.SetPoints2D(points2D);

        for (point2D_t point2D_idx = 0; point2D_idx < image.NumPoints2D();
            ++point2D_idx) {
            if (point3D_ids[point2D_idx] != kInvalidPoint3DId) {
                image.SetPoint3DForPoint2D(point2D_idx, point3D_ids[point2D_idx]);
            }
        }

        image.SetRegistered(true);
        reg_image_ids_.push_back(image.ImageId());

        images_.emplace(image.ImageId(), image);
    }
}
void Reconstruction::ReadPoints3DBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary);

    const size_t num_points3D = ReadBinaryLittleEndian<uint64_t>(&file);
    for (size_t i = 0; i < num_points3D; ++i) {
        class Point3D point3D;

        const point3D_t point3D_id = ReadBinaryLittleEndian<point3D_t>(&file);
        num_added_points3D_ = std::max(num_added_points3D_, point3D_id);

        point3D.XYZ()(0) = ReadBinaryLittleEndian<double>(&file);
        point3D.XYZ()(1) = ReadBinaryLittleEndian<double>(&file);
        point3D.XYZ()(2) = ReadBinaryLittleEndian<double>(&file);
        point3D.Color(0) = ReadBinaryLittleEndian<uint8_t>(&file);
        point3D.Color(1) = ReadBinaryLittleEndian<uint8_t>(&file);
        point3D.Color(2) = ReadBinaryLittleEndian<uint8_t>(&file);
        point3D.SetError(ReadBinaryLittleEndian<double>(&file));

        const size_t track_length = ReadBinaryLittleEndian<uint64_t>(&file);
        for (size_t j = 0; j < track_length; ++j) {
            const image_t image_id = ReadBinaryLittleEndian<image_t>(&file);
            const point2D_t point2D_idx = ReadBinaryLittleEndian<point2D_t>(&file);
            point3D.Track().AddElement(image_id, point2D_idx);
        }
        point3D.Track().Compress();

        points3D_.emplace(point3D_id, point3D);
    }
}
void Reconstruction::ReadText(const std::string& path) {
    ReadCamerasText(JoinPaths(path, "cameras.txt"));
    ReadImagesText(JoinPaths(path, "images.txt"));
    ReadPoints3DText(JoinPaths(path, "points3D.txt"));
}
void Reconstruction::ReadBinary(const std::string& path) {
    ReadCamerasBinary(JoinPaths(path, "cameras.bin"));
    ReadImagesBinary(JoinPaths(path, "images.bin"));
    ReadPoints3DBinary(JoinPaths(path, "points3D.bin"));
}
void Reconstruction::Read(const std::string& path) {
    if (ExistsFile(JoinPaths(path, "cameras.bin")) &&
        ExistsFile(JoinPaths(path, "images.bin")) &&
        ExistsFile(JoinPaths(path, "points3D.bin"))) {
        ReadBinary(path);
    }
    else if (ExistsFile(JoinPaths(path, "cameras.txt")) &&
        ExistsFile(JoinPaths(path, "images.txt")) &&
        ExistsFile(JoinPaths(path, "points3D.txt"))) {
        ReadText(path);
    }
    else {
        
    }
}
