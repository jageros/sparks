#!/usr/bin/env python

import rospy
from geometry_msgs.msg import Twist
from std_msgs.msg import String,Int16
from math import copysign

class VoiceNav:
    def __init__(self):
        rospy.init_node('voice_command')
        
        rospy.on_shutdown(self.cleanup)
        
        # Set a number of parameters affecting the robot's speed
        self.max_speed = rospy.get_param("~max_speed", 0.4)
        self.max_angular_speed = rospy.get_param("~max_angular_speed", 1.5)
        self.speed = rospy.get_param("~start_speed", 0.1)
        self.angular_speed = rospy.get_param("~start_angular_speed", 0.5)
        self.linear_increment = rospy.get_param("~linear_increment", 0.05)
        self.angular_increment = rospy.get_param("~angular_increment", 0.4)
        
        self.rate = rospy.get_param("~rate", 5)
        r = rospy.Rate(self.rate)
        
        # A flag to determine whether or not voice control is paused
        self.paused = False
        
        # Initialize the Twist message we will publish.
        self.cmd_vel = Twist()

        # Publish the Twist message to the cmd_vel topic
        self.cmd_vel_pub = rospy.Publisher('cmd_vel', Twist, queue_size=5)
        

        rospy.Subscriber('/command', String, self.speech_callback)  
        
        while not rospy.is_shutdown():
            self.cmd_vel_pub.publish(self.cmd_vel)
            r.sleep()                       
            
        
    def speech_callback(self, msg):

        command = msg.data        

        if command == 'start':
            self.paused = True
            
        elif command == 'end':
        	self.paused = False 
        	
        # If voice control is paused, simply return without
        # performing any action
        if self.paused:
            return       
        
        # The list of if-then statements should be fairly
        # self-explanatory
        if command == 'forward':    
            self.cmd_vel.linear.x = self.speed
            self.cmd_vel.angular.z = 0
            
        elif command == 'round':
            self.cmd_vel.linear.x = 0
            self.cmd_vel.angular.z = self.angular_speed
                
        elif command == 'rround':  
            self.cmd_vel.linear.x = 0      
            self.cmd_vel.angular.z = -self.angular_speed
            
        elif command == 'right':
            if self.cmd_vel.linear.x != 0:
                self.cmd_vel.angular.z += self.angular_increment
            else:        
                self.cmd_vel.angular.z = self.angular_speed
                
        elif command == 'left':    
            if self.cmd_vel.linear.x != 0:
                self.cmd_vel.angular.z -= self.angular_increment
            else:        
                self.cmd_vel.angular.z = -self.angular_speed
                
        elif command == 'bakward':
            self.cmd_vel.linear.x = -self.speed
            self.cmd_vel.angular.z = 0
            
        elif command == 'stop': 
            # Stop the robot!  Publish a Twist message consisting of all zeros.         
            self.cmd_vel = Twist()
        
        elif command == 'fast':
            self.speed += self.linear_increment
            self.angular_speed += self.angular_increment
            if self.cmd_vel.linear.x != 0:
                self.cmd_vel.linear.x += copysign(self.linear_increment, self.cmd_vel.linear.x)
            if self.cmd_vel.angular.z != 0:
                self.cmd_vel.angular.z += copysign(self.angular_increment, self.cmd_vel.angular.z)
            
        elif command == 'slow':
            self.speed -= self.linear_increment
            self.angular_speed -= self.angular_increment
            if self.cmd_vel.linear.x != 0:
                self.cmd_vel.linear.x -= copysign(self.linear_increment, self.cmd_vel.linear.x)
            if self.cmd_vel.angular.z != 0:
                self.cmd_vel.angular.z -= copysign(self.angular_increment, self.cmd_vel.angular.z)
                
       
        else:
            return

        self.cmd_vel.linear.x = min(self.max_speed, max(-self.max_speed, self.cmd_vel.linear.x))
        self.cmd_vel.angular.z = min(self.max_angular_speed, max(-self.max_angular_speed, self.cmd_vel.angular.z))

    def cleanup(self):
        # When shutting down be sure to stop the robot!
        twist = Twist()
        self.cmd_vel_pub.publish(twist)
        rospy.sleep(1)

if __name__=="__main__":
    try:
        VoiceNav()
        rospy.spin()
    except rospy.ROSInterruptException:
        rospy.loginfo("Voice navigation terminated.")

