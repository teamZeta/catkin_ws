#include <pcl/point_cloud.h>
#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/sample_consensus/sac_model_cylinder.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/extract_indices.h>

#include <iostream>
#include <vector>
#include <pcl/search/search.h>
#include <pcl/search/kdtree.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/filters/passthrough.h>
#include <pcl/segmentation/region_growing_rgb.h>

#include <pcl/features/normal_3d.h>





ros::Publisher pub;

// This function is a callback for incoming pointcloud data
void callback (const pcl::PCLPointCloud2ConstPtr& cloud_blob) {
  pcl::search::KdTree<pcl::PointXYZRGB>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZRGB> ());
  pcl::NormalEstimation<pcl::PointXYZRGB, pcl::Normal> ne;
  pcl::PCLPointCloud2::Ptr cloud_filtered_blob (new pcl::PCLPointCloud2);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZRGB>), cloud_p (new pcl::PointCloud<pcl::PointXYZRGB>), cloud_f (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered2 (new pcl::PointCloud<pcl::PointXYZRGB>);
 pcl::PointCloud<pcl::Normal>::Ptr cloud_normals (new pcl::PointCloud<pcl::Normal>);
  pcl::PointCloud<pcl::Normal>::Ptr cloud_normals2 (new pcl::PointCloud<pcl::Normal>);
  // Create the filtering object: downsample the dataset using a leaf size of 1cm
  

  // Convert to the templated PointCloud
  pcl::fromPCLPointCloud2 (*cloud_blob, *cloud_filtered);

  pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients ());
  pcl::PointIndices::Ptr inliers (new pcl::PointIndices ());

  // Estimate point normals
  ne.setSearchMethod (tree);
  ne.setInputCloud (cloud_filtered);
  ne.setKSearch (50);
  ne.compute (*cloud_normals);

  //pcl::SACSegmentation<pcl::PointXYZRGB> seg;
  pcl::SACSegmentationFromNormals<pcl::PointXYZRGB, pcl::Normal> seg; 
  seg.setOptimizeCoefficients (true);
  seg.setModelType (pcl::SACMODEL_NORMAL_PLANE);
  seg.setNormalDistanceWeight (0.1);
  seg.setMethodType (pcl::SAC_RANSAC);
  seg.setMaxIterations (100);
  seg.setDistanceThreshold (0.01);
  seg.setInputCloud (cloud_filtered);
  seg.setInputNormals (cloud_normals);
    
  seg.segment(*inliers, *coefficients);

 
  // Exit if no plane found
  if (inliers->indices.size () == 0) return;

  // Extract points of found plane
  pcl::ExtractIndices<pcl::PointXYZRGB> extract;
  pcl::ExtractIndices<pcl::Normal> extract_normals;

  extract.setInputCloud(cloud_filtered);
  extract.setIndices(inliers);
  extract.setNegative(true);
  extract.filter(*cloud_filtered2);
  
  extract_normals.setNegative (true);
  extract_normals.setInputCloud (cloud_normals);
  extract_normals.setIndices (inliers);
  extract_normals.filter (*cloud_normals2);


  pcl::ModelCoefficients::Ptr coefficients2 (new pcl::ModelCoefficients ());
  pcl::PointIndices::Ptr inliers2 (new pcl::PointIndices ()); 
  //pcl::fromPCLPointCloud2 (*cloud_filtered_blob, *cloud_filtered2);
  pcl::SACSegmentationFromNormals<pcl::PointXYZRGB, pcl::Normal> seg2;
  seg2.setOptimizeCoefficients (true);
  seg2.setModelType (pcl::SACMODEL_CYLINDER);
  seg2.setMethodType (pcl::SAC_RANSAC);
  seg2.setNormalDistanceWeight (0.1);
  seg2.setMaxIterations (10000);
  seg2.setDistanceThreshold (0.05);
  seg2.setRadiusLimits (0.02, 0.25);
  seg2.setInputCloud (cloud_filtered2);
  seg2.setInputNormals (cloud_normals2);

  seg2.segment (*inliers2, *coefficients2);
  // Extract points of found plane
  printf("velikost %ld", inliers2->indices.size ());
  if (inliers2->indices.size () == 0) return;


  extract.setInputCloud(cloud_filtered2);
  extract.setIndices(inliers2);
  extract.setNegative(false);
  extract.filter(*cloud_f);

  // Convert to ROS data type
  pcl::PCLPointCloud2 outcloud;
  pcl::toPCLPointCloud2 (*cloud_f, outcloud);
  outcloud.header.frame_id = "/camera_depth_frame";
  // Publish the data
  /*pcl::PCLPointCloud2 out_filtered_cloud;
  pcl::VoxelGrid<pcl::PCLPointCloud2> sor;
  sor.setInputCloud (outcloud);
  sor.setLeafSize (0.01f, 0.01f, 0.01f);
  sor.filter (out_filtered_cloud);*/

  pub.publish (outcloud);

/*
//nasa koda
//std::vector< PointIndices > vector;
// pcl::search<pcl::Search> iskanje (new pcl::search<pcl::Search>);
 // pcl::extractEuclideanClusters(*cloud_f, iskanje->nearestKSearch, 0.1, *vector, 100, 20000);
 pcl::search::Search <pcl::PointXYZRGB>::Ptr tree = boost::shared_ptr<pcl::search::Search<pcl::PointXYZRGB> > (new pcl::search::KdTree<pcl::PointXYZRGB>);

pcl::IndicesPtr indices (new std::vector <int>);
  pcl::PassThrough<pcl::PointXYZRGB> pass;
  pass.setInputCloud (cloud_f);
  pass.setFilterFieldName ("z");
  pass.setFilterLimits (0.0, 1.0);
  pass.filter (*indices);

  pcl::RegionGrowingRGB<pcl::PointXYZRGB> reg;
  reg.setInputCloud (cloud_f);
  reg.setIndices (indices);
  reg.setSearchMethod (tree);
  reg.setDistanceThreshold (3);
  reg.setPointColorThreshold (26);
  reg.setRegionColorThreshold (25);
  reg.setMinClusterSize (60);

  std::vector <pcl::PointIndices> clusters;
  reg.extract (clusters);

  pcl::PointCloud <pcl::PointXYZRGB>::Ptr colored_cloud = reg.getColoredCloud ();
  

  // Publish the plane to a new topic.
  pcl::PCLPointCloud2 outcloud;
  pcl::toPCLPointCloud2 (*colored_cloud, outcloud);
  outcloud.header.frame_id = "/camera_depth_frame";
  //outcloud.header.stamp = ros::Time::now();
  pub.publish (outcloud);*/
}

int
main (int argc, char** argv)
{
  // Initialize ROS
  ros::init (argc, argv, "my_pcl_tutorial");
  ros::NodeHandle nh;

  // Create a ROS subscriber for the input point cloud
  ros::Subscriber sub = nh.subscribe<pcl::PCLPointCloud2> ("input", 1, callback);

  // Create a ROS publisher for the output point cloud
  pub = nh.advertise<sensor_msgs::PointCloud2> ("cluster", 1);

  // Spin
  ros::spin ();
}

