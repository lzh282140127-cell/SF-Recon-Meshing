#pragma once
#include "Sim3D.h"
#include "types.h"
#include "Camera.h"
#include "Image.h"
#include "basestring.h"
#include "Point2D.h"
#include "Point3D.h"
#include "Track.h"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tbb/concurrent_unordered_map.h>

#include <Eigen/Core>


struct PlyPoint;
struct RANSACOptions;
class DatabaseCache;
class CorrespondenceGraph;

// Reconstruction class holds all information about a single reconstructed
// model. It is used by the mapping and bundle adjustment classes and can be
// written to and read from disk.
class Reconstruction {
public:
    struct ImagePairStat {
        // The number of triangulated correspondences between two images.
        size_t num_tri_corrs = 0;
        // The number of total correspondences/matches between two images.
        size_t num_total_corrs = 0;
    };

    Reconstruction();
    ~Reconstruction();

    // Get number of objects.
    inline size_t NumRegImages() const;
    inline size_t NumPoints3D() const;

    // Get const objects.
    inline const struct Camera& Camera(camera_t camera_id) const;
    inline const class Image& Image(image_t image_id) const;
    inline const struct Point3D& Point3D(point3D_t point3D_id) const;

    // Get mutable objects.
    inline class Camera& Camera(camera_t camera_id)
    {
        return cameras_.at(camera_id);
    }
    inline class Image& Image(image_t image_id)
    {
        return images_.at(image_id);
    }
    inline class Point3D& Point3D(point3D_t point3D_id)
    {
        return points3D_.at(point3D_id);
    }
    inline ImagePairStat& ImagePair(image_pair_t pair_id)
    {
        return image_pair_stats_.at(pair_id);
    }

    // Get reference to all objects.
    inline const tbb::concurrent_unordered_map<camera_t, class Camera> Cameras() const
    {
        return cameras_;
    }
    inline const tbb::concurrent_unordered_map<image_t, class Image>& Images() const
    {
        return images_;
    }
    inline const std::vector<image_t>& RegImageIds() const
    {
        return reg_image_ids_;
    }
    inline const tbb::concurrent_unordered_map<point3D_t, class Point3D>& Points3D() const
    {
        return points3D_;
    }
    void Read(const std::string& path);
    void ReadText(const std::string& path);
    void ReadBinary(const std::string& path);

private:
    void ReadCamerasText(const std::string& path);
    void ReadImagesText(const std::string& path);
    void ReadPoints3DText(const std::string& path);

    void ReadCamerasBinary(const std::string& path);
    void ReadImagesBinary(const std::string& path);
    void ReadPoints3DBinary(const std::string& path);

    std::shared_ptr<const CorrespondenceGraph> correspondence_graph_;

    tbb::concurrent_unordered_map<camera_t, class Camera> cameras_;
    tbb::concurrent_unordered_map<image_t, class Image> images_;
    tbb::concurrent_unordered_map<point3D_t, class Point3D> points3D_;

    tbb::concurrent_unordered_map<image_pair_t, ImagePairStat> image_pair_stats_;

    // { image_id, ... } where `images_.at(image_id).registered == true`.
    std::vector<image_t> reg_image_ids_;

    // Total number of added 3D points, used to generate unique identifiers.
    point3D_t num_added_points3D_;
};

size_t Reconstruction::NumRegImages() const { return reg_image_ids_.size(); }

size_t Reconstruction::NumPoints3D() const { return points3D_.size(); }

const struct Camera& Reconstruction::Camera(const camera_t camera_id) const {
    try {
        return cameras_.at(camera_id);
    }
    catch (const std::out_of_range&) {
        throw std::out_of_range(
            StringPrintf("Camera with ID %d does not exist", camera_id));
    }
}

const class Image& Reconstruction::Image(const image_t image_id) const {
    try {
        return images_.at(image_id);
    }
    catch (const std::out_of_range&) {
        throw std::out_of_range(
            StringPrintf("Image with ID %d does not exist", image_id));
    }
}

const struct Point3D& Reconstruction::Point3D(
    const point3D_t point3D_id) const {
    try {
        return points3D_.at(point3D_id);
    }
    catch (const std::out_of_range&) {
        throw std::out_of_range(
            StringPrintf("Point3D with ID %d does not exist", point3D_id));
    }
}