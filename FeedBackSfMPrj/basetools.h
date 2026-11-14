#pragma once
#include <vector>
#include <iostream>
#include <nanoflann.h>
#include <cmath>
#include <omp.h> 

//void MainWindow::AlignMeshToReconstruction()
// Point-cloud alignment.

struct PointWithID {
    double x;
    double y;
    double z;
    int id; 

    PointWithID(double x_val, double y_val, double z_val, int id_val)
        : x(x_val), y(y_val), z(z_val), id(id_val) {}
};

struct PointCloud {
    std::vector<PointWithID> pts;

    inline size_t kdtree_get_point_count() const { return pts.size(); }

    inline double kdtree_get_pt(const size_t idx, const size_t dim) const {
        if (dim == 0) return pts[idx].x;
        else if (dim == 1) return pts[idx].y;
        else return pts[idx].z;
    }

    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
};

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<double, PointCloud>,
    PointCloud,
    3 /* dimensions */
> my_kd_tree_t;

void assignIDs(std::vector<PointWithID>& a, const std::vector<PointWithID>& b) {
    PointCloud cloud;
    cloud.pts = b;

    my_kd_tree_t index(3, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10 /* max leaf */));
    index.buildIndex();

    const double threshold_sq = 1e-6;

#pragma omp parallel for
    for (size_t i = 0; i < a.size(); ++i) {
        const PointWithID& pointA = a[i];
        double query_pt[3] = { pointA.x, pointA.y, pointA.z };

        size_t ret_index;
        double out_dist_sqr;
        nanoflann::KNNResultSet<double> resultSet(1);
        resultSet.init(&ret_index, &out_dist_sqr);
        index.findNeighbors(resultSet, query_pt, nanoflann::SearchParameters(10));

        if (out_dist_sqr < threshold_sq) {
            a[i].id = cloud.pts[ret_index].id;
        }
        else {
            a[i].id = -1;
        }
    }
}
