import cv2
import numpy as np 
from PIL import Image
import roslib

#!/usr/bin/env python
import roslib
roslib.load_manifest('facedetector')
import rospy
import sys, select, termios, tty
import cv
import time
import Image
import os
import cv2, numpy
from std_msgs.msg import String
from std_msgs.msg import Bool
import sensor_msgs.msg
from dynamic_reconfigure.server import Server as DynamicReconfigureServer
from facedetector.msg import Detection
from cv_bridge import CvBridge, CvBridgeError
from sensor_msgs.msg import Image

class Faces():
	def face_callback(self, data):
		print("v call back")
		if(time.time() - self.cas > 5 ):
			print("berem image")
			n = len(data.x)
			image = numpy.zeros((self.face_size * n, self.face_size), dtype=numpy.uint8)
			try:

				for i in xrange(0, n):
					image = self.bridge.imgmsg_to_cv2(data.image[i], "mono8")
					self.recognize(image)

			except CvBridgeError, e:
				print e


	def read_images(self,urls):
		image_paths = [os.path.join(path, f) for f in os.listdir(path)]
		images = []
		labels = []
		from url in urls:
			images += [images, cv2.imread(url)]
			nbr = int(os.path.split(image_path)[1].split(".")[0])
			labels += [labels, nbr]
		return [images, labels]


	def learn(self):
		[images, labels] = read_images("images/")
		self.recognizer.train(images, np.array(labels))

	def recognize(self,image):
		nbr_predicted, conf = recognizer.predict(image)
		print("predicted" + str(conf))

	def __init__(self):
		self.recognizer = cv2.createLBPHFaceRecognizer()
		print("ucim")
		self.learn()
		print("naucil")
		faces_topic = rospy.get_param('~topic', rospy.resolve_name('/faces'))
		
		self.face_size = rospy.get_param('~face_size', 128)

		self.bridge = CvBridge()
		self.faces_sub = rospy.Subscriber(faces_topic, Detection, self.face_callback)
		self.cas = time.time()
		self.tabela_imen = ["blank" , "Potter", "Filip", "Tina", "Peter", "Tom", "Ellen", "Kim", "Scarlet", "Matt"]



def main():
	rospy.init_node('facereco')
		try:
			fd = Faces()
			r = rospy.Rate(30)
			while not rospy.is_shutdown():
				cv2.waitKey(1)
				r.sleep()
		except rospy.ROSInterruptException: pass


