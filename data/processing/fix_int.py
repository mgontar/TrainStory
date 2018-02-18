import pandas as pd
df = pd.read_csv('graph_waits.csv')
df['wait_time:int'] = df['wait_time:int'].astype(int)
df.to_csv('graph_waits.csv', index=False)