#!/usr/bin/env python
# coding:utf-8
#

'''
{
  "sn":1,
  "ls":true,
  "bg":0,
  "ed":0,
  "ws":[{
      "bg":0,
      "cw":[{
          "id":65535,
          "w":"你几岁了",
          "gm":0,
          "sc":48
        }],
      "slot":"<text>"
    }],
  "sc":48
}

'''

import rospy
from std_msgs.msg import String,Int16
from geometry_msgs.msg import Twist
import json

pub1 = object()
pub2 = object()
#command = {u'停止':0,u'stop':0,u'去端茶':1,u'get me the tea':1,u'回家':2,u'home':2,u'左':3,u'右':4,u'前进':5,u'后退':6,u'转圈':7,u'快':8,u'慢':9}
commands = {'stop':[u'停止','stop'],
			'goal':[u'去端茶','get me the tea'],
			'home':[u'回家','home'],
			'left':[u'左',u'向左','left'],
			'right':[u'右',u'向右','right'],
			'forward':[u'前进','forward','ahead'],
			'bakward':[u'后退','back','backward'],			
			'fast':[u'快',u'快点','speed up', 'faster'],
			'slow':[u'慢',u'慢点','slow down', 'slower'],
			'blank':[],
			'round':[u'转圈',u'向左转圈'],
			'rround':[u'向右转圈'],
			'begin':[u'开始语音控制'],
			'end':[u'结束语音控制'],}
chats =    {'请叫我spark':[u'你叫什么',u'你叫什么名字',"what's your name"],
			'我不告诉你':[u'你几岁了',u'你多大了'],
			'这个世界太不安静了':[u'陪我聊聊天',u'聊聊天',u'聊天'],
			'你也很好':[u'你好']}
#l = [[[0, 0, 0],[0,0,0.4]],[[0, 0, 0],[0,0,-0.4]],[[1, 0, 0],[0,0,0]],[[-1, 0, 0],[0,0,0]],[[2, 0.0, 0.0],[0,0,-1.8]]]
def parsetext(data):   
    jdata = json.loads(data.data)
    text = jdata['ws'][0]['cw'][0]['w']
    sc = jdata['sc']
        
    for (index,cmd) in commands.iteritems():
    	if text in cmd:
    		 pub2.publish(index)
    		 
    for (talk,chat) in chats.iteritems():
    	if text in chat:
    		 pub1.publish(String(talk))
'''
    if text in command.keys():
        print "in command"        
        index = command[text]
        pub2.publish(index)
    elif text in chat.keys():
    	talk = chat[text]
    	pub1.publish(String(talk))
'''


def listener():
    global pub1
    global pub2

    rospy.init_node('parse')

    rospy.Subscriber('xfunderstand', String, parsetext)
    pub1 = rospy.Publisher('xfsaywords', String, queue_size=5)
    pub2 = rospy.Publisher('command',String,queue_size=5)
    
    # spin() simply keeps python from exiting until this node is stopped
    rospy.spin()

if __name__ == '__main__':
    listener()
