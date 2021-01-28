import paho.mqtt.client as mqtt
import numpy as np
#import matplotlib.pyplot as plt
#import matplotlib.patches as mpatches
import sys
import time
import readchar
import signal
global plt


def signal_handler(sig,frame):
    global plt
    print('Wait for the results')
    #plt.show()
    client.loop_stop()
    time.sleep(2)
    sys.exit(0)




#Configuration of all plots



#plt.subplot(3, 1, 1)

#red_patch = mpatches.Patch(color='red', label='Outliers')
#green_patch = mpatches.Patch(color='green', label='Mean')
#blue_patch = mpatches.Patch(color='blue', label='Value')


#plt.legend(handles=[red_patch,green_patch,blue_patch])
#plt.title('X Aceleration')
#plt.ylabel('X Aceleration')



#plt.subplot(3, 1, 2)

#plt.legend(handles=[red_patch,green_patch,blue_patch])
#plt.title('Y Aceleration')
#plt.ylabel('Y Aceleration')
#plt.xlabel('Time')


#plt.subplot(3, 1, 3)

#plt.legend(handles=[red_patch,green_patch,blue_patch])
#plt.title('Z Aceleration')
#plt.ylabel('Z Aceleration')
#plt.xlabel('Time')

#plt.axis([0, 10, -1000, 1000])


class StreamingMeanAndVariance:
    def __init__(self):
        self.mean = 0
        self.variance = 0
        self.n_elements = 0

    def update(self, element):
        self.variance = ((self.variance + self.mean ** 2) * self.n_elements + element ** 2) / (self.n_elements + 1)
        self.mean = ((self.mean * self.n_elements) + element) / (self.n_elements + 1)
        self.variance = self.variance - self.mean ** 2
        self.n_elements += 1



global ix,iy,iz



ix=0
iy=0
iz=0

#plt.axis([0, 6, 0, 20])
broker_address="192.168.1.5"

client =mqtt.Client("danielc")

client.connect(broker_address) #connect to broker

client.subscribe("aceleracao_x")
client.subscribe("aceleracao_y")
client.subscribe("aceleracao_z")

mx=StreamingMeanAndVariance()
mz=StreamingMeanAndVariance()
my=StreamingMeanAndVariance()




def on_message(client, userdata, message):
    global ix,iy,iz
    #print("message received " ,str(message.payload.decode("utf-8")))
    #print("message topic=",message.topic)
    #print("message qos=",message.qos)
    #print("message retain flag=",message.retain)

    if time.time()-initial_seconds<10:
        if message.topic == "aceleracao_x":
            mx.update(float(message.payload.decode("utf-8")))
        if message.topic == "aceleracao_y":
            my.update(float(message.payload.decode("utf-8")))
        if message.topic == "aceleracao_z":
            mz.update(float(message.payload.decode("utf-8")))
        return

    #color='b'

    if message.topic=="aceleracao_x":
        #plt.subplot(3, 1, 1)
        ix+=1
        if not - 3 <= (float(message.payload.decode("utf-8")) - mx.mean) / np.sqrt(mx.variance) <= 3:
            #print outliers whenever they ocuurs
            #print(i, message.payload.decode("utf-8"))
            #print('X Outlier Detected')
            #color='r'
            
            client.publish("OutlierValueX",str(message.payload.decode("utf-8")))
            #publicar o valor do outlier para o node red
            
        #print(ix)
        mx.update(float(message.payload.decode("utf-8")))
        #plt.scatter(ix, mx.mean, c='g',s=5)
        #plt.scatter(ix,float(message.payload.decode("utf-8")),c=color)

    if message.topic=="aceleracao_y":
        #plt.subplot(3, 1, 2)
        iy+=1
        if not - 3 <= (float(message.payload.decode("utf-8")) - my.mean) / np.sqrt(my.variance) <= 3:
            #print outliers whenever they ocuurs
            #print(i, message.payload.decode("utf-8"))
            #print('Y Outlier Detected')
            #color='r'
            
            client.publish("OutlierValueY",str(message.payload.decode("utf-8")))
        #print(iy)
        my.update(float(message.payload.decode("utf-8")))
        #plt.scatter(iy, my.mean, c='g',s=5)
        #plt.scatter(iy,float(message.payload.decode("utf-8")),c=color)


    if message.topic=="aceleracao_z":
        
        #plt.subplot(3, 1, 3)
        iz+=1
        if not - 3 <= (float(message.payload.decode("utf-8")) - mz.mean) / np.sqrt(mz.variance) <= 3:
            #print outliers whenever they ocuurs
            #print(i, message.payload.decode("utf-8"))
            #print('Z Outlier Detected')
            #color='r'
            
            client.publish("OutlierValueZ",str(message.payload.decode("utf-8")))
        #print(iz)
        mz.update(float(message.payload.decode("utf-8")))
        #plt.scatter(iz, mz.mean, c='g',s=5)
        #plt.scatter(iz,float(message.payload.decode("utf-8")),c=color)

client.on_message=on_message        #attach function to callback



client.loop_start()    #start the loop

initial_seconds = time.time()




signal.signal(signal.SIGINT,signal_handler)
signal.pause()
while True:
  pass



#plt.show()
client.loop_stop()



