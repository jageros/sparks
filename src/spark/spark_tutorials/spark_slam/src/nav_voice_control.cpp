#include <ros/ros.h>
#include <std_msgs/String.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <tf/transform_listener.h>

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int robot_state = 0;
int voice_cmd = 0;

void commandCallback(const std_msgs::String::ConstPtr& cmd)
{
  if (strcmp(cmd->data.c_str(),"goal") == 0)
    voice_cmd = 1;
  else if (strcmp(cmd->data.c_str(),"home") == 0)
    voice_cmd = 2;
  else
    voice_cmd = 0;
}

int main(int argc, char** argv){
  ros::init(argc, argv, "simple_navigation_goals");
  ros::NodeHandle vc;
  ros::Subscriber sub = vc.subscribe("command",1,commandCallback);
  //tell the action client that we want to spin a thread by default
  MoveBaseClient ac("move_base", true);

  //wait for the action server to come up
  while(!ac.waitForServer(ros::Duration(2.0))){
    ROS_INFO("Waiting for the move_base action server to come up");
  }

  move_base_msgs::MoveBaseGoal goal;
  goal.target_pose.header.frame_id = "map";
  goal.target_pose.header.stamp = ros::Time::now();

  goal.target_pose.pose.position.x = 1;
  goal.target_pose.pose.position.y = 0;
  goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(0);

  move_base_msgs::MoveBaseGoal home;
  home.target_pose.header.frame_id = "map";
  home.target_pose.header.stamp = ros::Time::now();

  home.target_pose.pose.position.x = 0;
  home.target_pose.pose.position.y = 0;
  home.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(0);

  while(ros::ok())
  {
    if (robot_state == 0 || robot_state == 2)
      if (voice_cmd == 1)
      {
        ROS_INFO("sending goal ");
        ac.sendGoal(goal);

        ac.waitForResult();

        if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
          ROS_INFO("Hooray, the base moved 1 meter forward");
        else
          ROS_INFO("The base failed to move forward 1 meter for some reason");
      }

    if (robot_state == 0 || robot_state == 1)
      if (voice_cmd == 2)
      {
        ROS_INFO("sending home ");
        ac.sendGoal(home);

        ac.waitForResult();

        if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
          ROS_INFO("Hooray, the base moved 1 meter forward");
        else
          ROS_INFO("The base failed to move forward 1 meter for some reason");
      }


    ros::spinOnce();
  }
  return 0;
}
