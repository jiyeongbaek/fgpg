/*
 * BSD 2-Clause License
 * 
 * Copyright (c) 2020, Suhan Park
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "fgpg/fcl_eigen_utils.h"
#include "fgpg/geometrics.h"
#include "fgpg/vtk_mesh_utils.h"

#include <fcl/traversal/traversal_node_bvhs.h>
#include <fcl/traversal/traversal_node_setup.h>
#include <fcl/collision_node.h>
#include <fcl/collision.h>
#include <fcl/BV/BV.h>
#include <fcl/BV/OBBRSS.h>
#include <fcl/shape/geometric_shapes.h>
#include <fcl/narrowphase/narrowphase.h>
#include <pcl/visualization/pcl_visualizer.h>

#include <fcl/broadphase/broadphase.h>
#include <fcl/collision.h>
#include <fcl/distance.h>

#include <ros/package.h>
#include <cmath>
typedef fcl::OBBRSS BV;
typedef fcl::BVHModel<BV> BVHM;
typedef std::shared_ptr<BVHM> BVHMPtr;
using fcl::Box;
typedef std::shared_ptr<fcl::Box> BoxPtr;
using fcl::CollisionObject;
typedef std::shared_ptr<fcl::CollisionObject> CollisionObjectPtr;

typedef pcl::PointCloud<pcl::PointXYZRGBNormal> CloudT;
typedef pcl::PointXYZRGBNormal PointT;
typedef std::shared_ptr<PointT> PointTPtr;

using namespace pcl;
struct FCLGripper
{
  // BoxPtr g[4];
  // Eigen::Isometry3d t[4];

  BVHMPtr g[3];
  Eigen::Isometry3d t[3];
  pcl::PolygonMesh mesh[3];
  std::string path = "/home/jiyeong/catkin_ws/src/3_constraint_planning/fgpg";

  double DXL_RAD = 13.5;
  Eigen::Isometry3d T_DXL_CTR;

  /// d: depth of the gripper
  /// h: width of the gripper
  /// l: total length of the gripper
  /// x1l: size of bottom gripper (width)
  /// y1l: size of bottom gripper (height)
  /// z2l: size of gripper finger (width)
  /// x4l: size of bar of the gripper (length)
  /// z4l: size of bar of the gripper (width)
  double l, h, d, x1l, y1l, z2l, x4l, z4l;

  // void setParams(double id, double ih, double il, double ix1l, double iy1l, double iz2l, double ix4l = 0.08, double iz4l = 0.02)
  // {
  //   l = il;
  //   h = ih;
  //   d = id;
  //   x1l = ix1l;
  //   y1l = iy1l;
  //   z2l = iz2l;
  //   x4l = ix4l;
  //   z4l = iz4l;

  //   makeModel();
  // }
  // void makeModel()
  // {
  //   double z1l = 2 * (h + z2l);
  //   g[0] = std::make_shared<Box>(x1l, y1l, z1l);
  //   g[1] = std::make_shared<Box>(l, y1l, z2l);
  //   g[2] = std::make_shared<Box>(l, y1l, z2l);
  //   g[3] = std::make_shared<Box>(x4l, y1l, z4l);

  //   t[0].linear().setIdentity();
  //   t[0].translation() << -d - x1l / 2, 0, 0;

  //   t[1].linear().setIdentity();
  //   t[1].translation() << -d + l / 2, 0, h + z2l / 2;

  //   t[2].linear().setIdentity();
  //   t[2].translation() << -d + l / 2, 0, -h - z2l / 2;

  //   t[3].linear().setIdentity();
  //   t[3].translation() << -d - x1l - x4l / 2, 0, 0;
  // }

  void makeRealModel()
  {
    pcl::io::loadPolygonFile(path + "/mesh/gripper_base.stl", mesh[0]);
    std::vector<TrianglePlaneData> triangles0 = buildTriangleData(mesh[0]);

    pcl::io::loadPolygonFile(path + "/mesh/gripper_tip_left.stl", mesh[1]);
    std::vector<TrianglePlaneData> triangles1 = buildTriangleData(mesh[1]);

    pcl::io::loadPolygonFile(path + "/mesh/gripper_tip_right.stl", mesh[2]);
    std::vector<TrianglePlaneData> triangles2 = buildTriangleData(mesh[2]);


    g[0] = std::make_shared<BVHM>();
    g[0] = loadMesh(triangles0);

    g[1] = std::make_shared<BVHM>();
    g[1] = loadMesh(triangles1);

    g[2] = std::make_shared<BVHM>();
    g[2] = loadMesh(triangles2);
      
    T_DXL_CTR.linear() << cos(DXL_RAD), 0, -sin(DXL_RAD),
                          0, 1.0000, 0,
                          sin(DXL_RAD), 0, cos(DXL_RAD);
    T_DXL_CTR.translation() << 0.0129, 0, 0.1132;
    Eigen::Isometry3d temp_frame;
    temp_frame.linear() << 0, 1, 0,
                          0, 0, 1,
                          1, 0, 0;
    Eigen::Isometry3d base_frame;
    base_frame.linear() = Eigen::AngleAxisd(-M_PI/2, Eigen::Vector3d::UnitX())
                          * Eigen::AngleAxisd(0, Eigen::Vector3d::UnitY())
                          * Eigen::AngleAxisd(-M_PI/2, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    t[0] = (T_DXL_CTR * temp_frame).inverse() * base_frame;

    // left
    Eigen::Isometry3d finger_frame;
    finger_frame.linear() = Eigen::AngleAxisd(-M_PI/2, Eigen::Vector3d::UnitX())
                          * Eigen::AngleAxisd(0, Eigen::Vector3d::UnitY())
                          * Eigen::AngleAxisd(-M_PI/2, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    finger_frame.translation() << 0, -0.05, 0.0745;
    t[1] = t[0] * finger_frame;

    // right
    finger_frame.translation() << 0, 0.05, 0.0745;
    t[2] = t[0] * finger_frame;
    
  }

  BVHMPtr loadMesh(const std::vector<TrianglePlaneData> &mesh)
  {
    std::vector<fcl::Vec3f> points;
    std::vector<fcl::Triangle> triangles;
    BVHMPtr mesh_model_ = std::make_shared<BVHM>();

    for (const auto &tri_plane : mesh)
    {
      fcl::Triangle tri;

      for (int i = 0; i < 3; i++)
      {
        tri[i] = points.size();
        points.push_back(
            fcl::Vec3f(
                tri_plane.points[i](0) * 0.001,
                tri_plane.points[i](1) * 0.001,
                tri_plane.points[i](2) * 0.001));
      }
      triangles.push_back(tri);
    }
    mesh_model_->beginModel();
    mesh_model_->addSubModel(points, triangles);
    mesh_model_->endModel();
    
    return mesh_model_;
  }

  void changeWidth(double new_h)
  {
    t[1].linear().setIdentity();
    t[1].translation() << -d + l / 2, 0, new_h + z2l / 2;

    t[2].linear().setIdentity();
    t[2].translation() << -d + l / 2, 0, -new_h - z2l / 2;
  }

  void drawGripper(pcl::visualization::PCLVisualizer &vis,
                   const Eigen::Isometry3d gripper_transform,
                   const std::string &id,
                   double r, double g_c, double b, double opacity,
                   double dist = -1.0)
  {
    if (dist < 0)
      changeWidth(h);
    else
      changeWidth(dist);
    
    for (int i = 0; i < 3; i++)
    {
      // auto T = gripper_transform * t[i];
      // Eigen::Vector3d position(T.translation());
      // Eigen::Quaterniond quat(T.linear());
      // Eigen::Vector3f posf;
      // Eigen::Quaternionf quatf;

      // posf = position.cast<float>();
      // quatf = quat.cast<float>();
      // std::string id_total = "cube" + id + std::to_string(i);
      // vis.addCube(posf, quatf, g[i]->side[0], g[i]->side[1], g[i]->side[2], id_total);
      // vis.setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, r, g_c, b, id_total);
      // vis.setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_OPACITY, opacity, id_total, 0);

      // std::string id_total_line = "cube_line" + id + std::to_string(i);
      // vis.addCube(posf, quatf, g[i]->side[0], g[i]->side[1], g[i]->side[2], id_total_line);
      // vis.setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 0, 0, 0, id_total_line);
      // vis.setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_OPACITY, opacity, id_total_line, 0);
      // vis.setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION, pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, id_total_line, 0);
    }
  }

  Eigen::Vector3d getPalmNormalVector(const Eigen::Isometry3d &transform) const
  {
    Eigen::Vector4d vec;
    vec << 1, 0, 0, 0;
    return (transform * vec).head<3>();
  }
  Eigen::Vector3d getPalmOrigin(const Eigen::Isometry3d &transform) const
  {
    Eigen::Vector4d vec;
    vec << -d, 0, 0, 1;
    return (transform * vec).head<3>();
  }

  /**
   * @brief Get the Finger1 Plane Points object
   * 
   * @param transform 
   * @return Eigen::Matrix<double, 3, 5> 0~3: points represent the rectangle, 4: normal vector
   */
  Eigen::Matrix<double, 3, 5> getFinger1PlanePoints(const Eigen::Isometry3d &transform) const
  {
    Eigen::Matrix<double, 3, 5> points;
    Eigen::Vector4d point;
    Eigen::Vector4d trans_point;
    Eigen::Matrix4d rectangle_points;

    rectangle_points << -d, -y1l / 2, h, 1,
        l - d, -y1l / 2, h, 1,
        -d, y1l / 2, h, 1,
        l - d, y1l / 2, h, 1;

    for (int i = 0; i < 4; i++)
    {
      trans_point = transform * rectangle_points.row(i).transpose();
      points.col(i) = trans_point.head<3>();
    }

    Eigen::Vector3d normal_vector;
    normal_vector << 0, 0, 1; // inversed
    points.col(4) = transform.linear() * normal_vector;

    return points;
  }

  Eigen::Matrix<double, 3, 5> getFinger2PlanePoints(const Eigen::Isometry3d &transform) const
  {
    Eigen::Matrix<double, 3, 5> points;
    Eigen::Vector4d point;
    Eigen::Vector4d trans_point;
    Eigen::Matrix4d rectangle_points;

    rectangle_points << -d, -y1l / 2, -h, 1,
        l - d, -y1l / 2, -h, 1,
        -d, y1l / 2, -h, 1,
        l - d, y1l / 2, -h, 1;

    for (int i = 0; i < 4; i++)
    {
      trans_point = transform * rectangle_points.row(i).transpose();
      points.col(i) = trans_point.head<3>();
    }

    Eigen::Vector3d normal_vector;
    normal_vector << 0, 0, -1; // inversed
    points.col(4) = transform.linear() * normal_vector;

    return points;
  }

  Eigen::Matrix<double, 3, 5> getPalmPlanePoints(const Eigen::Isometry3d &transform) const
  {
    Eigen::Matrix<double, 3, 5> points;
    Eigen::Vector4d point;
    Eigen::Vector4d trans_point;
    Eigen::Matrix4d rectangle_points;

    rectangle_points << -d, -y1l / 2, -h, 1,
        -d, -y1l / 2, h, 1,
        -d, y1l / 2, -h, 1,
        -d, y1l / 2, h, 1;

    for (int i = 0; i < 4; i++)
    {
      trans_point = transform * rectangle_points.row(i).transpose();
      points.col(i) = trans_point.head<3>();
    }

    Eigen::Vector3d normal_vector;
    normal_vector << -1, 0, 0; // inversed
    points.col(4) = transform.linear() * normal_vector;

    return points;
  }
};

class CollisionCheck
{
public:
  BVHMPtr mesh_model_;
  FCLGripper gripper_model_;

  void loadMesh(const std::vector<TrianglePlaneData> &mesh)
  {
    std::vector<fcl::Vec3f> points;
    std::vector<fcl::Triangle> triangles;
    mesh_model_ = std::make_shared<BVHM>();

    for (const auto &tri_plane : mesh)
    {
      fcl::Triangle tri;

      for (int i = 0; i < 3; i++)
      {
        tri[i] = points.size();
        points.push_back(
            fcl::Vec3f(
                tri_plane.points[i](0),
                tri_plane.points[i](1),
                tri_plane.points[i](2)));
      }
      triangles.push_back(tri);
    }
    mesh_model_->beginModel();
    mesh_model_->addSubModel(points, triangles);
    mesh_model_->endModel();
  }

  bool isFeasible(Eigen::Isometry3d gripper_transform, double distance)
  {
    // set the collision request structure, here we just use the default setting
    fcl::CollisionRequest request;
    // result will be returned via the collision result structure
    fcl::CollisionResult result[3];
    fcl::Transform3f init;
    init.setIdentity();
    // gripper_model_.changeWidth(distance);

    bool is_collided = false;
    for (int i = 0; i < 3 ; ++i)
    {
      Eigen::Isometry3d cur_transform = gripper_transform * gripper_model_.t[i];
      fcl::Transform3f fcl_transform;
      FCLEigenUtils::convertTransform(cur_transform, fcl_transform);

      fcl::collide(mesh_model_.get(), init, gripper_model_.g[i].get(), fcl_transform,
                   request, result[i]);      
      if (result[i].isCollision() == true)
      {
        is_collided = true;
        break;
      }
    }

    return !is_collided;
  }
};