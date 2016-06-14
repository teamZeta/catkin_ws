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
#include <visualization_msgs/Marker.h>
#include <cstdlib>
#include <std_msgs/String.h>
#include <sstream>
#include <unistd.h>

using namespace std;


//ros::Publisher pub;
static ros::Publisher marker_pose;
static ros::Publisher marker_pose_white;




typedef struct {
    double r;       // percent
    double g;       // percent
    double b;       // percent
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // percent
    double v;       // percent
} hsv;

static hsv   rgb2hsv(rgb in);
static rgb   hsv2rgb(hsv in);

hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
            // s = 0, v is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}






static void mark_cluster(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_cluster, std::string ns ,int id, float r, float g, float b)
{

  bool esc = false;
 float rc = 0;
 float gc = 0;
 float bc = 0;
 int count = 0;
 float max2 = 0;
 for( size_t i = 0; i<cloud_cluster->points.size(); i+=10){
 	if(cloud_cluster->points[i].a > 0){
 		rc += cloud_cluster->points[i].r;
	 	gc += cloud_cluster->points[i].g;
	 	bc += cloud_cluster->points[i].b;
	 	count++;
 	}
 }
 rc /= count;
 gc /= count;
 bc /= count;
 if(max2 < rc){
 	max2 = rc;
 }
 if(max2 < gc){
 	max2 = gc;
 }
 if(max2 < bc){
	max2 = bc;
 }
  rc /= max2;
  gc /= max2;
  bc /= max2;

  rgb rgb_barva;
  rgb_barva.r = rc;
  rgb_barva.g = gc;
  rgb_barva.b = bc;

  hsv hsv_barva;
  hsv_barva = rgb2hsv(rgb_barva);


  uint8_t ru = 0x00;
  uint8_t gu = 0x00;
  uint8_t bu = 0x00;
  r = 0;//reinterpret_cast<float&>(ru)/256;
  g = 0;//reinterpret_cast<float&>(gu)/256;
  b = 0;//reinterpret_cast<float&>(bu)/256;
/*
  if(hsv_barva.s < 0.001){
  	return;
  }
  if(hsv_barva.v < 0.001){
  	return;
  }*/
  	int barva = 0;
  if(hsv_barva.s < 25 && hsv_barva.v > -1){
  	if(hsv_barva.h > 30 && hsv_barva.h < 80){
	  	g = 1;
	  	r = 1;
	  	b = 0;
	  	ru = 0xff;
	  	gu = 0xff;
	  	barva = 4;
	  }else if(hsv_barva.h > 85 && hsv_barva.h < 140){
	  	g = 1;
	  	r = 0;
	  	b = 0;
	  	gu = 0xff;
	  	barva = 2;

	  }else if(hsv_barva.h > 170 && hsv_barva.h < 270){
	  	g = 1;
	  	r = 0;
	  	b = 1;
	  	gu = 0xff;
	  	bu = 0xff;
	  	barva = 3;
	  }else if(hsv_barva.h > 340 || hsv_barva.h < 20){
	  	g = 0;
	  	r = 1;
	  	b = 0;
	  	ru = 0xff;
	  	barva = 1;
	  }
  }
  
  
  

  const uint32_t green = 0x00ff00;
  const uint32_t magenta = 0xff00ff;
  const uint32_t cyan = 0x00ffff;
  const uint32_t yellow = 0xffff00;

  

  printf("%f", r);
  Eigen::Vector4f centroid;
  Eigen::Vector4f min;
  Eigen::Vector4f max;
 
  pcl::compute3DCentroid (*cloud_cluster, centroid);
  pcl::getMinMax3D (*cloud_cluster, min, max);
 
  uint32_t shape = visualization_msgs::Marker::CUBE;
  visualization_msgs::Marker marker;
  marker.header.frame_id = cloud_cluster->header.frame_id;
  marker.header.stamp = ros::Time::now();
 
  marker.ns = ns;

  
  if(barva == 0){
  	esc = true;
  }

  marker.id = barva;
  marker.type = shape;
  marker.action = visualization_msgs::Marker::ADD;
 
  marker.pose.position.x = centroid[0];
  marker.pose.position.y = centroid[1];
  marker.pose.position.z = centroid[2];
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = 1.0;
 
  marker.scale.x = (max[0]-min[0]);
  marker.scale.y = (max[1]-min[1]);
  marker.scale.z = (max[2]-min[2]);
 
  if (marker.scale.x ==0){
      marker.scale.x=0.1;
      esc = true;
  }

  if (marker.scale.y ==0){

    	marker.scale.y=0.1;
    	esc = true;
	}

  if (marker.scale.z ==0){
    marker.scale.z=0.1;
    esc = true;
}

if((marker.scale.z < 0.15 || marker.scale.z > 0.5) || 
	(marker.scale.x < 0.15 || marker.scale.x > 0.5) ||
	(marker.scale.y < 0.15 || marker.scale.y > 0.5)){
	esc = true;
}
   
	

  marker.color.r = r;
  marker.color.g = g;
  marker.color.b = b;
  marker.color.a = 1;

  marker.lifetime = ros::Duration();
  //marker.lifetime = ros::Duration();
  if(esc == false){
    marker_pose.publish (marker);
  }
  
  marker_pose_white.publish(marker);
  
} 



// This function is a callback for incoming pointcloud data
void callback (const pcl::PCLPointCloud2ConstPtr& cloud_blob) {
  pcl::search::KdTree<pcl::PointXYZRGB>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZRGB> ());
  pcl::NormalEstimation<pcl::PointXYZRGB, pcl::Normal> ne;
  pcl::PCLPointCloud2::Ptr cloud_filtered_blob (new pcl::PCLPointCloud2);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZRGB>), cloud_p (new pcl::PointCloud<pcl::PointXYZRGB>), cloud_f (new pcl::PointCloud<pcl::PointXYZRGB>);
  

  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered_raw (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered_x (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered_xy (new pcl::PointCloud<pcl::PointXYZRGB>);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered_xyz (new pcl::PointCloud<pcl::PointXYZRGB>);

  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_filtered2 (new pcl::PointCloud<pcl::PointXYZRGB>);
 pcl::PointCloud<pcl::Normal>::Ptr cloud_normals (new pcl::PointCloud<pcl::Normal>);
  pcl::PointCloud<pcl::Normal>::Ptr cloud_normals2 (new pcl::PointCloud<pcl::Normal>);
  // Create the filtering object: downsample the dataset using a leaf size of 1cm
  

  // Create the filtering object: downsample the dataset using a leaf size of 1cm
  pcl::VoxelGrid<pcl::PCLPointCloud2> sor;
  sor.setInputCloud (cloud_blob);
  sor.setLeafSize (0.01f, 0.01f, 0.01f);
  sor.filter (*cloud_filtered_blob);

  // Convert to the templated PointCloud
  pcl::fromPCLPointCloud2 (*cloud_blob, *cloud_filtered_raw);

  pcl::PassThrough<pcl::PointXYZRGB> pass_x;
  pass_x.setInputCloud (cloud_filtered_raw);
  pass_x.setFilterFieldName ("x");
  pass_x.setFilterLimits (0, 2);
  //pass_x.setFilterLimitsNegative (true);
  pass_x.filter (*cloud_filtered_x); 

  pcl::PassThrough<pcl::PointXYZRGB> pass_y;
  pass_y.setInputCloud (cloud_filtered_x);
  pass_y.setFilterFieldName ("y");
  pass_y.setFilterLimits (0, 2);
  //pass_x.setFilterLimitsNegative (true);
  pass_y.filter (*cloud_filtered_xy);

  pcl::PassThrough<pcl::PointXYZRGB> pass_z;
  pass_z.setInputCloud (cloud_filtered_xy);
  pass_z.setFilterFieldName ("z");
  pass_z.setFilterLimits (0, 1);
  //pass_x.setFilterLimitsNegative (true);
  pass_z.filter (*cloud_filtered);

  pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients ());
  pcl::PointIndices::Ptr inliers (new pcl::PointIndices ());

  // Estimate point normals
  ne.setSearchMethod (tree);
  ne.setInputCloud (cloud_filtered);
  ne.setKSearch (10);
  ne.compute (*cloud_normals);

  //pcl::SACSegmentation<pcl::PointXYZRGB> seg;
  pcl::SACSegmentationFromNormals<pcl::PointXYZRGB, pcl::Normal> seg; 
  seg.setOptimizeCoefficients (true);
  seg.setModelType (pcl::SACMODEL_NORMAL_PLANE);
  seg.setNormalDistanceWeight (0.1);
  seg.setMethodType (pcl::SAC_RANSAC);
  seg.setMaxIterations (1000);
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
  seg2.setDistanceThreshold (0.03);
  seg2.setRadiusLimits (0.1, 0.14);
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

  //pub.publish (outcloud);
  mark_cluster(cloud_f, "boka", 1, 0.5,0.5,0.5);

}



int
main (int argc, char** argv)
{
  // Initialize ROS
  ros::init (argc, argv, "my_pcl_tutorial");
  ros::NodeHandle nh;
  ros::NodeHandle nh2;
  ros::NodeHandle nh3;

  // Create a ROS subscriber for the input point cloud
  ros::Subscriber sub = nh.subscribe<pcl::PCLPointCloud2> ("input", 1, callback);

  // Create a ROS publisher for the output point cloud
  //pub = nh.advertise<sensor_msgs::PointCloud2> ("cluster", 1);
  marker_pose = nh2.advertise<visualization_msgs:: Marker>("hotel", 1);
  marker_pose_white = nh3.advertise<visualization_msgs:: Marker>("hotelW", 1);
  // Spin
  ros::Rate r(0.5);
    while (ros::ok()){
      ros::spinOnce();               
      r.sleep();
    }
}

