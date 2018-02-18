import pandas as pd
import glob, os
from tqdm import tqdm

results = pd.DataFrame([])
pBarFrom = tqdm(total=len(glob.glob("waits\*.csv")), desc='Files', leave=False)
for counter, file in enumerate(glob.glob("waits\*.csv")):
    pBarFrom.update(1)
    df = pd.read_csv(file)
    df['wait_time:int'] = df['wait_time:int'].astype(int)
    results = results.append(df)
pBarFrom.close()
results.to_csv('graph_waits.csv', index=False)