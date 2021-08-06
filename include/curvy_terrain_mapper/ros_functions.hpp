#ifndef ROS_FUNCTIONS_
#define ROS_FUNCTIONS_

// Point Cloud Library
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>

// #include <curvy_terrain_mapper/ros_functions.hpp>
#include <curvy_terrain_mapper/regions.h>

#include <string>

#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <visualization_msgs/MarkerArray.h>
// #include <pcl_ros/point_cloud.h>
// #include <pcl_ros/transforms.h>
#include <pcl_conversions/pcl_conversions.h>
// #include <colormap/palettes.hpp>
#include <tf/transform_listener.h>
#include <tf_conversions/tf_eigen.h>
#include <yaml-cpp/yaml.h>

typedef pcl::PointXYZ PointT;
typedef pcl::PointNormal PointNT;
typedef pcl::Normal Normal;
typedef pcl::PointXYZRGB PointTC;

typedef pcl::PointCloud<PointT> PointCloudT;
typedef pcl::PointCloud<PointNT> PointCloudN;
typedef pcl::PointCloud<Normal> NormalCloud;
typedef pcl::PointCloud<PointTC> PointCloudC;

struct RGBColor{ float r, g, b; };
inline RGBColor getRGBColor(double ratio)
{
    //we want to normalize ratio so that it fits in to 6 regions
    //where each region is 256 units long
    int normalized = int(ratio * 255 * 5);

    //find the distance to the start of the closest region
    int x = normalized % 256;

    int red = 0, grn = 0, blu = 0;
    switch(normalized / 256)
    {
        case 0: red = 255;      grn = x;        blu = 0;       break;//red
        case 1: red = 255 - x;  grn = 255;      blu = 0;       break;//yellow
        case 2: red = 0;        grn = 255;      blu = x;       break;//green
        case 3: red = 0;        grn = 255 - x;  blu = 255;     break;//cyan
        case 4: red = x;        grn = 0;        blu = 255;     break;//blue
    }

    RGBColor out;
    out.r = (float)red/255.0;
    out.g = (float)grn/255.0;
    out.b = (float)blu/255.0;
    return out;
}

inline void pubMainCloud(ros::Publisher* pub, PointCloudT cloud, std::string fixed_frame_id, ros::Time stamp=ros::Time(0))
{
    sensor_msgs::PointCloud2 cloud_msg;
    pcl::toROSMsg(cloud, cloud_msg);
    cloud_msg.header.frame_id = fixed_frame_id;
    cloud_msg.header.stamp = stamp;
    pub->publish(cloud_msg);
    // ros::spinOnce();
}

// inline void pubCostmap(ros::Publisher* pub, std_msgs::Float64MultiArray map, float voxel_size, float std::string fixed_frame_id, ros::Time stamp=ros::Time(0))
// {
//     visualization_msgs::Marker cloud_msg;
//     cloud_msg.header.frame_id = fixed_frame_id;
//     cloud_msg.header.stamp = stamp;
//     cloud_msg.type = cloud_msg.POINTS;
//     cloud_msg.action = cloud_msg.ADD;
//     cloud_msg.scale.x = 0.005;
//     cloud_msg.scale.y = 0.005;
//     cloud_msg.scale.z = 0.005;
//     cloud_msg.lifetime = ros::Duration(0.0);
//     for(uint i=0; i<cloud.size(); i++)
//     {
//         PointTC pt = cloud.points[i];

//         geometry_msgs::Point pt_msg;
//         pt_msg.x = pt.x;
//         pt_msg.y = pt.y;
//         pt_msg.z = pt.z;
//         cloud_msg.points.push_back(pt_msg);

//         std_msgs::ColorRGBA rgb_msg;
//         rgb_msg.r = pt.r/255.0;
//         rgb_msg.g = pt.g/255.0;
//         rgb_msg.b = pt.b/255.0;
//         rgb_msg.a = 1.0;
//         cloud_msg.colors.push_back(rgb_msg);
//     }
//     pub->publish(cloud_msg);
//     ros::spinOnce();
// }

inline void pubNormalCloud(ros::Publisher* pub, PointCloudT tcloud, NormalCloud ncloud, std::string fixed_frame_id, ros::Time stamp=ros::Time(0))
{
    if (tcloud.size()!=ncloud.size())
    {
        ROS_ERROR("Invalid cloud inputs");
    }

    visualization_msgs::Marker cloud_msg;
    cloud_msg.header.frame_id = fixed_frame_id;
    cloud_msg.header.stamp = stamp;
    cloud_msg.type = cloud_msg.POINTS;
    cloud_msg.action = cloud_msg.ADD;
    cloud_msg.scale.x = 0.025;
    cloud_msg.scale.y = 0.025;
    cloud_msg.scale.z = 0.025;
    cloud_msg.lifetime = ros::Duration(0.0);
    for(uint i=0; i<tcloud.size(); i++)
    {

        PointT pt = tcloud.points[i];
        Normal nm = ncloud.points[i];

        geometry_msgs::Point pt_msg;
        pt_msg.x = pt.x;
        pt_msg.y = pt.y;
        pt_msg.z = pt.z;
        cloud_msg.points.push_back(pt_msg);

        std_msgs::ColorRGBA rgb_msg;
        // rgb_msg.r = nm.normal_z;
        // rgb_msg.g = nm.normal_x;
        // rgb_msg.b = nm.normal_y;
        Eigen::Vector3f upVec(0,0,1);
        Eigen::Vector3f nmVec(nm.normal_x, nm.normal_y, nm.normal_z);   
        RGBColor col = getRGBColor(pow(1-fabs(upVec.dot(nmVec)),3));
        rgb_msg.r = col.r;
        rgb_msg.g = col.g;
        rgb_msg.b = col.b;
        // rgb_msg.r = abs(nm.normal_x);
        // rgb_msg.g = abs(nm.normal_y);
        // rgb_msg.b = abs(nm.normal_z);
        // rgb_msg.r = ( (fabs(nm.normal_x)>fabs(nm.normal_y) && fabs(nm.normal_x)>fabs(nm.normal_z)) ? 1.0 : 0.0 );
        // rgb_msg.g = ( (fabs(nm.normal_y)>fabs(nm.normal_x) && fabs(nm.normal_y)>fabs(nm.normal_z)) ? 1.0 : 0.0 );
        // rgb_msg.b = ( (fabs(nm.normal_z)>fabs(nm.normal_x) && fabs(nm.normal_z)>fabs(nm.normal_y)) ? 1.0 : 0.0 );
        rgb_msg.a = 1.0;
        cloud_msg.colors.push_back(rgb_msg);
    }
    pub->publish(cloud_msg);
    // ros::spinOnce();
}

inline void pubCurvature(ros::Publisher* pub, PointCloudT tcloud, NormalCloud ncloud, std::string fixed_frame_id, ros::Time stamp=ros::Time(0))
{
    if (tcloud.size()!=ncloud.size())
    {
        ROS_ERROR("Invalid cloud inputs");
    }

    visualization_msgs::Marker cloud_msg;
    cloud_msg.header.frame_id = fixed_frame_id;
    cloud_msg.header.stamp = stamp;
    cloud_msg.type = cloud_msg.POINTS;
    cloud_msg.action = cloud_msg.ADD;
    cloud_msg.scale.x = 0.025;
    cloud_msg.scale.y = 0.025;
    cloud_msg.scale.z = 0.025;
    cloud_msg.lifetime = ros::Duration(0.0);
    for(uint i=0; i<tcloud.size(); i++)
    {

        PointT pt = tcloud.points[i];
        Normal nm = ncloud.points[i];

        geometry_msgs::Point pt_msg;
        pt_msg.x = pt.x;
        pt_msg.y = pt.y;
        pt_msg.z = pt.z;
        cloud_msg.points.push_back(pt_msg);

        std_msgs::ColorRGBA rgb_msg;
        // std::cout << std::to_string(nm.curvature)+" ";
        float curve_sat = 1.f;
        float curve_gain = 1/0.05*curve_sat;
        RGBColor color = getRGBColor(std::min(curve_sat,std::max(0.f,(float)fabs(nm.curvature*curve_gain))));
        rgb_msg.r = color.r;
        rgb_msg.g = color.g;
        rgb_msg.b = color.b;
        rgb_msg.a = 1.0;
        cloud_msg.colors.push_back(rgb_msg);
    }
    // std::cout<<std::endl;
    pub->publish(cloud_msg);
    // ros::spinOnce();
}

inline void pubRegions(ros::Publisher* pub, regions segments, std::string fixed_frame_id, ros::Time stamp=ros::Time(0))
{
    // auto pal = palettes.at("inferno").rescale(0.0, 1.0);
    visualization_msgs::Marker cloud_msg;
    cloud_msg.header.frame_id = fixed_frame_id;
    cloud_msg.header.stamp = stamp;
    cloud_msg.type = cloud_msg.POINTS;
    cloud_msg.action = cloud_msg.ADD;
    cloud_msg.scale.x = 0.05;
    cloud_msg.scale.y = 0.05;
    cloud_msg.scale.z = 0.05;
    cloud_msg.lifetime = ros::Duration(0.0);
    for(uint j=0; j<segments.size(); j++)
    {
        RGBColor color = getRGBColor((float)(j)/(float)(segments.size()));

        PointCloudT cloud = segments.at(j).segmentCloud;
        for(uint i=0; i<cloud.size(); i++)
        {
            PointT pt = cloud.points[i];
            geometry_msgs::Point pt_msg;
            pt_msg.x = pt.x;
            pt_msg.y = pt.y;
            pt_msg.z = pt.z;
            cloud_msg.points.push_back(pt_msg);

            std_msgs::ColorRGBA rgb_msg;
            rgb_msg.r = color.r;
            rgb_msg.g = color.g;
            rgb_msg.b = color.b;
            rgb_msg.a = 1.0;
            cloud_msg.colors.push_back(rgb_msg);
        }
    }
    pub->publish(cloud_msg);
    // ros::spinOnce();
}

// inline void getTransformFromTree(std::string parent_frame_id, std::string child_frame_id, Eigen::Transformd* tform)
inline bool getTransformFromTree(tf::TransformListener& listener, std::string parent_frame_id, std::string child_frame_id, Eigen::Matrix4d* tform_mat, ros::Time stamp=ros::Time(0))
{
    // ROS_INFO("Transforming cloud from %s to %s", child_frame_id.c_str(), parent_frame_id.c_str());
    // tf::TransformListener listener;
    tf::StampedTransform tform_msg;
    try
    {
        listener.waitForTransform(parent_frame_id, child_frame_id, ros::Time::now(), ros::Duration(1.0));
        listener.lookupTransform(parent_frame_id, child_frame_id, stamp, tform_msg);
    }
    catch (tf::TransformException ex)
    {
        ROS_ERROR("%s",ex.what());
        return false;
    }

    Eigen::Affine3d tform;
    tf::transformTFToEigen(tf::Transform(tform_msg),tform);
    (*tform_mat) = tform.matrix();
    return true;
}


struct CurvyTerrainMapperParams{
    std::string fixed_frame_id; // frame which input cloud is (transformed into and) processed, in this case it should be colocated with base_footprint and aligned with gravity
    // bool pub_viz; // doesnt work rn
    struct PreanalysisParams{
        bool dsFlag; // Set if downsample active
        double dsResolution; // Set downsample resolution
        int neNeighMethod; // Normal estimation - find N neareast neighbors (:=0) - find points within distance (very slow) (:=1)
        int neSearchNeighbours;
        double neSearchRadius;
        bool gpFlag; // Ghost point filter active?
        double gpAngle; // Ghost point filter angle in degress
        bool pfActive; // Point normal filter active?
        double pfAngle; // Point normal filter angle in degress
        bool fsActive; // Floor seperation active?
        double fsAngle; // Floor seperation angle in degrees
        double fsRange; // Floor seperation distance
        double rob_x, rob_y, rob_z; // Set the position of the LIDAR (required for floor separation)
        double robAngle; // Rotate pointcloud around z-axis
        bool dsMethod; // Downsample method - Standard: flase - Experimental version: true
        int neMethod; // Process ghost point filter and floor separation in separate steps
    } preanalysis;
    int segmentationmode; // 0 = Region Growing (recommended), 1 = Voxel SAC, 2 = Split & Merge
    struct RegionGrowingParams{
        bool enable; // Enable region growing
        int minClustSize; // Minimum cluster size
        int noNeigh; // Number of neighbors
        bool smoothFlag; // Smoothness flag (true: compare to seed point false: compare to neighboring point)
        double smoothThresh;  // Smoothness threshold
        bool resFlag; // Residual flag (true: compare to seed point false: compare to neighboring point)
        double resThresh;  // Residual distance
        bool curvFlag; // Curvature flag
        double curvThresh; // Curvature threshold (max)
        bool updateFlag; // Update seed point during growing
        bool pointUpdateFlag; //  Update pointwise
        int updateInterval; // If not pointwise, update every
    } regiongrowing;
    struct CostmapParams{
        double normal_gain;
        double curv_gain;
        double min_saturation_cost;
        double max_saturation_cost;
        bool set_min_saturation_cost_to_min_cost;
        bool set_max_saturation_cost_to_max_cost;
    } costmap;
};


#endif // ROS_FUNCTIONS_
