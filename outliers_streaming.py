import paho.mqtt.client as mqtt
import numpy as np
import sys
import time
import readchar
import signal

# Function to deal with Ctr + C from the user
def signal_handler(sig,frame):
    global plt
    print('Wait for the results')
    #plt.show()
    client.loop_stop()
    time.sleep(2)
    sys.exit(0)


# Class in which 3 objets will be created, the 3 accelerations
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

#broker address (Raspberry Address)
broker_address="192.168.1.5"

client =mqtt.Client("danielc")

client.connect(broker_address) #connect to broker

client.subscribe("aceleracao_x")
client.subscribe("aceleracao_y")
client.subscribe("aceleracao_z")

# Creating 3 objects
mx=StreamingMeanAndVariance()
mz=StreamingMeanAndVariance()
my=StreamingMeanAndVariance()


# Callback funtion for MQTT topics

def on_message(client, userdata, message):
    global ix,iy,iz


    if time.time()-initial_seconds<10:
        if message.topic == "aceleracao_x":
            mx.update(float(message.payload.decode("utf-8")))
        if message.topic == "aceleracao_y":
            my.update(float(message.payload.decode("utf-8")))
        if message.topic == "aceleracao_z":
            mz.update(float(message.payload.decode("utf-8")))
        return


    if message.topic=="aceleracao_x":
      
        ix+=1
        if not - 3 <= (float(message.payload.decode("utf-8")) - mx.mean) / np.sqrt(mx.variance) <= 3:

            
            client.publish("OutlierValueX",str(message.payload.decode("utf-8")))
            
            
        mx.update(float(message.payload.decode("utf-8")))


    if message.topic=="aceleracao_y":
     
        iy+=1
        if not - 3 <= (float(message.payload.decode("utf-8")) - my.mean) / np.sqrt(my.variance) <= 3:
 
        
            client.publish("OutlierValueY",str(message.payload.decode("utf-8")))
        
        my.update(float(message.payload.decode("utf-8")))
   


    if message.topic=="aceleracao_z":
        
      
        iz+=1
        if not - 3 <= (float(message.payload.decode("utf-8")) - mz.mean) / np.sqrt(mz.variance) <= 3:
  
            
            client.publish("OutlierValueZ",str(message.payload.decode("utf-8")))
        
        mz.update(float(message.payload.decode("utf-8")))
      

client.on_message=on_message        #attach function to callback



client.loop_start()    #start the loop

initial_seconds = time.time()




signal.signal(signal.SIGINT,signal_handler)
signal.pause()
while True:
  pass



#plt.show()
client.loop_stop()



