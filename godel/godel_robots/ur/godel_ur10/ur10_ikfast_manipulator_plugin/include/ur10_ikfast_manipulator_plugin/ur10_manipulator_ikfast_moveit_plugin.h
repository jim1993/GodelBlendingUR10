//
// Created by pieter on 11-12-17.
//

#ifndef PROJECT_UR10_MANIPULATOR_IKFAST_MOVEIT_PLUGIN_H
#define PROJECT_UR10_MANIPULATOR_IKFAST_MOVEIT_PLUGIN_H

#include <ros/ros.h>
#include <moveit/kinematics_base/kinematics_base.h>
#include <urdf/model.h>
#include <tf_conversions/tf_kdl.h>

// Need a floating point tolerance when checking joint limits, in case the joint starts at limit
const double LIMIT_TOLERANCE = .0000001;
/// \brief Search modes for searchPositionIK(), see there
enum SEARCH_MODE
{
    OPTIMIZE_FREE_JOINT = 1,
    OPTIMIZE_MAX_JOINT = 2
};

namespace ur10_ikfast_manipulator_plugin {
#define IKFAST_NO_MAIN  // Don't include main() from IKFast

/// \brief The types of inverse kinematics parameterizations supported.
///
/// The minimum degree of freedoms required is set in the upper 4 bits of each type.
/// The number of values used to represent the parameterization ( >= dof ) is the next 4 bits.
/// The lower bits contain a unique id of the type.
    enum IkParameterizationType {
        IKP_None = 0,
        IKP_Transform6D = 0x67000001,    ///< end effector reaches desired 6D transformation
        IKP_Rotation3D = 0x34000002,     ///< end effector reaches desired 3D rotation
        IKP_Translation3D = 0x33000003,  ///< end effector origin reaches desired 3D translation
        IKP_Direction3D = 0x23000004,    ///< direction on end effector coordinate system reaches desired direction
        IKP_Ray4D = 0x46000005,          ///< ray on end effector coordinate system reaches desired global ray
        IKP_Lookat3D = 0x23000006,       ///< direction on end effector coordinate system points to desired 3D position
        IKP_TranslationDirection5D = 0x56000007,  ///< end effector origin and direction reaches desired 3D translation and
        /// direction. Can be thought of as Ray IK where the origin of the ray must
        /// coincide.
                IKP_TranslationXY2D = 0x22000008,             ///< 2D translation along XY plane
        IKP_TranslationXYOrientation3D = 0x33000009,  ///< 2D translation along XY plane and 1D rotation around Z axis. The
        /// offset of the rotation is measured starting at +X, so at +X is it 0,
        /// at +Y it is pi/2.
                IKP_TranslationLocalGlobal6D = 0x3600000a,  ///< local point on end effector origin reaches desired 3D global point

        IKP_TranslationXAxisAngle4D = 0x4400000b,  ///< end effector origin reaches desired 3D translation, manipulator
        /// direction makes a specific angle with x-axis  like a cone, angle is from
        /// 0-pi. Axes defined in the manipulator base link's coordinate system)
                IKP_TranslationYAxisAngle4D = 0x4400000c,  ///< end effector origin reaches desired 3D translation, manipulator
        /// direction makes a specific angle with y-axis  like a cone, angle is from
        /// 0-pi. Axes defined in the manipulator base link's coordinate system)
                IKP_TranslationZAxisAngle4D = 0x4400000d,  ///< end effector origin reaches desired 3D translation, manipulator
        /// direction makes a specific angle with z-axis like a cone, angle is from
        /// 0-pi. Axes are defined in the manipulator base link's coordinate system.

        IKP_TranslationXAxisAngleZNorm4D = 0x4400000e,  ///< end effector origin reaches desired 3D translation, manipulator
        /// direction needs to be orthogonal to z-axis and be rotated at a
        /// certain angle starting from the x-axis (defined in the manipulator
        /// base link's coordinate system)
                IKP_TranslationYAxisAngleXNorm4D = 0x4400000f,  ///< end effector origin reaches desired 3D translation, manipulator
        /// direction needs to be orthogonal to x-axis and be rotated at a
        /// certain angle starting from the y-axis (defined in the manipulator
        /// base link's coordinate system)
                IKP_TranslationZAxisAngleYNorm4D = 0x44000010,  ///< end effector origin reaches desired 3D translation, manipulator
        /// direction needs to be orthogonal to y-axis and be rotated at a
        /// certain angle starting from the z-axis (defined in the manipulator
        /// base link's coordinate system)

        IKP_NumberOfParameterizations = 16,  ///< number of parameterizations (does not count IKP_None)

        IKP_VelocityDataBit =
        0x00008000,  ///< bit is set if the data represents the time-derivate velocity of an IkParameterization
        IKP_Transform6DVelocity = IKP_Transform6D | IKP_VelocityDataBit,
        IKP_Rotation3DVelocity = IKP_Rotation3D | IKP_VelocityDataBit,
        IKP_Translation3DVelocity = IKP_Translation3D | IKP_VelocityDataBit,
        IKP_Direction3DVelocity = IKP_Direction3D | IKP_VelocityDataBit,
        IKP_Ray4DVelocity = IKP_Ray4D | IKP_VelocityDataBit,
        IKP_Lookat3DVelocity = IKP_Lookat3D | IKP_VelocityDataBit,
        IKP_TranslationDirection5DVelocity = IKP_TranslationDirection5D | IKP_VelocityDataBit,
        IKP_TranslationXY2DVelocity = IKP_TranslationXY2D | IKP_VelocityDataBit,
        IKP_TranslationXYOrientation3DVelocity = IKP_TranslationXYOrientation3D | IKP_VelocityDataBit,
        IKP_TranslationLocalGlobal6DVelocity = IKP_TranslationLocalGlobal6D | IKP_VelocityDataBit,
        IKP_TranslationXAxisAngle4DVelocity = IKP_TranslationXAxisAngle4D | IKP_VelocityDataBit,
        IKP_TranslationYAxisAngle4DVelocity = IKP_TranslationYAxisAngle4D | IKP_VelocityDataBit,
        IKP_TranslationZAxisAngle4DVelocity = IKP_TranslationZAxisAngle4D | IKP_VelocityDataBit,
        IKP_TranslationXAxisAngleZNorm4DVelocity = IKP_TranslationXAxisAngleZNorm4D | IKP_VelocityDataBit,
        IKP_TranslationYAxisAngleXNorm4DVelocity = IKP_TranslationYAxisAngleXNorm4D | IKP_VelocityDataBit,
        IKP_TranslationZAxisAngleYNorm4DVelocity = IKP_TranslationZAxisAngleYNorm4D | IKP_VelocityDataBit,

        IKP_UniqueIdMask = 0x0000ffff,   ///< the mask for the unique ids
        IKP_CustomDataBit = 0x00010000,  ///< bit is set if the ikparameterization contains custom data, this is only used
        /// when serializing the ik parameterizations
    };

// Code generated by IKFast56/61
#include <ur10_ikfast_manipulator_plugin/ur10_manipulator_ikfast_solver.h>

    class IKFastKinematicsPlugin : public kinematics::KinematicsBase {

    protected:
        std::vector <std::string> joint_names_;
        std::vector<double> joint_min_vector_;
        std::vector<double> joint_max_vector_;
        std::vector<bool> joint_has_limits_vector_;
        std::vector <std::string> link_names_;
        size_t num_joints_;
        std::vector<int> free_params_;
        bool active_;  // Internal variable that indicates whether solvers are configured and ready

        const std::vector <std::string> &getJointNames() const {
            return joint_names_;
        }

        const std::vector <std::string> &getLinkNames() const {
            return link_names_;
        }

    public:
        /** @class
         *  @brief Interface for an IKFast kinematics plugin
         */
        IKFastKinematicsPlugin() : active_(false) {
            srand(time(NULL));
            supported_methods_.push_back(kinematics::DiscretizationMethods::NO_DISCRETIZATION);
            supported_methods_.push_back(kinematics::DiscretizationMethods::ALL_DISCRETIZED);
            supported_methods_.push_back(kinematics::DiscretizationMethods::ALL_RANDOM_SAMPLED);
        }

        /**
         * @brief Given a desired pose of the end-effector, compute the joint angles to reach it
         * @param ik_pose the desired pose of the link
         * @param ik_seed_state an initial guess solution for the inverse kinematics
         * @param solution the solution vector
         * @param error_code an error code that encodes the reason for failure or success
         * @return True if a valid solution was found, false otherwise
         */

        // Returns the IK solution that is within joint limits closest to ik_seed_state
        bool getPositionIK(const geometry_msgs::Pose &ik_pose, const std::vector<double> &ik_seed_state,
                           std::vector<double> &solution, moveit_msgs::MoveItErrorCodes &error_code,
                           const kinematics::KinematicsQueryOptions &options = kinematics::KinematicsQueryOptions()) const;

        /**
         * @brief Given a desired pose of the end-effector, compute the set joint angles solutions that are able to reach it.
         *
         * This is a default implementation that returns only one solution and so its result is equivalent to calling
         * 'getPositionIK(...)' with a zero initialized seed.
         *
         * @param ik_poses  The desired pose of each tip link
         * @param ik_seed_state an initial guess solution for the inverse kinematics
         * @param solutions A vector of vectors where each entry is a valid joint solution
         * @param result A struct that reports the results of the query
         * @param options An option struct which contains the type of redundancy discretization used. This default
         *                implementation only supports the KinmaticSearches::NO_DISCRETIZATION method; requesting any
         *                other will result in failure.
         * @return True if a valid set of solutions was found, false otherwise.
         */
        bool getPositionIK(const std::vector <geometry_msgs::Pose> &ik_poses, const std::vector<double> &ik_seed_state,
                           std::vector <std::vector<double>> &solutions, kinematics::KinematicsResult &result,
                           const kinematics::KinematicsQueryOptions &options) const;

        /**
         * @brief Given a desired pose of the end-effector, search for the joint angles required to reach it.
         * This particular method is intended for "searching" for a solutions by stepping through the redundancy
         * (or other numerical routines).
         * @param ik_pose the desired pose of the link
         * @param ik_seed_state an initial guess solution for the inverse kinematics
         * @return True if a valid solution was found, false otherwise
         */
        bool
        searchPositionIK(const geometry_msgs::Pose &ik_pose, const std::vector<double> &ik_seed_state, double timeout,
                         std::vector<double> &solution, moveit_msgs::MoveItErrorCodes &error_code,
                         const kinematics::KinematicsQueryOptions &options = kinematics::KinematicsQueryOptions()) const;

        /**
         * @brief Given a desired pose of the end-effector, search for the joint angles required to reach it.
         * This particular method is intended for "searching" for a solutions by stepping through the redundancy
         * (or other numerical routines).
         * @param ik_pose the desired pose of the link
         * @param ik_seed_state an initial guess solution for the inverse kinematics
         * @param the distance that the redundancy can be from the current position
         * @return True if a valid solution was found, false otherwise
         */
        bool
        searchPositionIK(const geometry_msgs::Pose &ik_pose, const std::vector<double> &ik_seed_state, double timeout,
                         const std::vector<double> &consistency_limits, std::vector<double> &solution,
                         moveit_msgs::MoveItErrorCodes &error_code,
                         const kinematics::KinematicsQueryOptions &options = kinematics::KinematicsQueryOptions()) const;

        /**
         * @brief Given a desired pose of the end-effector, search for the joint angles required to reach it.
         * This particular method is intended for "searching" for a solutions by stepping through the redundancy
         * (or other numerical routines).
         * @param ik_pose the desired pose of the link
         * @param ik_seed_state an initial guess solution for the inverse kinematics
         * @return True if a valid solution was found, false otherwise
         */
        bool
        searchPositionIK(const geometry_msgs::Pose &ik_pose, const std::vector<double> &ik_seed_state, double timeout,
                         std::vector<double> &solution, const IKCallbackFn &solution_callback,
                         moveit_msgs::MoveItErrorCodes &error_code,
                         const kinematics::KinematicsQueryOptions &options = kinematics::KinematicsQueryOptions()) const;

        /**
         * @brief Given a desired pose of the end-effector, search for the joint angles required to reach it.
         * This particular method is intended for "searching" for a solutions by stepping through the redundancy
         * (or other numerical routines).  The consistency_limit specifies that only certain redundancy positions
         * around those specified in the seed state are admissible and need to be searched.
         * @param ik_pose the desired pose of the link
         * @param ik_seed_state an initial guess solution for the inverse kinematics
         * @param consistency_limit the distance that the redundancy can be from the current position
         * @return True if a valid solution was found, false otherwise
         */
        bool
        searchPositionIK(const geometry_msgs::Pose &ik_pose, const std::vector<double> &ik_seed_state, double timeout,
                         const std::vector<double> &consistency_limits, std::vector<double> &solution,
                         const IKCallbackFn &solution_callback, moveit_msgs::MoveItErrorCodes &error_code,
                         const kinematics::KinematicsQueryOptions &options = kinematics::KinematicsQueryOptions()) const;

        /**
         * @brief Given a set of joint angles and a set of links, compute their pose
         *
         * @param link_names A set of links for which FK needs to be computed
         * @param joint_angles The state for which FK is being computed
         * @param poses The resultant set of poses (in the frame returned by getBaseFrame())
         * @return True if a valid solution was found, false otherwise
         */
        bool getPositionFK(const std::vector <std::string> &link_names, const std::vector<double> &joint_angles,
                           std::vector <geometry_msgs::Pose> &poses) const;

        /**
         * @brief Sets the discretization value for the redundant joint.
         *
         * Since this ikfast implementation allows for one redundant joint then only the first entry will be in the
         *discretization map will be used.
         * Calling this method replaces previous discretization settings.
         *
         * @param discretization a map of joint indices and discretization value pairs.
         */
        void setSearchDiscretization(const std::map<int, double> &discretization);

        /**
         * @brief Overrides the default method to prevent changing the redundant joints
         */
        bool setRedundantJoints(const std::vector<unsigned int> &redundant_joint_indices);

    protected:
        bool
        initialize(const std::string &robot_description, const std::string &group_name, const std::string &base_name,
                   const std::string &tip_name, double search_discretization);

        /**
         * @brief Calls the IK solver from IKFast
         * @return The number of solutions found
         */
        int solve(KDL::Frame &pose_frame, const std::vector<double> &vfree, IkSolutionList <IkReal> &solutions) const;

        /**
         * @brief Gets a specific solution from the set
         */
        void getSolution(const IkSolutionList <IkReal> &solutions, int i, std::vector<double> &solution) const;

        double harmonize(const std::vector<double> &ik_seed_state, std::vector<double> &solution) const;

        // void getOrderedSolutions(const std::vector<double> &ik_seed_state, std::vector<std::vector<double> >& solslist);
        void getClosestSolution(const IkSolutionList <IkReal> &solutions, const std::vector<double> &ik_seed_state,
                                std::vector<double> &solution) const;

        void fillFreeParams(int count, int *array);

        bool getCount(int &count, const int &max_count, const int &min_count) const;

        /**
        * @brief samples the designated redundant joint using the chosen discretization method
        * @param  method              An enumeration flag indicating the discretization method to be used
        * @param  sampled_joint_vals  Sampled joint values for the redundant joint
        * @return True if sampling succeeded.
        */
        bool
        sampleRedundantJoint(kinematics::DiscretizationMethod method, std::vector<double> &sampled_joint_vals) const;

    };  // end class

    bool IKFastKinematicsPlugin::initialize(const std::string& robot_description, const std::string& group_name,
                                            const std::string& base_name, const std::string& tip_name,
                                            double search_discretization)
    {
        setValues(robot_description, group_name, base_name, tip_name, search_discretization);

        ros::NodeHandle node_handle("~/" + group_name);

        std::string robot;
        node_handle.param("robot", robot, std::string());

        // IKFast56/61
        fillFreeParams(GetNumFreeParameters(), GetFreeParameters());
        num_joints_ = GetNumJoints();

        if (free_params_.size() > 1)
        {
            ROS_FATAL("Only one free joint parameter supported!");
            return false;
        }
        else if (free_params_.size() == 1)
        {
            redundant_joint_indices_.clear();
            redundant_joint_indices_.push_back(free_params_[0]);
            KinematicsBase::setSearchDiscretization(DEFAULT_SEARCH_DISCRETIZATION);
        }

        urdf::Model robot_model;
        std::string xml_string;

        std::string urdf_xml, full_urdf_xml;
        node_handle.param("urdf_xml", urdf_xml, robot_description);
        node_handle.searchParam(urdf_xml, full_urdf_xml);

        ROS_DEBUG_NAMED("ikfast", "Reading xml file from parameter server");
        if (!node_handle.getParam(full_urdf_xml, xml_string))
        {
            ROS_FATAL_NAMED("ikfast", "Could not load the xml from parameter server: %s", urdf_xml.c_str());
            return false;
        }

        node_handle.param(full_urdf_xml, xml_string, std::string());
        robot_model.initString(xml_string);

        ROS_DEBUG_STREAM_NAMED("ikfast", "Reading joints and links from URDF");

        urdf::LinkConstSharedPtr link = robot_model.getLink(getTipFrame());
        while (link->name != base_frame_ && joint_names_.size() <= num_joints_)
        {
            ROS_DEBUG_NAMED("ikfast", "Link %s", link->name.c_str());
            link_names_.push_back(link->name);
            urdf::JointSharedPtr joint = link->parent_joint;
            if (joint)
            {
                if (joint->type != urdf::Joint::UNKNOWN && joint->type != urdf::Joint::FIXED)
                {
                    ROS_DEBUG_STREAM_NAMED("ikfast", "Adding joint " << joint->name);

                    joint_names_.push_back(joint->name);
                    float lower, upper;
                    int hasLimits;
                    if (joint->type != urdf::Joint::CONTINUOUS)
                    {
                        if (joint->safety)
                        {
                            lower = joint->safety->soft_lower_limit;
                            upper = joint->safety->soft_upper_limit;
                        }
                        else
                        {
                            lower = joint->limits->lower;
                            upper = joint->limits->upper;
                        }
                        hasLimits = 1;
                    }
                    else
                    {
                        lower = -M_PI;
                        upper = M_PI;
                        hasLimits = 0;
                    }
                    if (hasLimits)
                    {
                        joint_has_limits_vector_.push_back(true);
                        joint_min_vector_.push_back(lower);
                        joint_max_vector_.push_back(upper);
                    }
                    else
                    {
                        joint_has_limits_vector_.push_back(false);
                        joint_min_vector_.push_back(-M_PI);
                        joint_max_vector_.push_back(M_PI);
                    }
                }
            }
            else
            {
                ROS_WARN_NAMED("ikfast", "no joint corresponding to %s", link->name.c_str());
            }
            link = link->getParent();
        }

        if (joint_names_.size() != num_joints_)
        {
            ROS_FATAL_STREAM_NAMED("ikfast", "Joint numbers mismatch: URDF has " << joint_names_.size() << " and IKFast has "
                                                                                 << num_joints_);
            return false;
        }

        std::reverse(link_names_.begin(), link_names_.end());
        std::reverse(joint_names_.begin(), joint_names_.end());
        std::reverse(joint_min_vector_.begin(), joint_min_vector_.end());
        std::reverse(joint_max_vector_.begin(), joint_max_vector_.end());
        std::reverse(joint_has_limits_vector_.begin(), joint_has_limits_vector_.end());

        for (size_t i = 0; i < num_joints_; ++i)
            ROS_DEBUG_STREAM_NAMED("ikfast", joint_names_[i] << " " << joint_min_vector_[i] << " " << joint_max_vector_[i]
                                                             << " " << joint_has_limits_vector_[i]);

        active_ = true;
        return true;
    }

    void IKFastKinematicsPlugin::setSearchDiscretization(const std::map<int, double>& discretization)
    {
        if (discretization.empty())
        {
            ROS_ERROR("The 'discretization' map is empty");
            return;
        }

        if (redundant_joint_indices_.empty())
        {
            ROS_ERROR_STREAM("This group's solver doesn't support redundant joints");
            return;
        }

        if (discretization.begin()->first != redundant_joint_indices_[0])
        {
            std::string redundant_joint = joint_names_[free_params_[0]];
            ROS_ERROR_STREAM("Attempted to discretize a non-redundant joint "
                                     << discretization.begin()->first << ", only joint '" << redundant_joint << "' with index "
                                     << redundant_joint_indices_[0] << " is redundant.");
            return;
        }

        if (discretization.begin()->second <= 0.0)
        {
            ROS_ERROR_STREAM("Discretization can not takes values that are <= 0");
            return;
        }

        redundant_joint_discretization_.clear();
        redundant_joint_discretization_[redundant_joint_indices_[0]] = discretization.begin()->second;
    }

    bool IKFastKinematicsPlugin::setRedundantJoints(const std::vector<unsigned int>& redundant_joint_indices)
    {
        ROS_ERROR_STREAM("Changing the redundant joints isn't permitted by this group's solver ");
        return false;
    }

    int IKFastKinematicsPlugin::solve(KDL::Frame& pose_frame, const std::vector<double>& vfree,
                                      IkSolutionList<IkReal>& solutions) const
    {
        // IKFast56/61
        solutions.Clear();

        double trans[3];
        trans[0] = pose_frame.p[0];  //-.18;
        trans[1] = pose_frame.p[1];
        trans[2] = pose_frame.p[2];

        KDL::Rotation mult;
        KDL::Vector direction;

        switch (GetIkType())
        {
            case IKP_Transform6D:
            case IKP_Translation3D:
                // For **Transform6D**, eerot is 9 values for the 3x3 rotation matrix. For **Translation3D**, these are ignored.

                mult = pose_frame.M;

                double vals[9];
                vals[0] = mult(0, 0);
                vals[1] = mult(0, 1);
                vals[2] = mult(0, 2);
                vals[3] = mult(1, 0);
                vals[4] = mult(1, 1);
                vals[5] = mult(1, 2);
                vals[6] = mult(2, 0);
                vals[7] = mult(2, 1);
                vals[8] = mult(2, 2);

                // IKFast56/61
                ComputeIk(trans, vals, vfree.size() > 0 ? &vfree[0] : NULL, solutions);
                return solutions.GetNumSolutions();

            case IKP_Direction3D:
            case IKP_Ray4D:
            case IKP_TranslationDirection5D:
                // For **Direction3D**, **Ray4D**, and **TranslationDirection5D**, the first 3 values represent the target
                // direction.

                direction = pose_frame.M * KDL::Vector(0, 0, 1);
                ComputeIk(trans, direction.data, vfree.size() > 0 ? &vfree[0] : NULL, solutions);
                return solutions.GetNumSolutions();

            case IKP_TranslationXAxisAngle4D:
            case IKP_TranslationYAxisAngle4D:
            case IKP_TranslationZAxisAngle4D:
                // For **TranslationXAxisAngle4D**, **TranslationYAxisAngle4D**, and **TranslationZAxisAngle4D**, the first value
                // represents the angle.
                ROS_ERROR_NAMED("ikfast", "IK for this IkParameterizationType not implemented yet.");
                return 0;

            case IKP_TranslationLocalGlobal6D:
                // For **TranslationLocalGlobal6D**, the diagonal elements ([0],[4],[8]) are the local translation inside the end
                // effector coordinate system.
                ROS_ERROR_NAMED("ikfast", "IK for this IkParameterizationType not implemented yet.");
                return 0;

            case IKP_Rotation3D:
            case IKP_Lookat3D:
            case IKP_TranslationXY2D:
            case IKP_TranslationXYOrientation3D:
            case IKP_TranslationXAxisAngleZNorm4D:
            case IKP_TranslationYAxisAngleXNorm4D:
            case IKP_TranslationZAxisAngleYNorm4D:
                ROS_ERROR_NAMED("ikfast", "IK for this IkParameterizationType not implemented yet.");
                return 0;

            default:
                ROS_ERROR_NAMED("ikfast", "Unknown IkParameterizationType! Was the solver generated with an incompatible version "
                        "of Openrave?");
                return 0;
        }
    }

    void IKFastKinematicsPlugin::getSolution(const IkSolutionList<IkReal>& solutions, int i,
                                             std::vector<double>& solution) const
    {
        solution.clear();
        solution.resize(num_joints_);

        // IKFast56/61
        const IkSolutionBase<IkReal>& sol = solutions.GetSolution(i);
        std::vector<IkReal> vsolfree(sol.GetFree().size());
        sol.GetSolution(&solution[0], vsolfree.size() > 0 ? &vsolfree[0] : NULL);

        // std::cout << "solution " << i << ":" ;
        // for(int j=0;j<num_joints_; ++j)
        //   std::cout << " " << solution[j];
        // std::cout << std::endl;

        // ROS_ERROR("%f %d",solution[2],vsolfree.size());
    }

    double IKFastKinematicsPlugin::harmonize(const std::vector<double>& ik_seed_state, std::vector<double>& solution) const
    {
        double dist_sqr = 0;
        std::vector<double> ss = ik_seed_state;
        for (size_t i = 0; i < ik_seed_state.size(); ++i)
        {
            while (ss[i] > 2 * M_PI)
            {
                ss[i] -= 2 * M_PI;
            }
            while (ss[i] < 2 * M_PI)
            {
                ss[i] += 2 * M_PI;
            }
            while (solution[i] > 2 * M_PI)
            {
                solution[i] -= 2 * M_PI;
            }
            while (solution[i] < 2 * M_PI)
            {
                solution[i] += 2 * M_PI;
            }
            dist_sqr += fabs(ik_seed_state[i] - solution[i]);
        }
        return dist_sqr;
    }

// void IKFastKinematicsPlugin::getOrderedSolutions(const std::vector<double> &ik_seed_state,
//                                  std::vector<std::vector<double> >& solslist)
// {
//   std::vector<double>
//   double mindist = 0;
//   int minindex = -1;
//   std::vector<double> sol;
//   for(size_t i=0;i<solslist.size();++i){
//     getSolution(i,sol);
//     double dist = harmonize(ik_seed_state, sol);
//     //std::cout << "dist[" << i << "]= " << dist << std::endl;
//     if(minindex == -1 || dist<mindist){
//       minindex = i;
//       mindist = dist;
//     }
//   }
//   if(minindex >= 0){
//     getSolution(minindex,solution);
//     harmonize(ik_seed_state, solution);
//     index = minindex;
//   }
// }

    void IKFastKinematicsPlugin::getClosestSolution(const IkSolutionList<IkReal>& solutions,
                                                    const std::vector<double>& ik_seed_state,
                                                    std::vector<double>& solution) const
    {
        double mindist = DBL_MAX;
        int minindex = -1;
        std::vector<double> sol;

        // IKFast56/61
        for (size_t i = 0; i < solutions.GetNumSolutions(); ++i)
        {
            getSolution(solutions, i, sol);
            double dist = harmonize(ik_seed_state, sol);
            ROS_INFO_STREAM_NAMED("ikfast", "Dist " << i << " dist " << dist);
            // std::cout << "dist[" << i << "]= " << dist << std::endl;
            if (minindex == -1 || dist < mindist)
            {
                minindex = i;
                mindist = dist;
            }
        }
        if (minindex >= 0)
        {
            getSolution(solutions, minindex, solution);
            harmonize(ik_seed_state, solution);
        }
    }

    void IKFastKinematicsPlugin::fillFreeParams(int count, int* array)
    {
        free_params_.clear();
        for (int i = 0; i < count; ++i)
            free_params_.push_back(array[i]);
    }

    bool IKFastKinematicsPlugin::getCount(int& count, const int& max_count, const int& min_count) const
    {
        if (count > 0)
        {
            if (-count >= min_count)
            {
                count = -count;
                return true;
            }
            else if (count + 1 <= max_count)
            {
                count = count + 1;
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (1 - count <= max_count)
            {
                count = 1 - count;
                return true;
            }
            else if (count - 1 >= min_count)
            {
                count = count - 1;
                return true;
            }
            else
                return false;
        }
    }

    bool IKFastKinematicsPlugin::getPositionFK(const std::vector<std::string>& link_names,
                                               const std::vector<double>& joint_angles,
                                               std::vector<geometry_msgs::Pose>& poses) const
    {
        if (GetIkType() != IKP_Transform6D)
        {
            // ComputeFk() is the inverse function of ComputeIk(), so the format of
            // eerot differs depending on IK type. The Transform6D IK type is the only
            // one for which a 3x3 rotation matrix is returned, which means we can only
            // compute FK for that IK type.
            ROS_ERROR_NAMED("ikfast", "Can only compute FK for Transform6D IK type!");
            return false;
        }

        KDL::Frame p_out;
        if (link_names.size() == 0)
        {
            ROS_WARN_STREAM_NAMED("ikfast", "Link names with nothing");
            return false;
        }

        if (link_names.size() != 1 || link_names[0] != getTipFrame())
        {
            ROS_ERROR_NAMED("ikfast", "Can compute FK for %s only", getTipFrame().c_str());
            return false;
        }

        bool valid = true;

        IkReal eerot[9], eetrans[3];
        IkReal angles[joint_angles.size()];
        for (unsigned char i = 0; i < joint_angles.size(); i++)
            angles[i] = joint_angles[i];

        // IKFast56/61
        ComputeFk(angles, eetrans, eerot);

        for (int i = 0; i < 3; ++i)
            p_out.p.data[i] = eetrans[i];

        for (int i = 0; i < 9; ++i)
            p_out.M.data[i] = eerot[i];

        poses.resize(1);
        tf::poseKDLToMsg(p_out, poses[0]);

        return valid;
    }

    bool IKFastKinematicsPlugin::searchPositionIK(const geometry_msgs::Pose& ik_pose,
                                                  const std::vector<double>& ik_seed_state, double timeout,
                                                  std::vector<double>& solution, moveit_msgs::MoveItErrorCodes& error_code,
                                                  const kinematics::KinematicsQueryOptions& options) const
    {
        const IKCallbackFn solution_callback = 0;
        std::vector<double> consistency_limits;

        return searchPositionIK(ik_pose, ik_seed_state, timeout, consistency_limits, solution, solution_callback, error_code,
                                options);
    }

    bool IKFastKinematicsPlugin::searchPositionIK(const geometry_msgs::Pose& ik_pose,
                                                  const std::vector<double>& ik_seed_state, double timeout,
                                                  const std::vector<double>& consistency_limits,
                                                  std::vector<double>& solution, moveit_msgs::MoveItErrorCodes& error_code,
                                                  const kinematics::KinematicsQueryOptions& options) const
    {
        const IKCallbackFn solution_callback = 0;
        return searchPositionIK(ik_pose, ik_seed_state, timeout, consistency_limits, solution, solution_callback, error_code,
                                options);
    }

    bool IKFastKinematicsPlugin::searchPositionIK(const geometry_msgs::Pose& ik_pose,
                                                  const std::vector<double>& ik_seed_state, double timeout,
                                                  std::vector<double>& solution, const IKCallbackFn& solution_callback,
                                                  moveit_msgs::MoveItErrorCodes& error_code,
                                                  const kinematics::KinematicsQueryOptions& options) const
    {
        std::vector<double> consistency_limits;
        return searchPositionIK(ik_pose, ik_seed_state, timeout, consistency_limits, solution, solution_callback, error_code,
                                options);
    }

    bool IKFastKinematicsPlugin::searchPositionIK(const geometry_msgs::Pose& ik_pose,
                                                  const std::vector<double>& ik_seed_state, double timeout,
                                                  const std::vector<double>& consistency_limits,
                                                  std::vector<double>& solution, const IKCallbackFn& solution_callback,
                                                  moveit_msgs::MoveItErrorCodes& error_code,
                                                  const kinematics::KinematicsQueryOptions& options) const
    {
        ROS_DEBUG_STREAM_NAMED("ikfast", "searchPositionIK");

        /// search_mode is currently fixed during code generation
        SEARCH_MODE search_mode = OPTIMIZE_MAX_JOINT;

        // Check if there are no redundant joints
        if (free_params_.size() == 0)
        {
            ROS_DEBUG_STREAM_NAMED("ikfast", "No need to search since no free params/redundant joints");

            // Find first IK solution, within joint limits
            if (!getPositionIK(ik_pose, ik_seed_state, solution, error_code))
            {
                ROS_DEBUG_STREAM_NAMED("ikfast", "No solution whatsoever");
                error_code.val = moveit_msgs::MoveItErrorCodes::NO_IK_SOLUTION;
                return false;
            }

            // check for collisions if a callback is provided
            if (!solution_callback.empty())
            {
                solution_callback(ik_pose, solution, error_code);
                if (error_code.val == moveit_msgs::MoveItErrorCodes::SUCCESS)
                {
                    ROS_DEBUG_STREAM_NAMED("ikfast", "Solution passes callback");
                    return true;
                }
                else
                {
                    ROS_DEBUG_STREAM_NAMED("ikfast", "Solution has error code " << error_code);
                    return false;
                }
            }
            else
            {
                return true;  // no collision check callback provided
            }
        }

        // -------------------------------------------------------------------------------------------------
        // Error Checking
        if (!active_)
        {
            ROS_ERROR_STREAM_NAMED("ikfast", "Kinematics not active");
            error_code.val = error_code.NO_IK_SOLUTION;
            return false;
        }

        if (ik_seed_state.size() != num_joints_)
        {
            ROS_ERROR_STREAM_NAMED("ikfast", "Seed state must have size " << num_joints_ << " instead of size "
                                                                          << ik_seed_state.size());
            error_code.val = error_code.NO_IK_SOLUTION;
            return false;
        }

        if (!consistency_limits.empty() && consistency_limits.size() != num_joints_)
        {
            ROS_ERROR_STREAM_NAMED("ikfast", "Consistency limits be empty or must have size "
                    << num_joints_ << " instead of size " << consistency_limits.size());
            error_code.val = error_code.NO_IK_SOLUTION;
            return false;
        }

        // -------------------------------------------------------------------------------------------------
        // Initialize

        KDL::Frame frame;
        tf::poseMsgToKDL(ik_pose, frame);

        std::vector<double> vfree(free_params_.size());

        ros::Time maxTime = ros::Time::now() + ros::Duration(timeout);
        int counter = 0;

        double initial_guess = ik_seed_state[free_params_[0]];
        vfree[0] = initial_guess;

        // -------------------------------------------------------------------------------------------------
        // Handle consitency limits if needed
        int num_positive_increments;
        int num_negative_increments;

        if (!consistency_limits.empty())
        {
            // moveit replaced consistency_limit (scalar) w/ consistency_limits (vector)
            // Assume [0]th free_params element for now.  Probably wrong.
            double max_limit = fmin(joint_max_vector_[free_params_[0]], initial_guess + consistency_limits[free_params_[0]]);
            double min_limit = fmax(joint_min_vector_[free_params_[0]], initial_guess - consistency_limits[free_params_[0]]);

            num_positive_increments = (int)((max_limit - initial_guess) / search_discretization_);
            num_negative_increments = (int)((initial_guess - min_limit) / search_discretization_);
        }
        else  // no consitency limits provided
        {
            num_positive_increments = (joint_max_vector_[free_params_[0]] - initial_guess) / search_discretization_;
            num_negative_increments = (initial_guess - joint_min_vector_[free_params_[0]]) / search_discretization_;
        }

        // -------------------------------------------------------------------------------------------------
        // Begin searching

        ROS_DEBUG_STREAM_NAMED("ikfast", "Free param is " << free_params_[0] << " initial guess is " << initial_guess
                                                          << ", # positive increments: " << num_positive_increments
                                                          << ", # negative increments: " << num_negative_increments);
        if ((search_mode & OPTIMIZE_MAX_JOINT) && (num_positive_increments + num_negative_increments) > 1000)
            ROS_WARN_STREAM_ONCE_NAMED("ikfast", "Large search space, consider increasing the search discretization");

        double best_costs = -1.0;
        std::vector<double> best_solution;
        int nattempts = 0, nvalid = 0;

        while (true)
        {
            IkSolutionList<IkReal> solutions;
            int numsol = solve(frame, vfree, solutions);

            ROS_DEBUG_STREAM_NAMED("ikfast", "Found " << numsol << " solutions from IKFast");

            // ROS_INFO("%f",vfree[0]);

            if (numsol > 0)
            {
                for (int s = 0; s < numsol; ++s)
                {
                    nattempts++;
                    std::vector<double> sol;
                    getSolution(solutions, s, sol);

                    bool obeys_limits = true;
                    for (unsigned int i = 0; i < sol.size(); i++)
                    {
                        if (joint_has_limits_vector_[i] && (sol[i] < joint_min_vector_[i] || sol[i] > joint_max_vector_[i]))
                        {
                            obeys_limits = false;
                            break;
                        }
                        // ROS_INFO_STREAM_NAMED("ikfast","Num " << i << " value " << sol[i] << " has limits " <<
                        // joint_has_limits_vector_[i] << " " << joint_min_vector_[i] << " " << joint_max_vector_[i]);
                    }
                    if (obeys_limits)
                    {
                        getSolution(solutions, s, solution);

                        // This solution is within joint limits, now check if in collision (if callback provided)
                        if (!solution_callback.empty())
                        {
                            solution_callback(ik_pose, solution, error_code);
                        }
                        else
                        {
                            error_code.val = error_code.SUCCESS;
                        }

                        if (error_code.val == error_code.SUCCESS)
                        {
                            nvalid++;
                            if (search_mode & OPTIMIZE_MAX_JOINT)
                            {
                                // Costs for solution: Largest joint motion
                                double costs = 0.0;
                                for (unsigned int i = 0; i < solution.size(); i++)
                                {
                                    double d = fabs(ik_seed_state[i] - solution[i]);
                                    if (d > costs)
                                        costs = d;
                                }
                                if (costs < best_costs || best_costs == -1.0)
                                {
                                    best_costs = costs;
                                    best_solution = solution;
                                }
                            }
                            else
                                // Return first feasible solution
                                return true;
                        }
                    }
                }
            }

            if (!getCount(counter, num_positive_increments, -num_negative_increments))
            {
                // Everything searched
                error_code.val = moveit_msgs::MoveItErrorCodes::NO_IK_SOLUTION;
                break;
            }

            vfree[0] = initial_guess + search_discretization_ * counter;
            // ROS_DEBUG_STREAM_NAMED("ikfast","Attempt " << counter << " with 0th free joint having value " << vfree[0]);
        }

        ROS_DEBUG_STREAM_NAMED("ikfast", "Valid solutions: " << nvalid << "/" << nattempts);

        if ((search_mode & OPTIMIZE_MAX_JOINT) && best_costs != -1.0)
        {
            solution = best_solution;
            error_code.val = error_code.SUCCESS;
            return true;
        }

        // No solution found
        error_code.val = moveit_msgs::MoveItErrorCodes::NO_IK_SOLUTION;
        return false;
    }

// Used when there are no redundant joints - aka no free params
    bool IKFastKinematicsPlugin::getPositionIK(const geometry_msgs::Pose& ik_pose, const std::vector<double>& ik_seed_state,
                                               std::vector<double>& solution, moveit_msgs::MoveItErrorCodes& error_code,
                                               const kinematics::KinematicsQueryOptions& options) const
    {
        ROS_DEBUG_STREAM_NAMED("ikfast", "getPositionIK");

        if (!active_)
        {
            ROS_ERROR("kinematics not active");
            return false;
        }

        if (ik_seed_state.size() < num_joints_)
        {
            ROS_ERROR_STREAM("ik_seed_state only has " << ik_seed_state.size() << " entries, this ikfast solver requires "
                                                       << num_joints_);
            return false;
        }

        // Check if seed is in bound
        for (std::size_t i = 0; i < ik_seed_state.size(); i++)
        {
            // Add tolerance to limit check
            if (joint_has_limits_vector_[i] && ((ik_seed_state[i] < (joint_min_vector_[i] - LIMIT_TOLERANCE)) ||
                                                (ik_seed_state[i] > (joint_max_vector_[i] + LIMIT_TOLERANCE))))
            {
                ROS_DEBUG_STREAM_NAMED("ikseed", "Not in limits! " << (int)i << " value " << ik_seed_state[i]
                                                                   << " has limit: " << joint_has_limits_vector_[i] << "  being  "
                                                                   << joint_min_vector_[i] << " to " << joint_max_vector_[i]);
                return false;
            }
        }

        std::vector<double> vfree(free_params_.size());
        for (std::size_t i = 0; i < free_params_.size(); ++i)
        {
            int p = free_params_[i];
            ROS_ERROR("%u is %f", p, ik_seed_state[p]);  // DTC
            vfree[i] = ik_seed_state[p];
        }

        KDL::Frame frame;
        tf::poseMsgToKDL(ik_pose, frame);

        IkSolutionList<IkReal> solutions;
        int numsol = solve(frame, vfree, solutions);
        ROS_DEBUG_STREAM_NAMED("ikfast", "Found " << numsol << " solutions from IKFast");

        // struct for storing and sorting solutions
        struct LimitObeyingSol
        {
            std::vector<double> value;
            double dist_from_seed;

            bool operator<(const LimitObeyingSol& a) const
            {
                return dist_from_seed < a.dist_from_seed;
            }
        };
        std::vector<LimitObeyingSol> solutions_obey_limits;

        if (numsol)
        {
            std::vector<double> solution_obey_limits;
            for (std::size_t s = 0; s < numsol; ++s)
            {
                std::vector<double> sol;
                getSolution(solutions, s, sol);
                ROS_DEBUG_NAMED("ikfast", "Sol %d: %e   %e   %e   %e   %e   %e", (int)s, sol[0], sol[1], sol[2], sol[3], sol[4],
                                sol[5]);

                bool obeys_limits = true;
                for (std::size_t i = 0; i < sol.size(); i++)
                {
                    // Add tolerance to limit check
                    if (joint_has_limits_vector_[i] && ((sol[i] < (joint_min_vector_[i] - LIMIT_TOLERANCE)) ||
                                                        (sol[i] > (joint_max_vector_[i] + LIMIT_TOLERANCE))))
                    {
                        // One element of solution is not within limits
                        obeys_limits = false;
                        ROS_DEBUG_STREAM_NAMED("ikfast", "Not in limits! " << (int)i << " value " << sol[i] << " has limit: "
                                                                           << joint_has_limits_vector_[i] << "  being  "
                                                                           << joint_min_vector_[i] << " to " << joint_max_vector_[i]);
                        break;
                    }
                }
                if (obeys_limits)
                {
                    // All elements of this solution obey limits
                    getSolution(solutions, s, solution_obey_limits);
                    double dist_from_seed = 0.0;
                    for (std::size_t i = 0; i < ik_seed_state.size(); ++i)
                    {
                        dist_from_seed += fabs(ik_seed_state[i] - solution_obey_limits[i]);
                    }

                    solutions_obey_limits.push_back({ solution_obey_limits, dist_from_seed });
                }
            }
        }
        else
        {
            ROS_DEBUG_STREAM_NAMED("ikfast", "No IK solution");
        }

        // Sort the solutions under limits and find the one that is closest to ik_seed_state
        if (!solutions_obey_limits.empty())
        {
            std::sort(solutions_obey_limits.begin(), solutions_obey_limits.end());
            solution = solutions_obey_limits[0].value;
            error_code.val = moveit_msgs::MoveItErrorCodes::SUCCESS;
            return true;
        }

        error_code.val = moveit_msgs::MoveItErrorCodes::NO_IK_SOLUTION;
        return false;
    }

    bool IKFastKinematicsPlugin::getPositionIK(const std::vector<geometry_msgs::Pose>& ik_poses,
                                               const std::vector<double>& ik_seed_state,
                                               std::vector<std::vector<double>>& solutions,
                                               kinematics::KinematicsResult& result,
                                               const kinematics::KinematicsQueryOptions& options) const
    {
        ROS_DEBUG_STREAM_NAMED("ikfast", "getPositionIK with multiple solutions");

        if (!active_)
        {
            ROS_ERROR("kinematics not active");
            result.kinematic_error = kinematics::KinematicErrors::SOLVER_NOT_ACTIVE;
            return false;
        }

        if (ik_poses.empty())
        {
            ROS_ERROR("ik_poses is empty");
            result.kinematic_error = kinematics::KinematicErrors::EMPTY_TIP_POSES;
            return false;
        }

        if (ik_poses.size() > 1)
        {
            ROS_ERROR("ik_poses contains multiple entries, only one is allowed");
            result.kinematic_error = kinematics::KinematicErrors::MULTIPLE_TIPS_NOT_SUPPORTED;
            return false;
        }

        if (ik_seed_state.size() < num_joints_)
        {
            ROS_ERROR_STREAM("ik_seed_state only has " << ik_seed_state.size() << " entries, this ikfast solver requires "
                                                       << num_joints_);
            return false;
        }

        KDL::Frame frame;
        tf::poseMsgToKDL(ik_poses[0], frame);

        // solving ik
        std::vector<IkSolutionList<IkReal>> solution_set;
        IkSolutionList<IkReal> ik_solutions;
        std::vector<double> vfree;
        int numsol = 0;
        std::vector<double> sampled_joint_vals;
        if (!redundant_joint_indices_.empty())
        {
            // initializing from seed
            sampled_joint_vals.push_back(ik_seed_state[redundant_joint_indices_[0]]);

            // checking joint limits when using no discretization
            if (options.discretization_method == kinematics::DiscretizationMethods::NO_DISCRETIZATION &&
                joint_has_limits_vector_[redundant_joint_indices_.front()])
            {
                double joint_min = joint_min_vector_[redundant_joint_indices_.front()];
                double joint_max = joint_max_vector_[redundant_joint_indices_.front()];

                double jv = sampled_joint_vals[0];
                if (!((jv > (joint_min - LIMIT_TOLERANCE)) && (jv < (joint_max + LIMIT_TOLERANCE))))
                {
                    result.kinematic_error = kinematics::KinematicErrors::IK_SEED_OUTSIDE_LIMITS;
                    ROS_ERROR_STREAM("ik seed is out of bounds");
                    return false;
                }
            }

            // computing all solutions sets for each sampled value of the redundant joint
            if (!sampleRedundantJoint(options.discretization_method, sampled_joint_vals))
            {
                result.kinematic_error = kinematics::KinematicErrors::UNSUPORTED_DISCRETIZATION_REQUESTED;
                return false;
            }

            for (unsigned int i = 0; i < sampled_joint_vals.size(); i++)
            {
                vfree.clear();
                vfree.push_back(sampled_joint_vals[i]);
                numsol += solve(frame, vfree, ik_solutions);
                solution_set.push_back(ik_solutions);
            }
        }
        else
        {
            // computing for single solution set
            numsol = solve(frame, vfree, ik_solutions);
            solution_set.push_back(ik_solutions);
        }

        ROS_DEBUG_STREAM_NAMED("ikfast", "Found " << numsol << " solutions from IKFast");
        bool solutions_found = false;
        if (numsol > 0)
        {
            /*
              Iterating through all solution sets and storing those that do not exceed joint limits.
            */
            for (unsigned int r = 0; r < solution_set.size(); r++)
            {
                ik_solutions = solution_set[r];
                numsol = ik_solutions.GetNumSolutions();
                for (int s = 0; s < numsol; ++s)
                {
                    std::vector<double> sol;
                    getSolution(ik_solutions, s, sol);

                    bool obeys_limits = true;
                    for (unsigned int i = 0; i < sol.size(); i++)
                    {
                        // Add tolerance to limit check
                        if (joint_has_limits_vector_[i] && ((sol[i] < (joint_min_vector_[i] - LIMIT_TOLERANCE)) ||
                                                            (sol[i] > (joint_max_vector_[i] + LIMIT_TOLERANCE))))
                        {
                            // One element of solution is not within limits
                            obeys_limits = false;
                            ROS_DEBUG_STREAM_NAMED(
                                    "ikfast", "Not in limits! " << i << " value " << sol[i] << " has limit: " << joint_has_limits_vector_[i]
                                                                << "  being  " << joint_min_vector_[i] << " to " << joint_max_vector_[i]);
                            break;
                        }
                    }
                    if (obeys_limits)
                    {
                        // All elements of solution obey limits
                        solutions_found = true;
                        solutions.push_back(sol);
                    }
                }
            }

            if (solutions_found)
            {
                result.kinematic_error = kinematics::KinematicErrors::OK;
                return true;
            }
        }
        else
        {
            ROS_DEBUG_STREAM_NAMED("ikfast", "No IK solution");
        }

        result.kinematic_error = kinematics::KinematicErrors::NO_SOLUTION;
        return false;
    }

    bool IKFastKinematicsPlugin::sampleRedundantJoint(kinematics::DiscretizationMethod method,
                                                      std::vector<double>& sampled_joint_vals) const
    {
        double joint_min = -M_PI;
        double joint_max = M_PI;
        int index = redundant_joint_indices_.front();
        double joint_dscrt = redundant_joint_discretization_.at(index);

        if (joint_has_limits_vector_[redundant_joint_indices_.front()])
        {
            joint_min = joint_min_vector_[index];
            joint_max = joint_max_vector_[index];
        }

        switch (method)
        {
            case kinematics::DiscretizationMethods::ALL_DISCRETIZED:
            {
                int steps = std::ceil((joint_max - joint_min) / joint_dscrt);
                for (unsigned int i = 0; i < steps; i++)
                {
                    sampled_joint_vals.push_back(joint_min + joint_dscrt * i);
                }
                sampled_joint_vals.push_back(joint_max);
            }
                break;
            case kinematics::DiscretizationMethods::ALL_RANDOM_SAMPLED:
            {
                int steps = std::ceil((joint_max - joint_min) / joint_dscrt);
                steps = steps > 0 ? steps : 1;
                double diff = joint_max - joint_min;
                for (int i = 0; i < steps; i++)
                {
                    sampled_joint_vals.push_back(((diff * std::rand()) / (static_cast<double>(RAND_MAX))) + joint_min);
                }
            }

                break;
            case kinematics::DiscretizationMethods::NO_DISCRETIZATION:

                break;
            default:
                ROS_ERROR_STREAM("Discretization method " << method << " is not supported");
                return false;
        }

        return true;
    }
}


#endif //PROJECT_UR10_MANIPULATOR_IKFAST_MOVEIT_PLUGIN_H
