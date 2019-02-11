#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jul 11 16:47:20 2018

This script reads the experiment data from the master and saves
the data to a text and a NPY file. The master has to be 
connected to the device running the script via USB. 

There are 4 files generated for every experiment recorded
with this script:
1) A README file for the experiment, where the start time,
    the variables, the units and the used ALPACAs are noted
2) A .txt file with the data (without gps data)
3) A .npy file with the data (without gps data)
4) (If applicable) a _gps.npy file with the gps data
All files start with the date (YYYYMMDDHHMMSS) and the experiment
name.
The README file starts with readme_DATE_EXPNAME.

Before starting the script, make sure to:
    - set an experiment name (experiment_name)
    - set the correct USB port (usb_port)
    - to state which instruments are part of the experiment
    - set verbose to True or False
@author: Jakob
"""

import serial
import processing_functions as pf
import time 
import numpy as np
from matplotlib.dates import date2num
from datetime import datetime
import pickle

##### Parameters ##################
experiment_name = 'Test'
usb_port = '/dev/cu.wchusbserial1410'
instruments = np.array([3]) # Instruments WITHOUT GPS
verbose = True
###################################


instruments = instruments - 1 # zero-based python
date = datetime.strftime(datetime.now(), '%Y%m%d%H%M%S') # get date 
data_filename = date + '_'+ experiment_name # create filename
baud_rate = 115200 # set baud rate for serial USB read
pf.readme(data_filename, instruments+1) # create README file
miss_val = -9999 # set missing value in data

# Initialize the arrays for data and gps data
save_array = {} # initialize array with data 
gott = np.zeros(4)+miss_val # initialize array for GPS data (gott)
for i in instruments:
    save_array[i+1] = np.zeros(5)    + miss_val 
    
f = open(data_filename+'.txt','w') # open the TXT file for the data


i = 0 # counter for new regular data packets
t = 0 # counter for serial USB requests
any_gott = False
# Start of main loop
while(True):
    # Inizialize the handle to the serial USB port (ser)
    if (t == 0):
        # Parity needs to be PARITY_NONE
        ser = serial.Serial(usb_port,baud_rate,timeout=0,parity=serial.PARITY_NONE, rtscts=1)
        time.sleep(1)  
    # Read the handle to the serial USB port 
    s = ser.read(100)

    # If there is new data, process it
    if ((len(s) < 48) and (len(s) > 15) and len(s) != 32):
        i += 1
        try:
            if int(s[0:1]) == 0 or int(s[0:1]) == 1: # If new data is GPS data
                lat = float(s[1:8])/100000
                lon = float(s[8:15])/100000
                ele = round(float(s[15:21])/100 - 1000,2)
                name = round(float(s[21:23]))-10
                count = int(s[23:])
                gott = np.vstack((gott,np.array([date2num(datetime.now()),lat,lon,ele])))
                if verbose: # if needed, print new GPS data to screen
                    print('GOTT: ' + str(lat) + ' °N, ' + str(lon) + ' °E, Elevation: ' + str(ele) + ' m')
                any_gott = True 
            else: # If new data is regular data
                name = round(float(s[18:20])) - 10
                count = int(s[20:])
                T = round(float(s[0:5])/100 - 273.15,2) 
                h = round(float(s[5:10])/100 - 100,2)
                P = round(float(s[10:18])/10000 - 1000,2)
    
                if (T > 40) | (T < 0) | (h > 100) | (h < 0) | (P >1100) |(P < 700):
                    T = np.nan
                    h = np.nan
                    P = np.nan
                if verbose: # if needed, print new values to screen
                    s = 'Arduino ' + str(name) + ': ' + str(T) + '°C,  '+ str(h)+ ' %, '+ str(P) + ' hPa, ' + str(count)
                    print(s)
                # Write new data to TXT file
                f.write(str(datetime.now())+';'+str(name)+';'+str(T)+';'+str(h)+';'+str(P)+';'+str(count)+'\n') #maybe on other systems "" needs to be ''
                save_array[name] = np.vstack((save_array[name],np.array([date2num(datetime.now()),T,h,P,count])))
        except: # Erroneous data was coming in
            print('NOPE...')
        # Save to NPY files only every 40 new data packets
        if np.mod(i,40) == 0 and i > 30:
            time1 = time.time()
            print('SAVING...')
            f.close()
            f = open(data_filename+'.txt','a')
            with open(data_filename+'.npy','wb') as myFile:
                pickle.dump(save_array,myFile)
                myFile.close()
            if any_gott:
                with open(data_filename+'_gps.npy','wb') as myFile:
                    pickle.dump(gott,myFile)
                    myFile.close()
                    
    t = t+1