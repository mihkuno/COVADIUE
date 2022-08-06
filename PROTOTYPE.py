def main():
    # array configuration
	from math import sqrt
	import numpy as np

	# computer vision
	import cv2
	import mediapipe as mp

	# # image classification
	# from keras.models import load_model

	# webcam client
	import urllib.request

	# intialize the facemask model
	# MODEL = load_model('C:/Users/caind/Desktop/COVADIUE/PROTOTYPE/maskdet.model')

	# intialize mediapipe utilites
	MP_DRAWING         = mp.solutions.drawing_utils
	MP_FACE_MESH       = mp.solutions.face_mesh
	MP_DRAWING_STYLES  = mp.solutions.drawing_styles

	DRAWING_SPEC = MP_DRAWING.DrawingSpec(thickness=1, circle_radius=1)

	# calculating the distance
	CM = 65 # measeured distance
	PX = 2000 # per measured area

	# to proportion display scale
	xDISPLAY = 320*2
	yDISPLAY = 240*2
	xOLED = 128
	yOLED = 64
 
	# mapping roi frame to oled
	y_min = int(yDISPLAY/2)
	y_max = -130
	x_min = int(xDISPLAY/2)+20
	x_max = -90
	
	# roi frame length and width
	xmin_frame = (x_min + x_max)
	ymin_frame = (y_min + y_max)

	cam = cv2.VideoCapture(0, cv2.CAP_DSHOW)
	cam.set(cv2.CAP_PROP_FPS, 30)
	cam.set(cv2.CAP_PROP_FRAME_WIDTH, xDISPLAY)
	cam.set(cv2.CAP_PROP_FRAME_HEIGHT, yDISPLAY)
	cam.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))

    # detect faces
	with MP_FACE_MESH.FaceMesh(
		# setup facemesh detect config
		max_num_faces=3,
		refine_landmarks=True, 
		min_detection_confidence=0.45,
		min_tracking_confidence=0.45) as face_mesh:

		metadata = a_oled = x_oled = y_oled = ""
		while True:

				# press escape to exit
				if cv2.waitKey(5) & 0xFF == 27:
						break
				
				urllib.request.urlopen('http://192.168.1.10/capture?metadata='+metadata)
				
				ret, frame = cam.read()
				frame = frame[y_min:y_max, x_min:x_max]
				# frame = cv2.flip(frame, 0) # flip camera vertically

				frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB) # rgb version

				# To improve performance, mark the writable false
				frame_rgb.flags.writeable = False
				results = face_mesh.process(frame_rgb)
				#------------------------------------------------------------------

				metadata = actual = distance = area = label = ""
				
				if results.multi_face_landmarks:
					# draw a square for every face detected
					for face_landmarks in results.multi_face_landmarks:
							# MP_DRAWING.draw_landmarks(image, face_landmarks, MP_FACE_MESH.FACEMESH_CONTOURS)

							# map the roi for the facemesh bounding box
							h, w, c = frame.shape
							cx_min=  w
							cy_min = h
							cx_max= cy_max= 0


							for id, lm in enumerate(face_landmarks.landmark):
									cx, cy = int(lm.x * w), int(lm.y * h)
									if cx<cx_min:
											cx_min=cx
									if cy<cy_min:
											cy_min=cy
									if cx>cx_max:
											cx_max=cx
									if cy>cy_max:
											cy_max=cy
							if (cx_min <= 0):
										cx_min = 0
							elif (cy_min <= 0):
									cy_min = 0

							stroke = (255, 255, 0)
							x,y,w,h = (cx_min, cy_min, cx_max-cx_min, cy_max-cy_min)

							# try: ##########################################################
							# 	# The model takes an image of dimensions (224,224) as input
							# 	# so reshape our image to the same.

							# 	bruh = cv2.resize(frame[cy_min-16:cy_max+16,cx_min-22:cx_max+22], (224, 224))

							# 	# Convert the image to a numpy array
							# 	img = np.asarray(bruh)

							# 	# Normalize the image
							# 	normalize = (img.astype(np.float32) / 127.0) - 1

							# 	# create the array of the right shape to feed into the keras model
							# 	data = np.ndarray(shape=(1, 224, 224, 3), dtype=np.float32)

							# 	#  Load the image into the array
							# 	data[0] = normalize

							# 	# Predict the class
							# 	prediction = MODEL.predict(data)

							# 	if prediction[0][0] > 0.88: # mask
							# 		label = 'MASK'
							# 		stroke = (0,255,0)
							# 		cv2.putText(frame, label, (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.3, stroke, 1)

							# 	elif prediction[0][1] > 0.7: # face
							# 		label = 'FACE'
							# 		stroke = (0,0,255)
							# 		cv2.putText(frame, label, (x, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.3, stroke, 1)

							# except: #####################################
							# 		print('bounding box exceeded window')

							area = w*h # input area
							distance = int((CM*PX)/area)

							print(xmin_frame, ymin_frame)

							a_oled = int(sqrt((xOLED * yOLED * area) / (xmin_frame * ymin_frame)))+10 # oled square area
							x_oled = int((xOLED * x) / xmin_frame)-5 # oled square x value
							y_oled = int((yOLED * y) / ymin_frame)-5 # oled square y value

							# update local information
							cv2.rectangle(frame, (x, y), (cx_max, cy_max), stroke, 1)
							cv2.putText(frame, str(distance)+'cm', (cx_max-30, y-10), cv2.FONT_HERSHEY_SIMPLEX, 0.3, stroke, 1)

							label = 0
							# submit metadata to webserver
							actual += f"{x}:{y}:{w}:{h}"
							label = 1 if label=='MASK' else 0
							metadata += f"{x_oled}:{y_oled}:{a_oled}:{label}:{distance}>"
		
							print(f"Distance {distance}cm <> Area {area}px")
				cv2.imshow("webcam", frame)
	cv2.destroyAllWindows()
