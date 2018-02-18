import pandas as pd
import numpy as np
from dateutil import parser
from datetime import timedelta
from datetime import date
from datetime import datetime
from datetime import time
from tqdm import tqdm


def getTimestamp(date_from, time_from, travel_minutes):
    t = parser.parse(time_from).time()
    dt =  datetime.combine(date_from, t)
    dt = dt + timedelta(minutes=travel_minutes)
    return dt.timestamp()
    
def timeDiff(time_from, time_to, prev_minutes):
    prev_hours = prev_minutes // 60
    td = parser.parse(time_to) - parser.parse(time_from)
    hours = td.total_seconds() // 3600
    if hours > 12 :
        td = td - timedelta(hours=12)
    if hours < prev_hours:
        td = td + timedelta(hours=24)
    minutes = td.total_seconds() // 60
    return minutes

df_stops = pd.read_csv('stops_all.csv')
train_id_list = df_stops.train_id.unique()

df_stations = pd.read_csv('stations_all.csv')
station_id_list = df_stations.id.unique()
df_stations[':LABEL'] = "Station"
df_stations.columns = ['station_id:ID(Station-ID)', 'name', ':LABEL']
df_stations.to_csv('graph_stations.csv', index=False, encoding='utf-8')

print("graph_stations.csv saved")

def updateGraphForTrainId(train_id):
    df_train = df_stops.loc[df_stops.train_id == train_id].copy()
    from_iter = 1
    pBarFrom = tqdm(total=len(df_train), desc='From', leave=False)
    for index_from, row_from in df_train.iterrows():
        pBarFrom.update(1) 
        prev_minutes = 0
        to_left = len(df_train) - from_iter
        pBarTo = tqdm(total=to_left, desc='To', leave=False)
        for index_to, row_to in df_train.loc[index_from+1:,].iterrows():
            pBarTo.update(1)
            travel_time = timeDiff(row_from.departure_time, row_to.arrival_time, prev_minutes)            
            for day in day_list:  
                ts_dep = getTimestamp(day, row_from.departure_time, prev_minutes)
                event_dep_id = str(train_id)+":"+str(row_from.station_id)+":"+str(int(ts_dep))+":dep"
                event_dep = (event_dep_id, train_id, row_from.station_id, int(ts_dep), "Event;Departure")
                event_list.append(event_dep)
                
                train_event_dep = (train_id,event_dep_id,"DEPARTED")
                train_event_list.append(train_event_dep)
                
                station_event_dep = (event_dep_id,row_from.station_id,"AT_STATION")
                station_event_list.append(station_event_dep)
                
                #print(str(train_id)+":"+str(row_to.station_id))
                ts_arr = getTimestamp(day, row_to.arrival_time, travel_time)
                # custom feature
                ts_arr = ts_arr + 10*60
                event_arr_id = str(train_id)+":"+str(row_to.station_id)+":"+str(int(ts_arr))+":arr"
                event_arr = (event_arr_id, train_id, row_to.station_id, int(ts_arr), "Event;Arrival")
                event_list.append(event_arr)
                
                train_event_arr = (train_id,event_arr_id,"ARRIVED")
                train_event_list.append(train_event_arr)
                
                station_event_arr = (event_arr_id,row_to.station_id,"AT_STATION")
                station_event_list.append(station_event_arr)
                
                trip = (event_dep_id, train_id, int(travel_time+10), event_arr_id, "TRAVEL_TO")
                trip_list.append(trip)
            prev_minutes = travel_time
        from_iter += 1
        pBarTo.close() 
    pBarFrom.close() 
    return

df_trains = pd.read_csv('trains_all.csv')
df_trains = df_trains.drop(['description','travel_time'], 1)
df_trains[':LABEL'] = "Train"
df_trains.columns = ['train_id:ID(Train-ID)', 'number', 'name', ':LABEL']
df_trains.to_csv('graph_trains.csv', index=False, encoding='utf-8')

print("graph_trains.csv saved")

day_list = []
start_day = date(2018, 7, 1)
for x in range(0, 7):
    day = start_day+timedelta(days=x)
    day_list.append(day)
print("Day list generated:")
print(*day_list, sep='\n')

print("generating graph event nodes...")

trip_list = []
event_list = []
station_event_list = []
train_event_list = []
for train_id in tqdm(train_id_list, desc='Trains'): 
    updateGraphForTrainId(train_id)
event_list = list(set(event_list))

columns = ['event_id:ID(Event-ID)', 'train_id(Train-ID)', 'station_id(Station-ID)', 'timestamp:double', ':LABEL']
df_events = pd.DataFrame(event_list, columns=columns)
df_events.to_csv('graph_events.csv', index=False)

print("graph_events.csv saved")

columns = [':START_ID(Event-ID)', 'train_id(Train-ID)', 'travel_time:int', ':END_ID(Event-ID)', ':TYPE']
df_trips = pd.DataFrame(trip_list, columns=columns)
df_trips.to_csv('graph_trips.csv', index=False)

print("graph_trips.csv saved")

columns = [':START_ID(Event-ID)', ':END_ID(Station-ID)', ':TYPE']
df_station_events = pd.DataFrame(station_event_list, columns=columns)
df_station_events.to_csv('graph_station_events.csv', index=False)

print("graph_station_events.csv saved")

columns = [':START_ID(Train-ID)', ':END_ID(Event-ID)', ':TYPE']
df_train_events = pd.DataFrame(train_event_list, columns=columns)
df_train_events.to_csv('graph_train_events.csv', index=False)

print("graph_train_events.csv saved")

stat_left = len(station_id_list)
from_left = 0
from_iter = 0
to_left = 0
wait_cnt = 0

for station_id in tqdm(station_id_list, desc='Stations'):
    wait_list = []
    df_station = df_events.loc[df_events['station_id(Station-ID)'] == station_id].copy()
    df_station = df_station.sort_values(by=['timestamp:double']).reset_index(drop=True)
    from_left = len(df_station)
    from_iter = 1
    pBarFrom = tqdm(total=len(df_station), desc='From', leave=False)
    for index_from, row_from in df_station.iterrows():
        pBarFrom.update(1)        
        if('Event;Arrival' == row_from[':LABEL']):
            to_left = len(df_station) - from_iter
            pBarTo = tqdm(total=to_left, desc='To', leave=False)
            for index_to, row_to in df_station.loc[index_from+1:,].iterrows():                
                pBarTo.update(1)
                if('Event;Departure' == row_to[':LABEL'] and (row_to['train_id(Train-ID)'] != row_from['train_id(Train-ID)'])):
                    wait_time = row_to['timestamp:double'] - row_from['timestamp:double']
                    if(wait_time > 0):
                        if(wait_time < 60*60*3):
                            event_from = row_from['event_id:ID(Event-ID)']
                            event_to = row_to['event_id:ID(Event-ID)']
                            wait_cnt += 1
                            wait = (event_from, event_to, int(wait_time), "WAITING")
                            wait_list.append(wait)
                        else:
                            pBarTo.update(to_left)
                            break
                to_left -= 1
            pBarTo.close()           
        from_left -= 1
        from_iter += 1       
    pBarFrom.close()
    stat_left -= 1    
    print("\nstat_left: "+str(stat_left)+" wait_cnt: "+str(wait_cnt))
    columns = [':START_ID(Event-ID)', ':END_ID(Event-ID)', 'wait_time:int', ':TYPE']
    df_waits = pd.DataFrame(wait_list, columns=columns)
    df_waits.to_csv('waits\graph_waits_'+str(station_id)+'.csv', index=False)