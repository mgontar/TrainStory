import pandas as pd
df = pd.read_csv('graph_waits.csv')
df.rename(columns={"trawel_time:int": "travel_time:int"}, inplace=True)
df.to_csv('graph_waits.csv', index=False)

#df = pd.read_csv('graph_station_events.csv')
#df["travel_time:int"] = 0
#df.to_csv('graph_station_events.csv', index=False)

#df = pd.read_csv('graph_train_events.csv')
#df["travel_time:int"] = 0
#df.to_csv('graph_train_events.csv', index=False)